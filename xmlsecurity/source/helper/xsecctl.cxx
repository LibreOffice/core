/*************************************************************************
 *
 *  $RCSfile: xsecctl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-15 08:12:08 $
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

#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/SignatureVerifyResult.hpp>
#include <com/sun/star/xml/crypto/sax/SignatureCreationResult.hpp>

#include <xmloff/attrlist.hxx>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxs = com::sun::star::xml::sax;
namespace cssxw = com::sun::star::xml::wrapper;
namespace cssb = com::sun::star::beans;

/* bridge component names */
#define XMLSIGNATURE_COMPONENT "com.sun.star.xml.crypto.XMLSignature"
#define XMLDOCUMENTWRAPPER_COMPONENT "com.sun.star.xml.wrapper.XMLDocumentWrapper"

/* xml security framework components */
#define SAXEVENTKEEPER_COMPONENT "com.sun.star.xml.crypto.sax.SAXEventKeeper"

/* string for package protocol */
#define PACKAGEPROTOCOL "vnd.sun.star.Package:"

XSecController::XSecController( )
    :m_nNextSecurityId(1),
      m_bIsSAXEventKeeperSticky(false),
      m_bIsSAXEventKeeperConnected(false),
     m_nStatusOfSecurityComponents(UNINITIALIZED),
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
int XSecController::findSignatureInfor( sal_Int32 nSecurityId)
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
 *   HISTORY
 *  08.05.2004 -    implemented
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
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    rtl::OUString sSAXEventKeeper(rtl::OUString::createFromAscii( SAXEVENTKEEPER_COMPONENT ));
    rtl::OUString sXMLSignature(rtl::OUString::createFromAscii( XMLSIGNATURE_COMPONENT ));
    rtl::OUString sXMLDocument(rtl::OUString::createFromAscii( XMLDOCUMENTWRAPPER_COMPONENT ));

    /*
     * marks all security components are not available.
     */
    m_nStatusOfSecurityComponents = FAILTOINITIALIZED;
    m_xXMLSignature = NULL;
    m_xXMLDocumentWrapper = NULL;
    m_xSAXEventKeeper = NULL;

    m_xXMLSignature = cssu::Reference< cssxc::XXMLSignature >(
        mxMSF->createInstance( sXMLSignature ),
        cssu::UNO_QUERY );

    bool bSuccess = (0!=m_xXMLSignature.is());
    if ( bSuccess )
    /*
     * XMLSignature created successfully.
     */
    {
        m_xXMLDocumentWrapper = cssu::Reference< cssxw::XXMLDocumentWrapper >(
            mxMSF->createInstance( sXMLDocument ),
            cssu::UNO_QUERY );
    }

    bSuccess &= (0!=m_xXMLDocumentWrapper.is());
    if ( bSuccess )
    /*
     * XMLDocumentWrapper created successfully.
     */
    {
        m_xSAXEventKeeper = cssu::Reference< cssxc::sax::XSecuritySAXEventKeeper >(
            mxMSF->createInstance( sSAXEventKeeper ),
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
            ->addXSAXEventKeeperStatusChangeListener( xStatusChangeListener );

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
 *   HISTORY
 *  05.01.2004 -    implemented
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
 *   HISTORY
 *  05.01.2004 -    implemented
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
 *   HISTORY
 *  05.01.2004 -    implemented
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
 *   HISTORY
 *  05.01.2004 -    implemented
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
 *   HISTORY
 *  15.04.2004 -    implemented
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

#if 0
sal_Int32 XSecController::getFastPropertyIndex(sal_Int32 nHandle) const
/****** XSecController/getFastPropertyIndex ***********************************
 *
 *   NAME
 *  getFastPropertyIndex -- gets the index of a particular fast property
 *
 *   SYNOPSIS
 *  nIndex = getFastPropertyIndex( nHandle );
 *
 *   FUNCTION
 *  See NAME.
 *
 *   INPUTS
 *  nHandle - the key for the fast property
 *
 *   RESULT
 *  nIndex - the index of the fast property, or -1
 *           if the key is not found.
 *
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    std::vector< sal_Int32 >::const_iterator ii = m_vFastPropertyIndexs.begin();
    sal_Int32 nIndex = 0;

    bool bFound = false;

    for( ; ii != m_vFastPropertyIndexs.end(); ++ii,++nIndex )
    {
        if ( nHandle == (*ii))
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        nIndex = -1;
    }

    return nIndex;
}
#endif

/*
 * public methods
 */

void XSecController::setFactory( const cssu::Reference<cssl::XMultiServiceFactory>& rxMSF)
/****** XSecController/setFactory *********************************************
 *
 *   NAME
 *  setFactory -- configures the service factory component.
 *
 *   SYNOPSIS
 *  setFactory( rxMSF );
 *
 *   FUNCTION
 *  See NAME.
 *
 *   INPUTS
 *  rxMSF - the service factory component
 *
 *   RESULT
 *  empty
 *
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    mxMSF = rxMSF;
}

#if 0
void XSecController::setSignatureCreationResultListener(
    const cssu::Reference< cssxc::sax::XSignatureCreationResultListener >& xSignatureCreationResultListener)
{
    m_xSignatureCreationResultListener = xSignatureCreationResultListener;
}

void XSecController::setSignatureVerifyResultListener(
    const cssu::Reference< cssxc::sax::XSignatureVerifyResultListener >& xSignatureVerifyResultListener)
{
    m_xSignatureVerifyResultListener = xSignatureVerifyResultListener;
}
#endif

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
 *   HISTORY
 *  05.01.2004 -    implemented
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
 *   HISTORY
 *  05.01.2004 -    implemented
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

void XSecController::setSAXChainConnector(
    const cssu::Reference< cssxs::XParser >& xParser,
    const cssu::Reference< cssxs::XDocumentHandler >& xDocumentHandler,
    const cssu::Reference< cssxc::sax::XElementStackKeeper >& xElementStackKeeper)
/****** XSecController/setSAXChainConnector ***********************************
 *
 *   NAME
 *  setSAXChainConnector -- configures the components which will
 *  collaborate with the SAXEventKeeper on the SAX chain.
 *
 *   SYNOPSIS
 *  setSAXChainConnector( xParser, xDocumentHandler, xElementStackKeeper );
 *
 *   FUNCTION
 *  See NAME.
 *
 *   INPUTS
 *  xParser             - the previous node on the SAX chain
 *  xDocumentHandler    - the next node on the SAX chain
 *  xElementStackKeeper -the ElementStackKeeper component which reserves
 *                        missed key SAX events for the SAXEventKeeper
 *
 *   RESULT
 *  empty
 *
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_bIsPreviousNodeInitializable = false;
    m_xPreviousNodeOnSAXChain = xParser;
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
 *   HISTORY
 *  05.01.2004 -    implemented
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
 *   HISTORY
 *  05.01.2004 -    implemented
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
            ->addXSAXEventKeeperStatusChangeListener( NULL );
    }
}

const char* XSecController::getErrorMessage()
/****** XSecController/getErrorMessage ****************************************
 *
 *   NAME
 *  getErrorMessage -- get the last error message
 *
 *   SYNOPSIS
 *  pErrorMessage = getErrorMessage( );
 *
 *   FUNCTION
 *  see NAME.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   HISTORY
 *  22.04.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    return m_pErrorMessage;
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
 *   HISTORY
 *  26.05.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    /*
     * defines all element tags in Signature element.
     */
    rtl::OUString tag_Signature(RTL_CONSTASCII_USTRINGPARAM(TAG_SIGNATURE));
        rtl::OUString tag_SignedInfo(RTL_CONSTASCII_USTRINGPARAM(TAG_SIGNEDINFO));
            rtl::OUString tag_CanonicalizationMethod(RTL_CONSTASCII_USTRINGPARAM(TAG_CANONICALIZATIONMETHOD));
            rtl::OUString tag_SignatureMethod(RTL_CONSTASCII_USTRINGPARAM(TAG_SIGNATUREMETHOD));
            rtl::OUString tag_Reference(RTL_CONSTASCII_USTRINGPARAM(TAG_REFERENCE));
                rtl::OUString tag_Transforms(RTL_CONSTASCII_USTRINGPARAM(TAG_TRANSFORMS));
                    rtl::OUString tag_Transform(RTL_CONSTASCII_USTRINGPARAM(TAG_TRANSFORM));
                rtl::OUString tag_DigestMethod(RTL_CONSTASCII_USTRINGPARAM(TAG_DIGESTMETHOD));
                rtl::OUString tag_DigestValue(RTL_CONSTASCII_USTRINGPARAM(TAG_DIGESTVALUE));
        rtl::OUString tag_SignatureValue(RTL_CONSTASCII_USTRINGPARAM(TAG_SIGNATUREVALUE));
        rtl::OUString tag_KeyInfo(RTL_CONSTASCII_USTRINGPARAM(TAG_KEYINFO));
            rtl::OUString tag_X509Data(RTL_CONSTASCII_USTRINGPARAM(TAG_X509DATA));
                rtl::OUString tag_X509IssuerSerial(RTL_CONSTASCII_USTRINGPARAM(TAG_X509ISSUERSERIAL));
                    rtl::OUString tag_X509IssuerName(RTL_CONSTASCII_USTRINGPARAM(TAG_X509ISSUERNAME));
                    rtl::OUString tag_X509SerialNumber(RTL_CONSTASCII_USTRINGPARAM(TAG_X509SERIALNUMBER));
                    rtl::OUString tag_X509Certificate(RTL_CONSTASCII_USTRINGPARAM(TAG_X509CERTIFICATE));

        rtl::OUString tag_Object(RTL_CONSTASCII_USTRINGPARAM(TAG_OBJECT));
            rtl::OUString tag_SignatureProperties(RTL_CONSTASCII_USTRINGPARAM(TAG_SIGNATUREPROPERTIES));
                rtl::OUString tag_SignatureProperty(RTL_CONSTASCII_USTRINGPARAM(TAG_SIGNATUREPROPERTY));
                    rtl::OUString tag_Timestamp(RTL_CONSTASCII_USTRINGPARAM(TAG_TIMESTAMP));
                        rtl::OUString tag_Date(RTL_CONSTASCII_USTRINGPARAM(TAG_DATE));
                        rtl::OUString tag_Time(RTL_CONSTASCII_USTRINGPARAM(TAG_TIME));

    const SignatureReferenceInformations& vReferenceInfors = signatureInfo.vSignatureReferenceInfors;
    SvXMLAttributeList *pAttributeList;

    /*
     * Write Signature element
     */
    pAttributeList = new SvXMLAttributeList();
    pAttributeList->AddAttribute(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_XMLNS)),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(NS_XMLDSIG)));

    if (signatureInfo.ouSignatureId.getLength()>0)
    {
        pAttributeList->AddAttribute(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_ID)),
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
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_ALGORITHM)),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ALGO_C14N)));
            xDocumentHandler->startElement( tag_CanonicalizationMethod, cssu::Reference< cssxs::XAttributeList > (pAttributeList) );
            xDocumentHandler->endElement( tag_CanonicalizationMethod );

            /* Write SignatureMethod element */
            pAttributeList = new SvXMLAttributeList();
            pAttributeList->AddAttribute(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_ALGORITHM)),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ALGO_RSASHA1)));
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
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_URI)),
                        refInfor.ouURI);
                }
                else
                /*
                 * same-document reference
                 */
                {
                    pAttributeList->AddAttribute(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_URI)),
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CHAR_FRAGMENT))+refInfor.ouURI);
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
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_ALGORITHM)),
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ALGO_C14N)));
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
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_ALGORITHM)),
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ALGO_XMLDSIGSHA1)));
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
                if (signatureInfo.ouX509Certificate.getLength()>0)
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

        if (signatureInfo.ouDate.getLength()>0 ||signatureInfo.ouTime.getLength()>0)
        {
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
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_ID)),
                        signatureInfo.ouPropertyId);
                    pAttributeList->AddAttribute(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_TARGET)),
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CHAR_FRAGMENT))+signatureInfo.ouSignatureId);
                    xDocumentHandler->startElement(
                        tag_SignatureProperty,
                        cssu::Reference< cssxs::XAttributeList > (pAttributeList));
                    {
                        /* Write timestamp element */
                        pAttributeList = new SvXMLAttributeList();
                        pAttributeList->AddAttribute(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ATTR_XMLNS)),
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(NS_DATETIME)));
                        xDocumentHandler->startElement(
                            tag_Timestamp,
                            cssu::Reference< cssxs::XAttributeList > (pAttributeList));
                        {
                            /* Write date element */
                            xDocumentHandler->startElement(
                                tag_Date,
                                cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
                            xDocumentHandler->characters( signatureInfo.ouDate );
                            xDocumentHandler->endElement( tag_Date );

                            /* Write time element */
                            xDocumentHandler->startElement(
                                tag_Time,
                                cssu::Reference< cssxs::XAttributeList > (new SvXMLAttributeList()));
                            xDocumentHandler->characters( signatureInfo.ouTime );
                            xDocumentHandler->endElement( tag_Time );
                        }
                        xDocumentHandler->endElement( tag_Timestamp );
                    }
                    xDocumentHandler->endElement( tag_SignatureProperty );
                }
                xDocumentHandler->endElement( tag_SignatureProperties );
            }
            xDocumentHandler->endElement( tag_Object );
        }
    }
    xDocumentHandler->endElement( tag_Signature );
}

bool XSecController::getSignatureInfor( sal_Int32 nSecurityId, SignatureInformation& signatureInfor )
{
    int index = findSignatureInfor(nSecurityId);
    if (index == -1)
    {
        return false;
    }
    else
    {
        signatureInfor = m_vInternalSignatureInformations[index].signatureInfor;
        return true;
    }
}

SignatureInformations XSecController::getSignatureInformations()
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
void SAL_CALL XSecController::setFastPropertyValue(
    sal_Int32 nHandle,
    const cssu::Any& aValue )
    throw ( cssb::UnknownPropertyException,
        cssb::PropertyVetoException,
        cssl::IllegalArgumentException,
        cssl::WrappedTargetException,
        cssu::RuntimeException)
{
    sal_Int32 nIndex = getFastPropertyIndex(nHandle);
    if (nIndex == -1)
    {
        m_vFastPropertyIndexs.push_back( nHandle );
        m_vFastPropertyValues.push_back( aValue );
    }
    else
    {
        m_vFastPropertyValues[nIndex] = aValue;
    }
}

cssu::Any SAL_CALL XSecController::getFastPropertyValue(
    sal_Int32 nHandle )
    throw (
        cssb::UnknownPropertyException,
        cssl::WrappedTargetException,
        cssu::RuntimeException)
{
    cssu::Any aValue;

    sal_Int32 nIndex = getFastPropertyIndex(nHandle);
    if (nIndex != -1)
    {
        aValue = m_vFastPropertyValues[nIndex];
    }

    return aValue;
}
*/

/*
 * XSAXEventKeeperStatusChangeListener
 */

void SAL_CALL XSecController::blockingStatusChanged( sal_Bool isBlocking )
    throw (cssu::RuntimeException)
{
    /*
    showMessageBox( rtl::OUString::createFromAscii((isBlocking?
                        "Blocking Status => TRUE":
                        "Blocking Status => FALSE")),
            rtl::OUString::createFromAscii("SAXEventKeeper Status"));
    */

    this->m_bIsBlocking = isBlocking;
    checkChainingStatus();
}

void SAL_CALL XSecController::collectionStatusChanged(
    sal_Bool isInsideCollectedElement )
    throw (cssu::RuntimeException)
{
    /*
    showMessageBox( rtl::OUString::createFromAscii((isInsideCollectedElement?
                        "Collection Status => TRUE":
                        "Collection Status => FALSE")),
            rtl::OUString::createFromAscii("SAXEventKeeper Status"));
    */

    this->m_bIsCollectingElement = isInsideCollectedElement;
    checkChainingStatus();
}

void SAL_CALL XSecController::bufferStatusChanged( sal_Bool isBufferEmpty )
    throw (cssu::RuntimeException)
{
    /*
    showMessageBox( rtl::OUString::createFromAscii((isBufferEmpty?
                        "Buffer Empty => TRUE":
                        "Buffer Empty => FALSE")),
            rtl::OUString::createFromAscii("SAXEventKeeper Status"));
    */
}

/*
 * XSignatureCreationResultListener
 */
void SAL_CALL XSecController::signatureCreated( sal_Int32 securityId, com::sun::star::xml::crypto::sax::SignatureCreationResult nResult )
        throw (com::sun::star::uno::RuntimeException)
{
    int index = findSignatureInfor(securityId);
    DBG_ASSERT( index != -1, "Signature Not Found!" );

    SignatureInformation& signatureInfor = m_vInternalSignatureInformations[index].signatureInfor;

    if (nResult == cssxc::sax::SignatureCreationResult_CREATIONSUCCEED)
    {
        signatureInfor.nStatus = STATUS_CREATION_SUCCEED;
    }
    else
    {
        signatureInfor.nStatus = STATUS_CREATION_FAIL;
    }
}

/*
 * XSignatureVerifyResultListener
 */
void SAL_CALL XSecController::signatureVerified( sal_Int32 securityId, com::sun::star::xml::crypto::sax::SignatureVerifyResult nResult )
        throw (com::sun::star::uno::RuntimeException)
{
    int index = findSignatureInfor(securityId);
    DBG_ASSERT( index != -1, "Signature Not Found!" );

    SignatureInformation& signatureInfor = m_vInternalSignatureInformations[index].signatureInfor;

    if (nResult == cssxc::sax::SignatureVerifyResult_VERIFYSUCCEED)
    {
        signatureInfor.nStatus = STATUS_VERIFY_SUCCEED;
    }
    else
    {
        signatureInfor.nStatus = STATUS_VERIFY_FAIL;
    }
}
