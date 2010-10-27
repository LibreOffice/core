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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
