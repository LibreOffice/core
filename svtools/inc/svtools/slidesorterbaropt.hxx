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

#include "svtools/svtdllapi.h"
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

/** forward declaration to our private date container implementation

    We use these class as internal member to support small memory requirements.
    You can create the container if it is neccessary. The class which use these mechanism
    is faster and smaller then a complete implementation!
*/
class SvtSlideSorterBarOptions_Impl;
class Link;

/** collect informations about sidebar group

    \attention This class is partially threadsafe.
*/
class SVT_DLLPUBLIC SvtSlideSorterBarOptions: public utl::detail::Options
{
    public:
        /** standard constructor and destructor

            This will initialize an instance with default values.
            We implement these class with a refcount mechanism! Every instance of this class increase it
            at create and decrease it at delete time - but all instances use the same data container!
            He is implemented as a static member ...

            \sa    member m_nRefCount
            \sa    member m_pDataContainer
        */
        SvtSlideSorterBarOptions();
        virtual ~SvtSlideSorterBarOptions();

        void AddListenerLink( const Link& rLink );
        void RemoveListenerLink( const Link& rLink );

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
            All access methods are'nt safe!
            We create a static mutex only for one ime and use at different times.

            \return     A reference to a static mutex member.*/
        SVT_DLLPRIVATE static ::osl::Mutex& GetInitMutex();

    private:

        /**
            \attention
            Don't initialize these static member in these header!
            \li Double defined symbols will be detected ...
            \li and unresolved externals exist at linking time.
            Do it in your source only.
        */
        static SvtSlideSorterBarOptions_Impl* m_pDataContainer    ;   ///< impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32                      m_nRefCount         ;   ///< internal ref count mechanism

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
