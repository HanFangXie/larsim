/**
 * @file   DumpSimPhotons_module.cc
 * @brief  Module dumping SimPhotons information on screen
 * @date   March 30, 2016
 * @author Gianluca Petrillo (petrillo@fnal.gov)
 *
 */


// nutools libraries
#include "larsim/Simulation/SimPhotons.h"

// framework libraries
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "messagefacility/MessageLogger/MessageLogger.h"


namespace sim {
  class DumpSimPhotons;
} // namespace sim

namespace {
  using namespace fhicl;
  
  /// Collection of configuration parameters for the module
  struct Config {
    using Name = fhicl::Name;
    using Comment = fhicl::Comment;
    
    fhicl::Atom<art::InputTag> InputPhotons {
      Name("InputPhotons"),
      Comment("data product with the SimPhotons to be dumped")
      };
    
    fhicl::Atom<std::string> OutputCategory {
      Name("OutputCategory"),
      Comment("name of the output stream (managed by the message facility)"),
      "DumpSimPhotons" /* default value */
      };
    
  }; // struct Config
  
  
} // local namespace


class sim::DumpSimPhotons: public art::EDAnalyzer {
    public:
  // type to enable module parameters description by art
  using Parameters = art::EDAnalyzer::Table<Config>;
  
  /// Configuration-checking constructor
  explicit DumpSimPhotons(Parameters const& config);
  
  // Plugins should not be copied or assigned.
  DumpSimPhotons(DumpSimPhotons const&) = delete;
  DumpSimPhotons(DumpSimPhotons &&) = delete;
  DumpSimPhotons& operator = (DumpSimPhotons const&) = delete;
  DumpSimPhotons& operator = (DumpSimPhotons &&) = delete;
  
  
  // Operates on the event
  void analyze(art::Event const& event) override;
  
  
  /**
   * @brief Dumps the content of the specified SimPhotons in the output stream
   * @tparam Stream the type of output stream
   * @param out the output stream
   * @param simphotons the SimPhotons to be dumped
   * @param indent base indentation string (default: none)
   * @param bIndentFirst if first output line should be indented (default: yes)
   * 
   * The indent string is prepended to every line of output, with the possible
   * exception of the first one, in case bIndentFirst is true.
   * 
   * The output starts on the current line, and the last line is NOT broken.
   */
  template <typename Stream>
  void DumpElement(
    Stream&& out, sim::SimPhotons const& simphotons,
    std::string indent = "", bool bIndentFirst = true
    ) const;
  
  /// Dumps a sim::OnePhoton on a single line
  template <typename Stream>
  void DumpOnePhoton(Stream&& out, sim::OnePhoton const& photon) const;
  
  
    private:
  
  art::InputTag fInputPhotons; ///< name of SimPhotons's data product
  std::string fOutputCategory; ///< name of the stream for output
  
}; // class sim::DumpSimPhotons


//------------------------------------------------------------------------------
//---  module implementation
//---
//------------------------------------------------------------------------------
sim::DumpSimPhotons::DumpSimPhotons(Parameters const& config)
  : EDAnalyzer(config)
  , fInputPhotons(config().InputPhotons())
  , fOutputCategory(config().OutputCategory())
{}

//------------------------------------------------------------------------------
template <typename Stream>
void sim::DumpSimPhotons::DumpOnePhoton
  (Stream&& out, sim::OnePhoton const& onephoton) const
{
  out << "E=" << onephoton.Energy << " t=" << onephoton.Time
    << " from (" << onephoton.InitialPosition.X()
    << ", " << onephoton.InitialPosition.Y()
    << ", " << onephoton.InitialPosition.Z() << ") cm"
    << " to (" << onephoton.FinalLocalPosition.X()
    << ", " << onephoton.FinalLocalPosition.Y()
    << ", " << onephoton.FinalLocalPosition.Z() << ") cm"
    ;
  if (onephoton.SetInSD) out << " [in SD]"; // in sensitive detector?
} // sim::DumpSimPhotons::DumpOnePhoton()


//------------------------------------------------------------------------------
template <typename Stream>
void sim::DumpSimPhotons::DumpElement(
  Stream&& out, sim::SimPhotons const& simphotons,
  std::string indent /* = "" */, bool bIndentFirst /* = true */
) const {
  if (bIndentFirst) out << indent;
  out << "channel=" << simphotons.OpChannel() << " has ";
  if (simphotons.empty()) {
    out << simphotons.size() << " no photons";
  }
  else {
    out << simphotons.size() << " photons:";
    for (auto const& onephoton: simphotons) {
      out << "\n" << indent << "  ";
      DumpOnePhoton(out, onephoton);
    } // for
  }
  
} // sim::DumpSimPhotons::DumpSimPhotons()


//------------------------------------------------------------------------------
void sim::DumpSimPhotons::analyze(art::Event const& event) {
  
  // get the particles from the event
  auto const& SimPhotons
    = *(event.getValidHandle<std::vector<sim::SimPhotons>>(fInputPhotons));
  
  mf::LogVerbatim(fOutputCategory) << "Event " << event.id()
    << " : data product '" << fInputPhotons.encode() << "' contains "
    << SimPhotons.size() << " SimPhotons";
  
  unsigned int iPhoton = 0;
  for (sim::SimPhotons const& photons: SimPhotons) {
    
    mf::LogVerbatim log(fOutputCategory);
    // a bit of a header
    log << "[#" << (iPhoton++) << "] ";
    DumpElement(log, photons, "  ", false);
    
  } // for
  mf::LogVerbatim(fOutputCategory) << "\n"; // just an empty line
  
} // sim::DumpSimPhotons::analyze()


//------------------------------------------------------------------------------
DEFINE_ART_MODULE(sim::DumpSimPhotons)

//------------------------------------------------------------------------------
