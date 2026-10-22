/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <sax/fastattribs.hxx>
#include <comphelper/seqstream.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/xml/xpath/XPathException.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>

using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::cpo;
using namespace ::cpo::uno;
using namespace css::xml::sax;
using namespace css::xml::dom;

namespace
{

class DummyDocumentHandler : public cppu::WeakImplHelper<XDocumentHandler>
{
public:
    virtual void startDocument() override {}
    virtual void endDocument() override {}
    virtual void startElement(const OUString&, const Reference<XAttributeList>&) override {}
    virtual void endElement(const OUString&) override {}
    virtual void characters(const OUString&) override {}
    virtual void ignorableWhitespace(const OUString&) override {}
    virtual void processingInstruction(const OUString&, const OUString&) override {}
    virtual void setDocumentLocator(const Reference<XLocator>&) override {}
};

// just for importNode...
class MockDoc;
class MockNode : public cppu::WeakImplHelper<XNode>
{
protected:
    rtl::Reference<MockDoc> m_document;
    rtl::Reference<MockNode> m_parent;
    rtl::Reference<MockNode> m_prev;
    rtl::Reference<MockNode> m_next;
    std::vector<rtl::Reference<MockNode>> m_children;
    OUString m_localname;

public:
    void init(const rtl::Reference<MockDoc>& doc, const rtl::Reference<MockNode>& parent,
              const rtl::Reference<MockNode>& prev, const rtl::Reference<MockNode>& next,
              std::vector<rtl::Reference<MockNode>> children)
    {
        m_document = doc;
        m_parent = parent;
        m_prev = prev;
        m_next = next;
        m_children = children;
    }

    virtual Reference<XNode> appendChild(const Reference<XNode>&) override
    {
        CPPUNIT_FAIL("MockNode.appendChild called?");
    }
    Reference<XNode> cloneNode(bool) override
    {
        CPPUNIT_FAIL("MockNode->cloneNode called?");
    }
    Reference<XNamedNodeMap> getAttributes() override
    {
        CPPUNIT_FAIL("MockNode->getAttributes not implemented");
    }
    Reference<XNodeList> getChildNodes() override
    {
        CPPUNIT_FAIL("MockNode->getChildList not implemented");
    }
    Reference<XNode> getFirstChild() override
    {
        return (m_children.size() != 0) ? m_children[0] : nullptr;
    }
    Reference<XNode> getLastChild() override
    {
        return (m_children.size() != 0) ? m_children[m_children.size() - 1] : nullptr;
    }
    OUString getLocalName() override { return m_localname; }
    OUString getNamespaceURI() override { return u""_ustr; }
    Reference<XNode> getNextSibling() override { return m_next; }
    OUString getNodeName() override { return m_localname; }
    //        NodeType getNodeType() { return m_type; }
    OUString getNodeValue() override { return u""_ustr; }
    Reference<XDocument> getOwnerDocument() override;
    Reference<XNode> getParentNode() override { return m_parent; }
    OUString getPrefix() override { return u""_ustr; }
    Reference<XNode> getPreviousSibling() override { return m_prev; }
    bool hasAttributes() override { return false; }
    bool hasChildNodes() override { return m_children.size() != 0; }
    Reference<XNode> insertBefore(const Reference<XNode>&, const Reference<XNode>&) override
    {
        CPPUNIT_FAIL("MockNode.insertBefore called?");
    }
    bool isSupported(const OUString&, const OUString&) override { return false; }
    void normalize() override { CPPUNIT_FAIL("MockNode.normalize called?"); }
    Reference<XNode> removeChild(const Reference<XNode>&) override
    {
        CPPUNIT_FAIL("MockNode->removeChild called?");
    }
    Reference<XNode> replaceChild(const Reference<XNode>&, const Reference<XNode>&) override
    {
        CPPUNIT_FAIL("MockNode.replaceChild called?");
    }
    void setNodeValue(const OUString&) override { CPPUNIT_FAIL("MockNode.setNodeValue called?"); }
    void setPrefix(const OUString&) override { CPPUNIT_FAIL("MockNode.setPrefix called?"); }
};
class MockDoc final : public cppu::ImplInheritanceHelper<MockNode, XDocument>
{
public:
    void init(std::vector<rtl::Reference<MockNode>> children)
    {
        MockNode::init(this, nullptr, nullptr, nullptr, children);
    }

    NodeType getNodeType() override { return NodeType_DOCUMENT_NODE; }

    Reference<XAttr> createAttribute(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createAttribute called?");
    }
    Reference<XAttr> createAttributeNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createAttributeNS called?");
    }
    Reference<XCDATASection> createCDATASection(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createCDATASection called?");
    }
    Reference<XComment> createComment(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createCDATASection called?");
    }
    Reference<XDocumentFragment> createDocumentFragment() override
    {
        CPPUNIT_FAIL("MockNode.createDocumentFragment called?");
    }
    Reference<XElement> createElement(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createElement called?");
    }
    Reference<XElement> createElementNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createElementNS called?");
    }
    Reference<XEntityReference> createEntityReference(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createEntityReference called?");
    }
    Reference<XProcessingInstruction> createProcessingInstruction(const OUString&,
                                                                  const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createEntityReference called?");
    }
    Reference<XText> createTextNode(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.createTextNode called?");
    }
    Reference<XDocumentType> getDoctype() override
    {
        CPPUNIT_FAIL("MockNode.getDoctype called?");
    }
    Reference<XElement> getDocumentElement() override
    {
        CPPUNIT_FAIL("MockNode.getDocumentElement called?");
    }
    Reference<XElement> getElementById(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.getElementById called?");
    }
    Reference<XNodeList> getElementsByTagName(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.getElementsByTagName called?");
    }
    Reference<XNodeList> getElementsByTagNameNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.getElementsByTagNameNS called?");
    }
    Reference<XDOMImplementation> getImplementation() override
    {
        CPPUNIT_FAIL("MockNode.getImplementation called?");
    }
    Reference<XNode> importNode(const Reference<XNode>&, bool) override
    {
        CPPUNIT_FAIL("MockNode.importNode called?");
    }

    // resolve uno inheritance problems...
    // --- delegation for XNode base.
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    appendChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild) override
    {
        return MockNode::appendChild(newChild);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> cloneNode(bool deep) override
    {
        return MockNode::cloneNode(deep);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNamedNodeMap> getAttributes() override
    {
        return MockNode::getAttributes();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNodeList> getChildNodes() override
    {
        return MockNode::getChildNodes();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getFirstChild() override
    {
        return MockNode::getFirstChild();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getLastChild() override
    {
        return MockNode::getLastChild();
    }
    virtual ::rtl::OUString getLocalName() override { return MockNode::getLocalName(); }
    virtual ::rtl::OUString getNamespaceURI() override { return MockNode::getNamespaceURI(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getNextSibling() override
    {
        return MockNode::getNextSibling();
    }
    virtual ::rtl::OUString getNodeName() override { return MockNode::getNodeName(); }
    virtual ::rtl::OUString getNodeValue() override { return MockNode::getNodeValue(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XDocument> getOwnerDocument() override
    {
        return MockNode::getOwnerDocument();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getParentNode() override
    {
        return MockNode::getParentNode();
    }
    virtual ::rtl::OUString getPrefix() override { return MockNode::getPrefix(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getPreviousSibling() override
    {
        return MockNode::getPreviousSibling();
    }
    virtual bool hasAttributes() override { return MockNode::hasAttributes(); }
    virtual bool hasChildNodes() override { return MockNode::hasChildNodes(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    insertBefore(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild,
                 const ::cpo::uno::Reference<::css::xml::dom::XNode>& refChild) override
    {
        return MockNode::insertBefore(newChild, refChild);
    }
    virtual bool isSupported(const ::rtl::OUString& feature, const ::rtl::OUString& ver) override
    {
        return MockNode::isSupported(feature, ver);
    }
    virtual void normalize() override { return MockNode::normalize(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    removeChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& oldChild) override
    {
        return MockNode::removeChild(oldChild);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    replaceChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild,
                 const ::cpo::uno::Reference<::css::xml::dom::XNode>& oldChild) override
    {
        return MockNode::replaceChild(newChild, oldChild);
    }
    virtual void setNodeValue(const ::rtl::OUString& nodeValue) override
    {
        return MockNode::setNodeValue(nodeValue);
    }
    virtual void setPrefix(const ::rtl::OUString& prefix) override
    {
        return MockNode::setPrefix(prefix);
    }
};

Reference<XDocument> MockNode::getOwnerDocument() { return m_document; }

class MockAttr;
class MockNodeMap : public cppu::WeakImplHelper<XNamedNodeMap>
{
    std::vector<rtl::Reference<MockAttr>> m_attributes;

public:
    MockNodeMap(std::vector<rtl::Reference<MockAttr>> attrs) { m_attributes = attrs; }

    sal_Int32 getLength() override { return m_attributes.size(); }
    Reference<XNode> getNamedItem(const OUString&) override
    {
        CPPUNIT_FAIL("MockNodeMap->getNamedItem not implemented");
    }
    Reference<XNode> getNamedItemNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNodeMap->getNamedItemNS not implemented");
    }
    Reference<XNode> item(sal_Int32 index) override;
    Reference<XNode> removeNamedItem(const OUString&) override
    {
        CPPUNIT_FAIL("MockNodeMap->removeNamedItem called?");
    }
    Reference<XNode> removeNamedItemNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNodeMap->removeNamedItemNS called?");
    }
    Reference<XNode> setNamedItem(const Reference<XNode>&) override
    {
        CPPUNIT_FAIL("MockNodeMap.setNamedItem called?");
    }
    Reference<XNode> setNamedItemNS(const Reference<XNode>&) override
    {
        CPPUNIT_FAIL("MockNodeMap.setNamedItemNS called?");
    }
};
class MockElement : public cppu::ImplInheritanceHelper<MockNode, XElement>
{
    std::vector<rtl::Reference<MockAttr>> m_attributes;

public:
    MockElement(const OUString& name, std::vector<rtl::Reference<MockAttr>> attrs)
    {
        m_localname = name;
        m_attributes = std::move(attrs);
    }

    NodeType getNodeType() override { return NodeType_ELEMENT_NODE; }
    Reference<XNamedNodeMap> getAttributes() override { return new MockNodeMap(m_attributes); }
    bool hasAttributes() override { return m_attributes.size() != 0; }

    OUString getAttribute(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->getAttribute not implemented");
    }
    Reference<XAttr> getAttributeNode(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->getAttributeNode not implemented");
    }
    Reference<XAttr> getAttributeNodeNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->getAttributeNodeNS not implemented");
    }
    OUString getAttributeNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->getAttributeNS not implemented");
    }
    Reference<XNodeList> getElementsByTagName(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.getElementsByTagName called?");
    }
    Reference<XNodeList> getElementsByTagNameNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode.getElementsByTagNameNS called?");
    }
    OUString getTagName() override { return MockNode::getLocalName(); }
    bool hasAttribute(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->hasAttribute not implemented");
    }
    bool hasAttributeNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->hasAttributeNS not implemented");
    }
    void removeAttribute(const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->removeAttribute called?");
    }
    Reference<XAttr> removeAttributeNode(const Reference<XAttr>&) override
    {
        CPPUNIT_FAIL("MockNode->removeAttributeNode called?");
    }
    void removeAttributeNS(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->removeAttributeNS called?");
    }
    void setAttribute(const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->setAttribute called?");
    }
    Reference<XAttr> setAttributeNode(const Reference<XAttr>&) override
    {
        CPPUNIT_FAIL("MockNode->setAttributeNode called?");
    }
    Reference<XAttr> setAttributeNodeNS(const Reference<XAttr>&) override
    {
        CPPUNIT_FAIL("MockNode->setAttributeNodeNS called?");
    }
    void setAttributeNS(const OUString&, const OUString&, const OUString&) override
    {
        CPPUNIT_FAIL("MockNode->setAttributeNS called?");
    }

    // resolve uno inheritance problems...
    // --- delegation for XNode base.
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    appendChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild) override
    {
        return MockNode::appendChild(newChild);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> cloneNode(bool deep) override
    {
        return MockNode::cloneNode(deep);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNodeList> getChildNodes() override
    {
        return MockNode::getChildNodes();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getFirstChild() override
    {
        return MockNode::getFirstChild();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getLastChild() override
    {
        return MockNode::getLastChild();
    }
    virtual ::rtl::OUString getLocalName() override { return MockNode::getLocalName(); }
    virtual ::rtl::OUString getNamespaceURI() override { return MockNode::getNamespaceURI(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getNextSibling() override
    {
        return MockNode::getNextSibling();
    }
    virtual ::rtl::OUString getNodeName() override { return MockNode::getNodeName(); }
    virtual ::rtl::OUString getNodeValue() override { return MockNode::getNodeValue(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XDocument> getOwnerDocument() override
    {
        return MockNode::getOwnerDocument();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getParentNode() override
    {
        return MockNode::getParentNode();
    }
    virtual ::rtl::OUString getPrefix() override { return MockNode::getPrefix(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getPreviousSibling() override
    {
        return MockNode::getPreviousSibling();
    }
    virtual bool hasChildNodes() override { return MockNode::hasChildNodes(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    insertBefore(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild,
                 const ::cpo::uno::Reference<::css::xml::dom::XNode>& refChild) override
    {
        return MockNode::insertBefore(newChild, refChild);
    }
    virtual bool isSupported(const ::rtl::OUString& feature, const ::rtl::OUString& ver) override
    {
        return MockNode::isSupported(feature, ver);
    }
    virtual void normalize() override { return MockNode::normalize(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    removeChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& oldChild) override
    {
        return MockNode::removeChild(oldChild);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    replaceChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild,
                 const ::cpo::uno::Reference<::css::xml::dom::XNode>& oldChild) override
    {
        return MockNode::replaceChild(newChild, oldChild);
    }
    virtual void setNodeValue(const ::rtl::OUString& nodeValue) override
    {
        return MockNode::setNodeValue(nodeValue);
    }
    virtual void setPrefix(const ::rtl::OUString& prefix) override
    {
        return MockNode::setPrefix(prefix);
    }
};
class MockAttr : public cppu::ImplInheritanceHelper<MockNode, XAttr>
{
    OUString m_value;

public:
    MockAttr(const OUString& name, const OUString& value)
    {
        m_localname = name;
        m_value = value;
    }

    NodeType getNodeType() override { return NodeType_ATTRIBUTE_NODE; }

    OUString getName() override { return m_localname; }
    Reference<XElement> getOwnerElement() override
    {
        return dynamic_cast<MockElement*>(m_parent.get());
    }
    bool getSpecified() override { return true; }
    OUString getValue() override { return m_value; }
    void setValue(const OUString&) override { CPPUNIT_FAIL("MockNode.setValue called?"); }

    // resolve uno inheritance problems...
    // --- delegation for XNode base.
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    appendChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild) override
    {
        return MockNode::appendChild(newChild);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> cloneNode(bool deep) override
    {
        return MockNode::cloneNode(deep);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNamedNodeMap> getAttributes() override
    {
        return MockNode::getAttributes();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNodeList> getChildNodes() override
    {
        return MockNode::getChildNodes();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getFirstChild() override
    {
        return MockNode::getFirstChild();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getLastChild() override
    {
        return MockNode::getLastChild();
    }
    virtual ::rtl::OUString getLocalName() override { return MockNode::getLocalName(); }
    virtual ::rtl::OUString getNamespaceURI() override { return MockNode::getNamespaceURI(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getNextSibling() override
    {
        return MockNode::getNextSibling();
    }
    virtual ::rtl::OUString getNodeName() override { return MockNode::getNodeName(); }
    virtual ::rtl::OUString getNodeValue() override { return MockNode::getNodeValue(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XDocument> getOwnerDocument() override
    {
        return MockNode::getOwnerDocument();
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getParentNode() override
    {
        return MockNode::getParentNode();
    }
    virtual ::rtl::OUString getPrefix() override { return MockNode::getPrefix(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode> getPreviousSibling() override
    {
        return MockNode::getPreviousSibling();
    }
    virtual bool hasAttributes() override { return MockNode::hasAttributes(); }
    virtual bool hasChildNodes() override { return MockNode::hasChildNodes(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    insertBefore(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild,
                 const ::cpo::uno::Reference<::css::xml::dom::XNode>& refChild) override
    {
        return MockNode::insertBefore(newChild, refChild);
    }
    virtual bool isSupported(const ::rtl::OUString& feature, const ::rtl::OUString& ver) override
    {
        return MockNode::isSupported(feature, ver);
    }
    virtual void normalize() override { return MockNode::normalize(); }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    removeChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& oldChild) override
    {
        return MockNode::removeChild(oldChild);
    }
    virtual ::cpo::uno::Reference<::css::xml::dom::XNode>
    replaceChild(const ::cpo::uno::Reference<::css::xml::dom::XNode>& newChild,
                 const ::cpo::uno::Reference<::css::xml::dom::XNode>& oldChild) override
    {
        return MockNode::replaceChild(newChild, oldChild);
    }
    virtual void setNodeValue(const ::rtl::OUString& nodeValue) override
    {
        return MockNode::setNodeValue(nodeValue);
    }
    virtual void setPrefix(const ::rtl::OUString& prefix) override
    {
        return MockNode::setPrefix(prefix);
    }
};

Reference<XNode> MockNodeMap::item(sal_Int32 index)
{
    return static_cast<MockNode*>(m_attributes[index].get());
}

class BasicTest : public test::BootstrapFixture
{
protected:
    uno::Reference<XDocumentBuilder> mxDomBuilder;

public:
    BasicTest() {}

    void setUp() override
    {
        test::BootstrapFixture::setUp();

        uno::Reference<XDocumentBuilder> xDB(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            uno::UNO_QUERY_THROW);
        mxDomBuilder.set(xDB);
    }

    void testXSAXDocumentBuilder()
    {
        Reference<XSAXDocumentBuilder>(getMultiServiceFactory()->createInstance(
                                           u"com.sun.star.xml.dom.SAXDocumentBuilder"_ustr),
                                       UNO_QUERY_THROW);
        //FIXME TODO
    }

    void testXDocument()
    {
        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);
        Reference<XDocument> xDoc = xBuilder->newDocument();

        {
            Reference<XAttr> xAttr = xDoc->createAttribute(u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XDocument.createAttribute", xAttr != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.createAttribute", u"foo"_ustr,
                                         xAttr->getNodeName());
        }

        OUString ns = u"http://example.com/"_ustr;
        {
            Reference<XAttr> xAttr = xDoc->createAttributeNS(ns, u"e:foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XDocument.createAttributeNS", xAttr != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.createAttributeNS", u"foo"_ustr,
                                         xAttr->getNodeName());
        }

        Reference<XCDATASection> xCDS = xDoc->createCDATASection(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createCDATASection", xCDS != nullptr);

        Reference<XComment> xComment = xDoc->createComment(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createComment", xComment != nullptr);

        Reference<XDocumentFragment> xDF = xDoc->createDocumentFragment();
        CPPUNIT_ASSERT_MESSAGE("XDocument.createDocumentFragment", xDF != nullptr);

        Reference<XElement> xElemFoo = xDoc->createElement(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createElement(\"foo\")", xElemFoo != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.createElement(\"foo\")", u"foo"_ustr,
                                     xElemFoo->getNodeName());

        Reference<XElement> xElemFooNs = xDoc->createElementNS(ns, u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createElementNS(\"foo\")", xElemFooNs != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.createElementNS(\"foo\")", u"foo"_ustr,
                                     xElemFooNs->getNodeName());

        Reference<XEntityReference> xER = xDoc->createEntityReference(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createEntityReference", xER != nullptr);

        Reference<XProcessingInstruction> xPI
            = xDoc->createProcessingInstruction(u"foo"_ustr, u"bar"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createProcessingInstruction", xPI != nullptr);

        Reference<XText> xText = xDoc->createTextNode(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createTextNode", xText != nullptr);

        Reference<XDocumentType> xDT = xDoc->getDoctype();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getDoctype", xDT, Reference<XDocumentType>());

        {
            Reference<XElement> xDE = xDoc->getDocumentElement();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getDocumentElement", xDE,
                                         Reference<XElement>());
        }
        {
            Reference<XElement> xById = xDoc->getElementById(u"foo"_ustr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getDocumentElement", xById,
                                         Reference<XElement>());
        }

        {
            Reference<XNodeList> xNodeList = xDoc->getElementsByTagName(u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XDocument.getElementsByTagName", xNodeList != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getElementsByTagName", sal_Int32(0),
                                         xNodeList->getLength());
        }

        {
            Reference<XNodeList> xNodeList = xDoc->getElementsByTagNameNS(ns, u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XDocument.getElementsByTagNameNS", xNodeList != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getElementsByTagNameNS", sal_Int32(0),
                                         xNodeList->getLength());
        }

        Reference<XDOMImplementation> xDOMImpl = xDoc->getImplementation();
        CPPUNIT_ASSERT_MESSAGE("XDocument.getImplementation", xDOMImpl != nullptr);

        {
            Reference<XNode> xRet = xElemFooNs->appendChild(xElemFoo);
            CPPUNIT_ASSERT_MESSAGE("XElement.appendChild(xElemFoo)", xElemFoo == xRet);
        }
        {
            Reference<XNode> xRet = xDoc->appendChild(xElemFooNs);
            //            CPPUNIT_ASSERT_MESSAGE("XDocument.appendChild(xElemFooNs)", xElemFooNs->equals(xRet));
        }

        Reference<XElement> xDE = xDoc->getDocumentElement();
        CPPUNIT_ASSERT_MESSAGE("XDocument.getDocumentElement", xDE != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getDocumentElement", xElemFooNs, xDE);

        {
            Reference<XNodeList> xNodeList = xDoc->getElementsByTagName(u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XDocument.getElementsByTagName", xNodeList != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getElementsByTagName", sal_Int32(2),
                                         xNodeList->getLength());
            CPPUNIT_ASSERT_MESSAGE("XDocument.getElementsByTagNameNS",
                                   xElemFooNs == xNodeList->item(0));
            CPPUNIT_ASSERT_MESSAGE("XDocument.getElementsByTagName",
                                   xElemFoo == xNodeList->item(1));
        }

        {
            Reference<XNodeList> xNodeList = xDoc->getElementsByTagNameNS(ns, u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XDocument.getElementsByTagNameNS", xNodeList != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getElementsByTagNameNS", sal_Int32(1),
                                         xNodeList->getLength());
            CPPUNIT_ASSERT_MESSAGE("XDocument.getElementsByTagNameNS",
                                   xElemFooNs == xNodeList->item(0));
        }

        xElemFoo->setAttributeNS(u"http://www.w3.org/XML/1998/namespace"_ustr, u"xml:id"_ustr,
                                 u"bar"_ustr);

        Reference<XElement> xById = xDoc->getElementById(u"bar"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.getDocumentElement", xById != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getDocumentElement", xElemFoo, xById);

        try
        {
            xDoc->importNode(nullptr, false);
            CPPUNIT_FAIL("XDocument.importNode(null)");
        }
        catch (Exception)
        { /* expected */
        }
        {
            Reference<XNode> xImported = xDoc->importNode(xElemFoo, false);
            CPPUNIT_ASSERT_MESSAGE("XDocument.importNode()", xImported != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XDocument.importNode()", xElemFoo == xImported);
        }
        {
            rtl::Reference<MockAttr> xMockAttrBar = new MockAttr(u"bar"_ustr, u"blah"_ustr);
            rtl::Reference<MockAttr> xMockAttrBaz = new MockAttr(u"baz"_ustr, u"quux"_ustr);
            rtl::Reference<MockElement> xMockElemFoo = new MockElement(
                u"foo"_ustr, std::vector<rtl::Reference<MockAttr>>{ xMockAttrBar, xMockAttrBaz });
            rtl::Reference<MockElement> xMockElemBar = new MockElement(u"bar"_ustr, {});
            rtl::Reference<MockElement> xMockElemRoot = new MockElement(u"root"_ustr, {});
            rtl::Reference<MockDoc> xMockDoc = new MockDoc;
            xMockDoc->init(std::vector<rtl::Reference<MockNode>>{ xMockElemRoot });
            xMockElemRoot->init(
                xMockDoc, xMockDoc, nullptr, nullptr,
                std::vector<rtl::Reference<MockNode>>{ xMockElemFoo, xMockElemBar });
            xMockElemFoo->init(xMockDoc, xMockElemRoot, nullptr, xMockElemBar, {});
            xMockElemBar->init(xMockDoc, xMockElemRoot, xMockElemFoo, nullptr, {});

            {
                Reference<XNode> xImported
                    = xDoc->importNode(static_cast<MockNode*>(xMockElemRoot.get()), false);
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(false)", xImported != nullptr);
                Reference<XElement> xE(xImported, UNO_QUERY);
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(false)", xE != nullptr);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.importNode(false)", u"root"_ustr,
                                             xE->getLocalName());
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(false)", !xE->hasAttributes());
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(false)", !xE->hasChildNodes());
            }

            {
                Reference<XNode> xImported
                    = xDoc->importNode(static_cast<MockNode*>(xMockElemRoot.get()), true);
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", xImported != nullptr);
                Reference<XElement> xImpRoot(xImported, UNO_QUERY);
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", xImpRoot != nullptr);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.importNode(true)", u"root"_ustr,
                                             xImpRoot->getLocalName());
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", !xImpRoot->hasAttributes());
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", xImpRoot->hasChildNodes());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.importNode(true)", u"root"_ustr,
                                             xImpRoot->getNodeName());

                Reference<XNode> xImpFooN = xImpRoot->getFirstChild();
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", xImpFooN != nullptr);
                Reference<XElement> xImpFoo(xImpFooN, UNO_QUERY);
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", xImpFoo != nullptr);
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", xImpFoo->hasAttributes());
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", !xImpFoo->hasChildNodes());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.importNode(true)", u"foo"_ustr,
                                             xImpFoo->getNodeName());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.importNode(true)", u"blah"_ustr,
                                             xImpFoo->getAttribute(u"bar"_ustr));
                CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.importNode(true)", u"quux"_ustr,
                                             xImpFoo->getAttribute(u"baz"_ustr));
                Reference<XNode> xImpBarN = xImpFooN->getNextSibling();
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", xImpBarN != nullptr);
                Reference<XElement> xImpBar(xImpBarN, UNO_QUERY);
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", xImpBar != nullptr);
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", !xImpBar->hasAttributes());
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)", !xImpBar->hasChildNodes());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.importNode(true)", u"bar"_ustr,
                                             xImpBar->getNodeName());
                CPPUNIT_ASSERT_MESSAGE("XDocument.importNode(true)",
                                       xImpBar->getNextSibling() == nullptr);
            }
        }

        // XNode

        {
            Reference<XNode> xDocCloneN = xDoc->cloneNode(false);
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(false)", xDocCloneN != nullptr);
            Reference<XDocument> xDocClone(xDocCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(false)", xDocClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(false)", !xDocClone->hasChildNodes());
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(false)",
                                   xDocClone->getFirstChild() == nullptr);
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(false)",
                                   xDocClone->getDocumentElement() == nullptr);
        }
        {
            Reference<XNode> xDocCloneN = xDoc->cloneNode(true);
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(true)", xDocCloneN != nullptr);
            Reference<XDocument> xDocClone(xDocCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(true)", xDocClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(true)", xDocClone->hasChildNodes());
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(true)",
                                   xDocClone->getFirstChild() != nullptr);
            Reference<XElement> xE = xDocClone->getDocumentElement();
            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(true)", xE != nullptr);
            //            CPPUNIT_ASSERT_MESSAGE("XDocument->cloneNode(true)", !xElemFooNs->equals(xE));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument->cloneNode(true)", u"foo"_ustr,
                                         xE->getLocalName());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument->cloneNode(true)", ns, xE->getNamespaceURI());
        }

        CPPUNIT_ASSERT_MESSAGE("XDocument->getAttributes()", xDoc->getAttributes() == nullptr);

        {
            Reference<XNodeList> xChildren = xDoc->getChildNodes();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument->getChildNodes()", sal_Int32(1), xChildren->getLength());
            CPPUNIT_ASSERT_MESSAGE("XDocument->getChildNodes()", xElemFooNs == xChildren->item(0));

            Reference<XNode> xFirst = xDoc->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XDocument->getFirstChild()", xElemFooNs == xFirst);
            Reference<XNode> xLast = xDoc->getLastChild();
            CPPUNIT_ASSERT_MESSAGE("XDocument.getLastChild()", xElemFooNs == xLast);
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument->getLocalName()", u""_ustr, xDoc->getLocalName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument->getNamespaceURI()", u""_ustr,
                                     xDoc->getNamespaceURI());

        CPPUNIT_ASSERT_MESSAGE("XDocument.getNextSibling()", xDoc->getNextSibling() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getNodeName()", u"#document"_ustr,
                                     xDoc->getNodeName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getNodeType()", NodeType_DOCUMENT_NODE,
                                     xDoc->getNodeType());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getNodeValue()", u""_ustr, xDoc->getNodeValue());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getOwnerDocument()", xDoc, xDoc->getOwnerDocument());

        CPPUNIT_ASSERT_MESSAGE("XDocument.getParentNode()", xDoc->getParentNode() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.getPrefix()", u""_ustr, xDoc->getPrefix());

        CPPUNIT_ASSERT_MESSAGE("XDocument.getPreviousSibling()",
                               xDoc->getPreviousSibling() == nullptr);

        CPPUNIT_ASSERT_MESSAGE("XDocument->hasAttributes()", !xDoc->hasAttributes());

        CPPUNIT_ASSERT_MESSAGE("XDocument->hasChildNodes()", xDoc->hasChildNodes());

        CPPUNIT_ASSERT_MESSAGE("XDocument.isSupported()",
                               !xDoc->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xDoc->normalize();

        try
        {
            xDoc->setNodeValue(u"42"_ustr);
            CPPUNIT_FAIL("XDocument.setNodeValue()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.setNodeValue()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        try
        {
            xDoc->setPrefix(u"foo"_ustr);
            CPPUNIT_FAIL("XDocument.setPrefix()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.setPrefix()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        try
        {
            xDoc->appendChild(nullptr);
            CPPUNIT_FAIL("XDocument.appendChild(null)");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xDoc->insertBefore(nullptr, xText);
            CPPUNIT_FAIL("XDocument.insertBefore(null,)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDoc->insertBefore(xText, nullptr);
            CPPUNIT_FAIL("XDocument.insertBefore(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDoc->insertBefore(xText, xText);
            CPPUNIT_FAIL("XDocument.insertBefore(x, x)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.insertBefore(x, x)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        {
            Reference<XNode> xRet = xDoc->insertBefore(xComment, xElemFooNs);
            CPPUNIT_ASSERT_MESSAGE("XDocument.insertBefore(xComment, xElemFooNs)",
                                   xRet == xElemFooNs); // why does this return the old node?
            CPPUNIT_ASSERT_MESSAGE("XDocument.insertBefore(xComment, xElemFooNs)",
                                   xComment == xDoc->getFirstChild());
            CPPUNIT_ASSERT_MESSAGE("XDocument.insertBefore(xComment, xElemFooNs)",
                                   xDoc == xComment->getParentNode());
            CPPUNIT_ASSERT_MESSAGE("XDocument.insertBefore(xCommnet, xElemFooNs)",
                                   xElemFooNs == xDoc->getLastChild());
        }

        try
        {
            xDoc->replaceChild(nullptr, xText);
            CPPUNIT_FAIL("XDocument.replaceChild(null, )");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDoc->replaceChild(xText, nullptr);
            CPPUNIT_FAIL("XDocument.replaceChild(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDoc->replaceChild(xElemFoo, xElemFoo); // not child
            CPPUNIT_FAIL("XDocument.replaceChild(xElemFoo, xElemFoo)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.replaceChild(xElemFoo, xElemFoo)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        try
        {
            xDoc->replaceChild(xElemFooNs, xElemFooNs); // child
            CPPUNIT_ASSERT_MESSAGE("XDocument.replaceChild(xElemFooNs, xElemFooNs)", true);
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.replaceChild(xElemFooNs, xElemFooNs)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        Reference<XNode> xReplaced = xDoc->replaceChild(xPI, xComment);
        CPPUNIT_ASSERT_MESSAGE("XDocument.replaceChild(xPI, xComment)", xReplaced == xComment);
        CPPUNIT_ASSERT_MESSAGE("XDocument.replaceChild(xPI, xComment)",
                               xPI == xDoc->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XDocument.replaceChild(xPI, xComment)",
                               xElemFooNs == xDoc->getLastChild());

        try
        {
            xDoc->removeChild(nullptr);
            CPPUNIT_FAIL("XDocument->removeChild(null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDoc->removeChild(xElemFoo);
            CPPUNIT_FAIL("XDocument->removeChild()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument->removeChild()",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        Reference<XNode> xRemoved = xDoc->removeChild(xPI);
        CPPUNIT_ASSERT_MESSAGE("XDocument->removeChild(xPI)", xRemoved == xPI);
        CPPUNIT_ASSERT_MESSAGE("XDocument->removeChild(xPI)", xDoc->hasChildNodes());
        CPPUNIT_ASSERT_MESSAGE("XDocument->removeChild(xPI)", xElemFooNs == xDoc->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XDocument->removeChild(xPI)", xElemFooNs == xDoc->getLastChild());
    }

    void testXDocumentFragment()
    {
        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);
        Reference<XDocument> xDoc = xBuilder->newDocument();

        Reference<XDocumentFragment> xDF = xDoc->createDocumentFragment();
        CPPUNIT_ASSERT_MESSAGE("XDocument.createDocumentFragment", xDF != nullptr);

        Reference<XElement> xElemFoo = xDoc->createElement(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createElement", xElemFoo != nullptr);

        xDF->appendChild(xElemFoo);

        // XNode

        Reference<XText> xText = xDoc->createTextNode(u"foo"_ustr);
        Reference<XComment> xComment = xDoc->createComment(u"foo"_ustr);

        {
            Reference<XNode> xDFCloneN = xDF->cloneNode(false);
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(false)", xDFCloneN != nullptr);
            Reference<XDocumentFragment> xDFClone(xDFCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(false)", xDFClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(false)",
                                   !xDFClone->hasChildNodes());
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(false)",
                                   xDFClone->getFirstChild() == nullptr);
        }
        {
            Reference<XNode> xDFCloneN = xDF->cloneNode(true);
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(true)", xDFCloneN != nullptr);
            Reference<XDocumentFragment> xDFClone(xDFCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(true)", xDFClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(true)", xDFClone->hasChildNodes());
            Reference<XNode> xChild = xDFClone->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(true)", xChild != nullptr);
            Reference<XElement> xE(xChild, UNO_QUERY);
            //            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->cloneNode(true)", !xElemFoo->equals(xE));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment->cloneNode(true)", u"foo"_ustr,
                                         xE->getLocalName());
        }

        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->getAttributes()",
                               xDF->getAttributes() == nullptr);

        {
            Reference<XNodeList> xChildren = xDF->getChildNodes();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment->getChildNodes()", sal_Int32(1),
                                         xChildren->getLength());
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->getChildNodes()",
                                   xElemFoo == xChildren->item(0));

            Reference<XNode> xFirst = xDF->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->getFirstChild()", xElemFoo == xFirst);
            Reference<XNode> xLast = xDF->getLastChild();
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.getLastChild()", xElemFoo == xLast);
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment->getLocalName()", u""_ustr,
                                     xDF->getLocalName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment->getNamespaceURI()", u""_ustr,
                                     xDF->getNamespaceURI());

        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.getNextSibling()",
                               xDF->getNextSibling() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.getNodeName()", u"#document-fragment"_ustr,
                                     xDF->getNodeName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.getNodeType()",
                                     NodeType_DOCUMENT_FRAGMENT_NODE, xDF->getNodeType());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.getNodeValue()", u""_ustr,
                                     xDF->getNodeValue());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.getOwnerDocument()", xDoc,
                                     xDF->getOwnerDocument());

        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.getParentNode()",
                               xDF->getParentNode() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.getPrefix()", u""_ustr, xDF->getPrefix());

        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.getPreviousSibling()",
                               xDF->getPreviousSibling() == nullptr);

        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->hasAttributes()", !xDF->hasAttributes());

        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->hasChildNodes()", xDF->hasChildNodes());

        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.isSupported()",
                               !xDF->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xDF->normalize();

        try
        {
            xDF->setNodeValue(u"42"_ustr);
            CPPUNIT_FAIL("XDocumentFragment.setNodeValue()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.setNodeValue()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        try
        {
            xDF->setPrefix(u"foo"_ustr);
            CPPUNIT_FAIL("XDocumentFragment.setPrefix()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.setPrefix()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        try
        {
            xDF->appendChild(nullptr);
            CPPUNIT_FAIL("XDocumentFragment.appendChild(null)");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xDF->insertBefore(nullptr, xText);
            CPPUNIT_FAIL("XDocumentFragment.insertBefore(null,)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDF->insertBefore(xText, nullptr);
            CPPUNIT_FAIL("XDocumentFragment.insertBefore(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDF->insertBefore(xText, xText);
            CPPUNIT_FAIL("XDocumentFragment.insertBefore(x, x)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.insertBefore(x, x)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        {
            Reference<XNode> xRet = xDF->insertBefore(xComment, xElemFoo);
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.insertBefore(xComment, xElemFoo)",
                                   xRet == xElemFoo); // why does this return the old node?
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.insertBefore(xComment, xElemFoo)",
                                   xComment == xDF->getFirstChild());
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.insertBefore(xComment, xElemFoo)",
                                   xDF == xComment->getParentNode());
            CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.insertBefore(xCommnet, xElemFoo)",
                                   xElemFoo == xDF->getLastChild());
        }

        try
        {
            xDF->replaceChild(nullptr, xText);
            CPPUNIT_FAIL("XDocumentFragment.replaceChild(null, )");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDF->replaceChild(xText, nullptr);
            CPPUNIT_FAIL("XDocumentFragment.replaceChild(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDF->replaceChild(xElemFoo, xElemFoo); // not child
            CPPUNIT_FAIL("XDocumentFragment.replaceChild(xElemFoo, xElemFoo)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.replaceChild(xElemFoo, xElemFoo)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        try
        {
            xDF->replaceChild(xElemFoo, xElemFoo);
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment.replaceChild(xElemFoo, xElemFoo)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        Reference<XNode> xReplaced = xDF->replaceChild(xText, xComment);
        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.replaceChild(xText, xComment)",
                               xReplaced == xComment);
        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.replaceChild(xText, xComment)",
                               xText == xDF->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment.replaceChild(xText, xComment)",
                               xElemFoo == xDF->getLastChild());

        try
        {
            xDF->removeChild(nullptr);
            CPPUNIT_FAIL("XDocumentFragment->removeChild(null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xDF->removeChild(xComment);
            CPPUNIT_FAIL("XDocumentFragment->removeChild()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocumentFragment->removeChild()",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        Reference<XNode> xRemoved = xDF->removeChild(xText);
        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->removeChild(xText)", xRemoved == xText);
        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->removeChild(xText)", xDF->hasChildNodes());
        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->removeChild(xText)",
                               xElemFoo == xDF->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XDocumentFragment->removeChild(xText)",
                               xElemFoo == xDF->getLastChild());
    }

    void testXElement()
    {
        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);
        Reference<XDocument> xDoc = xBuilder->newDocument();

        OUString ns = u"http://example.com/"_ustr;

        Reference<XElement> xElemFoo = xDoc->createElement(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createElement(\"foo\")", xElemFoo != nullptr);

        Reference<XElement> xElemFooNs = xDoc->createElementNS(ns, u"e:foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createElementNs(\"foo\")", xElemFooNs != nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getTagName", u"foo"_ustr, xElemFoo->getTagName());

        {
            Reference<XNodeList> xNodeList = xElemFoo->getElementsByTagName(u"bar"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XElement.getElementsByTagName", xNodeList != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getElementsByTagName", sal_Int32(0),
                                         xNodeList->getLength());
        }

        {
            Reference<XNodeList> xNodeList = xElemFoo->getElementsByTagNameNS(ns, u"bar"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XElement.getElementsByTagNameNS", xNodeList != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getElementsByTagNameNS", sal_Int32(0),
                                         xNodeList->getLength());
        }

        xElemFoo->appendChild(xElemFooNs);

        {
            Reference<XNodeList> xNodeList = xElemFoo->getElementsByTagName(u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XElement.getElementsByTagName", xNodeList != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getElementsByTagName", sal_Int32(2),
                                         xNodeList->getLength());
            CPPUNIT_ASSERT_MESSAGE("XElement.getElementsByTagName", xElemFoo == xNodeList->item(0));
            CPPUNIT_ASSERT_MESSAGE("XElement.getElementsByTagName",
                                   xElemFooNs == xNodeList->item(1));
        }
        {
            Reference<XNodeList> xNodeList = xElemFoo->getElementsByTagNameNS(ns, u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XElement.getElementsByTagNameNS", xNodeList != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getElementsByTagNameNS", sal_Int32(1),
                                         xNodeList->getLength());
            CPPUNIT_ASSERT_MESSAGE("XElement.getElementsByTagNameNS",
                                   xElemFooNs == xNodeList->item(0));
        }

        {
            OUString ret = xElemFoo->getAttribute(u"foo"_ustr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getAttribute", u""_ustr, ret);
        }
        {
            OUString ret = xElemFoo->getAttributeNS(ns, u"foo"_ustr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getAttributeNS", u""_ustr, ret);
        }
        {
            Reference<XNode> xAttr = xElemFoo->getAttributeNode(u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XElement->getAttributeNode", xAttr == nullptr);
        }
        {
            Reference<XNode> xAttr = xElemFoo->getAttributeNodeNS(ns, u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XElement->getAttributeNodeNS", xAttr == nullptr);
        }
        CPPUNIT_ASSERT_MESSAGE("XElement->hasAttribute", !xElemFoo->hasAttribute(u"foo"_ustr));
        CPPUNIT_ASSERT_MESSAGE("XElement->hasAttributeNS",
                               !xElemFoo->hasAttributeNS(ns, u"foo"_ustr));

        // surprisingly this does not throw?
        xElemFoo->removeAttribute(u"foo"_ustr);
        xElemFoo->removeAttributeNS(ns, u"foo"_ustr);

        Reference<XAttr> xAttr = xDoc->createAttribute(u"foo"_ustr);
        Reference<XAttr> xAttrNs = xDoc->createAttributeNS(ns, u"foo"_ustr);

        try
        {
            xElemFoo->removeAttributeNode(nullptr);
            CPPUNIT_FAIL("XElement->removeAttributeNode(null)");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xElemFoo->removeAttributeNode(xAttr);
            CPPUNIT_FAIL("XElement->removeAttributeNode(xAttr)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->removeAttributeNode(xAttr)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        Reference<XAttr> xAttrSet = xElemFoo->setAttributeNode(xAttr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->setAttributeNode(xAttr)", xAttrSet,
                                     xElemFoo->getAttributeNode(u"foo"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->setAttributeNode(xAttr)", xElemFoo,
                                     xAttrSet->getOwnerElement());
        try
        {
            xElemFooNs->setAttributeNode(xAttrSet);
            CPPUNIT_FAIL("XElement->setAttributeNode(xAttrSet)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->setAttributeNode(xAttrSet)",
                                         DOMExceptionType_INUSE_ATTRIBUTE_ERR, e.Code);
        }

        Reference<XAttr> xAttrNsSet = xElemFooNs->setAttributeNodeNS(xAttrNs);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->setAttributeNodeNS(xAttr)",
                               xAttrNsSet, xElemFooNs->getAttributeNodeNS(ns, u"foo"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->setAttributeNodeNS(xAttrNs)",
                               xElemFooNs, xAttrNsSet->getOwnerElement());
        try
        {
            xElemFooNs->setAttributeNodeNS(xAttrNsSet);
            CPPUNIT_FAIL("XElement->setAttributeNodeNS(xAttrNsSet)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->setAttributeNodeNS(xAttrNsSet)",
                                         DOMExceptionType_INUSE_ATTRIBUTE_ERR, e.Code);
        }

        Reference<XAttr> xAttrRemoved = xElemFoo->removeAttributeNode(xAttrSet);
        CPPUNIT_ASSERT_MESSAGE("XElement->removeAttributeNode(xAttrSet)", xAttrRemoved != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->removeAttributeNode(xAttrSet)", u"foo"_ustr,
                                     xAttrRemoved->getName());
        CPPUNIT_ASSERT_MESSAGE("XElement->removeAttributeNode(xAttrSet)",
                               xAttrRemoved->getOwnerElement() == nullptr);

        Reference<XAttr> xAttrNsRemoved = xElemFooNs->removeAttributeNode(xAttrNsSet);
        CPPUNIT_ASSERT_MESSAGE("XElement->removeAttributeNode(xAttrNsSet)",
                               xAttrNsRemoved != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->removeAttributeNode(xAttrNsSet)", u"foo"_ustr,
                                     xAttrNsRemoved->getName());
        CPPUNIT_ASSERT_MESSAGE("XElement->removeAttributeNode(xAttrNsSet)",
                               xAttrNsRemoved->getOwnerElement() == nullptr);

        xElemFoo->setAttribute(u"foo"_ustr, u"bar"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->setAttribute()", u"bar"_ustr,
                                     xElemFoo->getAttribute(u"foo"_ustr));

        xElemFooNs->setAttributeNS(ns, u"foo"_ustr, u"bar"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->setAttributeNS()", u"bar"_ustr,
                                     xElemFooNs->getAttributeNS(ns, u"foo"_ustr));

        xElemFoo->removeAttribute(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XElement->removeAttribute",
                               xElemFoo->getAttributeNode(u"foo"_ustr) == nullptr);

        xElemFooNs->removeAttributeNS(ns, u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XElement->removeAttributeNS",
                               xElemFooNs->getAttributeNodeNS(ns, u"foo"_ustr) == nullptr);

        // XNode

        Reference<XText> xText = xDoc->createTextNode(u"foo"_ustr);
        Reference<XComment> xComment = xDoc->createComment(u"foo"_ustr);

        {
            Reference<XNamedNodeMap> xAttrMap = xElemFoo->getAttributes();
            CPPUNIT_ASSERT_MESSAGE("XElement->getAttributes", xAttrMap != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getAttributes", sal_Int32(0), xAttrMap->getLength());
            CPPUNIT_ASSERT_MESSAGE("XElement->hasAttributes()", !xElemFoo->hasAttributes());
        }

        xElemFooNs->setAttribute(u"foo"_ustr, u"bar"_ustr);
        xElemFoo->setAttributeNS(ns, u"foo"_ustr, u"bar"_ustr);

        {
            Reference<XNamedNodeMap> xAttrMap = xElemFoo->getAttributes();
            CPPUNIT_ASSERT_MESSAGE("XElement->getAttributes", xAttrMap != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getAttributes", sal_Int32(1), xAttrMap->getLength());
            Reference<XNode> xAttr_ = xAttrMap->getNamedItemNS(ns, u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XElement->getAttributes", xAttr_ != nullptr);
        }
        {
            Reference<XNamedNodeMap> xAttrMap = xElemFooNs->getAttributes();
            CPPUNIT_ASSERT_MESSAGE("XElement->getAttributes", xAttrMap != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getAttributes", sal_Int32(1), xAttrMap->getLength());
            Reference<XNode> xAttr_ = xAttrMap->getNamedItem(u"foo"_ustr);
            CPPUNIT_ASSERT_MESSAGE("XElement->getAttributes", xAttr_ != nullptr);
        }

        {
            Reference<XNode> xElemFooCloneN = xElemFoo->cloneNode(false);
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(false)", xElemFooCloneN != nullptr);
            Reference<XElement> xElemFooClone(xElemFooCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(false)", xElemFooClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(false)", !xElemFooClone->hasChildNodes());
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(false)",
                                   xElemFooClone->getFirstChild() == nullptr);
        }
        {
            Reference<XNode> xElemFooCloneN = xElemFoo->cloneNode(true);
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(true)", xElemFooCloneN != nullptr);
            Reference<XElement> xElemFooClone(xElemFooCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(true)", xElemFooClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(true)", xElemFooClone->hasChildNodes());
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(true)",
                                   xElemFooClone->hasAttributeNS(ns, u"foo"_ustr));
            Reference<XNode> xChild = xElemFooClone->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(true)", xChild != nullptr);
            Reference<XElement> xElemFooNsClone(xChild, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(true)", xElemFooNsClone != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->cloneNode(true)", u"foo"_ustr,
                                         xElemFooNsClone->getLocalName());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->cloneNode(true)", ns,
                                         xElemFooNsClone->getNamespaceURI());
            CPPUNIT_ASSERT_MESSAGE("XElement->cloneNode(true)",
                                   xElemFooNsClone->hasAttribute(u"foo"_ustr));
        }

        {
            Reference<XNodeList> xChildren = xElemFoo->getChildNodes();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getChildNodes()", sal_Int32(1), xChildren->getLength());
            CPPUNIT_ASSERT_MESSAGE("XElement->getChildNodes()", xElemFooNs == xChildren->item(0));

            Reference<XNode> xFirst = xElemFoo->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XDocument->getFirstChild()", xElemFooNs == xFirst);
            Reference<XNode> xLast = xElemFoo->getLastChild();
            CPPUNIT_ASSERT_MESSAGE("XDocument.getLastChild()", xElemFooNs == xLast);
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getLocalName()", u"foo"_ustr,
                                     xElemFoo->getLocalName());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getLocalName()", u"foo"_ustr,
                                     xElemFooNs->getLocalName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getNamespaceURI()", u""_ustr,
                                     xElemFoo->getNamespaceURI());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->getNamespaceURI()", ns,
                                     xElemFooNs->getNamespaceURI());

        CPPUNIT_ASSERT_MESSAGE("XElement.getNextSibling()", xElemFoo->getNextSibling() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getNodeName()", u"foo"_ustr,
                                     xElemFoo->getNodeName());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getNodeName()", u"foo"_ustr,
                                     xElemFooNs->getNodeName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getNodeType()", NodeType_ELEMENT_NODE,
                                     xElemFoo->getNodeType());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getNodeValue()", u""_ustr, xElemFoo->getNodeValue());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getOwnerDocument()", xDoc, xElemFoo->getOwnerDocument());

        CPPUNIT_ASSERT_MESSAGE("XElement.getParentNode()", xElemFoo->getParentNode() == nullptr);
        CPPUNIT_ASSERT_MESSAGE("XElement.getParentNode()", xElemFoo == xElemFooNs->getParentNode());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getPrefix()", u""_ustr, xElemFoo->getPrefix());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getPrefix()", u"e"_ustr, xElemFooNs->getPrefix());

        CPPUNIT_ASSERT_MESSAGE("XElement.getPreviousSibling()",
                               xElemFoo->getPreviousSibling() == nullptr);

        CPPUNIT_ASSERT_MESSAGE("XElement->hasAttributes()", xElemFoo->hasAttributes());

        CPPUNIT_ASSERT_MESSAGE("XElement->hasChildNodes()", xElemFoo->hasChildNodes());
        CPPUNIT_ASSERT_MESSAGE("XElement->hasChildNodes()", !xElemFooNs->hasChildNodes());

        CPPUNIT_ASSERT_MESSAGE(
            "XElement.isSupported()",
            !xElemFoo->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xElemFoo->normalize();

        try
        {
            xElemFoo->setNodeValue(u"42"_ustr);
            CPPUNIT_FAIL("XElement.setNodeValue()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.setNodeValue()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        xElemFooNs->setPrefix(u"f"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.getPrefix()", u"f"_ustr, xElemFooNs->getPrefix());

        try
        {
            xElemFoo->appendChild(nullptr);
            CPPUNIT_FAIL("XElement.appendChild(null)");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xElemFoo->insertBefore(nullptr, xText);
            CPPUNIT_FAIL("XElemFoo.insertBefore(null,)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xElemFoo->insertBefore(xText, nullptr);
            CPPUNIT_FAIL("XElemFoo.insertBefore(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xElemFoo->insertBefore(xText, xText);
            CPPUNIT_FAIL("XElement.insertBefore(x, x)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XDocument.insertBefore(x, x)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        {
            Reference<XNode> xRet = xElemFoo->insertBefore(xText, xElemFooNs);
            CPPUNIT_ASSERT_MESSAGE("XElement.insertBefore(xText, xElemFooNs)",
                                   xRet == xElemFooNs); // why does this return the old node?
            CPPUNIT_ASSERT_MESSAGE("XElement.insertBefore(xText, xElemFooNs)",
                                   xText == xElemFoo->getFirstChild());
            CPPUNIT_ASSERT_MESSAGE("XElement.insertBefore(xText, xElemFooNs)",
                                   xElemFoo == xText->getParentNode());
            CPPUNIT_ASSERT_MESSAGE("XElement.insertBefore(xText, xElemFooNs)",
                                   xElemFooNs == xElemFoo->getLastChild());
        }

        try
        {
            xElemFoo->replaceChild(nullptr, xText);
            CPPUNIT_FAIL("XElement.replaceChild(null, )");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xElemFoo->replaceChild(xText, nullptr);
            CPPUNIT_FAIL("XElement.replaceChild(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xElemFoo->replaceChild(xElemFoo, xElemFoo); // not child
            CPPUNIT_FAIL("XElement.replaceChild(xElemFoo, xElemFoo)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.replaceChild(xElemFoo, xElemFoo)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        try
        {
            xElemFoo->replaceChild(xElemFooNs, xElemFooNs);
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement.replaceChild(xElemFooNs, xElemFooNs)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        Reference<XNode> xReplaced = xElemFoo->replaceChild(xComment, xText);
        CPPUNIT_ASSERT_MESSAGE("XElement.replaceChild(xComment, xText)", xReplaced == xText);
        CPPUNIT_ASSERT_MESSAGE("XElement.replaceChild(xComment, xText)",
                               xComment == xElemFoo->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XElement.replaceChild(xComment, xText)",
                               xElemFooNs == xElemFoo->getLastChild());

        try
        {
            xElemFoo->removeChild(nullptr);
            CPPUNIT_FAIL("XElement->removeChild(null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xElemFoo->removeChild(xElemFoo);
            CPPUNIT_FAIL("XElement->removeChild()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XElement->removeChild()",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        Reference<XNode> xRemoved = xElemFoo->removeChild(xComment);
        CPPUNIT_ASSERT_MESSAGE("XElement->removeChild(xComment)", xRemoved == xComment);
        CPPUNIT_ASSERT_MESSAGE("XElement->removeChild(xComment)", xElemFoo->hasChildNodes());
        CPPUNIT_ASSERT_MESSAGE("XElement->removeChild(xComment)",
                               xElemFooNs == xElemFoo->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XElement->removeChild(xComment)",
                               xElemFooNs == xElemFoo->getLastChild());
    }

    void testXAttr()
    {
        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);
        Reference<XDocument> xDoc = xBuilder->newDocument();

        OUString ns = u"http://example.com/"_ustr;

        Reference<XAttr> xAttr = xDoc->createAttribute(u"foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createAttribute", xAttr != nullptr);

        Reference<XAttr> xAttrNs = xDoc->createAttributeNS(ns, u"e:foo"_ustr);
        CPPUNIT_ASSERT_MESSAGE("XDocument.createAttribute", xAttr != nullptr);

        CPPUNIT_ASSERT_MESSAGE("XAttr.getSpecified", xAttr->getSpecified());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->getName()", u"foo"_ustr, xAttr->getName());

        CPPUNIT_ASSERT_MESSAGE("XAttr.getOwnerElement()", xAttr->getOwnerElement() == nullptr);

        Reference<XElement> xElemFoo = xDoc->createElement(u"foo"_ustr);
        Reference<XNode> xInserted = xElemFoo->appendChild(xAttr);
        Reference<XAttr> xAttrIns(xInserted, UNO_QUERY);
        CPPUNIT_ASSERT(xAttrIns != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getOwnerElement()", xElemFoo, xAttrIns->getOwnerElement());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getValue()", u""_ustr, xAttr->getValue());

        xAttr->setValue(u"bar"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.setValue()", u"bar"_ustr, xAttr->getValue());

        // XNode

        {
            Reference<XNode> xAttrCloneN = xAttr->cloneNode(false);
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(false)", xAttrCloneN != nullptr);
            Reference<XAttr> xAttrClone(xAttrCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(false)", xAttrClone != nullptr);
            // actually the children are copied even if bDeep=false
            // does that make sense for attributes?
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(true)", xAttrClone->hasChildNodes());
            Reference<XNode> xChild = xAttrClone->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(true)", xChild != nullptr);
            Reference<XText> xText(xChild, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(true)", xText != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->cloneNode(true)", u"bar"_ustr,
                                         xText->getNodeValue());
        }
        {
            Reference<XNode> xAttrCloneN = xAttr->cloneNode(true);
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(true)", xAttrCloneN != nullptr);
            Reference<XAttr> xAttrClone(xAttrCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(true)", xAttrClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(true)", xAttrClone->hasChildNodes());
            Reference<XNode> xChild = xAttrClone->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(true)", xChild != nullptr);
            Reference<XText> xText(xChild, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XAttr->cloneNode(true)", xText != nullptr);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->cloneNode(true)", u"bar"_ustr,
                                         xText->getNodeValue());
        }

        CPPUNIT_ASSERT_MESSAGE("XAttr->getAttributes()", xAttr->getAttributes() == nullptr);

        {
            Reference<XNodeList> xChildren = xAttr->getChildNodes();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->getChildNodes()", sal_Int32(1), xChildren->getLength());
            Reference<XNode> xChild = xChildren->item(0);
            CPPUNIT_ASSERT_MESSAGE("XAttr->getChildNodes()", xChild != nullptr);
            Reference<XText> xText(xChild, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XAttr->getChildNodes()", xText != nullptr);

            Reference<XNode> xFirst = xAttr->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XAttr->getFirstChild()", xText == xFirst);
            Reference<XNode> xLast = xAttr->getLastChild();
            CPPUNIT_ASSERT_MESSAGE("XAttr.getLastChild()", xText == xLast);
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->getLocalName()", u"foo"_ustr, xAttr->getLocalName());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->getLocalName()", u"foo"_ustr, xAttrNs->getLocalName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->getNamespaceURI()", u""_ustr,
                                     xAttr->getNamespaceURI());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->getNamespaceURI()", ns, xAttrNs->getNamespaceURI());

        CPPUNIT_ASSERT_MESSAGE("XAttr.getNextSibling()", xAttr->getNextSibling() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getNodeName()", u"foo"_ustr, xAttr->getNodeName());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getNodeName()", u"foo"_ustr, xAttrNs->getNodeName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getNodeType()", NodeType_ATTRIBUTE_NODE,
                                     xAttr->getNodeType());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getNodeValue()", u"bar"_ustr, xAttr->getNodeValue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getNodeValue()", u""_ustr, xAttrNs->getNodeValue());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getOwnerDocument()", xDoc, xDoc->getOwnerDocument());

        CPPUNIT_ASSERT_MESSAGE("XAttr.getParentNode()", xAttr->getParentNode() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getPrefix()", u""_ustr, xAttr->getPrefix());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.getPrefix()", u"e"_ustr, xAttrNs->getPrefix());

        CPPUNIT_ASSERT_MESSAGE("XAttr.getPreviousSibling()",
                               xAttr->getPreviousSibling() == nullptr);

        CPPUNIT_ASSERT_MESSAGE("XAttr->hasAttributes()", !xAttr->hasAttributes());

        CPPUNIT_ASSERT_MESSAGE("XAttr->hasChildNodes()", xAttr->hasChildNodes());

        CPPUNIT_ASSERT_MESSAGE("XAttr.isSupported()",
                               !xAttr->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xAttr->normalize();

        xAttr->setNodeValue(u"42"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.setNodeValue()", u"42"_ustr, xAttr->getNodeValue());

        xAttrNs->setPrefix(u"f"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.setPrefix()", u"f"_ustr, xAttrNs->getPrefix());

        Reference<XText> xText = xDoc->createTextNode(u"baz"_ustr);
        Reference<XText> xTextNew = xDoc->createTextNode(u"quux"_ustr);

        try
        {
            xAttr->appendChild(nullptr);
            CPPUNIT_FAIL("XAttr.appendChild(null)");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xAttr->insertBefore(nullptr, xText);
            CPPUNIT_FAIL("XAttr.insertBefore(null,)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xAttr->insertBefore(xText, nullptr);
            CPPUNIT_FAIL("XAttr.insertBefore(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xAttr->insertBefore(xText, xText);
            CPPUNIT_FAIL("XAttr.insertBefore(x, x)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.insertBefore(x, x)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        Reference<XNode> xChild = xAttr->getFirstChild();
        CPPUNIT_ASSERT(xChild != nullptr);

        {
            Reference<XNode> xRet = xAttr->insertBefore(xText, xChild);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.insertBefore(xText, xChild)",
                                   xRet, xChild); // why does this return the old node?
            CPPUNIT_ASSERT_MESSAGE("XAttr.insertBefore(xText, xChild)",
                                   xText == xAttr->getFirstChild());
            CPPUNIT_ASSERT_MESSAGE("XAttr.insertBefore(xText, xChild)",
                                   xAttr == xText->getParentNode());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.insertBefore(xText, xChild)",
                                   xChild, xAttr->getLastChild());
        }

        try
        {
            xAttr->replaceChild(nullptr, xText);
            CPPUNIT_FAIL("XAttr.replaceChild(null, )");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xAttr->replaceChild(xText, nullptr);
            CPPUNIT_FAIL("XAttr.replaceChild(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xAttr->replaceChild(xAttrNs, xAttrNs); // not child
            CPPUNIT_FAIL("XAttr.replaceChild(xAttrNs, xAttrNs)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.replaceChild(xAttrNs, xAttrNs)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        try
        {
            xAttr->replaceChild(xChild, xChild);
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.replaceChild(xChild, xChild)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        Reference<XNode> xReplaced = xAttr->replaceChild(xTextNew, xChild);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr.replaceChild(xTextNew, xChild)", xChild, xReplaced);
        CPPUNIT_ASSERT_MESSAGE("XAttr.replaceChild(xTextNew, xChild)",
                               xText == xAttr->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XAttr.replaceChild(xTextNew, xChild)",
                               xTextNew == xAttr->getLastChild());

        try
        {
            xAttr->removeChild(nullptr);
            CPPUNIT_FAIL("XAttr->removeChild(null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xAttr->removeChild(xAttrNs);
            CPPUNIT_FAIL("XAttr->removeChild()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XAttr->removeChild()",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        Reference<XNode> xRemoved = xAttr->removeChild(xTextNew);
        CPPUNIT_ASSERT_MESSAGE("XAttr->removeChild(xText)", xRemoved == xTextNew);
        CPPUNIT_ASSERT_MESSAGE("XAttr->removeChild(xText)", xAttr->hasChildNodes());
        CPPUNIT_ASSERT_MESSAGE("XAttr->removeChild(xText)", xText == xAttr->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XAttr->removeChild(xText)", xText == xAttr->getLastChild());
    }

    void testXCDataSection()
    {
        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);
        Reference<XDocument> xDoc = xBuilder->newDocument();

        Reference<XCDATASection> xCDS = xDoc->createCDATASection(u"foobar"_ustr);
        CPPUNIT_ASSERT(xCDS != nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.getData", u"foobar"_ustr, xCDS->getData());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection->getLength", sal_Int32(6), xCDS->getLength());

        xCDS->setData(u"foo"_ustr);

        xCDS->appendData(u"baz"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.appendData", u"foobaz"_ustr, xCDS->getData());

        try
        {
            xCDS->deleteData(999, 999);
            CPPUNIT_FAIL("XCDATASection.deleteData(999,999)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.deleteData(999,999)",
                                         DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }
        xCDS->deleteData(0, 3);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.deleteData", u"baz"_ustr, xCDS->getData());

        try
        {
            xCDS->insertData(999, u"blah"_ustr);
            CPPUNIT_FAIL("XCDATASection.insertData(999,\"blah\")");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.insertData(999,\"blah\")",
                                         DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }
        xCDS->insertData(1, u"arb"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.insertData", u"barbaz"_ustr, xCDS->getData());

        try
        {
            xCDS->replaceData(999, 999, u"x"_ustr);
            CPPUNIT_FAIL("XCDATASection.replaceData(999,999,\"x\")");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.replaceData(999,999,\"x\")",
                                         DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }
        xCDS->replaceData(3, 3, u"foo"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.replaceData", u"barfoo"_ustr, xCDS->getData());

        xCDS->setData(u"quux"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.setData", u"quux"_ustr, xCDS->getData());

        try
        {
            xCDS->subStringData(999, 999);
            CPPUNIT_FAIL("XCDATASection.subStringData(999,999)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.subStringData(999,999)",
                                         DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.subStringData", u"x"_ustr,
                                     xCDS->subStringData(3, 1));

        // XNode

        {
            Reference<XNode> xCDSCloneN = xCDS->cloneNode(false);
            CPPUNIT_ASSERT_MESSAGE("XCDATASection->cloneNode(false)", xCDSCloneN != nullptr);
            Reference<XCDATASection> xCDSClone(xCDSCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XCDATASection->cloneNode(false)", xCDSClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XCDATASection->cloneNode(false)", !xCDSClone->hasChildNodes());
        }
        {
            Reference<XNode> xCDSCloneN = xCDS->cloneNode(true);
            CPPUNIT_ASSERT_MESSAGE("XCDATASection->cloneNode(true)", xCDSCloneN != nullptr);
            Reference<XCDATASection> xCDSClone(xCDSCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XCDATASection->cloneNode(true)", xCDSClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XCDATASection->cloneNode(true)", !xCDSClone->hasChildNodes());
        }

        CPPUNIT_ASSERT_MESSAGE("XCDATASection->getAttributes()", xCDS->getAttributes() == nullptr);

        {
            Reference<XNodeList> xChildren = xCDS->getChildNodes();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection->getChildNodes()", sal_Int32(0),
                                         xChildren->getLength());
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection->getLocalName()", u""_ustr,
                                     xCDS->getLocalName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection->getNamespaceURI()", u""_ustr,
                                     xCDS->getNamespaceURI());

        CPPUNIT_ASSERT_MESSAGE("XCDATASection.getNextSibling()", xCDS->getNextSibling() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.getNodeName()", u"#cdata-section"_ustr,
                                     xCDS->getNodeName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.getNodeType()", NodeType_CDATA_SECTION_NODE,
                                     xCDS->getNodeType());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.getNodeValue()", u"quux"_ustr,
                                     xCDS->getNodeValue());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.getOwnerDocument()", xDoc,
                                     xCDS->getOwnerDocument());

        CPPUNIT_ASSERT_MESSAGE("XCDATASection.getParentNode()", xCDS->getParentNode() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.getPrefix()", u""_ustr, xCDS->getPrefix());

        CPPUNIT_ASSERT_MESSAGE("XCDATASection.getPreviousSibling()",
                               xCDS->getPreviousSibling() == nullptr);

        CPPUNIT_ASSERT_MESSAGE("XCDATASection->hasAttributes()", !xCDS->hasAttributes());

        CPPUNIT_ASSERT_MESSAGE("XCDATASection->hasChildNodes()", !xCDS->hasChildNodes());

        CPPUNIT_ASSERT_MESSAGE("XCDATASection.isSupported()",
                               !xCDS->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xCDS->normalize();

        xCDS->setNodeValue(u"42"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.setNodeValue()", u"42"_ustr,
                                     xCDS->getNodeValue());

        try
        {
            xCDS->setPrefix(u"foo"_ustr);
            CPPUNIT_FAIL("XCDATASection.setPrefix()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.setPrefix()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        Reference<XCDATASection> xCDS2 = xDoc->createCDATASection(u"foobar"_ustr);
        Reference<XCDATASection> xCDS3 = xDoc->createCDATASection(u"foobar"_ustr);

        try
        {
            xCDS->appendChild(nullptr);
            CPPUNIT_FAIL("XCDATASection.appendChild(null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xCDS->appendChild(xCDS2);
            CPPUNIT_FAIL("XCDATASection.appendChild(xCDS2)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection.appendChild(xCDS2)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        try
        {
            xCDS->insertBefore(xCDS2, xCDS3);
            CPPUNIT_FAIL("XCDATASection.insertBefore");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xCDS->replaceChild(xCDS2, xCDS3);
            CPPUNIT_FAIL("XCDATASection.insertBefore");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xCDS->removeChild(nullptr);
            CPPUNIT_FAIL("XCDATASection->removeChild(null)");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xCDS->removeChild(xCDS2);
            CPPUNIT_FAIL("XCDATASection->removeChild");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XCDATASection->removeChild",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
    }

    void testXComment()
    {
        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);
        Reference<XDocument> xDoc = xBuilder->newDocument();

        Reference<XComment> xComment = xDoc->createComment(u"foo"_ustr);
        CPPUNIT_ASSERT(xComment != nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.getData", u"foo"_ustr, xComment->getData());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment->getLength", sal_Int32(3), xComment->getLength());

        xComment->appendData(u"baz"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.appendData", u"foobaz"_ustr, xComment->getData());

        try
        {
            xComment->deleteData(999, 999);
            CPPUNIT_FAIL("XComment.deleteData(999,999)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.deleteData(999,999)",
                                         DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }
        xComment->deleteData(0, 3);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.deleteData", u"baz"_ustr, xComment->getData());

        try
        {
            xComment->insertData(999, u"blah"_ustr);
            CPPUNIT_FAIL("XComment.insertData(999,\"blah\")");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.insertData(999,\"blah\")",
                                         DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }
        xComment->insertData(1, u"arb"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.insertData", u"barbaz"_ustr, xComment->getData());

        try
        {
            xComment->replaceData(999, 999, u"x"_ustr);
            CPPUNIT_FAIL("XComment.replaceData(999,999,\"x\")");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.replaceData(999,999,\"x\")",
                                         DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }
        xComment->replaceData(3, 3, u"foo"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.replaceData", u"barfoo"_ustr, xComment->getData());

        xComment->setData(u"quux"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.setData", u"quux"_ustr, xComment->getData());

        try
        {
            xComment->subStringData(999, 999);
            CPPUNIT_FAIL("XComment.subStringData(999,999)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.subStringData(999,999)",
                                         DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.subStringData", u"x"_ustr,
                                     xComment->subStringData(3, 1));

        // XNode

        {
            Reference<XNode> xCommentCloneN = xComment->cloneNode(false);
            CPPUNIT_ASSERT_MESSAGE("XComment->cloneNode(false)", xCommentCloneN != nullptr);
            Reference<XComment> xCommentClone(xCommentCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XComment->cloneNode(false)", xCommentClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XComment->cloneNode(false)", !xCommentClone->hasChildNodes());
        }
        {
            Reference<XNode> xCommentCloneN = xComment->cloneNode(true);
            CPPUNIT_ASSERT_MESSAGE("XComment->cloneNode(true)", xCommentCloneN != nullptr);
            Reference<XComment> xCommentClone(xCommentCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XComment->cloneNode(true)", xCommentClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XComment->cloneNode(true)", !xCommentClone->hasChildNodes());
        }

        CPPUNIT_ASSERT_MESSAGE("XComment->getAttributes()", xComment->getAttributes() == nullptr);

        {
            Reference<XNodeList> xChildren = xComment->getChildNodes();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment->getChildNodes()", sal_Int32(0), xChildren->getLength());
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment->getLocalName()", u""_ustr,
                                     xComment->getLocalName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment->getNamespaceURI()", u""_ustr,
                                     xComment->getNamespaceURI());

        CPPUNIT_ASSERT_MESSAGE("XComment.getNextSibling()", xComment->getNextSibling() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.getNodeName()", u"#comment"_ustr,
                                     xComment->getNodeName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.getNodeType()", NodeType_COMMENT_NODE,
                                     xComment->getNodeType());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.getNodeValue()", u"quux"_ustr,
                                     xComment->getNodeValue());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.getOwnerDocument()", xDoc,
                                     xComment->getOwnerDocument());

        CPPUNIT_ASSERT_MESSAGE("XComment.getParentNode()", xComment->getParentNode() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.getPrefix()", u""_ustr, xComment->getPrefix());

        CPPUNIT_ASSERT_MESSAGE("XComment.getPreviousSibling()",
                               xComment->getPreviousSibling() == nullptr);

        CPPUNIT_ASSERT_MESSAGE("XComment->hasAttributes()", !xComment->hasAttributes());

        CPPUNIT_ASSERT_MESSAGE("XComment->hasChildNodes()", !xComment->hasChildNodes());

        CPPUNIT_ASSERT_MESSAGE(
            "XComment.isSupported()",
            !xComment->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xComment->normalize();

        xComment->setNodeValue(u"42"_ustr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.setNodeValue()", u"42"_ustr,
                                     xComment->getNodeValue());

        try
        {
            xComment->setPrefix(u"foo"_ustr);
            CPPUNIT_FAIL("XComment.setPrefix()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.setPrefix()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        Reference<XComment> xComment2 = xDoc->createComment(u"foobar"_ustr);
        Reference<XComment> xComment3 = xDoc->createComment(u"foobar"_ustr);

        try
        {
            xComment->appendChild(nullptr);
            CPPUNIT_FAIL("XComment.appendChild(null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xComment->appendChild(xComment2);
            CPPUNIT_FAIL("XComment.appendChild(xComment2)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment.appendChild(xComment2)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        try
        {
            xComment->insertBefore(xComment2, xComment3);
            CPPUNIT_FAIL("XComment.insertBefore");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xComment->replaceChild(xComment2, xComment3);
            CPPUNIT_FAIL("XComment.insertBefore");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xComment->removeChild(nullptr);
            CPPUNIT_FAIL("XComment->removeChild(null)");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xComment->removeChild(xComment2);
            CPPUNIT_FAIL("XComment->removeChild");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XComment->removeChild",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
    }

    void testXEntityReference()
    {
        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);
        Reference<XDocument> xDoc = xBuilder->newDocument();

        Reference<XEntityReference> xER = xDoc->createEntityReference(u"foobar"_ustr);
        CPPUNIT_ASSERT(xER != nullptr);

        Reference<XEntityReference> xERChild = xDoc->createEntityReference(u"baz"_ustr);
        CPPUNIT_ASSERT(xERChild != nullptr);

        xER->appendChild(xERChild);

        // XNode

        Reference<XText> xText = xDoc->createTextNode(u"foo"_ustr);
        Reference<XComment> xComment = xDoc->createComment(u"foo"_ustr);

        {
            Reference<XNode> xERCloneN = xER->cloneNode(false);
            CPPUNIT_ASSERT_MESSAGE("XEntityReference->cloneNode(false)", xERCloneN != nullptr);
            Reference<XEntityReference> xERClone(xERCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XEntityReference->cloneNode(false)", xERClone != nullptr);
            CPPUNIT_ASSERT_MESSAGE("XEntityReference->cloneNode(false)",
                                   !xERClone->hasChildNodes());
            CPPUNIT_ASSERT_MESSAGE("XEntityReference->cloneNode(false)",
                                   xERClone->getFirstChild() == nullptr);
        }
        {
            Reference<XNode> xERCloneN = xER->cloneNode(true);
            CPPUNIT_ASSERT_MESSAGE("XEntityReference->cloneNode(true)", xERCloneN != nullptr);
            Reference<XEntityReference> xERClone(xERCloneN, UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XEntityReference->cloneNode(true)", xERClone != nullptr);
        }

        CPPUNIT_ASSERT_MESSAGE("XEntityReference->getAttributes()",
                               xER->getAttributes() == nullptr);

        {
            Reference<XNodeList> xChildren = xER->getChildNodes();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference->getChildNodes()", sal_Int32(1),
                                         xChildren->getLength());
            CPPUNIT_ASSERT_MESSAGE("XEntityReference->getChildNodes()",
                                   xERChild == xChildren->item(0));

            Reference<XNode> xFirst = xER->getFirstChild();
            CPPUNIT_ASSERT_MESSAGE("XEntityReference->getFirstChild()", xERChild == xFirst);
            Reference<XNode> xLast = xER->getLastChild();
            CPPUNIT_ASSERT_MESSAGE("XEntityReference.getLastChild()", xERChild == xLast);
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference->getLocalName()", u""_ustr,
                                     xER->getLocalName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference->getNamespaceURI()", u""_ustr,
                                     xER->getNamespaceURI());

        CPPUNIT_ASSERT_MESSAGE("XEntityReference.getNextSibling()",
                               xER->getNextSibling() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.getNodeName()", u"foobar"_ustr,
                                     xER->getNodeName());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.getNodeType()",
                                     NodeType_ENTITY_REFERENCE_NODE, xER->getNodeType());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.getNodeValue()", u""_ustr,
                                     xER->getNodeValue());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.getOwnerDocument()", xDoc,
                                     xER->getOwnerDocument());

        CPPUNIT_ASSERT_MESSAGE("XEntityReference.getParentNode()", xER->getParentNode() == nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.getPrefix()", u""_ustr, xER->getPrefix());

        CPPUNIT_ASSERT_MESSAGE("XEntityReference.getPreviousSibling()",
                               xER->getPreviousSibling() == nullptr);

        CPPUNIT_ASSERT_MESSAGE("XEntityReference->hasAttributes()", !xER->hasAttributes());

        CPPUNIT_ASSERT_MESSAGE("XEntityReference->hasChildNodes()", xER->hasChildNodes());

        CPPUNIT_ASSERT_MESSAGE("XEntityReference.isSupported()",
                               !xER->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xER->normalize();

        try
        {
            xER->setNodeValue(u"42"_ustr);
            CPPUNIT_FAIL("XEntityReference.setNodeValue()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.setNodeValue()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        try
        {
            xER->setPrefix(u"foo"_ustr);
            CPPUNIT_FAIL("XEntityReference.setPrefix()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.setPrefix()",
                                         DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        try
        {
            xER->appendChild(nullptr);
            CPPUNIT_FAIL("XEntityReference.appendChild(null)");
        }
        catch (Exception)
        { /* expected */
        }

        try
        {
            xER->insertBefore(nullptr, xText);
            CPPUNIT_FAIL("XEntityReference.insertBefore(null,)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xER->insertBefore(xText, nullptr);
            CPPUNIT_FAIL("XEntityReference.insertBefore(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xER->insertBefore(xText, xText);
            CPPUNIT_FAIL("XEntityReference.insertBefore(x, x)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.insertBefore(x, x)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        {
            Reference<XNode> xRet = xER->insertBefore(xComment, xERChild);
            CPPUNIT_ASSERT_MESSAGE("XEntityReference.insertBefore(xComment, xERChild)",
                                   xRet == xERChild); // why does this return the old node?
            CPPUNIT_ASSERT_MESSAGE("XEntityReference.insertBefore(xComment, xERChild)",
                                   xComment == xER->getFirstChild());
            CPPUNIT_ASSERT_MESSAGE("XEntityReference.insertBefore(xComment, xERChild)",
                                   xER == xComment->getParentNode());
            CPPUNIT_ASSERT_MESSAGE("XEntityReference.insertBefore(xCommnet, xERChild)",
                                   xERChild == xER->getLastChild());
        }

        try
        {
            xER->replaceChild(nullptr, xText);
            CPPUNIT_FAIL("XEntityReference.replaceChild(null, )");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xER->replaceChild(xText, nullptr);
            CPPUNIT_FAIL("XEntityReference.replaceChild(, null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xER->replaceChild(xText, xText); // not child
            CPPUNIT_FAIL("XEntityReference.replaceChild(xElemFoo, xElemFoo)");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.replaceChild(xElemFoo, xElemFoo)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        try
        {
            xER->replaceChild(xERChild, xERChild);
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference.replaceChild(xERChild, xERChild)",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
        Reference<XNode> xReplaced = xER->replaceChild(xText, xComment);
        CPPUNIT_ASSERT_MESSAGE("XEntityReference.replaceChild(xText, xComment)",
                               xReplaced == xComment);
        CPPUNIT_ASSERT_MESSAGE("XEntityReference.replaceChild(xText, xComment)",
                               xText == xER->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XEntityReference.replaceChild(xText, xComment)",
                               xERChild == xER->getLastChild());

        try
        {
            xER->removeChild(nullptr);
            CPPUNIT_FAIL("XEntityReference->removeChild(null)");
        }
        catch (Exception)
        { /* expected */
        }
        try
        {
            xER->removeChild(xER);
            CPPUNIT_FAIL("XEntityReference->removeChild()");
        }
        catch (DOMException e)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("XEntityReference->removeChild()",
                                         DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        Reference<XNode> xRemoved = xER->removeChild(xText);
        CPPUNIT_ASSERT_MESSAGE("XEntityReference->removeChild(xText)", xRemoved == xText);
        CPPUNIT_ASSERT_MESSAGE("XEntityReference->removeChild(xText)", xER->hasChildNodes());
        CPPUNIT_ASSERT_MESSAGE("XEntityReference->removeChild(xText)",
                               xERChild == xER->getFirstChild());
        CPPUNIT_ASSERT_MESSAGE("XEntityReference->removeChild(xText)",
                               xERChild == xER->getLastChild());
    }

    void testXNodeList_ElementList()
    {
        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);
        Reference<XDocument> xDoc = xBuilder->newDocument();

        Reference<XElement> xRoot = xDoc->createElement(u"root"_ustr);
        Reference<XElement> xBar = xDoc->createElement(u"bar"_ustr);
        Reference<XElement> xFoo1 = xDoc->createElement(u"foo"_ustr);
        Reference<XElement> xFoo2 = xDoc->createElement(u"foo"_ustr);
        Reference<XElement> xFoo3 = xDoc->createElement(u"foo"_ustr);

        xDoc->appendChild(xRoot);

        Reference<XNodeList> xElementList = xRoot->getElementsByTagName(u"foo"_ustr);
        CPPUNIT_ASSERT(xElementList != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ElementList->getLength()", sal_Int32(0), xElementList->getLength());

        try
        {
            xElementList->item(4);
        }
        catch (Exception)
        { /* expected */
        }

        xRoot->appendChild(xFoo1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ElementList->getLength()", sal_Int32(1), xElementList->getLength());
        CPPUNIT_ASSERT_MESSAGE("ElementList.item", xFoo1 == xElementList->item(0));

        xFoo1->appendChild(xBar);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ElementList->getLength()", sal_Int32(1), xElementList->getLength());
        CPPUNIT_ASSERT_MESSAGE("ElementList.item", xFoo1 == xElementList->item(0));

        xRoot->appendChild(xFoo3);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ElementList->getLength()", sal_Int32(2), xElementList->getLength());
        CPPUNIT_ASSERT_MESSAGE("ElementList.item", xFoo1 == xElementList->item(0));
        CPPUNIT_ASSERT_MESSAGE("ElementList.item", xFoo3 == xElementList->item(1));

        xBar->appendChild(xFoo2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ElementList->getLength()", sal_Int32(3), xElementList->getLength());
        CPPUNIT_ASSERT_MESSAGE("ElementList.item", xFoo1 == xElementList->item(0));
        CPPUNIT_ASSERT_MESSAGE("ElementList.item", xFoo2 == xElementList->item(1));
        CPPUNIT_ASSERT_MESSAGE("ElementList.item", xFoo3 == xElementList->item(2));

        xRoot->removeChild(xFoo1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ElementList->getLength()", sal_Int32(1), xElementList->getLength());
        CPPUNIT_ASSERT_MESSAGE("ElementList.item", xFoo3 == xElementList->item(0));
    }

    void testXSAXSerialize()
    {
        const char* const TestString
            = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
              "<office:document-content "
              "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" "
              "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
              "xmlns=\"\" "
              "office:version=\"1.0\">"
              "<office:scripts/>"
              "<xlink:test/>"
              "<office:automatic-styles teststyle=\"test\"/>"
              "<moretest/>"
              "some text \u00F6\u00E4\u00FC"
              "</office:document-content>";
        Sequence<sal_Int8> seqBytes(strlen(TestString));
        memcpy(seqBytes.getArray(), TestString, strlen(TestString));

        Reference<XDocumentBuilder> xBuilder(
            getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr),
            UNO_QUERY_THROW);

        Reference<css::io::XInputStream> xIn
            = css::io::SequenceInputStream::createStreamFromSequence(m_xContext, seqBytes);

        Reference<XDocument> xDoc = xBuilder->parse(xIn);

        Reference<XDocumentHandler> xHandler(new DummyDocumentHandler);

        Reference<XSAXSerializable> serializable(xDoc, UNO_QUERY);

        serializable->serialize(xHandler, {});
    }

    CPPUNIT_TEST_SUITE(BasicTest);
    CPPUNIT_TEST(testXSAXDocumentBuilder);
    CPPUNIT_TEST(testXDocument);
    CPPUNIT_TEST(testXDocumentFragment);
    CPPUNIT_TEST(testXElement);
    CPPUNIT_TEST(testXAttr);
    CPPUNIT_TEST(testXCDataSection);
    CPPUNIT_TEST(testXComment);
    CPPUNIT_TEST(testXEntityReference);
    CPPUNIT_TEST(testXNodeList_ElementList);
    CPPUNIT_TEST(testXSAXSerialize);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasicTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
