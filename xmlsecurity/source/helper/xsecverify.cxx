/*************************************************************************
 *
 *  $RCSfile: xsecverify.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-15 08:12:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <xsecctl.hxx>
#include "xsecparser.hxx"

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultBroadcaster.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxs = com::sun::star::xml::sax;

/* xml security framework components */
#define SIGNATUREVERIFIER_COMPONENT "com.sun.star.xml.crypto.sax.SignatureVerifier"

/* protected: for signature verify */
cssu::Reference< cssxc::sax::XReferenceResolvedListener > XSecController::prepareSignatureToRead(
    sal_Int32 nSecurityId)
{
    if ( m_nStatusOfSecurityComponents != INITIALIZED )
    {
        return NULL;
    }

    sal_Int32 nIdOfSignatureElementCollector;
    cssu::Reference< cssxc::sax::XReferenceResolvedListener > xReferenceResolvedListener;

    nIdOfSignatureElementCollector =
        m_xSAXEventKeeper->addSecurityElementCollector( cssxc::sax::ElementMarkPriority_PRI_BEFOREMODIFY, sal_False);

    m_xSAXEventKeeper->setSecurityId(nIdOfSignatureElementCollector, nSecurityId);

        /*
         * create a SignatureVerifier
         */
    xReferenceResolvedListener = cssu::Reference< cssxc::sax::XReferenceResolvedListener >(
        mxMSF->createInstance(
            rtl::OUString::createFromAscii( SIGNATUREVERIFIER_COMPONENT )),
        cssu::UNO_QUERY);

    cssu::Reference<cssl::XInitialization> xInitialization(xReferenceResolvedListener, cssu::UNO_QUERY);

    cssu::Sequence<cssu::Any> args(5);
    args[0] = cssu::makeAny(rtl::OUString::valueOf(nSecurityId));
    args[1] = cssu::makeAny(m_xSAXEventKeeper);
    args[2] = cssu::makeAny(rtl::OUString::valueOf(nIdOfSignatureElementCollector));
    args[3] = cssu::makeAny(m_xSecurityContext);
    args[4] = cssu::makeAny(m_xXMLSignature);
    xInitialization->initialize(args);

    cssu::Reference< cssxc::sax::XSignatureVerifyResultBroadcaster >
        signatureVerifyResultBroadcaster(xReferenceResolvedListener, cssu::UNO_QUERY);

    signatureVerifyResultBroadcaster->addSignatureVerifyResultListener( this );

    cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster> xReferenceResolvedBroadcaster
        (m_xSAXEventKeeper,
        cssu::UNO_QUERY);

    xReferenceResolvedBroadcaster->addReferenceResolvedListener(
        nIdOfSignatureElementCollector,
        xReferenceResolvedListener);

    cssu::Reference<cssxc::sax::XKeyCollector> keyCollector (xReferenceResolvedListener, cssu::UNO_QUERY);
    keyCollector->setKeyId(0);

    return xReferenceResolvedListener;
}

void XSecController::addSignature()
{
    cssu::Reference< cssxc::sax::XReferenceResolvedListener > xReferenceResolvedListener = NULL;
    sal_Int32 nSignatureId = 0;


    if (m_bVerifyCurrentSignature)
    {
        chainOn(true);
        xReferenceResolvedListener = prepareSignatureToRead( m_nReservedSignatureId );
        m_bVerifyCurrentSignature = false;
        nSignatureId = m_nReservedSignatureId;
    }

    InternalSignatureInformation isi( nSignatureId, xReferenceResolvedListener );
    m_vInternalSignatureInformations.push_back( isi );
}

void XSecController::addReference( const rtl::OUString& ouUri)
{
    InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];
    isi.addReference(TYPE_SAMEDOCUMENT_REFERENCE,ouUri, -1 );
}

void XSecController::addStreamReference(
    const rtl::OUString& ouUri,
    bool isBinary )
{
        sal_Int32 type = (isBinary?TYPE_BINARYSTREAM_REFERENCE:TYPE_XMLSTREAM_REFERENCE);

    InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];

    if ( isi.xReferenceResolvedListener.is() )
    {
            /*
             * get the input stream
             */
            cssu::Reference< com::sun::star::io::XInputStream > xObjectInputStream
                = getObjectInputStream( ouUri );

        if ( xObjectInputStream.is() )
        {
            cssu::Reference<cssxc::XUriBinding> xUriBinding
                (isi.xReferenceResolvedListener, cssu::UNO_QUERY);
            xUriBinding->setUriBinding(ouUri, xObjectInputStream);
        }
    }

    isi.addReference(type, ouUri, -1);
}

void XSecController::setReferenceCount() const
{
    const InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];

    if ( isi.xReferenceResolvedListener.is() )
    {
        const SignatureReferenceInformations &refInfors = isi.signatureInfor.vSignatureReferenceInfors;

        int refNum = refInfors.size();
        sal_Int32 referenceCount = 0;

        for(int i=0 ; i<refNum; ++i)
        {
            if (refInfors[i].nType == TYPE_SAMEDOCUMENT_REFERENCE )
            /*
             * same-document reference
             */
            {
                referenceCount++;
            }
        }

        cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
            (isi.xReferenceResolvedListener, cssu::UNO_QUERY);
        xReferenceCollector->setReferenceCount( referenceCount );
    }
}

void XSecController::setX509IssuerName( rtl::OUString& ouX509IssuerName )
{
    InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];
    isi.signatureInfor.ouX509IssuerName = ouX509IssuerName;
}

void XSecController::setX509SerialNumber( rtl::OUString& ouX509SerialNumber )
{
    InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];
    isi.signatureInfor.ouX509SerialNumber = ouX509SerialNumber;
}

void XSecController::setX509Certificate( rtl::OUString& ouX509Certificate )
{
    InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];
    isi.signatureInfor.ouX509Certificate = ouX509Certificate;
}

void XSecController::setSignatureValue( rtl::OUString& ouSignatureValue )
{
    InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];
    isi.signatureInfor.ouSignatureValue = ouSignatureValue;
}

void XSecController::setDigestValue( rtl::OUString& ouDigestValue )
{
    SignatureInformation &si = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1].signatureInfor;
    SignatureReferenceInformation &reference = si.vSignatureReferenceInfors[si.vSignatureReferenceInfors.size()-1];
    reference.ouDigestValue = ouDigestValue;
}

void XSecController::setDate( rtl::OUString& ouDate )
{
    InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];
    isi.signatureInfor.ouDate = ouDate;
}

void XSecController::setTime( rtl::OUString& ouTime )
{
    InternalSignatureInformation &isi = m_vInternalSignatureInformations[m_vInternalSignatureInformations.size()-1];
    isi.signatureInfor.ouTime = ouTime;
}

/* public: for signature verify */
void XSecController::collectToVerify( const rtl::OUString& referenceId )
{
    /* DBG_ASSERT( m_xSAXEventKeeper.is(), "the SAXEventKeeper is NULL" ); */

    if ( m_nStatusOfSecurityComponents == INITIALIZED )
    /*
     * if all security components are ready, verify the signature.
     */
    {
        bool bJustChainingOn = false;
        cssu::Reference< cssxs::XDocumentHandler > xHandler = NULL;

        int i,j;
        int sigNum = m_vInternalSignatureInformations.size();

        for (i=0; i<sigNum; ++i)
        {
            InternalSignatureInformation& isi = m_vInternalSignatureInformations[i];
            SignatureReferenceInformations& vReferenceInfors = isi.signatureInfor.vSignatureReferenceInfors;
            int refNum = vReferenceInfors.size();

            for (j=0; j<refNum; ++j)
            {
                SignatureReferenceInformation &refInfor = vReferenceInfors[j];

                if (refInfor.ouURI == referenceId)
                {
                    if (chainOn(false))
                    {
                        bJustChainingOn = true;
                        xHandler = m_xSAXEventKeeper->setNextHandler(NULL);
                    }

                    sal_Int32 nKeeperId = m_xSAXEventKeeper->addSecurityElementCollector(
                        cssxc::sax::ElementMarkPriority_PRI_BEFOREMODIFY, sal_False );

                    cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster> xReferenceResolvedBroadcaster
                        (m_xSAXEventKeeper,
                        cssu::UNO_QUERY );

                    cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
                        ( isi.xReferenceResolvedListener, cssu::UNO_QUERY );

                    m_xSAXEventKeeper->setSecurityId(nKeeperId, isi.signatureInfor.nSecurityId);
                    xReferenceResolvedBroadcaster->addReferenceResolvedListener( nKeeperId, isi.xReferenceResolvedListener);
                    xReferenceCollector->setReferenceId( nKeeperId );

                    isi.vKeeperIds[j] = nKeeperId;
                    break;
                }
            }
        }

        if ( bJustChainingOn )
        {
            cssu::Reference< cssxs::XDocumentHandler > xSEKHandler(m_xSAXEventKeeper, cssu::UNO_QUERY);
            if (m_xElementStackKeeper.is())
            {
                m_xElementStackKeeper->retrieve(xSEKHandler, sal_True);
            }
            m_xSAXEventKeeper->setNextHandler(xHandler);
        }
    }
}

void XSecController::addSignature( sal_Int32 nSignatureId )
{
    DBG_ASSERT( m_pXSecParser != NULL, "No XSecParser initialized" );

    m_nReservedSignatureId = nSignatureId;
    m_bVerifyCurrentSignature = true;
}

cssu::Reference< cssxs::XDocumentHandler > XSecController::createSignatureReader()
{
    m_pXSecParser = new XSecParser( this, NULL );
    cssu::Reference< cssl::XInitialization > xInitialization = m_pXSecParser;

    setSAXChainConnector(xInitialization, NULL, NULL);

    return m_pXSecParser;
}

void XSecController::releaseSignatureReader()
{
    clearSAXChainConnector( );
    m_pXSecParser = NULL;
}

