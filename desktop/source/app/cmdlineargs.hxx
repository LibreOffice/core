/*************************************************************************
 *
 *  $RCSfile: cmdlineargs.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:39:27 $
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
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif


namespace desktop
{

class CommandLineArgs
{
    public:
        enum BoolParam  // must be zero based!
        {
            CMD_BOOLPARAM_MINIMIZED,
            CMD_BOOLPARAM_INVISIBLE,
            CMD_BOOLPARAM_NORESTORE,
            CMD_BOOLPARAM_BEAN,
            CMD_BOOLPARAM_PLUGIN,
            CMD_BOOLPARAM_SERVER,
            CMD_BOOLPARAM_HEADLESS,
            CMD_BOOLPARAM_QUICKSTART,
            CMD_BOOLPARAM_TERMINATEAFTERINIT,
            CMD_BOOLPARAM_NOLOGO,
            CMD_BOOLPARAM_NOLOCKCHECK,
            CMD_BOOLPARAM_NODEFAULT,
            CMD_BOOLPARAM_HELP,
            CMD_BOOLPARAM_WRITER,
            CMD_BOOLPARAM_CALC,
            CMD_BOOLPARAM_DRAW,
            CMD_BOOLPARAM_IMPRESS,
            CMD_BOOLPARAM_GLOBAL,
            CMD_BOOLPARAM_MATH,
            CMD_BOOLPARAM_WEB,
            CMD_BOOLPARAM_BASE,
            CMD_BOOLPARAM_HELPWRITER,
            CMD_BOOLPARAM_HELPCALC,
            CMD_BOOLPARAM_HELPDRAW,
            CMD_BOOLPARAM_HELPBASIC,
            CMD_BOOLPARAM_HELPMATH,
            CMD_BOOLPARAM_HELPIMPRESS,
            CMD_BOOLPARAM_HELPBASE,
            CMD_BOOLPARAM_COUNT             // must be last element!
        };

        enum StringParam // must be zero based!
        {
            CMD_STRINGPARAM_PORTAL,
            CMD_STRINGPARAM_ACCEPT,
            CMD_STRINGPARAM_UNACCEPT,
            CMD_STRINGPARAM_USERDIR,
            CMD_STRINGPARAM_CLIENTDISPLAY,
            CMD_STRINGPARAM_OPENLIST,
            CMD_STRINGPARAM_VIEWLIST,
            CMD_STRINGPARAM_STARTLIST,
            CMD_STRINGPARAM_FORCEOPENLIST,
            CMD_STRINGPARAM_FORCENEWLIST,
            CMD_STRINGPARAM_PRINTLIST,
            CMD_STRINGPARAM_VERSION,
            CMD_STRINGPARAM_PRINTTOLIST,
            CMD_STRINGPARAM_PRINTERNAME,
            CMD_STRINGPARAM_DISPLAY,
            CMD_STRINGPARAM_COUNT           // must be last element!
        };

        enum GroupParamId
        {
            CMD_GRPID_MODULE,
            CMD_GRPID_COUNT
        };

        CommandLineArgs();
        CommandLineArgs( const ::vos::OExtCommandLine& aExtCmdLine );
        CommandLineArgs( const ::rtl::OUString& aIPCThreadCmdLine );

        // generic methods to access parameter
        sal_Bool                GetBoolParam( BoolParam eParam ) const;
        void                    SetBoolParam( BoolParam eParam, sal_Bool bNewValue );

        const rtl::OUString&    GetStringParam( BoolParam eParam ) const;
        void                    SetStringParam( BoolParam eParam, const rtl::OUString& bNewValue );

        // Access to bool parameters
        sal_Bool                IsMinimized() const;
        sal_Bool                IsInvisible() const;
        sal_Bool                IsNoRestore() const;
        sal_Bool                IsNoDefault() const;
        sal_Bool                IsBean() const;
        sal_Bool                IsPlugin() const;
        sal_Bool                IsServer() const;
        sal_Bool                IsHeadless() const;
        sal_Bool                IsQuickstart() const;
        sal_Bool                IsTerminateAfterInit() const;
        sal_Bool                IsNoLogo() const;
        sal_Bool                IsNoLockcheck() const;
        sal_Bool                IsHelp() const;
        sal_Bool                IsHelpWriter() const;
        sal_Bool                IsHelpCalc() const;
        sal_Bool                IsHelpDraw() const;
        sal_Bool                IsHelpImpress() const;
        sal_Bool                IsHelpBase() const;
        sal_Bool                IsHelpMath() const;
        sal_Bool                IsHelpBasic() const;
        sal_Bool                IsWriter() const;
        sal_Bool                IsCalc() const;
        sal_Bool                IsDraw() const;
        sal_Bool                IsImpress() const;
        sal_Bool                IsBase() const;
        sal_Bool                IsGlobal() const;
        sal_Bool                IsMath() const;
        sal_Bool                IsWeb() const;
        sal_Bool                HasModuleParam() const;

        // Access to string parameters
        sal_Bool                GetPortalConnectString( ::rtl::OUString& rPara) const;
        sal_Bool                GetAcceptString( ::rtl::OUString& rPara) const;
        sal_Bool                GetUnAcceptString( ::rtl::OUString& rPara) const;
        sal_Bool                GetUserDir( ::rtl::OUString& rPara) const;
        sal_Bool                GetClientDisplay( ::rtl::OUString& rPara) const;
        sal_Bool                GetOpenList( ::rtl::OUString& rPara) const;
        sal_Bool                GetViewList( ::rtl::OUString& rPara) const;
        sal_Bool                GetStartList( ::rtl::OUString& rPara) const;
        sal_Bool                GetForceOpenList( ::rtl::OUString& rPara) const;
        sal_Bool                GetForceNewList( ::rtl::OUString& rPara) const;
        sal_Bool                GetPrintList( ::rtl::OUString& rPara) const;
        sal_Bool                GetVersionString( ::rtl::OUString& rPara) const;
        sal_Bool                GetPrintToList( ::rtl::OUString& rPara ) const;
        sal_Bool                GetPrinterName( ::rtl::OUString& rPara ) const;
        sal_Bool                GetDisplay( ::rtl::OUString& rPara ) const;

        // Special analyzed states (does not match directly to a command line parameter!)
        sal_Bool                IsPrinting() const;
        sal_Bool                IsEmpty() const;

    private:
        struct GroupDefinition
        {
            sal_Int32   nCount;
            BoolParam*  pGroupMembers;
        };

        // no copy and operator=
        CommandLineArgs( const CommandLineArgs& );
        CommandLineArgs operator=( const CommandLineArgs& );

        sal_Bool                InterpretCommandLineParameter( const ::rtl::OUString& );
        void                    ParseCommandLine_Impl( const ::vos::OExtCommandLine& );
        void                    ParseCommandLine_String( const ::rtl::OUString& );
        void                    ResetParamValues();
        sal_Bool                CheckGroupMembers( GroupParamId nGroup, BoolParam nExcludeMember ) const;

        void                    AddStringListParam_Impl( StringParam eParam, const rtl::OUString& aParam );
        void                    SetBoolParam_Impl( BoolParam eParam, sal_Bool bValue );

        sal_Bool                m_aBoolParams[ CMD_BOOLPARAM_COUNT ];       // Stores boolean parameters
        rtl::OUString           m_aStrParams[ CMD_STRINGPARAM_COUNT ];      // Stores string parameters
        sal_Bool                m_aStrSetParams[ CMD_STRINGPARAM_COUNT ];   // Stores if string parameters are provided on cmdline
        sal_Bool                m_bEmpty;                                   // indicates an empty command line
        mutable ::osl::Mutex    m_aMutex;

        // static definition for groups where only one member can be true
        static GroupDefinition  m_pGroupDefinitions[ CMD_GRPID_COUNT ];
};

}

#endif
