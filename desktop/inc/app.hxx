/*************************************************************************
 *
 *  $RCSfile: app.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: cd $ $Date: 2001-08-07 11:24:59 $
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

#ifndef _DESK_APP_HXX
#define _DESK_APP_HXX

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif

/*--------------------------------------------------------------------
    Description:    Application-class
 --------------------------------------------------------------------*/
class IntroWindow_Impl;
class Desktop : public Application //public SfxApplicationClass
{
    public:
        enum BootstrapError
        {
            BE_OK,
            BE_UNO_SERVICEMANAGER,
            BE_UNO_SERVICE_CONFIG_MISSING,
            BE_PATHINFO_MISSING
        };

                            Desktop();
        virtual void        Main( );
        virtual void        Init();
        virtual void        DeInit();
        virtual BOOL        QueryExit();
        virtual USHORT      Exception(USHORT nError);
        virtual void        SystemSettingsChanging( AllSettings& rSettings, Window* pFrame );
        virtual void        AppEvent( const ApplicationEvent& rAppEvent );

        DECL_LINK(          OpenClients_Impl, void* );

        static void         OpenClients();
        static void         OpenDefault();
        static void         HandleAppEvent( const ApplicationEvent& rAppEvent );
        static ResMgr*      GetDesktopResManager();

        void                HandleBootstrapErrors( BootstrapError );

    private:
        void                StartSetup( const ::rtl::OUString& aParameters );

        void                OpenStartupScreen();
        void                CloseStartupScreen();
        void                EnableOleAutomation();
        DECL_LINK(          AsyncInitFirstRun, void* );

        /** checks if the office is run the first time
            <p>If so, <method>DoFirstRunInitializations</method> is called (asynchronously and delayed) and the
            respective flag in the configuration is reset.</p>
        */
        void                CheckFirstRun( );

        /// does initializations which are necessary for the first run of the office
        void                DoFirstRunInitializations();

        sal_Bool            m_bMinimized;
        sal_Bool            m_bInvisible;
        USHORT              m_nAppEvents;
        IntroWindow_Impl*   m_pIntro;
        BootstrapError      m_aBootstrapError;

        static ResMgr*      pResMgr;
};

#endif // DESK_APP_HXX_
