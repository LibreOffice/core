/*************************************************************************
 *
 *  $RCSfile: adc_cl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:37:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    bool                Display_SinceTag() const;

    /// @see command::SinceTagTransformationData::StripSinceTagValue()
    bool                Strip_SinceTagText(
                            String &            io_sSinceTagValue ) const;

    /// @see command::SinceTagTransformationData::DisplayOf()
    const String &      DisplayOf_SinceTagValue(
                            const String &      i_sVersionNumber ) const;


    // ACCESS
    static const CommandLine &
                        Get_();
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

