/*************************************************************************
 *
 *  $RCSfile: adc_cl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:27 $
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
        class Parse;
        class Load;
        class CreateHtml;
        class CreateXml;
        class Save;

        struct S_ProjectData;
    }


class CommandLine : public csv::CommandLine_Ifc
{
  public:
                        CommandLine();
                        ~CommandLine();

    bool                DebugStyle_ShowText() const;
    bool                DebugStyle_ShowStoredObjects() const;
    bool                DebugStyle_ShowTokens() const;

    // ACCESS
    void                SetUpdate(
                            const char *        i_sRepositoryDir );
    void                SetCurProject(
                            command::S_ProjectData &
                                                io_rProject );
    command::S_ProjectData &
                        CurProject();

    static const CommandLine &
                        Get_();

    const Dyn<command::Parse> &
                        Cmd_Parse() const;
    const Dyn<command::Load> &
                        Cmd_Load() const;
    const Dyn<command::CreateHtml> &
                        Cmd_CreateHtml() const;
    const Dyn<command::CreateXml> &
                        Cmd_CreateXml() const;
    const Dyn<command::Save> &
                        Cmd_Save() const;

  private:
    // Interface cosv::CommandLine_Ifc:
    virtual void        do_Init(
                            int                 argc,
                            char *              argv[] );
    virtual void        do_PrintUse() const;
    virtual bool        inq_CheckParameters() const;

    // Locals

    // DATA
    uintt               nDebugStyle;
    Dyn<command::Parse> pCmd_Parse;
    Dyn<command::Load>  pCmd_Load;
    Dyn<command::CreateHtml>
                        pCmd_CreateHtml;
    Dyn<command::CreateXml>
                        pCmd_CreateXml;
    Dyn<command::Save>  pCmd_Save;

    command::S_ProjectData *
                        pCurProject;
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

inline const Dyn<command::Parse> &
CommandLine::Cmd_Parse() const
    { return pCmd_Parse; }
inline const Dyn<command::Load> &
CommandLine::Cmd_Load() const
    { return pCmd_Load; }
inline const Dyn<command::CreateHtml> &
CommandLine::Cmd_CreateHtml() const
    { return pCmd_CreateHtml; }
inline const Dyn<command::CreateXml> &
CommandLine::Cmd_CreateXml() const
    { return pCmd_CreateXml; }
inline const Dyn<command::Save> &
CommandLine::Cmd_Save() const
    { return pCmd_Save; }




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

