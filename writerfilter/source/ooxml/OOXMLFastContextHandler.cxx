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

#include <stdio.h>
#include <iostream>
#include <set>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <resourcemodel/QNameToString.hxx>
#include <resourcemodel/XPathLogger.hxx>
#include <resourcemodel/util.hxx>
#include <ooxml/resourceids.hxx>
#include <doctok/sprmids.hxx>
#include <ooxml/OOXMLnamespaceids.hxx>
#include <dmapper/DomainMapper.hxx>
#include <dmapper/GraphicHelpers.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include "OOXMLFastContextHandler.hxx"
#include "OOXMLFactory.hxx"
#include "Handler.hxx"
#include "ooxmlLoggers.hxx"

static const sal_uInt8 s0x7[] = { 0x7, 0x0 };
static const sal_uInt8 s0xd[] = { 0xd, 0x0 };
static const sal_uInt8 sCR[] = { 0xd, 0x0 };
static const sal_uInt8 sFtnEdnRef[] = { 0x2, 0x0 };
static const sal_uInt8 sFtnEdnSep[] = { 0x3, 0x0 };
static const sal_uInt8 sFtnEdnCont[] = { 0x4, 0x0 };
static const sal_uInt8 sTab[] = { 0x9, 0x0 };
static const sal_uInt8 sPgNum[] = { 0x0, 0x0 };
static const sal_uInt8 sFieldStart[] = { 0x13  };
static const sal_uInt8 sFieldSep[] = { 0x14 };
static const sal_uInt8 sFieldEnd[] = { 0x15 };
static const sal_uInt8 sNoBreakHyphen[] = { 0x1e, 0x0 };
static const sal_uInt8 sSoftHyphen[] = { 0x1f, 0x0 };

namespace writerfilter {
namespace ooxml
{
using ::com::sun::star::lang::XMultiComponentFactory;
using namespace ::com::sun::star;
using namespace ::std;

#if OSL_DEBUG_LEVEL > 1
static string resourceToString
(OOXMLFastContextHandler::ResourceEnum_t eResource)
{
    string sResult;

    switch (eResource)
    {
    case OOXMLFastContextHandler::STREAM:
        sResult = "Stream";
        break;
    case OOXMLFastContextHandler::PROPERTIES:
        sResult = "Properties";
        break;
    case OOXMLFastContextHandler::TABLE:
        sResult = "Table";
        break;
    case OOXMLFastContextHandler::SHAPE:
        sResult = "Shape";
        break;
    default:
        sResult = "??";
    }

    return sResult;
}
#endif

set<OOXMLFastContextHandler *> aSetContexts;

#if OSL_DEBUG_LEVEL > 1
class OOXMLIdToString : public IdToString
{
public:
    OOXMLIdToString() : IdToString() {}
    virtual ~OOXMLIdToString() {}

    virtual string toString(const Id & rId) const
    {
        string s((*QNameToString::Instance())(rId));

        if (s.empty())
            s = "(fasttoken)" + fastTokenToId(rId);
        else
            s = "(qname)" + s;

        return s;
    }
};
#endif

/*
  class OOXMLFastContextHandler
 */

sal_uInt32 OOXMLFastContextHandler::mnInstanceCount = 0;

OOXMLFastContextHandler::OOXMLFastContextHandler
(uno::Reference< uno::XComponentContext > const & context)
: mpParent(NULL),
  mId(0),
  mnDefine(0),
  mnToken(OOXML_FAST_TOKENS_END),
  mpStream(NULL),
  mnTableDepth(0),
  mnInstanceNumber(mnInstanceCount),
  mnRefCount(0),
  inPositionV(false),
  m_xContext(context)
{
    mnInstanceCount++;
    aSetContexts.insert(this);

    if (mpParserState.get() == NULL)
        mpParserState.reset(new OOXMLParserState());

    mpParserState->incContextCount();
}

OOXMLFastContextHandler::OOXMLFastContextHandler
(OOXMLFastContextHandler * pContext)
: cppu::WeakImplHelper1<com::sun::star::xml::sax::XFastContextHandler>(),
  mpParent(pContext),
  mId(0),
  mnDefine(0),
  mnToken(OOXML_FAST_TOKENS_END),
  mpStream(NULL),
  mnTableDepth(0),
  mnInstanceNumber(mnInstanceCount),
  mnRefCount(0),
  inPositionV(pContext->inPositionV),
  m_xContext(pContext->m_xContext)
{
    if (pContext != NULL)
    {
        mpStream = pContext->mpStream;
        mpParserState = pContext->mpParserState;
        mnTableDepth = pContext->mnTableDepth;
        m_xContext = pContext->m_xContext;
    }

    if (mpParserState.get() == NULL)
        mpParserState.reset(new OOXMLParserState());

    mnInstanceCount++;
    aSetContexts.insert(this);
    mpParserState->incContextCount();
}

OOXMLFastContextHandler::~OOXMLFastContextHandler()
{
    aSetContexts.erase(this);
}

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL OOXMLFastContextHandler::startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.element");
    string sToken = fastTokenToId(Element);
    mpParserState->getXPathLogger().startElement(sToken);
    debug_logger->attribute("token", sToken);
    debug_logger->attribute("type", getType());
    debug_logger->attribute("xpath", mpParserState->getXPathLogger().getXPath());
    debug_logger->startElement("at-start");
    dumpXml( debug_logger );
    debug_logger->endElement();
#endif
    attributes(Attribs);
    lcl_startFastElement(Element, Attribs);
}

void SAL_CALL OOXMLFastContextHandler::startUnknownElement
(const OUString & Namespace, const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.unknown-element");
    debug_logger->attribute("namespace", Namespace);
    debug_logger->attribute("name", Name);
    mpParserState->getXPathLogger().startElement("unknown");
#else
    (void) Namespace;
    (void) Name;
#endif
}

void SAL_CALL OOXMLFastContextHandler::endFastElement(Token_t Element)
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_HANDLER
    string sToken = fastTokenToId(Element);
    (void) sToken;
#endif

    lcl_endFastElement(Element);

#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("at-end");
    dumpXml( debug_logger );
    debug_logger->endElement();
    debug_logger->endElement();
    mpParserState->getXPathLogger().endElement();
#endif
}

void OOXMLFastContextHandler::lcl_startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    OOXMLFactory::getInstance()->startAction(this, Element);
    if( Element == (NS_wordprocessingDrawing|OOXML_positionV) )
        inPositionV = true;
    else if( Element == (NS_wordprocessingDrawing|OOXML_positionH) )
        inPositionV = false;

}

void OOXMLFastContextHandler::lcl_endFastElement
(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    OOXMLFactory::getInstance()->endAction(this, Element);
}

void SAL_CALL OOXMLFastContextHandler::endUnknownElement
(const OUString & , const OUString & )
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
    mpParserState->getXPathLogger().endElement();
#endif
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
 OOXMLFastContextHandler::createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.createFastChildContext");
    debug_logger->attribute("token", fastTokenToId(Element));
    debug_logger->attribute("type", getType());
#endif

    uno::Reference< xml::sax::XFastContextHandler > xResult
        (lcl_createFastChildContext(Element, Attribs));

#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif

    return xResult;
}

uno::Reference< xml::sax::XFastContextHandler >
 OOXMLFastContextHandler::lcl_createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    return OOXMLFactory::getInstance()->createFastChildContext(this, Element);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandler::createUnknownChildContext
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.createUnknownChildContext");
    debug_logger->attribute("namespace", Namespace);
    debug_logger->attribute("name", Name);
    debug_logger->endElement();
#else
    (void) Namespace;
    (void) Name;
#endif

    return uno::Reference< xml::sax::XFastContextHandler >
        (new OOXMLFastContextHandler(*const_cast<const OOXMLFastContextHandler *>(this)));
}

void SAL_CALL OOXMLFastContextHandler::characters
(const OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    lcl_characters(aChars);
}

void OOXMLFastContextHandler::lcl_characters
(const OUString & rString)
throw (uno::RuntimeException, xml::sax::SAXException)
{
    OOXMLFactory::getInstance()->characters(this, rString);
}

namespace
{
    class theOOXMLFastContextHandlerUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theOOXMLFastContextHandlerUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & OOXMLFastContextHandler::getUnoTunnelId()
{
    return theOOXMLFastContextHandlerUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL OOXMLFastContextHandler::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>
            (reinterpret_cast<sal_IntPtr>(this));
    }

    return 0x0;
}

void OOXMLFastContextHandler::setStream(Stream * pStream)
{
    mpStream = pStream;
}

OOXMLValue::Pointer_t OOXMLFastContextHandler::getValue() const
{
    return OOXMLValue::Pointer_t();
}

void OOXMLFastContextHandler::attributes
(const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
 throw (uno::RuntimeException, xml::sax::SAXException)
{
    OOXMLFactory::getInstance()->attributes(this, Attribs);
}

void OOXMLFastContextHandler::startAction(Token_t Element)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.startAction");
#endif
    lcl_startAction(Element);
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif
}

void OOXMLFastContextHandler::lcl_startAction(Token_t Element)
{
    OOXMLFactory::getInstance()->startAction(this, Element);
}

void OOXMLFastContextHandler::endAction(Token_t Element)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.endAction");
#endif
    lcl_endAction(Element);
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif
}

void OOXMLFastContextHandler::lcl_endAction(Token_t Element)
{
    OOXMLFactory::getInstance()->endAction(this, Element);
}

#if OSL_DEBUG_LEVEL > 1
void OOXMLFastContextHandler::dumpXml( const TagLogger::Pointer_t pLogger ) const
{
    pLogger->startElement("context");

    static char sBuffer[128];
    snprintf(sBuffer, sizeof(sBuffer), "%p", this);

    pLogger->attribute("parent", std::string(sBuffer));
    pLogger->attribute("type", getType());
    pLogger->attribute("resource", resourceToString(getResource()));
    pLogger->attribute("token", fastTokenToId(getToken()));
    pLogger->attribute("id", (*QNameToString::Instance())(getId()));

    OOXMLValue::Pointer_t pVal(getValue());

    if (pVal.get() != NULL)
        pLogger->attribute("value", pVal->toString());
    else
        pLogger->attribute("value", std::string("(null)"));

    pLogger->propertySet(getPropertySet(),
            IdToString::Pointer_t(new OOXMLIdToString()));

    mpParserState->dumpXml( pLogger );

    pLogger->endElement();
}

#endif

void OOXMLFastContextHandler::setId(Id rId)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.setId");

    static char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%" SAL_PRIuUINT32, rId);

    debug_logger->attribute("id", std::string(sBuffer));
    debug_logger->attribute("name", (*QNameToString::Instance())(rId));
    debug_logger->endElement();
#endif

    mId = rId;
}

Id OOXMLFastContextHandler::getId() const
{
    return mId;
}

void OOXMLFastContextHandler::setDefine(Id nDefine)
{
    mnDefine = nDefine;
}

Id OOXMLFastContextHandler::getDefine() const
{
    return mnDefine;
}

OOXMLParserState::Pointer_t OOXMLFastContextHandler::getParserState() const
{
    return mpParserState;
}

void OOXMLFastContextHandler::setToken(Token_t nToken)
{
    mnToken = nToken;

#ifdef DEBUG_CONTEXT_HANDLER
    msTokenString = fastTokenToId(mnToken);
#endif
}

Token_t OOXMLFastContextHandler::getToken() const
{
    return mnToken;
}

void OOXMLFastContextHandler::mark(const Id & rId, OOXMLValue::Pointer_t pVal)
{
    OOXMLPropertySetImpl::Pointer_t pPropSet(new OOXMLPropertySetImpl());
    OOXMLPropertyImpl::Pointer_t pProperty
        (new OOXMLPropertyImpl(rId, pVal, OOXMLPropertyImpl::ATTRIBUTE));

    pPropSet->add(pProperty);
    mpStream->props(pPropSet);
}

void OOXMLFastContextHandler::setParent
(OOXMLFastContextHandler * pParent)
{
    mpParent = pParent;
}

OOXMLPropertySet * OOXMLFastContextHandler::getPicturePropSet
(const OUString & rId)
{
    return dynamic_cast<OOXMLDocumentImpl *>(mpParserState->getDocument())->
        getPicturePropSet(rId);
}

void OOXMLFastContextHandler::sendTableDepth() const
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.sendTableDepth");
#endif

    if (mnTableDepth > 0)
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(mnTableDepth));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(NS_sprm::LN_PTableDepth, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(1));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(NS_sprm::LN_PFInTable, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }

        mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
    }
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif
}

void OOXMLFastContextHandler::setHandle()
{
    mpParserState->setHandle();
    mpStream->info(mpParserState->getHandle());
}

void OOXMLFastContextHandler::startCharacterGroup()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.startCharacterGroup");
#endif

    if (isForwardEvents())
    {
        if (mpParserState->isInCharacterGroup())
            endCharacterGroup();

        if (! mpParserState->isInParagraphGroup())
            startParagraphGroup();

        if (! mpParserState->isInCharacterGroup())
        {
            mpStream->startCharacterGroup();
            mpParserState->setInCharacterGroup(true);
            mpParserState->resolveCharacterProperties(*mpStream);
        }
    }
}

void OOXMLFastContextHandler::endCharacterGroup()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.endCharacterGroup");
#endif

    if (isForwardEvents() && mpParserState->isInCharacterGroup())
    {
        mpStream->endCharacterGroup();
        mpParserState->setInCharacterGroup(false);
    }
}

void OOXMLFastContextHandler::startParagraphGroup()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.startParagraphGroup");
#endif

    if (isForwardEvents())
    {
        if (mpParserState->isInParagraphGroup())
            endParagraphGroup();

        if (! mpParserState->isInSectionGroup())
            startSectionGroup();

        if (! mpParserState->isInParagraphGroup())
        {
            mpStream->startParagraphGroup();
            mpParserState->setInParagraphGroup(true);
        }
    }
}

void OOXMLFastContextHandler::endParagraphGroup()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.endParagraphGroup");
#endif

    if (isForwardEvents())
    {
        if (mpParserState->isInCharacterGroup())
            endCharacterGroup();

        if (mpParserState->isInParagraphGroup())
        {
            mpStream->endParagraphGroup();
            mpParserState->setInParagraphGroup(false);
        }
    }
}

void OOXMLFastContextHandler::startSectionGroup()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.startSectionGroup");
#endif

    if (isForwardEvents())
    {
        if (mpParserState->isInSectionGroup())
            endSectionGroup();

        if (! mpParserState->isInSectionGroup())
        {
            mpStream->info(mpParserState->getHandle());
            mpStream->startSectionGroup();
            mpParserState->setInSectionGroup(true);
        }
    }
}

void OOXMLFastContextHandler::endSectionGroup()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.endSectionGroup");
#endif

    if (isForwardEvents())
    {
        if (mpParserState->isInParagraphGroup())
            endParagraphGroup();

        if (mpParserState->isInSectionGroup())
        {
            mpStream->endSectionGroup();
            mpParserState->setInSectionGroup(false);
        }
    }
}

void OOXMLFastContextHandler::setLastParagraphInSection()
{
    mpParserState->setLastParagraphInSection(true);
    mpStream->markLastParagraphInSection( );
}

void OOXMLFastContextHandler::newProperty
(const Id & /*nId*/, OOXMLValue::Pointer_t /*pVal*/)
{
}

void OOXMLFastContextHandler::setPropertySet
(OOXMLPropertySet::Pointer_t /* pPropertySet */)
{
}

OOXMLPropertySet::Pointer_t OOXMLFastContextHandler::getPropertySet() const
{
    return OOXMLPropertySet::Pointer_t();
}

void OOXMLFastContextHandler::startField()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.startField");
#endif
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(sFieldStart, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::fieldSeparator()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.fieldSeparator");
#endif
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(sFieldSep, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::endField()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.endField");
#endif
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(sFieldEnd, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::ftnednref()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.ftnednref");
#endif
    if (isForwardEvents())
        mpStream->utext(sFtnEdnRef, 1);
}

void OOXMLFastContextHandler::ftnednsep()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.ftnednsep");
#endif
    if (isForwardEvents())
        mpStream->utext(sFtnEdnSep, 1);
}

void OOXMLFastContextHandler::ftnedncont()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.ftnedncont");
#endif
    if (isForwardEvents())
        mpStream->text(sFtnEdnCont, 1);
}

void OOXMLFastContextHandler::pgNum()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.pgNum");
#endif
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)sPgNum, 1);
}

void OOXMLFastContextHandler::tab()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.tab");
#endif
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)sTab, 1);
}

void OOXMLFastContextHandler::cr()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.cr");
#endif
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)sCR, 1);
}

void OOXMLFastContextHandler::noBreakHyphen()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.noBreakHyphen");
#endif
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)sNoBreakHyphen, 1);
}

void OOXMLFastContextHandler::softHyphen()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.softHyphen");
#endif
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)sSoftHyphen, 1);
}

void OOXMLFastContextHandler::handleLastParagraphInSection()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.handleLastParagraphInSection");
#endif

    if (mpParserState->isLastParagraphInSection())
    {
        mpParserState->setLastParagraphInSection(false);
        startSectionGroup();
    }
}

void OOXMLFastContextHandler::endOfParagraph()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.endOfParagraph");
#endif
    if (! mpParserState->isInCharacterGroup())
        startCharacterGroup();
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)sCR, 1);
}

void OOXMLFastContextHandler::startTxbxContent()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.startTxbxContent");
#endif
/*
    This usually means there are recursive <w:p> elements, and the ones
    inside and outside of w:txbxContent should not interfere (e.g.
    the lastParagraphInSection setting). So save the whole state
    and possibly start new groups for the nested content (not section
    group though, as that'd cause the txbxContent to be moved onto
    another page, I'm not sure how that should work exactly).
*/
    mpParserState->startTxbxContent();
    startParagraphGroup();
}

void OOXMLFastContextHandler::endTxbxContent()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.endTxbxContent");
#endif
    endParagraphGroup();
    mpParserState->endTxbxContent();
}

void OOXMLFastContextHandler::text(const OUString & sText)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.text");
    debug_logger->chars(sText);
    debug_logger->endElement();
#endif
    if (isForwardEvents())
        mpStream->utext(reinterpret_cast < const sal_uInt8 * >
                        (sText.getStr()),
                        sText.getLength());
}

/*
 HACK. An ugly hack. The problem with wp:positionOffset, wp:alignV and wp:alignH
 is that they do not work in the usual OOXML way of <tag val="value"/> but instead
 it's <tag>value</tag>, which is otherwise used only things like <t>. And I really
 haven't managed to find out how to make this XML parsing monstrosity to handle this
 on its own, so the code is modelled after <t> handling and does it manually in a hackish
 way - it reads the value as text and converts itself, moreover the reading of the value
 is done sooner than lcl_sprms() actually results in processing the tags it is enclosed
 in, so the values are stored in PositionHandler for later use.
*/
void OOXMLFastContextHandler::positionOffset(const OUString & sText)
{
#ifdef DEBUG_ELEMENT
    debug_logger->startElement("positionOffset");
    debug_logger->chars(sText);
    debug_logger->endElement();
#endif
    if (isForwardEvents())
        ::writerfilter::dmapper::PositionHandler::setPositionOffset( sText, inPositionV );
}

void OOXMLFastContextHandler::alignH(const OUString & sText)
{
#ifdef DEBUG_ELEMENT
    debug_logger->startElement("alignH");
    debug_logger->chars(sText);
    debug_logger->endElement();
#endif
    if (isForwardEvents())
        ::writerfilter::dmapper::PositionHandler::setAlignH( sText );
}

void OOXMLFastContextHandler::alignV(const OUString & sText)
{
#ifdef DEBUG_ELEMENT
    debug_logger->startElement("alignV");
    debug_logger->chars(sText);
    debug_logger->endElement();
#endif
    if (isForwardEvents())
        ::writerfilter::dmapper::PositionHandler::setAlignV( sText );
}

void OOXMLFastContextHandler::propagateCharacterProperties()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.propagateCharacterProperties");
    debug_logger->propertySet(getPropertySet(),
            IdToString::Pointer_t(new OOXMLIdToString()));
    debug_logger->endElement();
#endif

    mpParserState->setCharacterProperties(getPropertySet());
}

void OOXMLFastContextHandler::propagateCharacterPropertiesAsSet(const Id & rId)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.propagateCharacterPropertiesAsSet");
    debug_logger->propertySet(getPropertySet(),
            IdToString::Pointer_t(new OOXMLIdToString()));
    debug_logger->endElement();
#endif

    OOXMLValue::Pointer_t pValue(new OOXMLPropertySetValue(getPropertySet()));
    OOXMLPropertySet::Pointer_t pPropertySet(new OOXMLPropertySetImpl());

    OOXMLProperty::Pointer_t pProp
        (new OOXMLPropertyImpl(rId, pValue, OOXMLPropertyImpl::SPRM));

    pPropertySet->add(pProp);
    mpParserState->setCharacterProperties(pPropertySet);
}

bool OOXMLFastContextHandler::propagatesProperties() const
{
    return false;
}

void OOXMLFastContextHandler::propagateCellProperties()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.propagateCellProperties");
#endif

    mpParserState->setCellProperties(getPropertySet());
}

void OOXMLFastContextHandler::propagateRowProperties()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.propagateRowProperties");
#endif

    mpParserState->setRowProperties(getPropertySet());
}

void OOXMLFastContextHandler::propagateTableProperties()
{
    OOXMLPropertySet::Pointer_t pProps = getPropertySet();
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.propagateTableProperties");
    debug_logger->propertySet(getPropertySet(),
            IdToString::Pointer_t(new OOXMLIdToString()));
    debug_logger->endElement();
#endif

    mpParserState->setTableProperties(pProps);
}

void OOXMLFastContextHandler::sendCellProperties()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.sendCellProperties");
#endif

    mpParserState->resolveCellProperties(*mpStream);

#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif
}

void OOXMLFastContextHandler::sendRowProperties()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.sendRowProperties");
#endif

    mpParserState->resolveRowProperties(*mpStream);

#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif
}

void OOXMLFastContextHandler::sendTableProperties()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.sendTableProperties");
#endif

    mpParserState->resolveTableProperties(*mpStream);

#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif
}

void OOXMLFastContextHandler::clearTableProps()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.clearTableProps");
#endif

    mpParserState->setTableProperties(OOXMLPropertySet::Pointer_t
                                     (new OOXMLPropertySetImpl()));
}

void OOXMLFastContextHandler::sendPropertiesWithId(const Id & rId)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.sendPropertiesWithId");
    debug_logger->attribute("id", fastTokenToId(rId));
#endif

    OOXMLValue::Pointer_t pValue(new OOXMLPropertySetValue(getPropertySet()));
    OOXMLPropertySet::Pointer_t pPropertySet(new OOXMLPropertySetImpl());

    OOXMLProperty::Pointer_t pProp
    (new OOXMLPropertyImpl(rId, pValue, OOXMLPropertyImpl::SPRM));

    pPropertySet->add(pProp);
    mpStream->props(pPropertySet);

#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->propertySet(getPropertySet(),
            IdToString::Pointer_t(new OOXMLIdToString()));
    debug_logger->endElement();
#endif
}

void OOXMLFastContextHandler::clearProps()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("contexthandler.clearProps");
#endif

    setPropertySet(OOXMLPropertySet::Pointer_t(new OOXMLPropertySetImpl()));
}

void OOXMLFastContextHandler::setDefaultBooleanValue()
{
}

void OOXMLFastContextHandler::setDefaultIntegerValue()
{
}

void OOXMLFastContextHandler::setDefaultHexValue()
{
}

void OOXMLFastContextHandler::setDefaultStringValue()
{
}

void OOXMLFastContextHandler::setDocument(OOXMLDocument * pDocument)
{
    mpParserState->setDocument(pDocument);
}

OOXMLDocument * OOXMLFastContextHandler::getDocument()
{
    return mpParserState->getDocument();
}

void OOXMLFastContextHandler::setForwardEvents(bool bForwardEvents)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.setForwardEvents");

    if (bForwardEvents)
        debug_logger->chars(std::string("true"));
    else
        debug_logger->chars(std::string("false"));

    debug_logger->endElement();
#endif

    mpParserState->setForwardEvents(bForwardEvents);
}

bool OOXMLFastContextHandler::isForwardEvents() const
{
    return mpParserState->isForwardEvents();
}

void OOXMLFastContextHandler::setXNoteId(const sal_Int32 nId)
{
    mpParserState->setXNoteId(nId);
}

void OOXMLFastContextHandler::setXNoteId(OOXMLValue::Pointer_t pValue)
{
    mpParserState->setXNoteId(sal_Int32(pValue->getInt()));
}

sal_Int32 OOXMLFastContextHandler::getXNoteId() const
{
    return mpParserState->getXNoteId();
}

void OOXMLFastContextHandler::resolveFootnote
(const sal_Int32 nId)
{
    mpParserState->getDocument()->resolveFootnote
        (*mpStream, 0, nId);
}

void OOXMLFastContextHandler::resolveEndnote(const sal_Int32 nId)
{
    mpParserState->getDocument()->resolveEndnote
        (*mpStream, 0, nId);
}

void OOXMLFastContextHandler::resolveComment(const sal_Int32 nId)
{
    mpParserState->getDocument()->resolveComment(*mpStream, nId);
}

void OOXMLFastContextHandler::resolvePicture(const OUString & rId)
{
    mpParserState->getDocument()->resolvePicture(*mpStream, rId);
}

void OOXMLFastContextHandler::resolveHeader
(const sal_Int32 type, const OUString & rId)
{
    mpParserState->getDocument()->resolveHeader(*mpStream, type, rId);
}

void OOXMLFastContextHandler::resolveFooter
(const sal_Int32 type, const OUString & rId)
{
    mpParserState->getDocument()->resolveFooter(*mpStream, type, rId);
}

// Add the data pointed to by the reference as another property.
void OOXMLFastContextHandler::resolveData(const OUString & rId)
{
    uno::Reference<io::XInputStream> xInputStream
        (mpParserState->getDocument()->getInputStreamForId(rId));

    OOXMLValue::Pointer_t aValue(new OOXMLInputStreamValue(xInputStream));

    newProperty(NS_ooxml::LN_inputstream, aValue);
}

OUString OOXMLFastContextHandler::getTargetForId
(const OUString & rId)
{
    return mpParserState->getDocument()->getTargetForId(rId);
}

void OOXMLFastContextHandler::resolvePropertySetAttrs()
{
}

void OOXMLFastContextHandler::sendPropertyToParent()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->element("sendPropertyToParent");
#endif

    if (mpParent != NULL)
    {
        OOXMLPropertySet::Pointer_t pProps(mpParent->getPropertySet());

        if (pProps.get() != NULL)
        {
            OOXMLProperty::Pointer_t
                pProp(new OOXMLPropertyImpl(mId, getValue(),
                                            OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
    }
}

void OOXMLFastContextHandler::sendPropertiesToParent()
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.sendPropertiesToParent");
#endif
    if (mpParent != NULL)
    {
        OOXMLPropertySet::Pointer_t pParentProps(mpParent->getPropertySet());

        if (pParentProps.get() != NULL)
        {
            OOXMLPropertySet::Pointer_t pProps(getPropertySet());

            if (pProps.get() != NULL)
            {
                OOXMLValue::Pointer_t pValue
                (new OOXMLPropertySetValue(getPropertySet()));

                OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(getId(), pValue, OOXMLPropertyImpl::SPRM));


                pParentProps->add(pProp);

            }
        }
    }
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif
}

uno::Reference< uno::XComponentContext >
OOXMLFastContextHandler::getComponentContext()
{
    return m_xContext;
}

/*
  class OOXMLFastContextHandlerStream
 */

OOXMLFastContextHandlerStream::OOXMLFastContextHandlerStream
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext),
  mpPropertySetAttrs(new OOXMLPropertySetImpl())
{
}

OOXMLFastContextHandlerStream::~OOXMLFastContextHandlerStream()
{
}

void OOXMLFastContextHandlerStream::newProperty(const Id & rId,
                                                OOXMLValue::Pointer_t pVal)
{
    if (rId != 0x0)
    {
        OOXMLPropertyImpl::Pointer_t pProperty
            (new OOXMLPropertyImpl(rId, pVal, OOXMLPropertyImpl::ATTRIBUTE));

        mpPropertySetAttrs->add(pProperty);
    }
}

void OOXMLFastContextHandlerStream::sendProperty(Id nId)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.sendProperty");
    debug_logger->attribute("id", (*QNameToString::Instance())(nId));
    debug_logger->chars(xmlify(getPropertySetAttrs()->toString()));
    debug_logger->endElement();
#endif

    OOXMLPropertySetEntryToString aHandler(nId);
    getPropertySetAttrs()->resolve(aHandler);
    const OUString & sText = aHandler.getString();
    mpStream->utext(reinterpret_cast < const sal_uInt8 * >
                    (sText.getStr()),
                    sText.getLength());
}

OOXMLPropertySet::Pointer_t
OOXMLFastContextHandlerStream::getPropertySetAttrs() const
{
    return mpPropertySetAttrs;
}

void OOXMLFastContextHandlerStream::resolvePropertySetAttrs()
{
#ifdef DEBUG_CONTEXT_HANDLER
        debug_logger->startElement("contexthandler.resolvePropertySetAttrs");
        debug_logger->chars(mpPropertySetAttrs->toString());
        debug_logger->endElement();
#endif
    mpStream->props(mpPropertySetAttrs);
}

OOXMLPropertySet::Pointer_t OOXMLFastContextHandlerStream::getPropertySet()
    const
{
    return getPropertySetAttrs();
}

void OOXMLFastContextHandlerStream::handleHyperlink()
{
    OOXMLHyperlinkHandler aHyperlinkHandler(this);
    getPropertySetAttrs()->resolve(aHyperlinkHandler);
}

/*
  class OOXMLFastContextHandlerProperties
 */
OOXMLFastContextHandlerProperties::OOXMLFastContextHandlerProperties
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext), mpPropertySet(new OOXMLPropertySetImpl()),
  mbResolve(false)
{
    if (pContext->getResource() == STREAM)
        mbResolve = true;
}

OOXMLFastContextHandlerProperties::~OOXMLFastContextHandlerProperties()
{
}

void OOXMLFastContextHandlerProperties::lcl_endFastElement
(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    endAction(Element);

    if (mbResolve)
    {
        if (isForwardEvents())
        {
            mpStream->props(mpPropertySet);
        }
    }
    else
    {
        sendPropertiesToParent();
    }
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerProperties::getValue() const
{
    return OOXMLValue::Pointer_t(new OOXMLPropertySetValue(mpPropertySet));
}

#if OSL_DEBUG_LEVEL > 1
void OOXMLFastContextHandlerProperties::dumpXml( const TagLogger::Pointer_t pLogger) const
{
    pLogger->startElement("context");

    static char sBuffer[128];
    snprintf(sBuffer, sizeof(sBuffer), "%p", this);

    pLogger->attribute("parent", std::string(sBuffer));
    pLogger->attribute("type", getType());
    pLogger->attribute("resource", resourceToString(getResource()));
    pLogger->attribute("token", fastTokenToId(getToken()));
    pLogger->attribute("id", (*QNameToString::Instance())(getId()));

    OOXMLValue::Pointer_t pVal(getValue());

    if (pVal.get() != NULL)
        pLogger->attribute("value", pVal->toString());
    else
        pLogger->attribute("value", std::string("(null)"));

    pLogger->attribute("resolve", mbResolve ? "resolve" : "noResolve");

    pLogger->propertySet(getPropertySet(),
            IdToString::Pointer_t(new OOXMLIdToString()));

    mpParserState->dumpXml( pLogger );

    pLogger->endElement();
}
#endif

void OOXMLFastContextHandlerProperties::newProperty
(const Id & rId, OOXMLValue::Pointer_t pVal)
{
    if (rId != 0x0)
    {
        OOXMLPropertyImpl::Pointer_t pProperty
            (new OOXMLPropertyImpl(rId, pVal, OOXMLPropertyImpl::ATTRIBUTE));

        mpPropertySet->add(pProperty);
    }
}

void OOXMLFastContextHandlerProperties::handleXNotes()
{
    switch (mnToken)
    {
    case NS_wordprocessingml|OOXML_footnoteReference:
        {
            OOXMLFootnoteHandler aFootnoteHandler(this);
            mpPropertySet->resolve(aFootnoteHandler);
        }
        break;
    case NS_wordprocessingml|OOXML_endnoteReference:
        {
            OOXMLEndnoteHandler aEndnoteHandler(this);
            mpPropertySet->resolve(aEndnoteHandler);
        }
        break;
    default:
        break;
    }
}

void OOXMLFastContextHandlerProperties::handleHdrFtr()
{
    switch (mnToken)
    {
    case NS_wordprocessingml|OOXML_footerReference:
        {
            OOXMLFooterHandler aFooterHandler(this);
            mpPropertySet->resolve(aFooterHandler);
        }
        break;
    case NS_wordprocessingml|OOXML_headerReference:
        {
            OOXMLHeaderHandler aHeaderHandler(this);
            mpPropertySet->resolve(aHeaderHandler);
        }
        break;
    default:
        break;
    }
}

void OOXMLFastContextHandlerProperties::handleComment()
{
#ifdef DEBUG_ELEMENT
    debug_logger->element("handleComment");
#endif

    OOXMLCommentHandler aCommentHandler(this);
    getPropertySet()->resolve(aCommentHandler);
}

void OOXMLFastContextHandlerProperties::handlePicture()
{
#ifdef DEBUG_ELEMENT
    debug_logger->element("handlePicture");
#endif

    OOXMLPictureHandler aPictureHandler(this);
    getPropertySet()->resolve(aPictureHandler);
}

void OOXMLFastContextHandlerProperties::handleBreak()
{
#ifdef DEBUG_ELEMENT
    debug_logger->element("handleBreak");
#endif

    OOXMLBreakHandler aBreakHandler(*mpStream);
    getPropertySet()->resolve(aBreakHandler);
}

void OOXMLFastContextHandlerProperties::handleOLE()
{
#ifdef DEBUG_ELEMENT
    debug_logger->element("handleOLE");
#endif

    OOXMLOLEHandler aOLEHandler(this);
    getPropertySet()->resolve(aOLEHandler);
}

void OOXMLFastContextHandlerProperties::handleFontRel()
{
    OOXMLEmbeddedFontHandler handler(this);
    getPropertySet()->resolve(handler);
}

void OOXMLFastContextHandlerProperties::setParent
(OOXMLFastContextHandler * pParent)
{
#ifdef DEBUG_ELEMENT
    debug_logger->startElement("setParent");
    debug_logger->chars(std::string("OOXMLFastContextHandlerProperties"));
    debug_logger->endElement();
#endif

    OOXMLFastContextHandler::setParent(pParent);

    if (mpParent->getResource() == STREAM)
        mbResolve = true;
}

void OOXMLFastContextHandlerProperties::setPropertySet
(OOXMLPropertySet::Pointer_t pPropertySet)
{
    if (pPropertySet.get() != NULL)
        mpPropertySet = pPropertySet;
}

OOXMLPropertySet::Pointer_t
OOXMLFastContextHandlerProperties::getPropertySet() const
{
    return mpPropertySet;
}

/*
 * class OOXMLFasContextHandlerPropertyTable
 */

OOXMLFastContextHandlerPropertyTable::OOXMLFastContextHandlerPropertyTable
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandlerProperties(pContext)
{
}

OOXMLFastContextHandlerPropertyTable::~OOXMLFastContextHandlerPropertyTable()
{
}

void OOXMLFastContextHandlerPropertyTable::lcl_endFastElement
(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    OOXMLPropertySet::Pointer_t pPropSet(mpPropertySet->clone());
    OOXMLTableImpl::ValuePointer_t pTmpVal
        (new OOXMLPropertySetValue(pPropSet));

    mTable.add(pTmpVal);

    writerfilter::Reference<Table>::Pointer_t pTable(mTable.clone());

#ifdef DEBUG_PROPERTIES
    debug_logger->startElement("table");
    debug_logger->attribute("id", (*QNameToString::Instance())(mId));
    debug_logger->endElement();
#endif

    mpStream->table(mId, pTable);

    endAction(Element);
}

/*
 class OOXMLFastContextHandlerValue
*/

OOXMLFastContextHandlerValue::OOXMLFastContextHandlerValue
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerValue::~OOXMLFastContextHandlerValue()
{
}

void OOXMLFastContextHandlerValue::setValue(OOXMLValue::Pointer_t pValue)
{
#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->startElement("contexthandler.setValue");
    debug_logger->attribute("value", pValue->toString());
#endif

    mpValue = pValue;

#ifdef DEBUG_CONTEXT_HANDLER
    debug_logger->endElement();
#endif
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerValue::getValue() const
{
    return mpValue;
}

void OOXMLFastContextHandlerValue::lcl_endFastElement
(Token_t Element)
throw (uno::RuntimeException, xml::sax::SAXException)
{
    sendPropertyToParent();

    endAction(Element);
}

void OOXMLFastContextHandlerValue::setDefaultBooleanValue()
{
#ifdef DEBUG_ELEMENT
    debug_logger->element("setDefaultBooleanValue");
#endif

    if (mpValue.get() == NULL)
    {
        OOXMLValue::Pointer_t pValue(new OOXMLBooleanValue(true));
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultIntegerValue()
{
#ifdef DEBUG_ELEMENT
    debug_logger->element("setDefaultIntegerValue");
#endif

    if (mpValue.get() == NULL)
    {
        OOXMLValue::Pointer_t pValue(new OOXMLIntegerValue(0));
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultHexValue()
{
#ifdef DEBUG_ELEMENT
    debug_logger->element("setDefaultHexValue");
#endif

    if (mpValue.get() == NULL)
    {
        OOXMLValue::Pointer_t pValue(new OOXMLHexValue(0));
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultStringValue()
{
#ifdef DEBUG_ELEMENT
    debug_logger->element("setDefaultStringValue");
#endif

    if (mpValue.get() == NULL)
    {
        OOXMLValue::Pointer_t pValue(new OOXMLStringValue(OUString()));
        setValue(pValue);
    }
}
/*
  class OOXMLFastContextHandlerTable
*/

OOXMLFastContextHandlerTable::OOXMLFastContextHandlerTable
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerTable::~OOXMLFastContextHandlerTable()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandlerTable::createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    addCurrentChild();

    mCurrentChild.set
        (OOXMLFastContextHandler::createFastChildContext(Element, Attribs));

    return mCurrentChild;
}

void OOXMLFastContextHandlerTable::lcl_endFastElement
(Token_t /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    addCurrentChild();

    writerfilter::Reference<Table>::Pointer_t pTable(mTable.clone());
    if (isForwardEvents() && mId != 0x0)
    {
#ifdef DEBUG_PROPERTIES
        debug_logger->startElement("table");
        string str = (*QNameToString::Instance())(mId);
        debug_logger->attribute("id", str);
        debug_logger->endElement();
#endif

        mpStream->table(mId, pTable);
    }
}

void OOXMLFastContextHandlerTable::addCurrentChild()
{
    OOXMLFastContextHandler * pHandler = mCurrentChild.getPointer();
    if ( pHandler != NULL)
    {
        OOXMLValue::Pointer_t pValue(pHandler->getValue());

        if (pValue.get() != NULL)
        {
            OOXMLTableImpl::ValuePointer_t pTmpVal(pValue->clone());
            mTable.add(pTmpVal);
        }
    }
}

void OOXMLFastContextHandlerTable::newPropertySet
(OOXMLPropertySet::Pointer_t /*pPropertySet*/)
{

}

/*
  class OOXMLFastContextHandlerXNote
 */

OOXMLFastContextHandlerXNote::OOXMLFastContextHandlerXNote
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandlerProperties(pContext), mbForwardEventsSaved(false)
{
}

OOXMLFastContextHandlerXNote::~OOXMLFastContextHandlerXNote()
{
}

void OOXMLFastContextHandlerXNote::lcl_startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    mbForwardEventsSaved = isForwardEvents();

    if (mnMyXNoteId == getXNoteId())
        setForwardEvents(true);
    else
        setForwardEvents(false);

    startAction(Element);
}

void OOXMLFastContextHandlerXNote::lcl_endFastElement
(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    endAction(Element);

    OOXMLFastContextHandlerProperties::lcl_endFastElement(Element);

    setForwardEvents(mbForwardEventsSaved);
}

void OOXMLFastContextHandlerXNote::checkId(OOXMLValue::Pointer_t pValue)
{
#ifdef DEBUG_ELEMENT
    debug_logger->startElement("checkId");
    debug_logger->attribute("myId", sal_Int32(pValue->getInt()));
    debug_logger->attribute("id", getXNoteId());
    debug_logger->endElement();
#endif

    mnMyXNoteId = sal_Int32(pValue->getInt());
}

/*
  class OOXMLFastContextHandlerTextTableCell
 */

OOXMLFastContextHandlerTextTableCell::OOXMLFastContextHandlerTextTableCell
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerTextTableCell::~OOXMLFastContextHandlerTextTableCell()
{
}

void OOXMLFastContextHandlerTextTableCell::startCell()
{
}

void OOXMLFastContextHandlerTextTableCell::endCell()
{
    if (isForwardEvents())
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(mnTableDepth));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(NS_sprm::LN_PTableDepth, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(1));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(NS_sprm::LN_PFInTable, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLBooleanValue(mnTableDepth > 0));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(NS_sprm::LN_PCell, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }

#ifdef DEBUG_PROPERTIES
        debug_logger->startElement("endcell");
        debug_logger->propertySet(OOXMLPropertySet::Pointer_t(pProps->clone()),
                IdToString::Pointer_t(new OOXMLIdToString()));
        debug_logger->endElement();
#endif
        mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
    }
}

/*
  class OOXMLFastContextHandlerTextTableRow
 */

OOXMLFastContextHandlerTextTableRow::OOXMLFastContextHandlerTextTableRow
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerTextTableRow::~OOXMLFastContextHandlerTextTableRow()
{
}

void OOXMLFastContextHandlerTextTableRow::startRow()
{
}

void OOXMLFastContextHandlerTextTableRow::endRow()
{
    startParagraphGroup();

    if (isForwardEvents())
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(mnTableDepth));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(NS_sprm::LN_PTableDepth, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(1));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(NS_sprm::LN_PFInTable, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(1));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(NS_sprm::LN_PRow, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }

#ifdef DEBUG_PROPERTIES
        debug_logger->startElement("endrow");
        debug_logger->propertySet(OOXMLPropertySet::Pointer_t(pProps->clone()),
                IdToString::Pointer_t(new OOXMLIdToString()));
        debug_logger->endElement();
#endif

        mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
    }

    startCharacterGroup();

    if (isForwardEvents())
        mpStream->utext(s0xd, 1);

    endCharacterGroup();
    endParagraphGroup();
}

/*
  class OOXMLFastContextHandlerTextTable
 */

OOXMLFastContextHandlerTextTable::OOXMLFastContextHandlerTextTable
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerTextTable::~OOXMLFastContextHandlerTextTable()
{
    clearTableProps();
}

void OOXMLFastContextHandlerTextTable::lcl_startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    mpParserState->startTable();
    mnTableDepth++;

    boost::shared_ptr<OOXMLPropertySet> pProps( new OOXMLPropertySetImpl );
    {
        OOXMLValue::Pointer_t pVal
            (new OOXMLIntegerValue(mnTableDepth));
        OOXMLProperty::Pointer_t pProp
            (new OOXMLPropertyImpl(NS_ooxml::LN_tblStart, pVal, OOXMLPropertyImpl::SPRM));
        pProps->add(pProp);
    }
    mpParserState->setCharacterProperties(pProps);

    startAction(Element);
}

void OOXMLFastContextHandlerTextTable::lcl_endFastElement
(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    endAction(Element);

    mnTableDepth--;
    mpParserState->endTable();
}

/*
  class OOXMLFastContextHandlerShape
 */

OOXMLFastContextHandlerShape::OOXMLFastContextHandlerShape
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandlerProperties(pContext), m_bShapeSent( false ),
    m_bShapeStarted(false)
{
    uno::Reference<uno::XComponentContext> xContext(getComponentContext());
    if (xContext.is())
    {
        uno::Reference<XMultiComponentFactory> rServiceManager
            (xContext->getServiceManager());

        mrShapeContext.set( getDocument( )->getShapeContext( ) );
        if ( !mrShapeContext.is( ) )
        {
            // Define the shape context for the whole document
            mrShapeContext.set
                (rServiceManager->
                  createInstanceWithContext
                  ("com.sun.star.xml.sax.FastShapeContextHandler", xContext),
                  uno::UNO_QUERY);
            getDocument()->setShapeContext( mrShapeContext );
        }

        if (mrShapeContext.is())
        {
            mrShapeContext->setModel(getDocument()->getModel());
            mrShapeContext->setDrawPage(getDocument()->getDrawPage());
            mrShapeContext->setInputStream(getDocument()->getStorageStream());

#ifdef DEBUG_ELEMENT
            debug_logger->startElement("setRelationFragmentPath");
            debug_logger->attribute("path", mpParserState->getTarget());
            debug_logger->endElement();
#endif
            mrShapeContext->setRelationFragmentPath
                (mpParserState->getTarget());
        }
#ifdef DEBUG_CONTEXT_STACK
        else
        {
            debug_logger->startElement("error");
            debug_logger->chars(std::string("failed to get shape handler"));
            debug_logger->endElement();
        }
#endif
    }
}

OOXMLFastContextHandlerShape::~OOXMLFastContextHandlerShape()
{
}

void OOXMLFastContextHandlerShape::lcl_startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    startAction(Element);

    if (mrShapeContext.is())
    {
        mrShapeContext->startFastElement(Element, Attribs);
    }
}

void SAL_CALL OOXMLFastContextHandlerShape::startUnknownElement
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mrShapeContext.is())
        mrShapeContext->startUnknownElement(Namespace, Name, Attribs);
}

void OOXMLFastContextHandlerShape::setToken(Token_t nToken)
{
    OOXMLFastContextHandler::setToken(nToken);

    if (mrShapeContext.is())
        mrShapeContext->setStartToken(nToken);
}

void OOXMLFastContextHandlerShape::sendShape( Token_t Element )
{
    if ( mrShapeContext.is() && !m_bShapeSent )
    {
        uno::Reference<drawing::XShape> xShape(mrShapeContext->getShape());
        if (xShape.is())
        {
            OOXMLValue::Pointer_t
                pValue(new OOXMLShapeValue(xShape));
            newProperty(NS_ooxml::LN_shape, pValue);
            m_bShapeSent = true;

            bool bIsPicture = Element == ( NS_picture | OOXML_pic );

            // Notify the dmapper that the shape is ready to use
            if ( !bIsPicture )
            {
                mpStream->startShape( xShape );
                m_bShapeStarted = true;
            }
        }
    }
}

void OOXMLFastContextHandlerShape::lcl_endFastElement
(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mrShapeContext.is())
    {
        mrShapeContext->endFastElement(Element);
        sendShape( Element );
    }

    OOXMLFastContextHandlerProperties::lcl_endFastElement(Element);

    // Ending the shape should be the last thing to do
    bool bIsPicture = Element == ( NS_picture | OOXML_pic );
    if ( !bIsPicture && m_bShapeStarted)
        mpStream->endShape( );
}

void SAL_CALL OOXMLFastContextHandlerShape::endUnknownElement
(const OUString & Namespace,
 const OUString & Name)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mrShapeContext.is())
        mrShapeContext->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler >
OOXMLFastContextHandlerShape::lcl_createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xContextHandler;

    sal_uInt32 nNamespace = Element & 0xffff0000;

    switch (nNamespace)
    {
        case NS_wordprocessingml:
        case NS_vml_wordprocessingDrawing:
        case NS_office:
            xContextHandler.set(OOXMLFactory::getInstance()->createFastChildContextFromStart(this, Element));
            break;
        default:
            if (mrShapeContext.is())
            {
                uno::Reference<XFastContextHandler> pChildContext =
                mrShapeContext->createFastChildContext(Element, Attribs);

                OOXMLFastContextHandlerWrapper * pWrapper =
                new OOXMLFastContextHandlerWrapper(this, pChildContext);

                pWrapper->addNamespace(NS_wordprocessingml);
                pWrapper->addNamespace(NS_vml_wordprocessingDrawing);
                pWrapper->addNamespace(NS_office);
                pWrapper->addToken( NS_vml|OOXML_textbox );

                xContextHandler.set(pWrapper);
            }
            else
                xContextHandler.set(this);
            break;
    }


    return xContextHandler;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandlerShape::createUnknownChildContext
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;

    if (mrShapeContext.is())
        xResult.set(mrShapeContext->createUnknownChildContext
            (Namespace, Name, Attribs));

    return xResult;
}

void OOXMLFastContextHandlerShape::lcl_characters
(const OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mrShapeContext.is())
        mrShapeContext->characters(aChars);
}

/*
  class OOXMLFastContextHandlerWrapper
*/

OOXMLFastContextHandlerWrapper::OOXMLFastContextHandlerWrapper
(OOXMLFastContextHandler * pParent,
 uno::Reference<XFastContextHandler> xContext)
: OOXMLFastContextHandler(pParent), mxContext(xContext)
{
    if (pParent != NULL)
    {
        setId(pParent->getId());
        setToken(pParent->getToken());
        setPropertySet(pParent->getPropertySet());
    }
}

OOXMLFastContextHandlerWrapper::~OOXMLFastContextHandlerWrapper()
{
}

void SAL_CALL OOXMLFastContextHandlerWrapper::startUnknownElement
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxContext.is())
        mxContext->startUnknownElement(Namespace, Name, Attribs);
}

void SAL_CALL OOXMLFastContextHandlerWrapper::endUnknownElement
(const OUString & Namespace,
 const OUString & Name)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxContext.is())
        mxContext->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandlerWrapper::createUnknownChildContext
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;

    if (mxContext.is())
        xResult = mxContext->createUnknownChildContext
            (Namespace, Name, Attribs);
    else
        xResult.set(this);

    return xResult;
}

void OOXMLFastContextHandlerWrapper::attributes
(const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL)
            pHandler->attributes(Attribs);
    }
}

OOXMLFastContextHandler::ResourceEnum_t
OOXMLFastContextHandlerWrapper::getResource() const
{
    return UNKNOWN;
}

void OOXMLFastContextHandlerWrapper::addNamespace(const Id & nId)
{
    mMyNamespaces.insert(nId);
}

void OOXMLFastContextHandlerWrapper::addToken( Token_t Token )
{
    mMyTokens.insert( Token );
}

void OOXMLFastContextHandlerWrapper::lcl_startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxContext.is())
        mxContext->startFastElement(Element, Attribs);
}

void OOXMLFastContextHandlerWrapper::lcl_endFastElement
(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxContext.is())
        mxContext->endFastElement(Element);
}

uno::Reference< xml::sax::XFastContextHandler >
OOXMLFastContextHandlerWrapper::lcl_createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;

    Id nNameSpace = Element & 0xffff0000;

#ifdef DEBUG_ELEMENT
    debug_logger->startElement("Wrapper-createChildContext");
    debug_logger->attribute("token", fastTokenToId(Element));

    set<Id>::const_iterator aIt(mMyNamespaces.begin());
    while (aIt != mMyNamespaces.end())
    {
        debug_logger->startElement("namespace");
        debug_logger->attribute("id", fastTokenToId(*aIt));
        debug_logger->endElement();

        aIt++;
    }

    debug_logger->endElement();
#endif

    bool bInNamespaces = mMyNamespaces.find(nNameSpace) != mMyNamespaces.end();
    bool bInTokens = mMyTokens.find( Element ) != mMyTokens.end( );

    // We have methods to _add_ individual tokens or whole namespaces to be
    // processed by writerfilter (instead of oox), but we have no method to
    // filter out a single token. Just hardwire the wrap token here till we
    // need a more generic solution.
    if ( bInNamespaces && Element != static_cast<sal_Int32>(NS_vml_wordprocessingDrawing | OOXML_wrap) )
        xResult.set(OOXMLFactory::getInstance()->createFastChildContextFromStart(this, Element));
    else if (mxContext.is())
    {
        OOXMLFastContextHandlerWrapper * pWrapper =
            new OOXMLFastContextHandlerWrapper
            (this, mxContext->createFastChildContext(Element, Attribs));
        pWrapper->mMyNamespaces = mMyNamespaces;
        pWrapper->setPropertySet(getPropertySet());
        xResult.set(pWrapper);
    }
    else
        xResult.set(this);

    if ( bInTokens )
    {
        OOXMLFastContextHandlerShape* pShapeCtx = (OOXMLFastContextHandlerShape*)mpParent;
        pShapeCtx->sendShape( Element );
    }

    return xResult;
}

void OOXMLFastContextHandlerWrapper::lcl_characters
(const OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxContext.is())
        mxContext->characters(aChars);
}

OOXMLFastContextHandler *
OOXMLFastContextHandlerWrapper::getFastContextHandler() const
{
    if (mxContext.is())
        return dynamic_cast<OOXMLFastContextHandler *>(mxContext.get());

    return NULL;
}

void OOXMLFastContextHandlerWrapper::newProperty
(const Id & rId, OOXMLValue::Pointer_t pVal)
{
    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL)
            pHandler->newProperty(rId, pVal);
    }
}

void OOXMLFastContextHandlerWrapper::setPropertySet
(OOXMLPropertySet::Pointer_t pPropertySet)
{
    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL)
            pHandler->setPropertySet(pPropertySet);
    }

    mpPropertySet = pPropertySet;
}

OOXMLPropertySet::Pointer_t OOXMLFastContextHandlerWrapper::getPropertySet()
    const
{
    OOXMLPropertySet::Pointer_t pResult(mpPropertySet);

    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL)
            pResult = pHandler->getPropertySet();
    }

    return pResult;
}

string OOXMLFastContextHandlerWrapper::getType() const
{
    string sResult = "Wrapper(";

    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL)
            sResult += pHandler->getType();
    }

    sResult += ")";

    return sResult;
}

void OOXMLFastContextHandlerWrapper::setId(Id rId)
{
    OOXMLFastContextHandler::setId(rId);

    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL)
            pHandler->setId(rId);
    }
}

Id OOXMLFastContextHandlerWrapper::getId() const
{
    Id nResult = OOXMLFastContextHandler::getId();

    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL && pHandler->getId() != 0)
            nResult = pHandler->getId();
    }

    return nResult;
}

void OOXMLFastContextHandlerWrapper::setToken(Token_t nToken)
{
    OOXMLFastContextHandler::setToken(nToken);

    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL)
            pHandler->setToken(nToken);
    }
}

Token_t OOXMLFastContextHandlerWrapper::getToken() const
{
    Token_t nResult = OOXMLFastContextHandler::getToken();

    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != NULL)
            nResult = pHandler->getToken();
    }

    return nResult;
}


/*
  class OOXMLFastContextHandlerLinear
 */

OOXMLFastContextHandlerLinear::OOXMLFastContextHandlerLinear(OOXMLFastContextHandler* pContext)
    : OOXMLFastContextHandlerProperties(pContext)
    , depthCount( 0 )
{
}

void OOXMLFastContextHandlerLinear::lcl_startFastElement(Token_t Element,
    const uno::Reference< xml::sax::XFastAttributeList >& Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    buffer.appendOpeningTag( Element, Attribs );
    ++depthCount;
}

void OOXMLFastContextHandlerLinear::lcl_endFastElement(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    buffer.appendClosingTag( Element );
    if( --depthCount == 0 )
        process();
}

uno::Reference< xml::sax::XFastContextHandler >
OOXMLFastContextHandlerLinear::lcl_createFastChildContext(Token_t,
    const uno::Reference< xml::sax::XFastAttributeList >&)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xContextHandler;
    xContextHandler.set( this );
    return xContextHandler;
}

void OOXMLFastContextHandlerLinear::lcl_characters(const OUString& aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    buffer.appendCharacters( aChars );
}

/*
  class OOXMLFastContextHandlerLinear
 */

OOXMLFastContextHandlerMath::OOXMLFastContextHandlerMath(OOXMLFastContextHandler* pContext)
    : OOXMLFastContextHandlerLinear(pContext)
{
}

void OOXMLFastContextHandlerMath::process()
{
    SvGlobalName name( SO3_SM_CLASSID );
    comphelper::EmbeddedObjectContainer container;
    OUString aName;
    uno::Reference< embed::XEmbeddedObject > ref = container.CreateEmbeddedObject( name.GetByteSequence(), aName );
    uno::Reference< uno::XInterface > component( ref->getComponent(), uno::UNO_QUERY );
// gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
// so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
// to RTLD_GLOBAL, so most probably a gcc bug.
    oox::FormulaImportBase* import = dynamic_cast< oox::FormulaImportBase* >( dynamic_cast< SfxBaseModel* >(component.get()));
    assert( import != NULL );
    import->readFormulaOoxml( buffer );
    if (isForwardEvents())
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
        OOXMLValue::Pointer_t pVal( new OOXMLStarMathValue( ref ));
        OOXMLProperty::Pointer_t pProp( new OOXMLPropertyImpl( NS_ooxml::LN_starmath, pVal, OOXMLPropertyImpl::ATTRIBUTE ));
        pProps->add( pProp );
        mpStream->props( writerfilter::Reference< Properties >::Pointer_t( pProps ));
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
