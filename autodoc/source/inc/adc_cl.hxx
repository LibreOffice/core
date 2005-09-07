/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adc_cl.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:08:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ADC_ADC_CL_HXX
#define ADC_ADC_CL_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/comdline.hxx>
    // COMPONENTS
    // PARAMETERS

namespace autodoc
{
namespace command
{
    class Command;
    class CreateHtml;
    class SinceTagTransformationData;
}


/** Reads and runs an Autodoc command line.
*/
class CommandLine : public csv::CommandLine_Ifc
{
  public:
    // LIFECYCLE
                        CommandLine();
                        ~CommandLine();
    // OPERATIONS
    int                 Run() const;

    // INQUIRY
        // debugging
    bool                DebugStyle_ShowText() const;
    bool                DebugStyle_ShowStoredObjects() const;
    bool                DebugStyle_ShowTokens() const;

        // @since tags
    bool                DoesTransform_SinceTag() const;

//    /// @see command::SinceTagTransformationData::StripSinceTagValue()
//    bool                Strip_SinceTagText(
//                            String &            io_sSinceTagValue ) const;

    /// @see command::SinceTagTransformationData::DisplayOf()
    const String &      DisplayOf_SinceTagValue(
                            const String &      i_sVersionNumber ) const;

        // extern IDL links
    const String &      ExternRoot() const      { return sExternRoot; }
    const String &      ExternNamespace() const { return sExternNamespace; }

    // ACCESS
    static const CommandLine &
                        Get_();
    void                Set_ExternRoot(
                            const String &      i_s )
                                                { sExternRoot = i_s; }
    void                Set_ExternNamespace(
                            const String &      i_s )
                                                { sExternNamespace = i_s; }
  private:
    // Interface cosv::CommandLine_Ifc:
    virtual void        do_Init(
                            int                 argc,
                            char *              argv[] );
    virtual void        do_PrintUse() const;
    virtual bool        inq_CheckParameters() const;

    // Locals
    typedef StringVector::const_iterator            opt_iter;
    typedef std::vector< DYN command::Command* >    CommandList;

    void                load_IncludedCommands(
                            StringVector &      out,
                            const char *        i_filePath );

    void                do_clVerbose(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clParse(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clCreateHtml(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clSinceFile(
                            opt_iter &          it,
                            opt_iter            itEnd );

//    void                do_clCreateXml(
//                            opt_iter &          it,
//                            opt_iter            itEnd );
//    void                do_clLoad(
//                            opt_iter &          it,
//                            opt_iter            itEnd );
//    void                do_clSave(
//                            opt_iter &          it,
//                            opt_iter            itEnd );

    void                sort_Commands();

    // DATA
    uintt               nDebugStyle;
    Dyn<command::SinceTagTransformationData>
                        pSinceTransformator;

    CommandList         aCommands;
    bool                bInitOk;
    static CommandLine *
                        pTheInstance_;
    command::CreateHtml *
                        pCommand_CreateHtml;

    String              sExternRoot;
    String              sExternNamespace;
};



// IMPLEMENTATION
inline bool
CommandLine::DebugStyle_ShowText() const
    { return (nDebugStyle & 2) != 0; }
inline bool
CommandLine::DebugStyle_ShowStoredObjects() const
    { return (nDebugStyle & 4) != 0; }
inline bool
CommandLine::DebugStyle_ShowTokens() const
    { return (nDebugStyle & 1) != 0; }

}   // namespace autodoc


inline bool
DEBUG_ShowText()
    { return autodoc::CommandLine::Get_().DebugStyle_ShowText(); }
inline bool
DEBUG_ShowStoring()
    { return autodoc::CommandLine::Get_().DebugStyle_ShowStoredObjects(); }
inline bool
DEBUG_ShowTokens()
    { return autodoc::CommandLine::Get_().DebugStyle_ShowTokens(); }

#endif

