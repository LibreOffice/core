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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INC_EMBEDDOCACCESS_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INC_EMBEDDOCACCESS_HXX

#define OLESERV_SAVEOBJECT      1
#define OLESERV_CLOSE           2
#define OLESERV_NOTIFY          3
#define OLESERV_NOTIFYCLOSING   4
#define OLESERV_SHOWOBJECT      5
#define OLESERV_DEACTIVATE      6

#include <oleidl.h>
#pragma warning(disable : 4265)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wmicrosoft"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include <atldbcli.h>
#if defined __clang__
#pragma clang diagnostic pop
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
