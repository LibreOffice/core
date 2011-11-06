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



#ifndef _EMBEDDOCACCESS_HXX_
#define _EMBEDDOCACCESS_HXX_

#include <cppuhelper/weak.hxx>

#define OLESERV_SAVEOBJECT      1
#define OLESERV_CLOSE           2
#define OLESERV_NOTIFY          3
#define OLESERV_NOTIFYCLOSING   4
#define OLESERV_SHOWOBJECT      5
#define OLESERV_DEACTIVATE      6

#include <oleidl.h>
#ifndef __MINGW32__
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4265)
#include <atldbcli.h>
#else
#include <atlcomcli.h>
#endif
#endif
#include <cppuhelper/weak.hxx>

class EmbedDocument_Impl;
struct LockedEmbedDocument_Impl
{
private:
    EmbedDocument_Impl* m_pEmbedDocument;

public:
    LockedEmbedDocument_Impl();
    LockedEmbedDocument_Impl( EmbedDocument_Impl* pEmbedDocument );
    LockedEmbedDocument_Impl( const LockedEmbedDocument_Impl& aDocLock );

    ~LockedEmbedDocument_Impl();

    LockedEmbedDocument_Impl& operator=( const LockedEmbedDocument_Impl& aDocLock );

    EmbedDocument_Impl* GetEmbedDocument() { return m_pEmbedDocument; }

    void ExecuteMethod( sal_Int16 nId );
};

class EmbeddedDocumentInstanceAccess_Impl : public ::cppu::OWeakObject
{
    ::osl::Mutex m_aMutex;
    EmbedDocument_Impl* m_pEmbedDocument;

    public:
    EmbeddedDocumentInstanceAccess_Impl( EmbedDocument_Impl* pDoc )
    : m_pEmbedDocument( pDoc )
    {}

    LockedEmbedDocument_Impl GetEmbedDocument();
    void ClearEmbedDocument();
};

#endif
