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

#ifndef INCLUDED_SVTOOLS_SLIDESORTERBAROPT_HXX
#define INCLUDED_SVTOOLS_SLIDESORTERBAROPT_HXX

#include <svtools/svtdllapi.h>
#include <unotools/options.hxx>
#include <memory>

namespace osl { class Mutex; }

/** forward declaration to our private date container implementation

    We use these class as internal member to support small memory requirements.
    You can create the container if it is necessary. The class which use these mechanism
    is faster and smaller then a complete implementation!
*/
class SvtSlideSorterBarOptions_Impl;

/** collect information about sidebar group

    \attention This class is partially threadsafe.
*/
class SVT_DLLPUBLIC SvtSlideSorterBarOptions: public utl::detail::Options
{
    public:
        SvtSlideSorterBarOptions();
        virtual ~SvtSlideSorterBarOptions() override;

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
        bool GetVisibleDrawView() const;
        void SetVisibleDrawView( bool bVisible );


    private:
        /** return a reference to a static mutex

            These class is partially threadsafe (for de-/initialization only).
            All access methods aren't safe!
            We create a static mutex only for one ime and use at different times.

            \return     A reference to a static mutex member.*/
        SVT_DLLPRIVATE static ::osl::Mutex& GetInitMutex();

    private:
        std::shared_ptr<SvtSlideSorterBarOptions_Impl> m_pImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
