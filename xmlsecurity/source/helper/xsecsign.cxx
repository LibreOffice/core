/*************************************************************************
 *
 *  $RCSfile: xsecsign.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-14 11:05:46 $
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

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultBroadcaster.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#include <stdio.h>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxs = com::sun::star::xml::sax;

/* xml security framework components */
#define SIGNATURECREATOR_COMPONENT "com.sun.star.xml.crypto.sax.SignatureCreator"

/* protected: for signature generation */
rtl::OUString XSecController::createId()
{
    cssu::Sequence< sal_Int8 > aSeq( 16 );
    rtl_createUuid ((sal_uInt8 *)aSeq.getArray(), 0, sal_True);

    char str[68]="ID_";
    int length = 3;
    for (int i=0; i<16; ++i)
    {
        length += sprintf(str+length, "%04x", (unsigned char)aSeq[i]);
    }

    return rtl::OUString::createFromAscii(str);
}

cssu::Reference< cssxc::sax::XReferenceResolvedListener > XSecController::prepareSignatureToWrite(
    InternalSignatureInformation& internalSignatureInfor )
{
    sal_Int32 nSecurityId = internalSignatureInfor.signatureInfor.nSecurityId;
    SignatureReferenceInformations& vReferenceInfors = internalSignatureInfor.signatureInfor.vSignatureReferenceInfors;

    sal_Int32 nIdOfSignatureElementCollector;
    cssu::Reference< cssxc::sax::XReferenceResolvedListener > xReferenceResolvedListener;

    nIdOfSignatureElementCollector =
        m_xSAXEventKeeper->addSecurityElementCollector( cssxc::sax::ElementMarkPriority_PRI_AFTERMODIFY, sal_True );

    m_xSAXEventKeeper->setSecurityId(nIdOfSignatureElementCollector, nSecurityId);

        /*
         * create a SignatureCreator
         */
    xReferenceResolvedListener = cssu::Reference< cssxc::sax::XReferenceResolvedListener >(
        mxMSF->createInstance(
            rtl::OUString::createFromAscii(SIGNATURECREATOR_COMPONENT)),
        cssu::UNO_QUERY);

    cssu::Reference<cssl::XInitialization> xInitialization(xReferenceResolvedListener, cssu::UNO_QUERY);

    cssu::Sequence<cssu::Any> args(5);
    args[0] = cssu::makeAny(rtl::OUString::valueOf(nSecurityId));
    args[1] = cssu::makeAny(m_xSAXEventKeeper);
    args[2] = cssu::makeAny(rtl::OUString::valueOf(nIdOfSignatureElementCollector));
    args[3] = cssu::makeAny(m_xSecurityContext);
    args[4] = cssu::makeAny(m_xXMLSignature);
    xInitialization->initialize(args);

    sal_Int32 nBlockerId = m_xSAXEventKeeper->addBlocker();
    m_xSAXEventKeeper->setSecurityId(nBlockerId, nSecurityId);

    cssu::Reference<cssxc::sax::XBlockerMonitor> xBlockerMonitor(xReferenceResolvedListener, cssu::UNO_QUERY);
    xBlockerMonitor->setBlockerId(nBlockerId);

    cssu::Reference< cssxc::sax::XSignatureCreationResultBroadcaster >
        xSignatureCreationResultBroadcaster(xReferenceResolvedListener, cssu::UNO_QUERY);

    xSignatureCreationResultBroadcaster->addSignatureCreationResultListener( this );

    cssu::Reference<cssxc::sax::XReferenceResolvedBroadcaster>
        xReferenceResolvedBroadcaster
        (m_xSAXEventKeeper,
        cssu::UNO_QUERY);

    xReferenceResolvedBroadcaster->addReferenceResolvedListener(
        nIdOfSignatureElementCollector,
        xReferenceResolvedListener);

    cssu::Reference<cssxc::sax::XReferenceCollector> xReferenceCollector
        (xReferenceResolvedListener, cssu::UNO_QUERY);

    int i;
    int size = vReferenceInfors.size();
    sal_Int32 nReferenceCount = 0;

    for(i=0; i<size; ++i)
    {
        sal_Int32 keeperId = internalSignatureInfor.vKeeperIds[i];

        if ( keeperId != -1)
        {
            m_xSAXEventKeeper->setSecurityId(keeperId, nSecurityId);
            xReferenceResolvedBroadcaster->addReferenceResolvedListener( keeperId, xReferenceResolvedListener);
            xReferenceCollector->setReferenceId( keeperId );
            nReferenceCount++;
        }
    }

    xReferenceCollector->setReferenceCount( nReferenceCount );

    /*
     * adds all URI binding
     */
    cssu::Reference<cssxc::XUriBinding> xUriBinding
        (xReferenceResolvedListener, cssu::UNO_QUERY);

    for(i=0; i<size; ++i)
    {
        const SignatureReferenceInformation& refInfor = vReferenceInfors[i];

        cssu::Reference< com::sun::star::io::XInputStream > xInputStream
            = getObjectInputStream( refInfor.ouURI );

        if (xInputStream.is())
        {
            xUriBinding->setUriBinding(refInfor.ouURI,xInputStream);
        }
    }

    cssu::Reference<cssxc::sax::XKeyCollector> keyCollector (xReferenceResolvedListener, cssu::UNO_QUERY);
    keyCollector->setKeyId(0);

    if (internalSignatureInfor.signatureInfor.ouDate.getLength()>0 ||internalSignatureInfor.signatureInfor.ouTime.getLength()>0)
    /*
     * add a reference to datatime property, this reference is transparent to the xmlsecurity
     * framework
     */
    {
        internalSignatureInfor.signatureInfor.ouSignatureId = createId();
        internalSignatureInfor.signatureInfor.ouPropertyId = createId();
        internalSignatureInfor.addReference(1, internalSignatureInfor.signatureInfor.ouPropertyId, -1 );
        size++;
    }

    /*
     * replace both digestValues and signatueValue to " "
     */
    for(i=0; i<size; ++i)
    {
        SignatureReferenceInformation& refInfor = vReferenceInfors[i];
        refInfor.ouDigestValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CHAR_BLANK));
    }

    internalSignatureInfor.signatureInfor.ouSignatureValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CHAR_BLANK));

    return xReferenceResolvedListener;
}

/* public: for signature generation */
void XSecController::collectToSign( sal_Int32 securityId, const rtl::OUString& referenceId )
{
    /* DBG_ASSERT( m_xSAXEventKeeper.is(), "the SAXEventKeeper is NULL" ); */

    chainOn(true);

    if ( m_nStatusOfSecurityComponents == INITIALIZED )
    /*
     * if all security components are ready, add a signature.
     */
    {
        sal_Int32 nKeeperId = m_xSAXEventKeeper->addSecurityElementCollector(
            cssxc::sax::ElementMarkPriority_PRI_AFTERMODIFY, sal_False);

        int index = findSignatureInfor( securityId );

        if ( index == -1 )
        {
            InternalSignatureInformation isi(securityId, NULL);
            isi.addReference(1, referenceId, nKeeperId );
            m_vInternalSignatureInformations.push_back( isi );
        }
        else
        {
            m_vInternalSignatureInformations[index].addReference(1, referenceId, nKeeperId );
        }
    }
}

void XSecController::signAStream( sal_Int32 securityId, const rtl::OUString& uri, const rtl::OUString& objectURL, sal_Bool isBinary)
{
        sal_Int32 type = ((isBinary==sal_True)?2:3);

    int index = findSignatureInfor( securityId );

    if (index == -1)
    {
        InternalSignatureInformation isi(securityId, NULL);
        isi.addReference(type, uri, -1);
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        m_vInternalSignatureInformations[index].addReference(type, uri, -1);
    }
}

void XSecController::setX509Certificate(
    sal_Int32 nSecurityId,
    const rtl::OUString& ouX509IssuerName,
    const rtl::OUString& ouX509SerialNumber)
{
    int index = findSignatureInfor( nSecurityId );

    if ( index == -1 )
    {
        InternalSignatureInformation isi(nSecurityId, NULL);
        isi.signatureInfor.ouX509IssuerName = ouX509IssuerName;
        isi.signatureInfor.ouX509SerialNumber = ouX509SerialNumber;
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        SignatureInformation &si
            = m_vInternalSignatureInformations[index].signatureInfor;
        si.ouX509IssuerName = ouX509IssuerName;
        si.ouX509SerialNumber = ouX509SerialNumber;
    }
}

void XSecController::setDateTime(
    sal_Int32 nSecurityId,
    const rtl::OUString& ouDate,
    const rtl::OUString& ouTime)
{
    int index = findSignatureInfor( nSecurityId );

    if ( index == -1 )
    {
        InternalSignatureInformation isi(nSecurityId, NULL);
        isi.signatureInfor.ouDate = ouDate;
        isi.signatureInfor.ouTime = ouTime;
        m_vInternalSignatureInformations.push_back( isi );
    }
    else
    {
        SignatureInformation &si
            = m_vInternalSignatureInformations[index].signatureInfor;
        si.ouDate = ouDate;
        si.ouTime = ouTime;
    }
}

bool XSecController::WriteSignature(
    const cssu::Reference<cssxs::XDocumentHandler>& xDocumentHandler )
{
    bool rc = false;

    DBG_ASSERT( xDocumentHandler.is(), "I really need a document handler!" );

    /*
     * chain the SAXEventKeeper to the SAX chain
     */
    chainOn(true);

    if ( m_nStatusOfSecurityComponents == INITIALIZED )
    /*
     * if all security components are ready, add the signature
     * stream.
     */
    {
        m_bIsSAXEventKeeperSticky = true;
        m_xSAXEventKeeper->setNextHandler(xDocumentHandler);

        try
        {
            /*
             * export the signature template
             */
            cssu::Reference<cssxs::XDocumentHandler> xSEKHandler( m_xSAXEventKeeper,cssu::UNO_QUERY);

            int i;
            int sigNum = m_vInternalSignatureInformations.size();

            for (i=0; i<sigNum; ++i)
            {
                InternalSignatureInformation &isi = m_vInternalSignatureInformations[i];

                /*
                 * prepare the signature creator
                 */
                isi.xReferenceResolvedListener
                    = prepareSignatureToWrite( isi );

                exportSignature( xSEKHandler, isi.signatureInfor );
            }

            m_bIsSAXEventKeeperSticky = false;
            chainOff();

            rc = true;
        }
        catch( cssxs::SAXException& )
        {
            m_pErrorMessage = ERROR_SAXEXCEPTIONDURINGCREATION;
        }
        catch( com::sun::star::io::IOException& )
        {
            m_pErrorMessage = ERROR_IOEXCEPTIONDURINGCREATION;
        }
        catch( cssu::Exception& )
        {
            m_pErrorMessage = ERROR_EXCEPTIONDURINGCREATION;
        }

        m_xSAXEventKeeper->setNextHandler( NULL );
        m_bIsSAXEventKeeperSticky = false;
    }
    else
    {
        m_pErrorMessage = ERROR_CANNOTCREATEXMLSECURITYCOMPONENT;
    }

    return rc;
}

