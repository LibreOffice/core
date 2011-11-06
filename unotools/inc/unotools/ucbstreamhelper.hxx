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


#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#define _UNTOOLS_UCBSTREAMHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include "unotools/unotoolsdllapi.h"

#include <tools/stream.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace task
            {
                class XInteractionHandler;
            }
            namespace io
            {
                class XStream;
                class XInputStream;
            }
        }
    }
}

#define NS_UNO ::com::sun::star::uno
#define NS_IO ::com::sun::star::io
#define NS_TASK ::com::sun::star::task

class String;
namespace utl
{
    class UcbLockBytesHandler;

    class UNOTOOLS_DLLPUBLIC UcbStreamHelper : public SvStream
    {
    public:
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                NS_UNO::Reference < NS_TASK::XInteractionHandler >,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                sal_Bool bFileExists,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XInputStream > xStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XStream > xStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XInputStream > xStream, sal_Bool bCloseStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XStream > xStream, sal_Bool bCloseStream );
    };
}

#endif
