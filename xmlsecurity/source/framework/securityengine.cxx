/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: securityengine.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:18:28 $
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

#include "securityengine.hxx"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

SecurityEngine::SecurityEngine( const cssu::Reference< cssl::XMultiServiceFactory >& rxMSF )
    :mxMSF( rxMSF ),
     m_nIdOfTemplateEC(-1),
     m_nIdOfKeyEC(-1),
     m_nNumOfResolvedReferences(0),
     m_bMissionDone(false),
     m_nSecurityId(-1),
     m_nStatus(::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN)
{
}

/* XReferenceResolvedListener */
void SAL_CALL SecurityEngine::referenceResolved( sal_Int32 referenceId )
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

