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

