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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_SAXEVENTKEEPERIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_SAXEVENTKEEPERIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XSecuritySAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeperStatusChangeListener.hpp>
#include <com/sun/star/xml/csax/XCompressedDocumentHandler.hpp>
#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>

#include "buffernode.hxx"
#include "elementmark.hxx"
#include "elementcollector.hxx"

#include <vector>

class SAXEventKeeperImpl : public cppu::WeakImplHelper
<
    css::xml::crypto::sax::XSecuritySAXEventKeeper,
    css::xml::crypto::sax::XReferenceResolvedBroadcaster,
    css::xml::crypto::sax::XSAXEventKeeperStatusChangeBroadcaster,
    css::xml::sax::XDocumentHandler,
    css::lang::XInitialization,
    css::lang::XServiceInfo
>
/****** SAXEventKeeperImpl.hxx/CLASS SAXEventKeeperImpl ***********************
 *
 *   NAME
 *  SAXEventKeeperImpl -- SAX events buffer controller
 *
 *   FUNCTION
 *  Controls SAX events to be bufferred, and controls bufferred SAX events
 *  to be released.
 ******************************************************************************/
{
private:
    /*
     * the XMLDocumentWrapper component which maintains all bufferred SAX
     * in DOM format.
     */
    css::uno::Reference< css::xml::wrapper::XXMLDocumentWrapper >
        m_xXMLDocument;

    /*
     * the document handler provided by the XMLDocumentWrapper component.
     */
    css::uno::Reference< css::xml::sax::XDocumentHandler > m_xDocumentHandler;

    /*
     * the compressed document handler provided by the XMLDocumentWrapper
     * component, the handler has more efficient method definition that the
     * normal document handler.
     */
    css::uno::Reference< css::xml::csax::XCompressedDocumentHandler >
        m_xCompressedDocumentHandler;

    /*
     * a listener which receives this SAXEventKeeper's status change
     * notification.
     * Based on the status changes, the listener can decide whether the
     * SAXEventKeeper should chain on/chain off the SAX chain, or whether
     * the SAXEventKeeper is useless any long.
     */
    css::uno::Reference< css::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >
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
    css::uno::Reference< css::xml::sax::XDocumentHandler > m_xNextHandler;

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
     * a flag to indicate whether the ElementMark releasing process is running.
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

    OUString printBufferNode(
        BufferNode* pBufferNode, sal_Int32 nIndent) const;

    static css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > >
        collectChildWorkingElement(BufferNode* pBufferNode);

    void smashBufferNode(
        BufferNode* pBufferNode, bool bClearRoot) const;

    static BufferNode* findNextBlockingBufferNode(
        BufferNode* pStartBufferNode);

    static void diffuse(BufferNode* pBufferNode);

    void releaseElementMarkBuffer();

    void markElementMarkBuffer(sal_Int32 nId);

    sal_Int32 createElementCollector(
        css::xml::crypto::sax::ElementMarkPriority nPriority,
        bool bModifyElement,
        const css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener>& xReferenceResolvedListener);

    sal_Int32 createBlocker();

public:
    SAXEventKeeperImpl();
    virtual ~SAXEventKeeperImpl();

    /* XSAXEventKeeper */
    virtual sal_Int32 SAL_CALL addElementCollector(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeElementCollector( sal_Int32 id )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL addBlocker(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeBlocker( sal_Int32 id )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isBlocking(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL
        getElement( sal_Int32 id )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setElement(
        sal_Int32 id,
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& aElement )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::xml::sax::XDocumentHandler > SAL_CALL
        setNextHandler( const css::uno::Reference<
            css::xml::sax::XDocumentHandler >& xNewHandler )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL printBufferNodeTree()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL
        getCurrentBlockingNode()
        throw (css::uno::RuntimeException, std::exception) override;

    /* XSecuritySAXEventKeeper */
    virtual sal_Int32 SAL_CALL addSecurityElementCollector(
        css::xml::crypto::sax::ElementMarkPriority priority,
        sal_Bool modifyElement )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL cloneElementCollector(
        sal_Int32 referenceId,
        css::xml::crypto::sax::ElementMarkPriority priority )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSecurityId( sal_Int32 id, sal_Int32 securityId )
        throw (css::uno::RuntimeException, std::exception) override;

    /* XReferenceResolvedBroadcaster */
    virtual void SAL_CALL addReferenceResolvedListener(
        sal_Int32 referenceId,
        const css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener >& listener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeReferenceResolvedListener(
        sal_Int32 referenceId,
        const css::uno::Reference< css::xml::crypto::sax::XReferenceResolvedListener >& listener )
        throw (css::uno::RuntimeException, std::exception) override;

    /* XSAXEventKeeperStatusChangeBroadcaster */
    virtual void SAL_CALL addSAXEventKeeperStatusChangeListener(
        const css::uno::Reference< css::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >& listener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSAXEventKeeperStatusChangeListener(
        const css::uno::Reference< css::xml::crypto::sax::XSAXEventKeeperStatusChangeListener >& listener )
        throw (css::uno::RuntimeException, std::exception) override;

    /* XDocumentHandler */
    virtual void SAL_CALL startDocument(  )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endDocument(  )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startElement(
        const OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList >&
        xAttribs )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement( const OUString& aName )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL characters( const OUString& aChars )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction(
        const OUString& aTarget, const OUString& aData )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator(
        const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    /* XInitialization */
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;
};

OUString SAXEventKeeperImpl_getImplementationName()
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL SAXEventKeeperImpl_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL SAXEventKeeperImpl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)
    throw ( css::uno::Exception );

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
