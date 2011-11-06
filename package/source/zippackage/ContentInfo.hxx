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


#ifndef _CONTENT_INFO_HXX_
#define _CONTENT_INFO_HXX_

#include <com/sun/star/container/XNameContainer.hpp>
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEl_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#include <ZipPackageFolder.hxx>
#include <ZipPackageStream.hxx>

namespace com { namespace sun { namespace star { namespace packages {
class ContentInfo : public cppu::OWeakObject
{
public:
    com::sun::star::uno::Reference < com::sun::star::lang::XUnoTunnel > xTunnel;
    bool bFolder;
    union
    {
        ZipPackageFolder *pFolder;
        ZipPackageStream *pStream;
    };
    ContentInfo ( ZipPackageStream * pNewStream )
    : xTunnel ( pNewStream )
    , bFolder ( false )
    , pStream ( pNewStream )
    {
    }
    ContentInfo ( ZipPackageFolder * pNewFolder )
    : xTunnel ( pNewFolder )
    , bFolder ( true )
    , pFolder ( pNewFolder )
    {
    }
    virtual ~ContentInfo ()
    {
        if ( bFolder )
            pFolder->releaseUpwardRef();
        else
            pStream->clearParent();
    }
};
} } } }
#endif
