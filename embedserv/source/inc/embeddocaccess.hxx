/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: embeddocaccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2006-09-25 13:32:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EMBEDDOCACCESS_HXX_
#define _EMBEDDOCACCESS_HXX_

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif

#include <oleidl.h>
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4265)
#include <atldbcli.h>
#else
#include <atlcomcli.h>
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
