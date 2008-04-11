/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: embeddocaccess.hxx,v $
 * $Revision: 1.5 $
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

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif

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
    EmbedDocument_Impl* m_pEmbedDocument;
    CComPtr< IPersistStorage > m_pLocker;
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
