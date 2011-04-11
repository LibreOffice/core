/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DESKTOP_COMMANDLINEARGS_HXX_
#define _DESKTOP_COMMANDLINEARGS_HXX_

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include "boost/optional.hpp"

namespace desktop
{

class CommandLineArgs
{
    public:
        enum BoolParam // must be zero based!
        {
            CMD_BOOLPARAM_MINIMIZED = 0,
            CMD_BOOLPARAM_INVISIBLE,
            CMD_BOOLPARAM_NORESTORE,
            CMD_BOOLPARAM_BEAN,
            CMD_BOOLPARAM_PLUGIN,
            CMD_BOOLPARAM_SERVER,
            CMD_BOOLPARAM_HEADLESS,
            CMD_BOOLPARAM_QUICKSTART,
            CMD_BOOLPARAM_NOQUICKSTART,
            CMD_BOOLPARAM_TERMINATEAFTERINIT,
            CMD_BOOLPARAM_NOFIRSTSTARTWIZARD,
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
            CMD_BOOLPARAM_PSN,
            CMD_BOOLPARAM_VERSION,
            CMD_BOOLPARAM_COUNT             // must be last element!
        };

        enum StringParam // must be zero based!
        {
            CMD_STRINGPARAM_PORTAL = 0,
            CMD_STRINGPARAM_SPLASHPIPE,
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
            CMD_STRINGPARAM_CONVERSIONLIST,
            CMD_STRINGPARAM_CONVERSIONPARAMS,
            CMD_STRINGPARAM_CONVERSIONOUT,
            CMD_STRINGPARAM_INFILTER,
            CMD_STRINGPARAM_DISPLAY,
            CMD_STRINGPARAM_LANGUAGE,
            CMD_STRINGPARAM_COUNT // must be last element!
        };

        enum GroupParamId
        {
            CMD_GRPID_MODULE = 0,
            CMD_GRPID_COUNT
        };

        struct Supplier
        {
            // Thrown from constructors and next:
            class Exception {
            public:
                Exception();
                Exception(Exception const &);
                virtual ~Exception();
                Exception & operator =(Exception const &);
            };

            virtual ~Supplier();
            virtual boost::optional< rtl::OUString > getCwdUrl() = 0;
            virtual bool next(rtl::OUString * argument) = 0;
        };

        CommandLineArgs();
        CommandLineArgs( Supplier& supplier );

        boost::optional< rtl::OUString > getCwdUrl() const { return m_cwdUrl; }

        // generic methods to access parameter
        void     SetBoolParam( BoolParam eParam, sal_Bool bNewValue );

        const rtl::OUString&    GetStringParam( StringParam eParam ) const;

        // Access to bool parameters
        sal_Bool                IsMinimized() const;
        sal_Bool                IsInvisible() const;
        sal_Bool                IsNoRestore() const;
        sal_Bool                IsNoDefault() const;
        sal_Bool                IsBean() const;
        sal_Bool                IsServer() const;
        sal_Bool                IsHeadless() const;
        sal_Bool                IsQuickstart() const;
        sal_Bool                IsNoQuickstart() const;
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
        sal_Bool                IsVersion() const;
        sal_Bool                HasModuleParam() const;
        sal_Bool                WantsToLoadDocument() const;

        // Access to string parameters
        sal_Bool                GetPortalConnectString( ::rtl::OUString& rPara) const;
        sal_Bool                GetAcceptString( ::rtl::OUString& rPara) const;
        sal_Bool                GetUnAcceptString( ::rtl::OUString& rPara) const;
        sal_Bool                GetOpenList( ::rtl::OUString& rPara) const;
        sal_Bool                GetViewList( ::rtl::OUString& rPara) const;
        sal_Bool                GetStartList( ::rtl::OUString& rPara) const;
        sal_Bool                GetForceOpenList( ::rtl::OUString& rPara) const;
        sal_Bool                GetForceNewList( ::rtl::OUString& rPara) const;
        sal_Bool                GetPrintList( ::rtl::OUString& rPara) const;
        sal_Bool                GetPrintToList( ::rtl::OUString& rPara ) const;
        sal_Bool                GetPrinterName( ::rtl::OUString& rPara ) const;
        sal_Bool                GetLanguage( ::rtl::OUString& rPara ) const;
        sal_Bool                GetInFilter( ::rtl::OUString& rPara ) const;
        sal_Bool                GetConversionList( ::rtl::OUString& rPara ) const;
        sal_Bool                GetConversionParams( ::rtl::OUString& rPara ) const;
        sal_Bool                GetConversionOut( ::rtl::OUString& rPara ) const;

        // Special analyzed states (does not match directly to a command line parameter!)
        sal_Bool IsPrinting() const;
        sal_Bool IsEmpty() const;
        sal_Bool IsEmptyOrAcceptOnly() const;

    private:
        enum Count { NONE, ONE, MANY };

        struct GroupDefinition
        {
            sal_Int32  nCount;
            BoolParam* pGroupMembers;
        };

        // no copy and operator=
        CommandLineArgs( const CommandLineArgs& );
        CommandLineArgs operator=( const CommandLineArgs& );

        sal_Bool                InterpretCommandLineParameter( const ::rtl::OUString&, ::rtl::OUString& );
        void                    ParseCommandLine_Impl( Supplier& supplier );
        void                    ResetParamValues();
        sal_Bool                CheckGroupMembers( GroupParamId nGroup, BoolParam nExcludeMember ) const;

        void     AddStringListParam_Impl( StringParam eParam, const rtl::OUString& aParam );
        void     SetBoolParam_Impl( BoolParam eParam, sal_Bool bValue );

        boost::optional< rtl::OUString > m_cwdUrl;
        sal_Bool                         m_aBoolParams[ CMD_BOOLPARAM_COUNT ];     // Stores boolean parameters
        rtl::OUString                    m_aStrParams[ CMD_STRINGPARAM_COUNT ];    // Stores string parameters
        sal_Bool                         m_aStrSetParams[ CMD_STRINGPARAM_COUNT ]; // Stores if string parameters are provided on cmdline
        Count                            m_eArgumentCount;                         // Number of Args
        bool                             m_bDocumentArgs;                          // A document creation/open/load arg is used
        mutable ::osl::Mutex             m_aMutex;

        // static definition for groups where only one member can be true
        static GroupDefinition  m_pGroupDefinitions[ CMD_GRPID_COUNT ];
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
