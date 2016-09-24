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

#ifndef INCLUDED_UNOTOOLS_PRINTWARNINGOPTIONS_HXX
#define INCLUDED_UNOTOOLS_PRINTWARNINGOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>
#include <memory>

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtPrintWarningOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about startup features
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtPrintWarningOptions : public utl::detail::Options
{
    public:
         SvtPrintWarningOptions();
        virtual ~SvtPrintWarningOptions() override;

        /*-****************************************************************************************************
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/Print/Warning..."
            @descr      These options describe internal states to enable/disable features of installed office.

            @seealso    configuration package "org.openoffice.Office.Common/_3D-Engine"
        *//*-*****************************************************************************************************/

        bool    IsPaperSize() const;
        bool    IsPaperOrientation() const;
        bool    IsTransparency() const;
        bool    IsModifyDocumentOnPrintingAllowed() const;

        void        SetPaperSize( bool bState );
        void        SetPaperOrientation( bool bState );
        void        SetTransparency( bool bState );
        void        SetModifyDocumentOnPrintingAllowed( bool bState );

    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class use his own static mutex to be threadsafe.
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

    private:
        std::shared_ptr<SvtPrintWarningOptions_Impl> m_pImpl;

};      // class SvtPrintWarningOptions

#endif // INCLUDED_UNOTOOLS_PRINTWARNINGOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
