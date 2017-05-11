/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLSignature.hxx"
#include "SecurityEnvironment.hxx"
#include "XMLSecurityContext.hxx"

#include <cppuhelper/supportsservice.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::xml::wrapper;
using namespace css::xml::crypto;

XMLSignatureGpg::XMLSignatureGpg()
{
}

XMLSignatureGpg::~XMLSignatureGpg()
{
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate > SAL_CALL XMLSignatureGpg::generate(const Reference< XXMLSignatureTemplate >& aTemplate,
                                                                   const Reference< XSecurityEnvironment >& aEnvironment)
{
    (void)aTemplate;
    (void)aEnvironment;
# if 0
    xmlSecKeysMngrPtr pMngr = nullptr ;
    xmlSecDSigCtxPtr pDsigCtx = nullptr ;
    xmlNodePtr pNode = nullptr ;

    if( !aTemplate.is() )
        throw RuntimeException() ;

    if( !aEnvironment.is() )
        throw RuntimeException() ;

    //Get the xml node
    Reference< XXMLElementWrapper > xElement = aTemplate->getTemplate() ;
    if( !xElement.is() ) {
        throw RuntimeException() ;
    }

    Reference< XUnoTunnel > xNodTunnel( xElement , UNO_QUERY_THROW ) ;
    XMLElementWrapper_XmlSecImpl* pElement =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xNodTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
    if( pElement == nullptr ) {
        throw RuntimeException() ;
    }

    pNode = pElement->getNativeElement() ;

    //Get the stream/URI binding
    Reference< XUriBinding > xUriBinding = aTemplate->getBinding() ;
    if( xUriBinding.is() ) {
        //Register the stream input callbacks into libxml2
        if( xmlRegisterStreamInputCallbacks( xUriBinding ) < 0 )
            throw RuntimeException() ;
    }

    // stubbed out - call xmlsec to calculate c14n & process
    // references, then hand that to gpgme for actual signing
    xmlSecTransformCtxPtr transformCtx;
    if( xmlSecTransformCtxExecute(transformCtx, pNode) < 0 )
    {
        xmlSecInternalError("xmlSecTransformCtxExecute", NULL);
        return nullptr;
    }
    // result = transformCtx->result;
#endif

    return nullptr;
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate > SAL_CALL XMLSignatureGpg::validate(const Reference< XXMLSignatureTemplate >& /*aTemplate*/,
                                                                   const Reference< XXMLSecurityContext >& /*aSecurityCtx*/)
{
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
