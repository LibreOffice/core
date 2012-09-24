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


#include <xsecctl.hxx>
#include <tools/debug.hxx>

#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeBroadcaster.hpp>
#include <com/sun/star/xml/crypto/SecurityOperationStatus.hpp>

#include <xmloff/attrlist.hxx>
#include <rtl/math.hxx>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxs = com::sun::star::xml::sax;
namespace cssxw = com::sun::star::xml::wrapper;

const sal_Int8 XML_MAXDIGITSCOUNT_TIME = 11;
const sal_Int8 XML_MAXDIGITSCOUNT_DATETIME = 6;

/* bridge component names */
#define XMLSIGNATURE_COMPONENT "com.sun.star.xml.crypto.XMLSignature"
#define XMLDOCUMENTWRAPPER_COMPONENT "com.sun.star.xml.wrapper.XMLDocumentWrapper"

/* xml security framework components */
#define SAXEVENTKEEPER_COMPONENT "com.sun.star.xml.crypto.sax.SAXEventKeeper"

/* string for package protocol */
#define PACKAGEPROTOCOL "vnd.sun.star.Package:"

XSecController::XSecController( const cssu::Reference<cssu::XComponentContext>& rxCtx )
    :mxCtx(rxCtx),
     m_nNextSecurityId(1),
      m_bIsSAXEventKeeperConnected(false),
     m_nStatusOfSecurityComponents(UNINITIALIZED),
      m_bIsSAXEventKeeperSticky(false),
     m_pErrorMessage(NULL),
     m_pXSecParser(NULL)
{
}

XSecController::~XSecController()
{
}


/*
 * private methods
 */
/** convert string to number with optional min and max values */
sal_Bool XSecController::convertNumber( sal_Int32& rValue,
                                        const rtl::OUString& rString,
                                        sal_Int32 /*nMin*/, sal_Int32 /*nMax*/ )
{
    sal_Bool bNeg = sal_False;
    rValue = 0;

    sal_Int32 nPos = 0L;
    sal_Int32 nLen = rString.getLength();

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rString[nPos] )
        nPos++;

    if( nPos < nLen && sal_Unicode('-') == rString[nPos] )
    {
        bNeg = sal_True;
        nPos++;
    }

    // get number
    while( nPos < nLen &&
           sal_Unicode('0') <= rString[nPos] &&
           sal_Unicode('9') >= rString[nPos] )
    {
        // TODO: check overflow!
        rValue *= 10;
        rValue += (rString[nPos] - sal_Unicode('0'));
        nPos++;
    }

    if( bNeg )
        rValue *= -1;

    return nPos == nLen;
}

/** convert util::DateTime to ISO Date String */
void XSecController::convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const com::sun::star::util::DateTime& rDateTime )
{
    rBuffer.append((sal_Int32) rDateTime.Year);
    rBuffer.append('-');
    if( rDateTime.Month < 10 )
        rBuffer.append('0');
    rBuffer.append((sal_Int32) rDateTime.Month);
    rBuffer.append('-');
    if( rDateTime.Day < 10 )
        rBuffer.append('0');
    rBuffer.append((sal_Int32) rDateTime.Day);

    if( rDateTime.Seconds != 0 ||
        rDateTime.Minutes != 0 ||
        rDateTime.Hours   != 0 )
    {
        rBuffer.append('T');
        if( rDateTime.Hours < 10 )
            rBuffer.append('0');
        rBuffer.append((sal_Int32) rDateTime.Hours);
        rBuffer.append(':');
        if( rDateTime.Minutes < 10 )
            rBuffer.append('0');
        rBuffer.append((sal_Int32) rDateTime.Minutes);
        rBuffer.append(':');
        if( rDateTime.Seconds < 10 )
            rBuffer.append('0');
        rBuffer.append((sal_Int32) rDateTime.Seconds);
        if ( rDateTime.HundredthSeconds > 0)
        {
            rBuffer.append(',');
            if (rDateTime.HundredthSeconds < 10)
                rBuffer.append('0');
            rBuffer.append((sal_Int32) rDateTime.HundredthSeconds);
        }
    }
}

/** convert ISO Date String to util::DateTime */
sal_Bool XSecController::convertDateTime( com::sun::star::util::DateTime& rDateTime,
                                     const ::rtl::OUString& rString )
{
    sal_Bool bSuccess = sal_True;

    rtl::OUString aDateStr, aTimeStr, sHundredth;
    sal_Int32 nPos = rString.indexOf( (sal_Unicode) 'T' );
    sal_Int32 nPos2 = rString.indexOf( (sal_Unicode) ',' );
    if ( nPos >= 0 )
    {
        aDateStr = rString.copy( 0, nPos );
        if ( nPos2 >= 0 )
        {
            aTimeStr = rString.copy( nPos + 1, nPos2 - nPos - 1 );

            //Get the fraction of a second with the accuracy of one hundreds second.
            //The fraction part of the date could have different accuracies. To calculate
            //the count of a hundredth units one could form a fractional number by appending
            //the value of the time string to 0. Then multiply it by 100 and use only the whole number.
            //For example: 5:27:46,1 -> 0,1 * 100 = 10
            //5:27:46,01 -> 0,01 * 100 = 1
            //5:27:46,001 -> 0,001 * 100 = 0
            //Due to the inaccuracy of floating point numbers the result may not be the same on different
            //platforms. We had the case where we had a value of 24 hundredth of second, which converted to
            //23 on Linux and 24 on Solaris and Windows.

            //we only support a hundredth second
            //make ,1 -> 10   ,01 -> 1    ,001 -> only use first two diggits
            sHundredth = rString.copy(nPos2 + 1);
            sal_Int32 len = sHundredth.getLength();
            if (len == 1)
                sHundredth += rtl::OUString("0");
            if (len > 2)
                sHundredth = sHundredth.copy(0, 2);
        }
        else
        {
            aTimeStr = rString.copy(nPos + 1);
            sHundredth = rtl::OUString("0");
        }
    }
    else
        aDateStr = rString;         // no separator: only date part

    sal_Int32 nYear  = 1899;
    sal_Int32 nMonth = 12;
    sal_Int32 nDay   = 30;
    sal_Int32 nHour  = 0;
    sal_Int32 nMin   = 0;
    sal_Int32 nSec   = 0;

    const sal_Unicode* pStr = aDateStr.getStr();
    sal_Int32 nDateTokens = 1;
    while ( *pStr )
    {
        if ( *pStr == '-' )
            nDateTokens++;
        pStr++;
    }
    if ( nDateTokens > 3 || aDateStr.isEmpty() )
        bSuccess = sal_False;
    else
    {
        sal_Int32 n = 0;
        if ( !convertNumber( nYear, aDateStr.getToken( 0, '-', n ), 0, 9999 ) )
            bSuccess = sal_False;
        if ( nDateTokens >= 2 )
            if ( !convertNumber( nMonth, aDateStr.getToken( 0, '-', n ), 0, 12 ) )
                bSuccess = sal_False;
        if ( nDateTokens >= 3 )
            if ( !convertNumber( nDay, aDateStr.getToken( 0, '-', n ), 0, 31 ) )
                bSuccess = sal_False;
    }

    if ( !aTimeStr.isEmpty() )           // time is optional
    {
        pStr = aTimeStr.getStr();
        sal_Int32 nTimeTokens = 1;
        while ( *pStr )
        {
            if ( *pStr == ':' )
                nTimeTokens++;
            pStr++;
        }
        if ( nTimeTokens > 3 )
            bSuccess = sal_False;
        else
        {
            sal_Int32 n = 0;
            if ( !convertNumber( nHour, aTimeStr.getToken( 0, ':', n ), 0, 23 ) )
                bSuccess = sal_False;
            if ( nTimeTokens >= 2 )
                if ( !convertNumber( nMin, aTimeStr.getToken( 0, ':', n ), 0, 59 ) )
                    bSuccess = sal_False;
            if ( nTimeTokens >= 3 )
                if ( !convertNumber( nSec, aTimeStr.getToken( 0, ':', n ), 0, 59 ) )
                    bSuccess = sal_False;
        }
    }

    if (bSuccess)
    {
        rDateTime.Year = (sal_uInt16)nYear;
        rDateTime.Month = (sal_uInt16)nMonth;
        rDateTime.Day = (sal_uInt16)nDay;
        rDateTime.Hours = (sal_uInt16)nHour;
        rDateTime.Minutes = (sal_uInt16)nMin;
        rDateTime.Seconds = (sal_uInt16)nSec;
 //       rDateTime.HundredthSeconds = sDoubleStr.toDouble() * 100;
        rDateTime.HundredthSeconds = static_cast<sal_uInt16>(sHundredth.toInt32());
    }
    return bSuccess;
}

int XSecController::findSignatureInfor( sal_Int32 nSecurityId) const
/****** XSecController/findSignatureInfor *************************************
 *
 *   NAME
 *  findSignatureInfor -- find SignatureInformation struct for a particular
 *                        signature
 *
 *   SYNOPSIS
 *  index = findSignatureInfor( nSecurityId );
 *
 *   FUNCTION
 *  see NAME.
 *
 *   INPUTS
 *  nSecurityId - the signature's id
 *
 *   RESULT
 *  index - the index of the signature, or -1 when no such signature
 *          existing
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    int i;
    int size = m_vInternalSignatureInformations.size();

    for (i=0; i<size; ++i)
    {
        if (m_vInternalSignatureInformations[i].signatureInfor.nSecurityId == nSecurityId)
        {
            return i;
        }
    }

    return -1;
}

void XSecController::createXSecComponent( )
/****** XSecController/createXSecComponent ************************************
 *
 *   NAME
 *  bResult = createXSecComponent -- creates xml security components
 *
 *   SYNOPSIS
 *  createXSecComponent( );
 *
 *   FUNCTION
 *  Creates xml security components, including:
 *  1. an xml signature bridge component ( Java based or C based)
 *  2. an XMLDocumentWrapper component ( Java based or C based)
 *  3. a SAXEventKeeper component
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    rtl::OUString sSAXEventKeeper( SAXEVENTKEEPER_COMPONENT );
    rtl::OUString sXMLSignature( XMLSIGNATURE_COMPONENT );
    rtl::OUString sXMLDocument( XMLDOCUMENTWRAPPER_COMPONENT );

    /*
     * marks all security components are not available.
     */
    m_nStatusOfSecurityComponents = FAILTOINITIALIZED;
    m_xXMLSignature = NULL;
    m_xXMLDocumentWrapper = NULL;
    m_xSAXEventKeeper = NULL;

    cssu::Reference< cssl::XMultiComponentFactory > xMCF( mxCtx->getServiceManager() );

    m_xXMLSignature = cssu::Reference< cssxc::XXMLSignature >(
        xMCF->createInstanceWithContext( sXMLSignature, mxCtx ),
        cssu::UNO_QUERY );

    bool bSuccess = (0!=m_xXMLSignature.is());
    if ( bSuccess )
    /*
     * XMLSignature created successfully.
     */
    {
        m_xXMLDocumentWrapper = cssu::Reference< cssxw::XXMLDocumentWrapper >(
            xMCF->createInstanceWithContext( sXMLDocument, mxCtx ),
            cssu::UNO_QUERY );
    }

    bSuccess &= (0!=m_xXMLDocumentWrapper.is());
    if ( bSuccess )
    /*
     * XMLDocumentWrapper created successfully.
     */
    {
        m_xSAXEventKeeper = cssu::Reference< cssxc::sax::XSecuritySAXEventKeeper >(
            xMCF->createInstanceWithContext( sSAXEventKeeper, mxCtx ),
            cssu::UNO_QUERY );
    }

    bSuccess &= (0!=m_xSAXEventKeeper.is());

    if (bSuccess)
    /*
     * SAXEventKeeper created successfully.
     */
    {
        cssu::Reference< cssl::XInitialization > xInitialization(m_xSAXEventKeeper,  cssu::UNO_QUERY);

        cssu::Sequence <cssu::Any> arg(1);
        arg[0] = cssu::makeAny(m_xXMLDocumentWrapper);
        xInitialization->initialize(arg);

        cssu::Reference<cssxc::sax::XSAXEventKeeperStatusChangeBroadcaster>
            xSAXEventKeeperStatusChangeBroadcaster(m_xSAXEventKeeper, cssu::UNO_QUERY);
        cssu::Reference< cssxc::sax::XSAXEventKeeperStatusChangeListener >
            xStatusChangeListener = this;

        xSAXEventKeeperStatusChangeBroadcaster
            ->addSAXEventKeeperStatusChangeListener( xStatusChangeListener );

        m_nStatusOfSecurityComponents = INITIALIZED;
    }
}

bool XSecController::chainOn( bool bRetrievingLastEvent )
/****** XSecController/chainOn ************************************************
 *
 *   NAME
 *  chainOn -- tyies to connect the SAXEventKeeper with the SAX chain.
 *
 *   SYNOPSIS
 *  bJustChainingOn = chainOn( bRetrievingLastEvent );
 *
 *   FUNCTION
 *  First, checks whether the SAXEventKeeper is on the SAX chain. If not,
 *  creates xml security components, and chains the SAXEventKeeper into
 *  the SAX chain.
 *  Before being chained in, the SAXEventKeeper needs to receive all
 *  missed key SAX events, which can promise the DOM tree bufferred by the
 *  SAXEventKeeper has the same structure with the original document.
 *
 *   INPUTS
 *  bRetrievingLastEvent - whether to retrieve the last key SAX event from
 *                         the ElementStackKeeper.
 *
 *   RESULT
 *  bJustChainingOn - whether the SAXEventKeeper is just chained into the
 *                    SAX chain.
 *
 *   NOTES
 *  Sometimes, the last key SAX event can't be transferred to the
 *  SAXEventKeeper together.
 *  For instance, at the time an referenced element is detected, the
 *  startElement event has already been reserved by the ElementStackKeeper.
 *  Meanwhile, an ElementCollector needs to be created before the
 *  SAXEventKeeper receives that startElement event.
 *  So for the SAXEventKeeper, it needs to receive all missed key SAX
 *  events except that startElement event, then adds a new
 *  ElementCollector, then receives that startElement event.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    bool rc = false;

    if (!m_bIsSAXEventKeeperSticky && !m_bIsSAXEventKeeperConnected)
    {
        if ( m_nStatusOfSecurityComponents == UNINITIALIZED )
        {
            createXSecComponent();
        }

        if ( m_nStatusOfSecurityComponents == INITIALIZED )
        /*
         * if all security components are ready, chains on the SAXEventKeeper
         */
        {
            /*
             * disconnect the SAXEventKeeper with its current output handler,
             * to make sure no SAX event is forwarded during the connecting
             * phase.
             */
            m_xSAXEventKeeper->setNextHandler( NULL );

            cssu::Reference< cssxs::XDocumentHandler > xSEKHandler(m_xSAXEventKeeper, cssu::UNO_QUERY);

            /*
             * connects the previous document handler on the SAX chain
             */
            if ( m_xPreviousNodeOnSAXChain.is() )
            {
                if ( m_bIsPreviousNodeInitializable )
                {
                    cssu::Reference< cssl::XInitialization > xInitialization
                        (m_xPreviousNodeOnSAXChain, cssu::UNO_QUERY);

                    cssu::Sequence<cssu::Any> aArgs( 1 );
                    aArgs[0] <<= xSEKHandler;
                    xInitialization->initialize(aArgs);
                }
                else
                {
                    cssu::Reference< cssxs::XParser > xParser
                        (m_xPreviousNodeOnSAXChain, cssu::UNO_QUERY);
                    xParser->setDocumentHandler( xSEKHandler );
                }
            }

            /*
             * get missed key SAX events
             */
            if (m_xElementStackKeeper.is())
            {
                m_xElementStackKeeper->retrieve(xSEKHandler, bRetrievingLastEvent);

                /*
                 * now the ElementStackKeeper can stop its work, because the
                 * SAXEventKeeper is on the SAX chain, no SAX events will be
                 * missed.
                 */
                m_xElementStackKeeper->stop();
            }

            /*
             * connects the next document handler on the SAX chain
             */
            m_xSAXEventKeeper->setNextHandler( m_xNextNodeOnSAXChain );

            m_bIsSAXEventKeeperConnected = true;

            rc = true;
        }
    }

    return rc;
}

void XSecController::chainOff()
/****** XSecController/chainOff ***********************************************
 *
 *   NAME
 *  chainOff -- disconnects the SAXEventKeeper from the SAX chain.
 *
 *   SYNOPSIS
 *  chainOff( );
 *
 *   FUNCTION
 *  See NAME.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    if (!m_bIsSAXEventKeeperSticky )
    {
        if (m_bIsSAXEventKeeperConnected)
        {
            m_xSAXEventKeeper->setNextHandler( NULL );

            if ( m_xPreviousNodeOnSAXChain.is() )
            {
                if ( m_bIsPreviousNodeInitializable )
                {
                    cssu::Reference< cssl::XInitialization > xInitialization
                        (m_xPreviousNodeOnSAXChain, cssu::UNO_QUERY);

                    cssu::Sequence<cssu::Any> aArgs( 1 );
                    aArgs[0] <<= m_xNextNodeOnSAXChain;
                    xInitialization->initialize(aArgs);
                }
                else
                {
                    cssu::Reference< cssxs::XParser > xParser(m_xPreviousNodeOnSAXChain, cssu::UNO_QUERY);
                    xParser->setDocumentHandler( m_xNextNodeOnSAXChain );
                }
            }

            if (m_xElementStackKeeper.is())
            {
                /*
                 * start the ElementStackKeeper to reserve any possible
                 * missed key SAX events
                 */
                m_xElementStackKeeper->start();
            }

            m_bIsSAXEventKeeperConnected = false;
        }
    }
}

void XSecController::checkChainingStatus()
/****** XSecController/checkChainingStatus ************************************
 *
 *   NAME
 *  checkChainingStatus -- connects or disconnects the SAXEventKeeper
 *  according to the current situation.
 *
 *   SYNOPSIS
 *  checkChainingStatus( );
 *
 *   FUNCTION
 *  The SAXEventKeeper is chained into the SAX chain, when:
 *  1. some element is being collected, or
 *  2. the SAX event stream is blocking.
 *  Otherwise, chain off the SAXEventKeeper.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    if ( m_bIsCollectingElement || m_bIsBlocking )
    {
        chainOn(true);
    }
    else
    {
        chainOff();
    }
}

void XSecController::initializeSAXChain()
/****** XSecController/initializeSAXChain *************************************
 *
 *   NAME
 *  initializeSAXChain -- initializes the SAX chain according to the
 *  current setting.
 *
 *   SYNOPSIS
 *  initializeSAXChain( );
 *
 *   FUNCTION
 *  Initializes the SAX chain, if the SAXEventKeeper is asked to be always
 *  on the SAX chain, chains it on. Otherwise, starts the
 *  ElementStackKeeper to reserve key SAX events.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_bIsSAXEventKeeperConnected = false;
    m_bIsCollectingElement = false;
    m_bIsBlocking = false;

    if (m_xElementStackKeeper.is())
    {
        /*
         * starts the ElementStackKeeper
         */
        m_xElementStackKeeper->start();
    }

    chainOff();
}

cssu::Reference< com::sun::star::io::XInputStream >
    XSecController::getObjectInputStream( const rtl::OUString& objectURL )
/****** XSecController/getObjectInputStream ************************************
 *
 *   NAME
 *  getObjectInputStream -- get a XInputStream interface from a SvStorage
 *
 *   SYNOPSIS
 *  xInputStream = getObjectInputStream( objectURL );
 *
 *   FUNCTION
 *  See NAME.
 *
 *   INPUTS
 *  objectURL - the object uri
 *
 *   RESULT
 *  xInputStream - the XInputStream interface
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
        cssu::Reference< com::sun::star::io::XInputStream > xObjectInputStream;

    DBG_ASSERT( m_xUriBinding.is(), "Need XUriBinding!" );

    xObjectInputStream = m_xUriBinding->getUriBinding(objectURL);

    return xObjectInputStream;
}

/*
 * public methods
 */

sal_Int32 XSecController::getNewSecurityId(  )
{
    sal_Int32 nId = m_nNextSecurityId;
    m_nNextSecurityId++;
    return nId;
}

void XSecController::startMission(
    const cssu::Reference< cssxc::XUriBinding >& xUriBinding,
    const cssu::Reference< cssxc::XXMLSecurityContext >& xSecurityContext )
/****** XSecController/startMission *******************************************
 *
 *   NAME
 *  startMission -- starts a new security mission.
 *
 *   SYNOPSIS
 *  startMission( xUriBinding, xSecurityContect );
 *
 *   FUNCTION
 *  get ready for a new mission.
 *
 *   INPUTS
 *  xUriBinding       - the Uri binding that provide maps between uris and
 *                          XInputStreams
 *  xSecurityContext  - the security context component which can provide
 *                      cryptoken
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_xUriBinding = xUriBinding;

    m_nStatusOfSecurityComponents = UNINITIALIZED;
    m_xSecurityContext = xSecurityContext;
    m_pErrorMessage = NULL;

    m_vInternalSignatureInformations.clear();

    m_bVerifyCurrentSignature = false;
}

void XSecController::setSAXChainConnector(
    const cssu::Reference< cssl::XInitialization >& xInitialization,
    const cssu::Reference< cssxs::XDocumentHandler >& xDocumentHandler,
    const cssu::Reference< cssxc::sax::XElementStackKeeper >& xElementStackKeeper)
/****** XSecController/setSAXChainConnector ***********************************
 *
 *   NAME
 *  setSAXChainConnector -- configures the components which will
 *  collaborate with the SAXEventKeeper on the SAX chain.
 *
 *   SYNOPSIS
 *  setSAXChainConnector( xInitialization,
 *                        xDocumentHandler,
 *                        xElementStackKeeper );
 *
 *   FUNCTION
 *  See NAME.
 *
 *   INPUTS
 *  xInitialization     - the previous node on the SAX chain
 *  xDocumentHandler    - the next node on the SAX chain
 *  xElementStackKeeper - the ElementStackKeeper component which reserves
 *                        missed key SAX events for the SAXEventKeeper
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_bIsPreviousNodeInitializable = true;
    m_xPreviousNodeOnSAXChain = xInitialization;
    m_xNextNodeOnSAXChain = xDocumentHandler;
    m_xElementStackKeeper = xElementStackKeeper;

    initializeSAXChain( );
}

void XSecController::clearSAXChainConnector()
/****** XSecController/clearSAXChainConnector *********************************
 *
 *   NAME
 *  clearSAXChainConnector -- resets the collaborating components.
 *
 *   SYNOPSIS
 *  clearSAXChainConnector( );
 *
 *   FUNCTION
 *  See NAME.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    /*
     * before reseting, if the ElementStackKeeper has kept something, then
     * those kept key SAX events must be transferred to the SAXEventKeeper
     * first. This is to promise the next node to the SAXEventKeeper on the
     * SAX chain always receives a complete document.
     */
    if (m_xElementStackKeeper.is() && m_xSAXEventKeeper.is())
    {
        cssu::Reference< cssxs::XDocumentHandler > xSEKHandler(m_xSAXEventKeeper, cssu::UNO_QUERY);
        m_xElementStackKeeper->retrieve(xSEKHandler, sal_True);
    }

    chainOff();

    m_xPreviousNodeOnSAXChain = NULL;
    m_xNextNodeOnSAXChain = NULL;
    m_xElementStackKeeper = NULL;
}

void XSecController::endMission()
/****** XSecController/endMission *********************************************
 *
 *   NAME
 *  endMission -- forces to end all missions
 *
 *   SYNOPSIS
 *  endMission( );
 *
 *   FUNCTION
 *  Deletes all signature information and forces all missions to an end.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    sal_Int32 size = m_vInternalSignatureInformations.size();

    for (int i=0; i<size; ++i)
    {
        if ( m_nStatusOfSecurityComponents == INITIALIZED )
        /*
         * ResolvedListener only exist when the security components are created.
         */
        {
            cssu::Reference< cssxc::sax::XMissionTaker > xMissionTaker
                ( m_vInternalSignatureInformations[i].xReferenceResolvedListener, cssu::UNO_QUERY );

            /*
             * askes the SignatureCreator/SignatureVerifier to release
             * all resouces it uses.
             */
            xMissionTaker->endMission();
        }
    }

    m_xUriBinding = NULL;
    m_xSecurityContext = NULL;

    /*
     * free the status change listener reference to this object
     */
    if (m_xSAXEventKeeper.is())
    {
        cssu::Reference<cssxc::sax::XSAXEventKeeperStatusChangeBroadcaster>
            xSAXEventKeeperStatusChangeBroadcaster(m_xSAXEventKeeper, cssu::UNO_QUERY);
        xSAXEventKeeperStatusChangeBroadcaster
            ->addSAXEventKeeperStatusChangeListener( NULL );
    }
}

void XSecController::exportSignature(
    const cssu::Reference<cssxs::XDocumentHandler>& xDocumentHandler,
    const SignatureInformation& signatureInfo )
/****** XSecController/exportSignature ****************************************
 *
 *   NAME
 *  exportSignature -- export a signature structure to an XDocumentHandler
 *
 *   SYNOPSIS
 *  exportSignature( xDocumentHandler, signatureInfo);
 *
 *   FUNCTION
 *  see NAME.
 *
 *   INPUTS
 *  xDocumentHandler    - the document handler to receive the signature
 *  signatureInfo       - signature to be exported
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    /*
     * defines all element tags in Signature element.
     */
    rtl::OUString tag_Signature(TAG_SIGNATURE);
    rtl::OUString tag_SignedInfo(TAG_SIGNEDINFO);
    rtl::OUString tag_CanonicalizationMethod(TAG_CANONICALIZATIONMETHOD);
    rtl::OUString tag_SignatureMethod(TAG_SIGNATUREMETHOD);
    rtl::OUString tag_Reference(TAG_REFERENCE);
    rtl::OUString tag_Transforms(TAG_TRANSFORMS);
    rtl::OUString tag_Transform(TAG_TRANSFORM);
    rtl::OUString tag_DigestMethod(TAG_DIGESTMETHOD);
    rtl::OUString tag_DigestValue(TAG_DIGESTVALUE);
    rtl::OUString tag_SignatureValue(TAG_SIGNATUREVALUE);
    rtl::OUString tag_KeyInfo(TAG_KEYINFO);
    rtl::OUString tag_X509Data(TAG_X509DATA);
    rtl::OUString tag_X509IssuerSerial(TAG_X509ISSUERSERIAL);
    rtl::OUString tag_X509IssuerName(TAG_X509ISSUERNAME);
    rtl::OUString tag_X509SerialNumber(TAG_X509SERIALNUMBER);
    rtl::OUString tag_X509Certificate(TAG_X509CERTIFICATE);
    rtl::OUString tag_Object(TAG_OBJECT);
    rtl::OUString tag_SignatureProperties(TAG_SIGNATUREPROPERTIES);
    rtl::OUString tag_SignatureProperty(TAG_SIGNATUREPROPERTY);
    rtl::OUString tag_Date(TAG_DATE);

    const SignatureReferenceInformations& vReferenceInfors = signatureInfo.vSignatureReferenceInfors;
    SvXMLAttributeList *pAttributeList;

    /*
     * Write Signature element
     */
    pAttributeList = new SvXMLAttributeList();
    pAttributeList->AddAttribute(
        rtl::OUString(ATTR_XMLNS),
        rtl::OUString(NS_XMLDSIG));

    if (!signatureInfo.ouSignatureId.isEmpty())
    {
        pAttributeList->AddAttribute(
            rtl::OUString(ATTR_ID),
            rtl::OUString(signatureInfo.ouSignatureId));
    }

    xDocumentHandler->startElement( tag_Signature, cssu::Reference< cssxs::XAttributeList > (pAttributeList));
    {
        /* Write SignedInfo element */
        xDocumentHandler->startElement(
            tag_SignedInfo,
            cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
        {
            /* Write CanonicalizationMethod element */
            pAttributeList = new SvXMLAttributeList();
            pAttributeList->AddAttribute(
                rtl::OUString(ATTR_ALGORITHM),
                rtl::OUString(ALGO_C14N));
            xDocumentHandler->startElement( tag_CanonicalizationMethod, cssu::Reference< cssxs::XAttributeList > (pAttributeList) );
            xDocumentHandler->endElement( tag_CanonicalizationMethod );

            /* Write SignatureMethod element */
            pAttributeList = new SvXMLAttributeList();
            pAttributeList->AddAttribute(
                rtl::OUString(ATTR_ALGORITHM),
                rtl::OUString(ALGO_RSASHA1));
            xDocumentHandler->startElement( tag_SignatureMethod, cssu::Reference< cssxs::XAttributeList > (pAttributeList) );
            xDocumentHandler->endElement( tag_SignatureMethod );

            /* Write Reference element */
            int j;
            int refNum = vReferenceInfors.size();

            for(j=0; j<refNum; ++j)
            {
                const SignatureReferenceInformation& refInfor = vReferenceInfors[j];

                pAttributeList = new SvXMLAttributeList();
                if ( refInfor.nType != TYPE_SAMEDOCUMENT_REFERENCE )
                /*
                 * stream reference
                 */
                {
                    pAttributeList->AddAttribute(
                        rtl::OUString(ATTR_URI),
                        refInfor.ouURI);
                }
                else
                /*
                 * same-document reference
                 */
                {
                    pAttributeList->AddAttribute(
                        rtl::OUString(ATTR_URI),
                        rtl::OUString(CHAR_FRAGMENT)+refInfor.ouURI);
                }

                xDocumentHandler->startElement( tag_Reference, cssu::Reference< cssxs::XAttributeList > (pAttributeList) );
                {
                    /* Write Transforms element */
                    if (refInfor.nType == TYPE_XMLSTREAM_REFERENCE)
                    /*
                     * xml stream, so c14n transform is needed
                     */
                    {
                        xDocumentHandler->startElement(
                            tag_Transforms,
                            cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
                        {
                            pAttributeList = new SvXMLAttributeList();
                            pAttributeList->AddAttribute(
                                rtl::OUString(ATTR_ALGORITHM),
                                rtl::OUString(ALGO_C14N));
                            xDocumentHandler->startElement(
                                tag_Transform,
                                cssu::Reference< cssxs::XAttributeList > (pAttributeList) );
                            xDocumentHandler->endElement( tag_Transform );
                        }
                        xDocumentHandler->endElement( tag_Transforms );
                    }

                    /* Write DigestMethod element */
                    pAttributeList = new SvXMLAttributeList();
                    pAttributeList->AddAttribute(
                        rtl::OUString(ATTR_ALGORITHM),
                        rtl::OUString(ALGO_XMLDSIGSHA1));
                    xDocumentHandler->startElement(
                        tag_DigestMethod,
                        cssu::Reference< cssxs::XAttributeList > (pAttributeList) );
                    xDocumentHandler->endElement( tag_DigestMethod );

                    /* Write DigestValue element */
                    xDocumentHandler->startElement(
                        tag_DigestValue,
                        cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
                    xDocumentHandler->characters( refInfor.ouDigestValue );
                    xDocumentHandler->endElement( tag_DigestValue );
                }
                xDocumentHandler->endElement( tag_Reference );
            }
        }
        xDocumentHandler->endElement( tag_SignedInfo );

        /* Write SignatureValue element */
        xDocumentHandler->startElement(
            tag_SignatureValue,
            cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
        xDocumentHandler->characters( signatureInfo.ouSignatureValue );
        xDocumentHandler->endElement( tag_SignatureValue );

        /* Write KeyInfo element */
        xDocumentHandler->startElement(
            tag_KeyInfo,
            cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
        {
            /* Write X509Data element */
            xDocumentHandler->startElement(
                tag_X509Data,
                cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
            {
                /* Write X509IssuerSerial element */
                xDocumentHandler->startElement(
                    tag_X509IssuerSerial,
                    cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
                {
                    /* Write X509IssuerName element */
                    xDocumentHandler->startElement(
                        tag_X509IssuerName,
                        cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
                    xDocumentHandler->characters( signatureInfo.ouX509IssuerName );
                    xDocumentHandler->endElement( tag_X509IssuerName );

                    /* Write X509SerialNumber element */
                    xDocumentHandler->startElement(
                        tag_X509SerialNumber,
                        cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
                    xDocumentHandler->characters( signatureInfo.ouX509SerialNumber );
                    xDocumentHandler->endElement( tag_X509SerialNumber );
                }
                xDocumentHandler->endElement( tag_X509IssuerSerial );

                /* Write X509Certificate element */
                if (!signatureInfo.ouX509Certificate.isEmpty())
                {
                    xDocumentHandler->startElement(
                        tag_X509Certificate,
                        cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
                    xDocumentHandler->characters( signatureInfo.ouX509Certificate );
                    xDocumentHandler->endElement( tag_X509Certificate );
                }
            }
            xDocumentHandler->endElement( tag_X509Data );
        }
        xDocumentHandler->endElement( tag_KeyInfo );

        /* Write Object element */
        xDocumentHandler->startElement(
            tag_Object,
            cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
        {
            /* Write SignatureProperties element */
            xDocumentHandler->startElement(
                tag_SignatureProperties,
                cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
            {
                /* Write SignatureProperty element */
                pAttributeList = new SvXMLAttributeList();
                pAttributeList->AddAttribute(
                    rtl::OUString(ATTR_ID),
                    signatureInfo.ouPropertyId);
                pAttributeList->AddAttribute(
                    rtl::OUString(ATTR_TARGET),
                    rtl::OUString(CHAR_FRAGMENT)+signatureInfo.ouSignatureId);
                xDocumentHandler->startElement(
                    tag_SignatureProperty,
                    cssu::Reference< cssxs::XAttributeList > (pAttributeList));
                {
                    /* Write timestamp element */

                    pAttributeList = new SvXMLAttributeList();
                    pAttributeList->AddAttribute(
                        rtl::OUString(ATTR_XMLNS)
                            +rtl::OUString(":")
                            +rtl::OUString(NSTAG_DC),
                        rtl::OUString(NS_DC));

                    xDocumentHandler->startElement(
                        rtl::OUString(NSTAG_DC)
                            +rtl::OUString(":")
                            +tag_Date,
                        cssu::Reference< cssxs::XAttributeList > (pAttributeList));

                    ::rtl::OUStringBuffer buffer;
                    //If the xml signature was already contained in the document,
                    //then we use the original date and time string, rather then the
                    //converted one. When the original string is converted to the DateTime
                    //structure then information may be lost because it only holds a fractional
                    //of a second with a accuracy of one hundredth of second. If the string contains
                    //milli seconds (document was signed by an application other than OOo)
                    //and the converted time is written back, then the string looks different
                    //and the signature is broken.
                    if (!signatureInfo.ouDateTime.isEmpty())
                        buffer = signatureInfo.ouDateTime;
                    else
                        convertDateTime( buffer, signatureInfo.stDateTime );
                    xDocumentHandler->characters( buffer.makeStringAndClear() );

                    xDocumentHandler->endElement(
                        rtl::OUString(NSTAG_DC)
                            +rtl::OUString(":")
                            +tag_Date);
                }
                xDocumentHandler->endElement( tag_SignatureProperty );
            }
            xDocumentHandler->endElement( tag_SignatureProperties );
        }
        xDocumentHandler->endElement( tag_Object );
    }
    xDocumentHandler->endElement( tag_Signature );
}

SignatureInformation XSecController::getSignatureInformation( sal_Int32 nSecurityId ) const
{
    SignatureInformation aInf( 0 );
    int nIndex = findSignatureInfor(nSecurityId);
    DBG_ASSERT( nIndex != -1, "getSignatureInformation - SecurityId is invalid!" );
    if ( nIndex != -1)
    {
        aInf = m_vInternalSignatureInformations[nIndex].signatureInfor;
    }
    return aInf;
}

SignatureInformations XSecController::getSignatureInformations() const
{
    SignatureInformations vInfors;
    int sigNum = m_vInternalSignatureInformations.size();

    for (int i=0; i<sigNum; ++i)
    {
        SignatureInformation si = m_vInternalSignatureInformations[i].signatureInfor;
        vInfors.push_back(si);
    }

    return vInfors;
}

/*
 * XSecurityController
 *
 * no methods
 */

/*
 * XFastPropertySet
 */

/*
 * XSAXEventKeeperStatusChangeListener
 */

void SAL_CALL XSecController::blockingStatusChanged( sal_Bool isBlocking )
    throw (cssu::RuntimeException)
{
    this->m_bIsBlocking = isBlocking;
    checkChainingStatus();
}

void SAL_CALL XSecController::collectionStatusChanged(
    sal_Bool isInsideCollectedElement )
    throw (cssu::RuntimeException)
{
    this->m_bIsCollectingElement = isInsideCollectedElement;
    checkChainingStatus();
}

void SAL_CALL XSecController::bufferStatusChanged( sal_Bool /*isBufferEmpty*/)
    throw (cssu::RuntimeException)
{

}

/*
 * XSignatureCreationResultListener
 */
void SAL_CALL XSecController::signatureCreated( sal_Int32 securityId, com::sun::star::xml::crypto::SecurityOperationStatus nResult )
        throw (com::sun::star::uno::RuntimeException)
{
    int index = findSignatureInfor(securityId);
    DBG_ASSERT( index != -1, "Signature Not Found!" );

    SignatureInformation& signatureInfor = m_vInternalSignatureInformations[index].signatureInfor;

    signatureInfor.nStatus = nResult;
}

/*
 * XSignatureVerifyResultListener
 */
void SAL_CALL XSecController::signatureVerified( sal_Int32 securityId, com::sun::star::xml::crypto::SecurityOperationStatus nResult )
        throw (com::sun::star::uno::RuntimeException)
{
    int index = findSignatureInfor(securityId);
    DBG_ASSERT( index != -1, "Signature Not Found!" );

    SignatureInformation& signatureInfor = m_vInternalSignatureInformations[index].signatureInfor;

    signatureInfor.nStatus = nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
