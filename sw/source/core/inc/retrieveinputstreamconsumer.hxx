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


#ifndef _RETRIEVEINPUTSTREAMCONSUMER_HXX
#define _RETRIEVEINPUTSTREAMCONSUMER_HXX

#include <com/sun/star/uno/Reference.hxx>
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HXX_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#include <osl/interlck.h>
#include <retrievedinputstreamdata.hxx>

class SwGrfNode;
class String;
class SwAsyncRetrieveInputStreamThread;

/** class to provide creation of a thread to retrieve an input stream given by
    an URL and to consume the retrieved input stream.

    OD 2007-01-29 #i73788#

    @author OD
*/
class SwAsyncRetrieveInputStreamThreadConsumer
{
    public:

        SwAsyncRetrieveInputStreamThreadConsumer( SwGrfNode& rGrfNode );
        ~SwAsyncRetrieveInputStreamThreadConsumer();

        /** method to create thread

            @author OD
        */
        void CreateThread( const String& rURL );

        /** method called to provide the retrieved input stream to the thread Consumer

            @author OD
        */
        void ApplyInputStream(
            com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
            const sal_Bool bIsStreamReadOnly );

    private:

        SwGrfNode& mrGrfNode;
        oslInterlockedCount mnThreadID;
};
#endif
