/*************************************************************************
 *
 *  $RCSfile: cmdlineargs.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mba $ $Date: 2001-09-10 14:00:16 $
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

#ifndef _DESKTOP_COMMANDLINEARGS_HXX_
#define _DESKTOP_COMMANDLINEARGS_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

namespace desktop
{

class CommandLineArgs
{
    private:
        sal_Bool        InterpretCommandLineParameter( const ::rtl::OUString& );
        void            ParseCommandLine_Impl( const ::vos::OExtCommandLine& );
        void            ParseCommandLine_String( const ::rtl::OUString& );
        void            ResetParamValues();

        sal_Bool            m_bMinimized;
        sal_Bool            m_bInvisible;
        sal_Bool            m_bNoRestore;
        sal_Bool            m_bBean;
        sal_Bool            m_bPlugin;
        sal_Bool            m_bServer;
        sal_Bool            m_bHeadless;
        sal_Bool            m_bQuickstart;
        sal_Bool            m_bOpenList;
        sal_Bool            m_bPrintList;
        sal_Bool            m_bPortalConnectString;
        sal_Bool            m_bAcceptString;
        sal_Bool            m_bUserDir;
        sal_Bool            m_bClientDisplay;
        sal_Bool            m_bTerminateAfterInit;
        ::rtl::OUString     m_aOpenList;
        ::rtl::OUString     m_aPrintList;
        ::rtl::OUString     m_aPortalConnectString;
        ::rtl::OUString     m_aAcceptString;
        ::rtl::OUString     m_aUserDir;
        ::rtl::OUString     m_aClientDisplay;

    public:
        CommandLineArgs();
        CommandLineArgs( const ::vos::OExtCommandLine& aExtCmdLine );
        CommandLineArgs( const ::rtl::OUString& aIPCThreadCmdLine );

        sal_Bool        IsMinimized(){ return m_bMinimized; }
        sal_Bool        IsInvisible(){ return m_bInvisible; }
        sal_Bool        IsNoRestore(){ return m_bNoRestore; }
        sal_Bool        IsBean(){ return m_bBean; }
        sal_Bool        IsPlugin(){ return m_bPlugin; }
        sal_Bool        IsServer(){ return m_bServer; }
        sal_Bool        IsHeadless(){ return m_bHeadless; }
        sal_Bool        IsQuickstart(){ return m_bQuickstart; }
        sal_Bool        IsTerminateAfterInit() { return m_bTerminateAfterInit; }

        sal_Bool        GetPortalConnectString( ::rtl::OUString& rPara){ rPara = m_aPortalConnectString; return m_bPortalConnectString; }
        sal_Bool        GetAcceptString( ::rtl::OUString& rPara){ rPara =  m_aAcceptString; return m_bAcceptString; }
        sal_Bool        GetUserDir( ::rtl::OUString& rPara){ rPara = m_aUserDir; return m_bUserDir; }
        sal_Bool        GetClientDisplay( ::rtl::OUString& rPara){ rPara = m_aClientDisplay; return m_bClientDisplay; }
        sal_Bool        GetOpenList( ::rtl::OUString& rPara){ rPara = m_aOpenList; return m_bOpenList; }
        sal_Bool        GetPrintList( ::rtl::OUString& rPara){ rPara = m_aPrintList; return m_bPrintList; }
};

}

#endif
