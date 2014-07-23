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
#include <com/sun/star/xml/sax/FastShapeContextHandler.hpp>
#include <resourcemodel/QNameToString.hxx>
#include <resourcemodel/util.hxx>
#include <ooxml/resourceids.hxx>
#include <oox/token/namespaces.hxx>
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
#include<boost/make_shared.hpp>

static const sal_Unicode uCR = 0xd;
static const sal_Unicode uFtnEdnRef = 0x2;
static const sal_Unicode uFtnEdnSep = 0x3;
static const sal_Unicode uTab = 0x9;
static const sal_Unicode uPgNum = 0x0;
static const sal_Unicode uNoBreakHyphen = 0x2011;
static const sal_Unicode uSoftHyphen = 0xAD;

static const sal_uInt8 cFtnEdnCont = 0x4;
static const sal_uInt8 cFieldStart = 0x13;
static const sal_uInt8 cFieldSep = 0x14;
static const sal_uInt8 cFieldEnd = 0x15;

namespace writerfilter {
namespace ooxml
{
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

#ifdef DEBUG_DOMAINMAPPER
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
  inPositionV(false),
  m_xContext(context),
  m_bDiscardChildren(false),
  m_bTookChoice(false)
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
  mpStream(pContext->mpStream),
  mnTableDepth(pContext->mnTableDepth),
  mnInstanceNumber(mnInstanceCount),
  inPositionV(pContext->inPositionV),
  m_xContext(pContext->m_xContext),
  m_bDiscardChildren(pContext->m_bDiscardChildren),
  m_bTookChoice(pContext->m_bTookChoice)
{
    mpParserState = pContext->mpParserState;

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

bool OOXMLFastContextHandler::prepareMceContext(Token_t nElement, const uno::Reference<xml::sax::XFastAttributeList>& rAttribs)
{
    switch (oox::getBaseToken(nElement))
    {
        case OOXML_AlternateContent:
            {
                SavedAlternateState aState;
                aState.m_bDiscardChildren = m_bDiscardChildren;
                m_bDiscardChildren = false;
                aState.m_bTookChoice = m_bTookChoice;
                m_bTookChoice = false;
                mpParserState->getSavedAlternateStates().push_back(aState);
            }
            break;
        case OOXML_Choice:
        {
            OUString aRequires = rAttribs->getOptionalValue(OOXML_Requires);
            static const char* aFeatures[] = {
                "wps",
                "wpg",
            };
            for (size_t i = 0; i < SAL_N_ELEMENTS(aFeatures); ++i)
            {
                if (aRequires.equalsAscii(aFeatures[i]))
                {
                    m_bTookChoice = true;
                    return false;
                }
            }
            return true;
        }
            break;
        case OOXML_Fallback:
            // If Choice is already taken, then let's ignore the Fallback.
            return m_bTookChoice;
            break;
        default:
            SAL_WARN("writerfilter", "OOXMLFastContextHandler::prepareMceContext: unhandled element:" << oox::getBaseToken(nElement));
            break;
    }
    return false;
}

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL OOXMLFastContextHandler::startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    if (oox::getNamespace(Element) == static_cast<sal_Int32>(NS_mce))
        m_bDiscardChildren = prepareMceContext(Element, Attribs);

    else if (!m_bDiscardChildren)
    {
        attributes(Attribs);
        lcl_startFastElement(Element, Attribs);
    }
}

void SAL_CALL OOXMLFastContextHandler::startUnknownElement
(const OUString & /*Namespace*/, const OUString & /*Name*/,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

void SAL_CALL OOXMLFastContextHandler::endFastElement(Token_t Element)
throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    if (Element == (NS_mce | OOXML_Choice) || Element == (NS_mce | OOXML_Fallback))
        m_bDiscardChildren = false;
    else if (Element == (NS_mce | OOXML_AlternateContent))
    {
        SavedAlternateState aState(mpParserState->getSavedAlternateStates().back());
        mpParserState->getSavedAlternateStates().pop_back();
        m_bDiscardChildren = aState.m_bDiscardChildren;
        m_bTookChoice = aState.m_bTookChoice;
    }
    else if (!m_bDiscardChildren)
        lcl_endFastElement(Element);
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
throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
 OOXMLFastContextHandler::createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;
    if ((Element & 0xffff0000) != NS_mce && !m_bDiscardChildren)
        xResult.set(lcl_createFastChildContext(Element, Attribs));
    else if ((Element & 0xffff0000) == NS_mce)
        xResult = this;

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
(const OUString &,
 const OUString &,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    return uno::Reference< xml::sax::XFastContextHandler >
        (new OOXMLFastContextHandler(*const_cast<const OOXMLFastContextHandler *>(this)));
}

void SAL_CALL OOXMLFastContextHandler::characters
(const OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    lcl_characters(aChars);
}

void OOXMLFastContextHandler::lcl_characters
(const OUString & rString)
throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (!m_bDiscardChildren)
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
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
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
    OOXMLFactory::getInstance()->startAction(this, Element);
}

void OOXMLFastContextHandler::endAction(Token_t Element)
{
    OOXMLFactory::getInstance()->endAction(this, Element);
}

#ifdef DEBUG_DOMAINMAPPER
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
            IdToString::Pointer_t(boost::make_shared<OOXMLIdToString>()));

    mpParserState->dumpXml( pLogger );

    pLogger->endElement();
}

#endif

void OOXMLFastContextHandler::setId(Id rId)
{
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



void OOXMLFastContextHandler::setToken(Token_t nToken)
{
    mnToken = nToken;
}

Token_t OOXMLFastContextHandler::getToken() const
{
    return mnToken;
}

void OOXMLFastContextHandler::setParent
(OOXMLFastContextHandler * pParent)
{
    mpParent = pParent;
}

OOXMLPropertySet * OOXMLFastContextHandler::getPicturePropSet
(const OUString & rId)
{
    return mpParserState->getDocument()->getPicturePropSet(rId);
}

void OOXMLFastContextHandler::sendTableDepth() const
{
    if (mnTableDepth > 0)
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
        {
            OOXMLValue::Pointer_t pVal
                (boost::make_shared<OOXMLIntegerValue>(mnTableDepth));
            OOXMLProperty::Pointer_t pProp
                (boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_tblDepth, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (boost::make_shared<OOXMLIntegerValue>(1));
            OOXMLProperty::Pointer_t pProp
                (boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_inTbl, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }

        mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
    }
}

void OOXMLFastContextHandler::setHandle()
{
    mpParserState->setHandle();
    mpStream->info(mpParserState->getHandle());
}

void OOXMLFastContextHandler::startCharacterGroup()
{
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
    if (isForwardEvents() && mpParserState->isInCharacterGroup())
    {
        mpStream->endCharacterGroup();
        mpParserState->setInCharacterGroup(false);
    }
}

void OOXMLFastContextHandler::startParagraphGroup()
{
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

void OOXMLFastContextHandler::startSdt()
{
    OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
    OOXMLValue::Pointer_t pVal(boost::make_shared<OOXMLIntegerValue>(1));
    OOXMLProperty::Pointer_t pProp(boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_CT_SdtBlock_sdtContent, pVal, OOXMLPropertyImpl::ATTRIBUTE));
    pProps->add(pProp);
    mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
}

void OOXMLFastContextHandler::endSdt()
{
    OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
    OOXMLValue::Pointer_t pVal(boost::make_shared<OOXMLIntegerValue>(1));
    OOXMLProperty::Pointer_t pProp(boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_CT_SdtBlock_sdtEndContent, pVal, OOXMLPropertyImpl::ATTRIBUTE));
    pProps->add(pProp);
    mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
}

void OOXMLFastContextHandler::startSectionGroup()
{
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

void OOXMLFastContextHandler::setLastSectionGroup()
{
    mpStream->markLastSectionGroup( );
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
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(&cFieldStart, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::fieldSeparator()
{
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(&cFieldSep, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::endField()
{
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(&cFieldEnd, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::ftnednref()
{
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uFtnEdnRef, 1);
}

void OOXMLFastContextHandler::ftnednsep()
{
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uFtnEdnSep, 1);
}

void OOXMLFastContextHandler::ftnedncont()
{
    if (isForwardEvents())
        mpStream->text(&cFtnEdnCont, 1);
}

void OOXMLFastContextHandler::pgNum()
{
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uPgNum, 1);
}

void OOXMLFastContextHandler::tab()
{
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uTab, 1);
}

void OOXMLFastContextHandler::cr()
{
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uCR, 1);
}

void OOXMLFastContextHandler::noBreakHyphen()
{
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uNoBreakHyphen, 1);
}

void OOXMLFastContextHandler::softHyphen()
{
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uSoftHyphen, 1);
}

void OOXMLFastContextHandler::handleLastParagraphInSection()
{
    if (mpParserState->isLastParagraphInSection())
    {
        mpParserState->setLastParagraphInSection(false);
        startSectionGroup();
    }
}

void OOXMLFastContextHandler::endOfParagraph()
{
    if (! mpParserState->isInCharacterGroup())
        startCharacterGroup();
    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uCR, 1);

    mpParserState->getDocument()->incrementProgress();
}

void OOXMLFastContextHandler::startTxbxContent()
{
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
    endParagraphGroup();
    mpParserState->endTxbxContent();
}

void OOXMLFastContextHandler::text(const OUString & sText)
{
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
    if (isForwardEvents())
        ::writerfilter::dmapper::PositionHandler::setPositionOffset( sText, inPositionV );
}

void OOXMLFastContextHandler::alignH(const OUString & sText)
{
    if (isForwardEvents())
        ::writerfilter::dmapper::PositionHandler::setAlignH( sText );
}

void OOXMLFastContextHandler::alignV(const OUString & sText)
{
    if (isForwardEvents())
        ::writerfilter::dmapper::PositionHandler::setAlignV( sText );
}

void OOXMLFastContextHandler::positivePercentage(const OUString& rText)
{
    if (isForwardEvents())
        mpStream->positivePercentage(rText);
}

void OOXMLFastContextHandler::propagateCharacterProperties()
{
    mpParserState->setCharacterProperties(getPropertySet());
}

void OOXMLFastContextHandler::propagateCharacterPropertiesAsSet(const Id & rId)
{
    OOXMLValue::Pointer_t pValue(boost::make_shared<OOXMLPropertySetValue>(getPropertySet()));
    OOXMLPropertySet::Pointer_t pPropertySet(boost::make_shared<OOXMLPropertySetImpl>());

    OOXMLProperty::Pointer_t pProp
        (boost::make_shared<OOXMLPropertyImpl>(rId, pValue, OOXMLPropertyImpl::SPRM));

    pPropertySet->add(pProp);
    mpParserState->setCharacterProperties(pPropertySet);
}

bool OOXMLFastContextHandler::propagatesProperties() const
{
    return false;
}

void OOXMLFastContextHandler::propagateCellProperties()
{
    mpParserState->setCellProperties(getPropertySet());
}

void OOXMLFastContextHandler::propagateRowProperties()
{
    mpParserState->setRowProperties(getPropertySet());
}

void OOXMLFastContextHandler::propagateTableProperties()
{
    OOXMLPropertySet::Pointer_t pProps = getPropertySet();

    mpParserState->setTableProperties(pProps);
}

void OOXMLFastContextHandler::sendCellProperties()
{
    mpParserState->resolveCellProperties(*mpStream);
}

void OOXMLFastContextHandler::sendRowProperties()
{
    mpParserState->resolveRowProperties(*mpStream);
}

void OOXMLFastContextHandler::sendTableProperties()
{
    mpParserState->resolveTableProperties(*mpStream);
}

void OOXMLFastContextHandler::clearTableProps()
{
    mpParserState->setTableProperties(OOXMLPropertySet::Pointer_t
                                     (boost::make_shared<OOXMLPropertySetImpl>()));
}

void OOXMLFastContextHandler::sendPropertiesWithId(const Id & rId)
{
    OOXMLValue::Pointer_t pValue(boost::make_shared<OOXMLPropertySetValue>(getPropertySet()));
    OOXMLPropertySet::Pointer_t pPropertySet(boost::make_shared<OOXMLPropertySetImpl>());

    OOXMLProperty::Pointer_t pProp
    (boost::make_shared<OOXMLPropertyImpl>(rId, pValue, OOXMLPropertyImpl::SPRM));

    pPropertySet->add(pProp);
    mpStream->props(pPropertySet);
}

void OOXMLFastContextHandler::clearProps()
{
    setPropertySet(OOXMLPropertySet::Pointer_t(boost::make_shared<OOXMLPropertySetImpl>()));
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

void OOXMLFastContextHandler::setDocument(OOXMLDocumentImpl* pDocument)
{
    mpParserState->setDocument(pDocument);
}

OOXMLDocumentImpl* OOXMLFastContextHandler::getDocument()
{
    return mpParserState->getDocument();
}

void OOXMLFastContextHandler::setForwardEvents(bool bForwardEvents)
{
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
    OOXMLDocument * objDocument = getDocument();
    SAL_WARN_IF(!objDocument, "writerfilter", "no document to resolveData");
    if (!objDocument)
        return;

    uno::Reference<io::XInputStream> xInputStream
        (objDocument->getInputStreamForId(rId));

    OOXMLValue::Pointer_t aValue(boost::make_shared<OOXMLInputStreamValue>(xInputStream));

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
    if (mpParent != NULL)
    {
        OOXMLPropertySet::Pointer_t pProps(mpParent->getPropertySet());

        if (pProps.get() != NULL)
        {
            OOXMLProperty::Pointer_t
                pProp(boost::make_shared<OOXMLPropertyImpl>(mId, getValue(),
                                            OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
    }
}

void OOXMLFastContextHandler::sendPropertiesToParent()
{
    if (mpParent != NULL)
    {
        OOXMLPropertySet::Pointer_t pParentProps(mpParent->getPropertySet());

        if (pParentProps.get() != NULL)
        {
            OOXMLPropertySet::Pointer_t pProps(getPropertySet());

            if (pProps.get() != NULL)
            {
                OOXMLValue::Pointer_t pValue
                (boost::make_shared<OOXMLPropertySetValue>(getPropertySet()));

                OOXMLProperty::Pointer_t pProp
                (boost::make_shared<OOXMLPropertyImpl>(getId(), pValue, OOXMLPropertyImpl::SPRM));


                pParentProps->add(pProp);

            }
        }
    }
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
            (boost::make_shared<OOXMLPropertyImpl>(rId, pVal, OOXMLPropertyImpl::ATTRIBUTE));

        mpPropertySetAttrs->add(pProperty);
    }
}

void OOXMLFastContextHandlerStream::sendProperty(Id nId)
{
    OOXMLPropertySetEntryToString aHandler(nId);
    getPropertySetAttrs()->resolve(aHandler);
    const OUString & sText = aHandler.getString();
    mpStream->utext(reinterpret_cast < const sal_uInt8 * >
                    (sText.getStr()),
                    sText.getLength());
}


void OOXMLFastContextHandlerStream::resolvePropertySetAttrs()
{
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
    return OOXMLValue::Pointer_t(boost::make_shared<OOXMLPropertySetValue>(mpPropertySet));
}

#ifdef DEBUG_DOMAINMAPPER
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
            IdToString::Pointer_t(boost::make_shared<OOXMLIdToString>()));

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
            (boost::make_shared<OOXMLPropertyImpl>(rId, pVal, OOXMLPropertyImpl::ATTRIBUTE));

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
    OOXMLCommentHandler aCommentHandler(this);
    getPropertySet()->resolve(aCommentHandler);
}

void OOXMLFastContextHandlerProperties::handlePicture()
{
    OOXMLPictureHandler aPictureHandler(this);
    getPropertySet()->resolve(aPictureHandler);
}

void OOXMLFastContextHandlerProperties::handleBreak()
{
    OOXMLBreakHandler aBreakHandler(*mpStream);
    getPropertySet()->resolve(aBreakHandler);
}

void OOXMLFastContextHandlerProperties::handleOLE()
{
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
        (boost::make_shared<OOXMLPropertySetValue>(pPropSet));

    mTable.add(pTmpVal);

    writerfilter::Reference<Table>::Pointer_t pTable(mTable.clone());

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
    mpValue = pValue;
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
    if (mpValue.get() == NULL)
    {
        OOXMLValue::Pointer_t pValue(boost::make_shared<OOXMLBooleanValue>(true));
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultIntegerValue()
{
    if (mpValue.get() == NULL)
    {
        OOXMLValue::Pointer_t pValue(boost::make_shared<OOXMLIntegerValue>(0));
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultHexValue()
{
    if (mpValue.get() == NULL)
    {
        OOXMLValue::Pointer_t pValue(boost::make_shared<OOXMLHexValue>(sal_uInt32(0)));
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultStringValue()
{
    if (mpValue.get() == NULL)
    {
        OOXMLValue::Pointer_t pValue(boost::make_shared<OOXMLStringValue>(OUString()));
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
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    addCurrentChild();

    mCurrentChild.set(OOXMLFastContextHandler::createFastChildContext(Element, Attribs));

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
        mpStream->table(mId, pTable);
    }
}

void OOXMLFastContextHandlerTable::addCurrentChild()
{
    OOXMLFastContextHandler * pHandler = dynamic_cast<OOXMLFastContextHandler*>(mCurrentChild.get());
    if (pHandler != NULL)
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
    : OOXMLFastContextHandlerProperties(pContext)
    , mbForwardEventsSaved(false)
    , mnMyXNoteId(0)
    , mnMyXNoteType(0)
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

    // If this is the note we're looking for or this is the footnote separator one.
    if (mnMyXNoteId == getXNoteId() || static_cast<sal_uInt32>(mnMyXNoteType) == NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_separator)
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
    mnMyXNoteId = sal_Int32(pValue->getInt());
}

void OOXMLFastContextHandlerXNote::checkType(OOXMLValue::Pointer_t pValue)
{
    mnMyXNoteType = pValue->getInt();
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
                (boost::make_shared<OOXMLIntegerValue>(mnTableDepth));
            OOXMLProperty::Pointer_t pProp
                (boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_tblDepth, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (boost::make_shared<OOXMLIntegerValue>(1));
            OOXMLProperty::Pointer_t pProp
                (boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_inTbl, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (boost::make_shared<OOXMLBooleanValue>(mnTableDepth > 0));
            OOXMLProperty::Pointer_t pProp
                (boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_tblCell, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }

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
                (boost::make_shared<OOXMLIntegerValue>(mnTableDepth));
            OOXMLProperty::Pointer_t pProp
                (boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_tblDepth, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (boost::make_shared<OOXMLIntegerValue>(1));
            OOXMLProperty::Pointer_t pProp
                (boost::make_shared<OOXMLPropertyImpl>(NS_ooxml::LN_inTbl, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (boost::make_shared<OOXMLIntegerValue>(1));
            OOXMLProperty::Pointer_t pProp
                (boost::make_shared< OOXMLPropertyImpl>(NS_ooxml::LN_tblRow, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }

        mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
    }

    startCharacterGroup();

    if (isForwardEvents())
        mpStream->utext((const sal_uInt8*)&uCR, 1);

    endCharacterGroup();
    endParagraphGroup();
}

// Handle w:gridBefore here by faking necessary input that'll fake cells. I'm apparently
// not insane enough to find out how to add cells in dmapper.
void OOXMLFastContextHandlerTextTableRow::handleGridBefore( OOXMLValue::Pointer_t val )
{
    int count = val->getInt();
    for( int i = 0;
         i < count;
         ++i )
    {
        endOfParagraph();

        if (isForwardEvents())
        {
            // This whole part is OOXMLFastContextHandlerTextTableCell::endCell() .
            OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
            {
                OOXMLValue::Pointer_t pVal
                    (boost::make_shared<OOXMLIntegerValue>(mnTableDepth));
                OOXMLProperty::Pointer_t pProp
                    (boost::make_shared< OOXMLPropertyImpl>(NS_ooxml::LN_tblDepth, pVal, OOXMLPropertyImpl::SPRM));
                pProps->add(pProp);
            }
            {
                OOXMLValue::Pointer_t pVal
                    (boost::make_shared<OOXMLIntegerValue>(1));
                OOXMLProperty::Pointer_t pProp
                    (boost::make_shared< OOXMLPropertyImpl>(NS_ooxml::LN_inTbl, pVal, OOXMLPropertyImpl::SPRM));
                pProps->add(pProp);
            }
            {
                OOXMLValue::Pointer_t pVal
                    (new OOXMLBooleanValue(mnTableDepth > 0));
                OOXMLProperty::Pointer_t pProp
                    (boost::make_shared< OOXMLPropertyImpl>(NS_ooxml::LN_tblCell, pVal, OOXMLPropertyImpl::SPRM));
                pProps->add(pProp);
            }

            mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));

            // fake <w:tcBorders> with no border
            OOXMLPropertySet::Pointer_t pCellProps( boost::make_shared<OOXMLPropertySetImpl>());
            {
                OOXMLPropertySet::Pointer_t pBorderProps( boost::make_shared<OOXMLPropertySetImpl>());
                static Id borders[] = { NS_ooxml::LN_CT_TcBorders_top, NS_ooxml::LN_CT_TcBorders_bottom,
                    NS_ooxml::LN_CT_TcBorders_start, NS_ooxml::LN_CT_TcBorders_end };
                for( size_t j = 0; j < SAL_N_ELEMENTS( borders ); ++j )
                    pBorderProps->add( fakeNoBorder( borders[ j ] ));
                OOXMLValue::Pointer_t pValue( boost::make_shared<OOXMLPropertySetValue>( pBorderProps ));
                OOXMLProperty::Pointer_t pProp
                    (boost::make_shared< OOXMLPropertyImpl>(NS_ooxml::LN_CT_TcPrBase_tcBorders, pValue, OOXMLPropertyImpl::SPRM));
                pCellProps->add(pProp);
                mpParserState->setCellProperties(pCellProps);
            }
        }

        sendCellProperties();
        endParagraphGroup();
    }
}

OOXMLProperty::Pointer_t OOXMLFastContextHandlerTextTableRow::fakeNoBorder( Id id )
{
    OOXMLPropertySet::Pointer_t pProps( boost::make_shared<OOXMLPropertySetImpl>());
    OOXMLValue::Pointer_t pVal(boost::make_shared<OOXMLIntegerValue>(0));
    OOXMLProperty::Pointer_t pPropVal
        (boost::make_shared< OOXMLPropertyImpl>(NS_ooxml::LN_CT_Border_val, pVal, OOXMLPropertyImpl::ATTRIBUTE));
    pProps->add(pPropVal);
    OOXMLValue::Pointer_t pValue( boost::make_shared<OOXMLPropertySetValue>( pProps ));
    OOXMLProperty::Pointer_t pProp
        (boost::make_shared< OOXMLPropertyImpl>(id, pValue, OOXMLPropertyImpl::SPRM));
    return pProp;
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

    boost::shared_ptr<OOXMLPropertySet> pProps( boost::make_shared<OOXMLPropertySetImpl>() );
    {
        OOXMLValue::Pointer_t pVal
            (boost::make_shared<OOXMLIntegerValue>(mnTableDepth));
        OOXMLProperty::Pointer_t pProp
            (boost::make_shared< OOXMLPropertyImpl>(NS_ooxml::LN_tblStart, pVal, OOXMLPropertyImpl::SPRM));
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

    boost::shared_ptr<OOXMLPropertySet> pProps( boost::make_shared<OOXMLPropertySetImpl>());
    {
        OOXMLValue::Pointer_t pVal
            (boost::make_shared<OOXMLIntegerValue>(mnTableDepth));
        OOXMLProperty::Pointer_t pProp
            (boost::make_shared< OOXMLPropertyImpl>(NS_ooxml::LN_tblEnd, pVal, OOXMLPropertyImpl::SPRM));
        pProps->add(pProp);
    }
    mpParserState->setCharacterProperties(pProps);

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
    mrShapeContext.set( getDocument( )->getShapeContext( ) );
    if ( !mrShapeContext.is( ) )
    {
        // Define the shape context for the whole document
        mrShapeContext = css::xml::sax::FastShapeContextHandler::create(
            getComponentContext());
        getDocument()->setShapeContext( mrShapeContext );
    }

    mrShapeContext->setModel(getDocument()->getModel());
    mrShapeContext->setDrawPage(getDocument()->getDrawPage());
    mrShapeContext->setInputStream(getDocument()->getStorageStream());

    mrShapeContext->setRelationFragmentPath
        (mpParserState->getTarget());
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
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
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
        awt::Point aPosition(writerfilter::dmapper::PositionHandler::getPositionOffset(false), writerfilter::dmapper::PositionHandler::getPositionOffset(true));
        mrShapeContext->setPosition(aPosition);
        uno::Reference<drawing::XShape> xShape(mrShapeContext->getShape());
        if (xShape.is())
        {
            OOXMLValue::Pointer_t
                pValue(boost::make_shared<OOXMLShapeValue>(xShape));
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
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
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

    bool bGroupShape = Element == Token_t(NS_vml | OOXML_group);
    // drawingML version also counts as a group shape.
    bGroupShape |= mrShapeContext->getStartToken() == Token_t(NS_wpg | OOXML_wgp);
    sal_uInt32 nNamespace = Element & 0xffff0000;

    switch (nNamespace)
    {
        case NS_wordprocessingml:
        case NS_vml_wordprocessingDrawing:
        case NS_office:
            if (!bGroupShape)
                xContextHandler.set(OOXMLFactory::getInstance()->createFastChildContextFromStart(this, Element));
        // no break;
        default:
            if (!xContextHandler.is())
            {
                if (mrShapeContext.is())
                {
                    uno::Reference<XFastContextHandler> pChildContext =
                        mrShapeContext->createFastChildContext(Element, Attribs);

                    OOXMLFastContextHandlerWrapper * pWrapper =
                        new OOXMLFastContextHandlerWrapper(this, pChildContext);

                    if (!bGroupShape)
                    {
                        pWrapper->addNamespace(NS_wordprocessingml);
                        pWrapper->addNamespace(NS_vml_wordprocessingDrawing);
                        pWrapper->addNamespace(NS_office);
                        pWrapper->addToken( NS_vml|OOXML_textbox );
                    }

                    xContextHandler.set(pWrapper);
                }
                else
                    xContextHandler.set(this);
            }
            break;
    }

    // VML import of shape text is already handled by
    // OOXMLFastContextHandlerWrapper::lcl_createFastChildContext(), here we
    // handle the WPS import of shape text, as there the parent context is a
    // Shape one, so a different situation.
    if (Element == static_cast<sal_Int32>(NS_wps | OOXML_txbx) ||
        Element == static_cast<sal_Int32>(NS_wps | OOXML_linkedTxbx) )
        sendShape(Element);

    return xContextHandler;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandlerShape::createUnknownChildContext
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
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
    setId(pParent->getId());
    setToken(pParent->getToken());
    setPropertySet(pParent->getPropertySet());
}

OOXMLFastContextHandlerWrapper::~OOXMLFastContextHandlerWrapper()
{
}

void SAL_CALL OOXMLFastContextHandlerWrapper::startUnknownElement
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    if (mxContext.is())
        mxContext->startUnknownElement(Namespace, Name, Attribs);
}

void SAL_CALL OOXMLFastContextHandlerWrapper::endUnknownElement
(const OUString & Namespace,
 const OUString & Name)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    if (mxContext.is())
        mxContext->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandlerWrapper::createUnknownChildContext
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
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

    bool bInNamespaces = mMyNamespaces.find(nNameSpace) != mMyNamespaces.end();
    bool bInTokens = mMyTokens.find( Element ) != mMyTokens.end( );

    OOXMLFastContextHandlerShape* pShapeCtx = (OOXMLFastContextHandlerShape*)mpParent;

    // We have methods to _add_ individual tokens or whole namespaces to be
    // processed by writerfilter (instead of oox), but we have no method to
    // filter out a single token. Just hardwire the wrap token here till we
    // need a more generic solution.
    bool bIsWrap = Element == static_cast<sal_Int32>(NS_vml_wordprocessingDrawing | OOXML_wrap);
    if ( bInNamespaces && ((pShapeCtx->isShapeSent() && bIsWrap) || !bIsWrap) )
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
        pShapeCtx->sendShape( Element );

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
    assert(ref.is());
    if (!ref.is())
        return;
    uno::Reference< uno::XInterface > component( ref->getComponent(), uno::UNO_QUERY );
// gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
// so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
// to RTLD_GLOBAL, so most probably a gcc bug.
    oox::FormulaImportBase* import = dynamic_cast< oox::FormulaImportBase* >( dynamic_cast< SfxBaseModel* >(component.get()));
    assert( import != NULL );
    if (!import)
        return;
    import->readFormulaOoxml( buffer );
    if (isForwardEvents())
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
        OOXMLValue::Pointer_t pVal( boost::make_shared<OOXMLStarMathValue>( ref ));
        OOXMLProperty::Pointer_t pProp( boost::make_shared<OOXMLPropertyImpl>( NS_ooxml::LN_starmath, pVal, OOXMLPropertyImpl::ATTRIBUTE ));
        pProps->add( pProp );
        mpStream->props( writerfilter::Reference< Properties >::Pointer_t( pProps ));
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
