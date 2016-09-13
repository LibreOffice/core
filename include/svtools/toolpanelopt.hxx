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

#ifndef INCLUDED_SVTOOLS_TOOLPANELOPT_HXX
#define INCLUDED_SVTOOLS_TOOLPANELOPT_HXX

#include <svtools/svtdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>
#include <memory>

class SvtToolPanelOptions_Impl;

/** collect information about sidebar group

    \attention This class is partially threadsafe.
*/
class SVT_DLLPUBLIC SvtToolPanelOptions: public utl::detail::Options
{
    public:
        SvtToolPanelOptions();
        virtual ~SvtToolPanelOptions() override;

        bool GetVisibleImpressView() const;
        void SetVisibleImpressView( bool bVisible );
        bool GetVisibleOutlineView() const;
        void SetVisibleOutlineView( bool bVisible );
        bool GetVisibleNotesView() const;
        void SetVisibleNotesView( bool bVisible );
        bool GetVisibleHandoutView() const;
        void SetVisibleHandoutView( bool bVisible );
        bool GetVisibleSlideSorterView() const;
        void SetVisibleSlideSorterView( bool bVisible );


    private:
        /** return a reference to a static mutex

            These class is partially threadsafe (for de-/initialization only).
            All access methods are'nt safe!
            We create a static mutex only for one ime and use at different times.

            \return     A reference to a static mutex member.*/
        SVT_DLLPRIVATE static ::osl::Mutex& GetInitMutex();

    private:
        std::shared_ptr<SvtToolPanelOptions_Impl> m_pImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
