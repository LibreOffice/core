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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include "securityengine.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

SecurityEngine::SecurityEngine( const cssu::Reference< cssl::XMultiServiceFactory >& rxMSF )
    :mxMSF( rxMSF ),
     m_nIdOfTemplateEC(-1),
     m_nNumOfResolvedReferences(0),
     m_nIdOfKeyEC(-1),
     m_bMissionDone(false),
     m_nSecurityId(-1),
     m_nStatus(::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN)
{
}

/* XReferenceResolvedListener */
void SAL_CALL SecurityEngine::referenceResolved( sal_Int32 /*referenceId*/)
    throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
{
    m_nNumOfResolvedReferences++;
    tryToPerform();
}

/* XKeyCollector */
void SAL_CALL SecurityEngine::setKeyId( sal_Int32 id )
    throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
{
    m_nIdOfKeyEC = id;
    tryToPerform();
}

/* XMissionTaker */
sal_Bool SAL_CALL SecurityEngine::endMission(  )
    throw (com::sun::star::uno::RuntimeException)
{
    sal_Bool rc = m_bMissionDone;

    if (!rc)
    {
        clearUp( );

        notifyResultListener();
        m_bMissionDone = true;
    }

    m_xResultListener = NULL;
    m_xSAXEventKeeper = NULL;

    return rc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
