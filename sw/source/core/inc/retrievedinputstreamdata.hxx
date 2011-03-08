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
#ifndef _RETRIEVEDINPUTSTREAMDATA_HXX
#define _RETRIEVEDINPUTSTREAMDATA_HXX

#include <tools/link.hxx>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>

#include <map>

#include <boost/weak_ptr.hpp>

class SwAsyncRetrieveInputStreamThreadConsumer;

/** Singleton class to manage retrieved input stream data in Writer

    OD 2007-01-29 #i73788#
    The instance of this class provides data container for retrieved input
    stream data. The data container is accessed via a key, which the data
    manager provides on creation of the data container.
    When a certain data container is filled with data, an user event is submitted
    to trigger the processing of with data.

    @author OD
*/
class SwRetrievedInputStreamDataManager
{
    public:

        typedef sal_uInt64 tDataKey;

        struct tData
        {
            boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > mpThreadConsumer;
            com::sun::star::uno::Reference<com::sun::star::io::XInputStream> mxInputStream;
            sal_Bool mbIsStreamReadOnly;

            tData()
                : mpThreadConsumer(),
                  mbIsStreamReadOnly( sal_False )
            {};

            tData( boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > pThreadConsumer )
                : mpThreadConsumer( pThreadConsumer ),
                  mbIsStreamReadOnly( sal_False )
            {};
        };

        static SwRetrievedInputStreamDataManager& GetManager();

        ~SwRetrievedInputStreamDataManager()
        {
        };

        tDataKey ReserveData( boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > pThreadConsumer );

        void PushData( const tDataKey nDataKey,
                       com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
                       const sal_Bool bIsStreamReadOnly );

        bool PopData( const tDataKey nDataKey,
                      tData& rData );

        DECL_LINK( LinkedInputStreamReady, SwRetrievedInputStreamDataManager::tDataKey* );

    private:

        static SwRetrievedInputStreamDataManager* mpManager;
        static tDataKey mnNextKeyValue;
        static osl::Mutex maGetManagerMutex;

        osl::Mutex maMutex;

        std::map< tDataKey, tData > maInputStreamData;

        SwRetrievedInputStreamDataManager()
        {
        };
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
