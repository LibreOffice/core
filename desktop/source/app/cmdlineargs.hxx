/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            virtual boost::optional< OUString > getCwdUrl() = 0;
            virtual bool next(OUString * argument) = 0;
        };

        CommandLineArgs();
        CommandLineArgs( Supplier& supplier );

        boost::optional< OUString > getCwdUrl() const { return m_cwdUrl; }

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
        bool                WantsToLoadDocument() const;

        OUString            GetUnknown() const;

        // Access to string parameters
        bool                    HasSplashPipe() const;
        std::vector< OUString > const & GetAccept() const;
        std::vector< OUString > const & GetUnaccept() const;
        std::vector< OUString > GetOpenList() const;
        std::vector< OUString > GetViewList() const;
        std::vector< OUString > GetStartList() const;
        std::vector< OUString > GetForceOpenList() const;
        std::vector< OUString > GetForceNewList() const;
        std::vector< OUString > GetPrintList() const;
        std::vector< OUString > GetPrintToList() const;
        OUString       GetPrinterName() const;
        OUString       GetLanguage() const;
        std::vector< OUString > const & GetInFilter() const;
        std::vector< OUString > GetConversionList() const;
        OUString       GetConversionParams() const;
        OUString       GetConversionOut() const;
        OUString       GetPidfileName() const;

        // Special analyzed states (does not match directly to a command line parameter!)
        bool IsEmpty() const;

    private:
        bool                InterpretCommandLineParameter( const OUString&, OUString& );
        void                    ParseCommandLine_Impl( Supplier& supplier );
        void                    InitParamValues();

        boost::optional< OUString > m_cwdUrl;

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
        bool m_version;
        bool m_splashpipe;

        OUString m_unknown;

        bool m_bEmpty; // No Args at all
        bool m_bDocumentArgs; // A document creation/open/load arg is used
        std::vector< OUString > m_accept;
        std::vector< OUString > m_unaccept;
        std::vector< OUString > m_openlist; // contains external URIs
        std::vector< OUString > m_viewlist; // contains external URIs
        std::vector< OUString > m_startlist; // contains external URIs
        std::vector< OUString > m_forceopenlist; // contains external URIs
        std::vector< OUString > m_forcenewlist; // contains external URIs
        std::vector< OUString > m_printlist; // contains external URIs
        std::vector< OUString > m_printtolist; // contains external URIs
        OUString m_printername;
        std::vector< OUString > m_conversionlist; // contains external URIs
        OUString m_conversionparams;
        OUString m_conversionout; // contains external URIs
        std::vector< OUString > m_infilter;
        OUString m_language;
        OUString m_pidfile;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
