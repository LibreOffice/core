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

#include "sal/config.h"

#include <vector>

#include <rtl/ustring.hxx>
#include "boost/noncopyable.hpp"
#include "boost/optional.hpp"

namespace desktop
{

class CommandLineArgs: private boost::noncopyable
{
    public:
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

        // Access to bool parameters
        bool                IsMinimized() const;
        bool                IsInvisible() const;
        bool                IsNoRestore() const;
        bool                IsNoDefault() const;
        bool                IsHeadless() const;
        bool                IsQuickstart() const;
        bool                IsNoQuickstart() const;
        bool                IsTerminateAfterInit() const;
        bool                IsNoLogo() const;
        bool                IsNoLockcheck() const;
        bool                IsHelp() const;
        bool                IsHelpWriter() const;
        bool                IsHelpCalc() const;
        bool                IsHelpDraw() const;
        bool                IsHelpImpress() const;
        bool                IsHelpBase() const;
        bool                IsHelpMath() const;
        bool                IsHelpBasic() const;
        bool                IsWriter() const;
        bool                IsCalc() const;
        bool                IsDraw() const;
        bool                IsImpress() const;
        bool                IsBase() const;
        bool                IsGlobal() const;
        bool                IsMath() const;
        bool                IsWeb() const;
        bool                IsVersion() const;
        bool                HasModuleParam() const;
        bool                HasUnknown() const;
        bool                WantsToLoadDocument() const;

        // Access to string parameters
        bool                    HasSplashPipe() const;
        std::vector< rtl::OUString > const & GetAccept() const;
        std::vector< rtl::OUString > const & GetUnaccept() const;
        std::vector< rtl::OUString > GetOpenList() const;
        std::vector< rtl::OUString > GetViewList() const;
        std::vector< rtl::OUString > GetStartList() const;
        std::vector< rtl::OUString > GetForceOpenList() const;
        std::vector< rtl::OUString > GetForceNewList() const;
        std::vector< rtl::OUString > GetPrintList() const;
        std::vector< rtl::OUString > GetPrintToList() const;
        rtl::OUString       GetPrinterName() const;
        rtl::OUString       GetLanguage() const;
        std::vector< rtl::OUString > const & GetInFilter() const;
        std::vector< rtl::OUString > GetConversionList() const;
        rtl::OUString       GetConversionParams() const;
        rtl::OUString       GetConversionOut() const;

        // Special analyzed states (does not match directly to a command line parameter!)
        bool IsEmpty() const;

    private:
        bool                InterpretCommandLineParameter( const ::rtl::OUString&, ::rtl::OUString& );
        void                    ParseCommandLine_Impl( Supplier& supplier );
        void                    InitParamValues();

        boost::optional< rtl::OUString > m_cwdUrl;

        bool m_minimized;
        bool m_invisible;
        bool m_norestore;
        bool m_headless;
        bool m_quickstart;
        bool m_noquickstart;
        bool m_terminateafterinit;
        bool m_nofirststartwizard;
        bool m_nologo;
        bool m_nolockcheck;
        bool m_nodefault;
        bool m_help;
        bool m_writer;
        bool m_calc;
        bool m_draw;
        bool m_impress;
        bool m_global;
        bool m_math;
        bool m_web;
        bool m_base;
        bool m_helpwriter;
        bool m_helpcalc;
        bool m_helpdraw;
        bool m_helpbasic;
        bool m_helpmath;
        bool m_helpimpress;
        bool m_helpbase;
        bool m_psn;
        bool m_version;
        bool m_unknown;
        bool m_splashpipe;

        bool m_bEmpty; // No Args at all
        bool m_bDocumentArgs; // A document creation/open/load arg is used
        std::vector< rtl::OUString > m_accept;
        std::vector< rtl::OUString > m_unaccept;
        std::vector< rtl::OUString > m_openlist; // contains external URIs
        std::vector< rtl::OUString > m_viewlist; // contains external URIs
        std::vector< rtl::OUString > m_startlist; // contains external URIs
        std::vector< rtl::OUString > m_forceopenlist; // contains external URIs
        std::vector< rtl::OUString > m_forcenewlist; // contains external URIs
        std::vector< rtl::OUString > m_printlist; // contains external URIs
        std::vector< rtl::OUString > m_printtolist; // contains external URIs
        rtl::OUString m_printername;
        std::vector< rtl::OUString > m_conversionlist; // contains external URIs
        rtl::OUString m_conversionparams;
        rtl::OUString m_conversionout; // contains external URIs
        std::vector< rtl::OUString > m_infilter;
        rtl::OUString m_language;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
