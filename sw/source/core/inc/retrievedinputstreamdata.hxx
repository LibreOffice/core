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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_RETRIEVEDINPUTSTREAMDATA_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_RETRIEVEDINPUTSTREAMDATA_HXX

#include <tools/link.hxx>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <map>
#include <memory>

namespace com::sun::star::io { class XInputStream; }

class SwAsyncRetrieveInputStreamThreadConsumer;

/** Singleton class to manage retrieved input stream data in Writer

    OD 2007-01-29 #i73788#
    The instance of this class provides data container for retrieved input
    stream data. The data container is accessed via a key, which the data
    manager provides on creation of the data container.
    When a certain data container is filled with data, an user event is submitted
    to trigger the processing of with data.
*/
class SwRetrievedInputStreamDataManager
{
    public:

        typedef sal_uInt64 tDataKey;

        struct tData
        {
            std::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > mpThreadConsumer;
            css::uno::Reference<css::io::XInputStream> mxInputStream;
            bool mbIsStreamReadOnly;

            tData()
                : mpThreadConsumer(),
                  mbIsStreamReadOnly( false )
            {};

            tData( std::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > pThreadConsumer )
                : mpThreadConsumer( pThreadConsumer ),
                  mbIsStreamReadOnly( false )
            {};
        };

        static SwRetrievedInputStreamDataManager& GetManager();

        tDataKey ReserveData( std::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > const & pThreadConsumer );

        void PushData( const tDataKey nDataKey,
                       css::uno::Reference<css::io::XInputStream> const & xInputStream,
                       const bool bIsStreamReadOnly );

        bool PopData( const tDataKey nDataKey,
                      tData& rData );

        DECL_LINK( LinkedInputStreamReady, void*, void );

    private:

        static tDataKey snNextKeyValue;

        osl::Mutex maMutex;

        std::map< tDataKey, tData > maInputStreamData;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
