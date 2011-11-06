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


#ifndef _RETRIEVEINPUTSTREAM_HXX
#define _RETRIEVEINPUTSTREAM_HXX

#include <observablethread.hxx>
#include <tools/string.hxx>
//#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
//#include <com/sun/star/uno/Reference.hxx>
//#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HXX_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#include <retrievedinputstreamdata.hxx>

/** class for a thread to retrieve an input stream given by an URL

    OD 2007-01-29 #i73788#

    @author OD
*/
class SwAsyncRetrieveInputStreamThread : public ObservableThread
{
    public:

        static ::rtl::Reference< ObservableThread > createThread(
                    const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                    const String& rLinkedURL );

        virtual ~SwAsyncRetrieveInputStreamThread();

    protected:

        virtual void threadFunction();

    private:

        SwAsyncRetrieveInputStreamThread( const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                                          const String& rLinkedURL );

        const SwRetrievedInputStreamDataManager::tDataKey mnDataKey;
        const String mrLinkedURL;
};
#endif
