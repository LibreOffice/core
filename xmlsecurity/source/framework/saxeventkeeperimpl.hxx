/*************************************************************************
 *
 *  $RCSfile: saxeventkeeperimpl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:22 $
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

#ifndef _SAXEVENTKEEPERIMPL_HXX
#define _SAXEVENTKEEPERIMPL_HXX

#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XSECURITYSAXEVENTKEEPER_HPP_
#include <com/sun/star/xml/crypto/sax/XSecuritySAXEventKeeper.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XREFERENCERESOLVEDBROADCASTER_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XSAXEVENTKEEPERSTATUSCHANGEBROADCASTER_HPP_
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XSAXEVENTKEEPERSTATUSCHANGELISTENER_HPP_
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CSAX_XCOMPRESSEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/csax/XCompressedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_WRAPPER_XXMLDOCUMENTWRAPPER_HPP_
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif

#include "buffernode.hxx"
#include "elementmark.hxx"
#include "elementcollector.hxx"

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

class SAXEventKeeperImpl : public cppu::WeakImplHelper6
<
    com::sun::star::xml::crypto::sax::XSecuritySAXEventKeeper,
    com::sun::star::xml::crypto::sax::XReferenceResolvedBroadcaster,
    com::sun::star::xml::crypto::sax::XSAXEventKeeperStatusChangeBroadcaster,
    com::sun::star::xml::sax::XDocumentHandler,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
/****** SAXEventKeeperImpl.hxx/CLASS SAXEventKeeperImpl ***********************
 *
 *   NAME
 *  SAXEventKeeperImpl -- SAX events buffer controller
 *
 *   FUNCTION
 *  Controls SAX events to be bufferred, and controls bufferred SAX events
 *  to be released.
 *
 *   HISTORY
 *  05.01.2004 -    Interface supported: XSecuritySAXEventKeeper,
 *                  XReferenceResolvedBroadcaster,
 *                  XSAXEventKeeperStatusChangeBroadcaster,
 *                  XDocumentHandler, XInitialization, XServiceInfo
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    /*
     * the XMLDocumentWrapper component which maintains all bufferred SAX
     * in DOM format.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLDocumentWrapper >
        m_xXMLDocument;

    /*
     * the document handler provided by the XMLDocumentWrapper component.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler > m_xDocumentHandler;

    /*
     * the compressed document handler provided by the XMLDocumentWrapper
     * component, the handler has more effient method definition that the
     * normal document handler.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::csax::XCompressedDocumentHandler >
        m_xCompressedDocumentHandler;

    /*
     * a listener which receives this SAXEventKeeper's status change
     * notification.
     * Based on the status changes, the listener can decide whether the
     * SAXEventKeeper should chain on/chain off the SAX chain, or whether
     * the SAXEventKeeper is useless any long.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >
        m_xSAXEventKeeperStatusChangeListener;

    /*
     * the root node of the BufferNode tree.
     * the BufferNode tree is used to keep track of all bufferred elements,
     * it has the same structure with the document which maintains those
     * elements physically.
     */
    BufferNode*  m_pRootBufferNode;

    /*
     * the current active BufferNode.
     * this is used to keep track the current location in the BufferNode tree,
     * the next generated BufferNode will become a child BufferNode of it.
     */
    BufferNode*  m_pCurrentBufferNode;

    /*
     * the next Id for a coming ElementMark.
     * the variable is increased by 1 when an new ElementMark is generated,
     * in this way, we can promise the Id of any ElementMark is unique.
     */
    sal_Int32    m_nNextElementMarkId;

    /*
     * maintains a collection of all ElementMarks.
     */
    std::vector< const ElementMark* > m_vElementMarkBuffers;

    /*
     * maintains a list of new ElementCollectors that will be created
     * on the element represented by the next incoming startElement SAX
     * event.
     * The reason that such the m_vNewElementCollectors is necessary
     * is: when an ElementCollector is asked to create, it can't be
     * created completely at once, because the BufferNode it will be
     * working on has not been created until the next startElement
     * SAX event comes.
     */
    std::vector< const ElementCollector* > m_vNewElementCollectors;

    /*
     * maintains the new Blocker that will be created
     * on the element represented by the next incoming startElement SAX
     * event.
     */
    ElementMark* m_pNewBlocker;

    /*
     * the document handler to which all received SAX events will be
     * forwarded.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler > m_xNextHandler;

    /*
     * the current BufferNode which prevents the SAX events to be
     * forwarded to the m_xNextHandler.
     */
    BufferNode*  m_pCurrentBlockingBufferNode;

    /*
     * maintains a list of ElementMark that has been asked to release.
     * Because during processing a request of releasing an ElementMark,
     * another releasing ElementMark request can be invoked. To avoid
     * reentering the same method, a such request only add that ElementMark
     * into this ElementMark list, then all ElementMarks will be processed in
     * order.
     */
    std::vector< sal_Int32 > m_vReleasedElementMarkBuffers;

    /*
     * a flag to indicate whether the ElementMark releasing process is runing.
     * When a releasing request comes, the assigned ElementMark is added to
     * the m_vReleasedElementMarkBuffers first, then this flag is checked.
     * If the ElementMark releasing process is not running, then call that
     * method.
     */
    bool     m_bIsReleasing;

    /*
     * a flag to indicate whether it is the "Forwarding" mode now.
     * A "Forwarding" mode means that all received SAX events are from the
     * XMLDocumentWrapper component, instead of up-stream component in the
     * SAX chain.
     * The difference between "Forwarding" mode and normal mode is that:
     * no SAX events need to be transferred to the XMLDocumentWrapper component
     * again even if a buffer request happens.
     */
    bool     m_bIsForwarding;

    void setCurrentBufferNode(BufferNode* pBufferNode);

    BufferNode* addNewElementMarkBuffers();

    ElementMark* findElementMarkBuffer(sal_Int32 nId) const;

    void removeElementMarkBuffer(sal_Int32 nId);

    rtl::OUString printBufferNode(
        BufferNode* pBufferNode, sal_Int32 nIndent) const;

    com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper > >
        collectChildWorkingElement(BufferNode* pBufferNode) const;

    void smashBufferNode(
        BufferNode* pBufferNode, bool bClearRoot) const;

    BufferNode* findNextBlockingBufferNode(
        BufferNode* pStartBufferNode) const;

    void diffuse(BufferNode* pBufferNode) const;

    void releaseElementMarkBuffer();

    void markElementMarkBuffer(sal_Int32 nId);

    sal_Int32 createElementCollector(
        sal_Int32 nSecurityId,
        com::sun::star::xml::crypto::sax::ElementMarkPriority nPriority,
        bool bModifyElement,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XReferenceResolvedListener>&
            xReferenceResolvedListener);

    sal_Int32 createBlocker(sal_Int32 nSecurityId);

public:
    SAXEventKeeperImpl();
    virtual ~SAXEventKeeperImpl();

    /* XSAXEventKeeper */
    virtual sal_Int32 SAL_CALL addElementCollector(  )
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeElementCollector( sal_Int32 id )
        throw (com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL addBlocker(  )
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeBlocker( sal_Int32 id )
        throw (com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isBlocking(  )
        throw (com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper > SAL_CALL
        getElement( sal_Int32 id )
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setElement(
        sal_Int32 id,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >&
            aElement )
        throw (com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler > SAL_CALL
        setNextHandler( const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XDocumentHandler >& xNewHandler )
        throw (com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL printBufferNodeTree()
        throw (com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper > SAL_CALL
        getCurrentBlockingNode()
        throw (com::sun::star::uno::RuntimeException);

    /* XSecuritySAXEventKeeper */
    virtual sal_Int32 SAL_CALL addSecurityElementCollector(
        com::sun::star::xml::crypto::sax::ElementMarkPriority priority,
        sal_Bool modifyElement )
        throw (com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL cloneElementCollector(
        sal_Int32 referenceId,
        com::sun::star::xml::crypto::sax::ElementMarkPriority priority )
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSecurityId( sal_Int32 id, sal_Int32 securityId )
        throw (com::sun::star::uno::RuntimeException);

    /* XReferenceResolvedBroadcaster */
    virtual void SAL_CALL addReferenceResolvedListener(
        sal_Int32 referenceId,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XReferenceResolvedListener >&
            listener )
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeReferenceResolvedListener(
        sal_Int32 referenceId,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XReferenceResolvedListener >&
            listener )
        throw (com::sun::star::uno::RuntimeException);

    /* XSAXEventKeeperStatusChangeBroadcaster */
    virtual void SAL_CALL addXSAXEventKeeperStatusChangeListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >&
            listener )
        throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeXSAXEventKeeperStatusChangeListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >&
            listener )
        throw (com::sun::star::uno::RuntimeException);

    /* XDocumentHandler */
    virtual void SAL_CALL startDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement(
        const rtl::OUString& aName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >&
        xAttribs )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement( const rtl::OUString& aName )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const rtl::OUString& aChars )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const rtl::OUString& aWhitespaces )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction(
        const rtl::OUString& aTarget, const rtl::OUString& aData )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XLocator >& xLocator )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    /* XInitialization */
    virtual void SAL_CALL initialize(
        const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    /* XServiceInfo */
    virtual rtl::OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException);
};

rtl::OUString SAXEventKeeperImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL SAXEventKeeperImpl_supportsService( const rtl::OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL SAXEventKeeperImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL SAXEventKeeperImpl_createInstance( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif



