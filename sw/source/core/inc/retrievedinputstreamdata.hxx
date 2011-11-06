/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _RETRIEVEDINPUTSTREAMDATA_HXX
#define _RETRIEVEDINPUTSTREAMDATA_HXX

#include <tools/link.hxx>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.hxx>
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HXX_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#include <map>

#include <boost/weak_ptr.hpp>
class SwAsyncRetrieveInputStreamThreadConsumer;
//#ifndef _RETRIEVEINPUTSTREAMCONSUMER_HXX
//#include <retrieveinputstreamconsumer.hxx>
//#endif

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
