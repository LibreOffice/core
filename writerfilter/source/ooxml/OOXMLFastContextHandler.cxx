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

#include <iostream>
#include <set>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/xml/sax/FastShapeContextHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <ooxml/QNameToString.hxx>
#include <ooxml/resourceids.hxx>
#include <oox/token/namespaces.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include "OOXMLFastContextHandler.hxx"
#include "OOXMLFactory.hxx"
#include "Handler.hxx"

static const sal_Unicode uCR = 0xd;
static const sal_Unicode uFtnEdnRef = 0x2;
static const sal_Unicode uFtnEdnSep = 0x3;
static const sal_Unicode uTab = 0x9;
static const sal_Unicode uPgNum = 0x0;
static const sal_Unicode uNoBreakHyphen = 0x2011;
static const sal_Unicode uSoftHyphen = 0xAD;

static const sal_uInt8 cFtnEdnCont = 0x4;
static const sal_uInt8 cFieldLock = 0x8;

namespace writerfilter {
namespace ooxml
{
using namespace ::com::sun::star;
using namespace oox;
using namespace ::std;

static set<OOXMLFastContextHandler *> aSetContexts;

/*
  class OOXMLFastContextHandler
 */

sal_uInt32 OOXMLFastContextHandler::mnInstanceCount = 0;

OOXMLFastContextHandler::OOXMLFastContextHandler
(uno::Reference< uno::XComponentContext > const & context)
: mpParent(nullptr),
  mId(0),
  mnDefine(0),
  mnToken(oox::XML_TOKEN_COUNT),
  mpStream(nullptr),
  mnTableDepth(0),
  inPositionV(false),
  m_xContext(context),
  m_bDiscardChildren(false),
  m_bTookChoice(false)
{
    mnInstanceCount++;
    aSetContexts.insert(this);

    if (mpParserState.get() == nullptr)
        mpParserState.reset(new OOXMLParserState());

    mpParserState->incContextCount();
}

OOXMLFastContextHandler::OOXMLFastContextHandler(OOXMLFastContextHandler * pContext)
: cppu::WeakImplHelper<xml::sax::XFastContextHandler>(),
  mpParent(pContext),
  mId(0),
  mnDefine(0),
  mnToken(oox::XML_TOKEN_COUNT),
  mpStream(pContext->mpStream),
  mpParserState(pContext->mpParserState),
  mnTableDepth(pContext->mnTableDepth),
  inPositionV(pContext->inPositionV),
  m_xContext(pContext->m_xContext),
  m_bDiscardChildren(pContext->m_bDiscardChildren),
  m_bTookChoice(pContext->m_bTookChoice)
{
    if (mpParserState.get() == nullptr)
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
        case XML_AlternateContent:
            {
                SavedAlternateState aState;
                aState.m_bDiscardChildren = m_bDiscardChildren;
                m_bDiscardChildren = false;
                aState.m_bTookChoice = m_bTookChoice;
                m_bTookChoice = false;
                mpParserState->getSavedAlternateStates().push_back(aState);
            }
            break;
        case XML_Choice:
        {
            OUString aRequires = rAttribs->getOptionalValue(XML_Requires);
            static const char* aFeatures[] = {
                "wps",
                "wpg",
            };
            for (const char *p : aFeatures)
            {
                if (aRequires.equalsAscii(p))
                {
                    m_bTookChoice = true;
                    return false;
                }
            }
            return true;
        }
            break;
        case XML_Fallback:
            // If Choice is already taken, then let's ignore the Fallback.
            return m_bTookChoice;
            break;
        default:
            SAL_WARN("writerfilter", "OOXMLFastContextHandler::prepareMceContext: unhandled element:" << oox::getBaseToken(nElement));
            break;
    }
    return false;
}

// xml::sax::XFastContextHandler:
void SAL_CALL OOXMLFastContextHandler::startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    if (oox::getNamespace(Element) == static_cast<sal_Int32>(NMSP_mce))
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
{
}

void SAL_CALL OOXMLFastContextHandler::endFastElement(Token_t Element)
{
    if (Element == (NMSP_mce | XML_Choice) || Element == (NMSP_mce | XML_Fallback))
        m_bDiscardChildren = false;
    else if (Element == (NMSP_mce | XML_AlternateContent))
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
{
    OOXMLFactory::startAction(this, Element);
    if( Element == (NMSP_dmlWordDr|XML_positionV) )
        inPositionV = true;
    else if( Element == (NMSP_dmlWordDr|XML_positionH) )
        inPositionV = false;

}

void OOXMLFastContextHandler::lcl_endFastElement
(Token_t Element)
{
    OOXMLFactory::endAction(this, Element);
}

void SAL_CALL OOXMLFastContextHandler::endUnknownElement
(const OUString & , const OUString & )
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
 OOXMLFastContextHandler::createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;
    if (oox::getNamespace(Element) != NMSP_mce && !m_bDiscardChildren)
        xResult.set(lcl_createFastChildContext(Element, Attribs));
    else if (oox::getNamespace(Element) == NMSP_mce)
        xResult = this;

    return xResult;
}

uno::Reference< xml::sax::XFastContextHandler >
 OOXMLFastContextHandler::lcl_createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
    return OOXMLFactory::createFastChildContext(this, Element);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandler::createUnknownChildContext
(const OUString &,
 const OUString &,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
    return uno::Reference< xml::sax::XFastContextHandler >
        (new OOXMLFastContextHandler(*const_cast<const OOXMLFastContextHandler *>(this)));
}

void SAL_CALL OOXMLFastContextHandler::characters
(const OUString & aChars)
{
    lcl_characters(aChars);
}

void OOXMLFastContextHandler::lcl_characters
(const OUString & rString)
{
    if (!m_bDiscardChildren)
        OOXMLFactory::characters(this, rString);
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
{
    OOXMLFactory::attributes(this, Attribs);
}

void OOXMLFastContextHandler::startAction(Token_t Element)
{
    OOXMLFactory::startAction(this, Element);
}

void OOXMLFastContextHandler::endAction(Token_t Element)
{
    OOXMLFactory::endAction(this, Element);
}

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

void OOXMLFastContextHandler::sendTableDepth() const
{
    if (mnTableDepth > 0)
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySet;
        {
            OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(mnTableDepth);
            OOXMLProperty::Pointer_t pProp
                (new OOXMLProperty(NS_ooxml::LN_tblDepth, pVal, OOXMLProperty::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(1);
            OOXMLProperty::Pointer_t pProp
                (new OOXMLProperty(NS_ooxml::LN_inTbl, pVal, OOXMLProperty::SPRM));
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
    OOXMLPropertySet * pProps = new OOXMLPropertySet;
    OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(1);
    OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_CT_SdtBlock_sdtContent, pVal, OOXMLProperty::ATTRIBUTE));
    pProps->add(pProp);
    mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
}

void OOXMLFastContextHandler::endSdt()
{
    OOXMLPropertySet * pProps = new OOXMLPropertySet;
    OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(1);
    OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_CT_SdtBlock_sdtEndContent, pVal, OOXMLProperty::ATTRIBUTE));
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
(Id /*nId*/, const OOXMLValue::Pointer_t& /*pVal*/)
{
}

void OOXMLFastContextHandler::setPropertySet
(const OOXMLPropertySet::Pointer_t& /* pPropertySet */)
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

void OOXMLFastContextHandler::lockField()
{
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(&cFieldLock, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::ftnednref()
{
    if (isForwardEvents())
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uFtnEdnRef), 1);
}

void OOXMLFastContextHandler::ftnednsep()
{
    if (isForwardEvents())
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uFtnEdnSep), 1);
}

void OOXMLFastContextHandler::ftnedncont()
{
    if (isForwardEvents())
        mpStream->text(&cFtnEdnCont, 1);
}

void OOXMLFastContextHandler::pgNum()
{
    if (isForwardEvents())
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uPgNum), 1);
}

void OOXMLFastContextHandler::tab()
{
    if (isForwardEvents())
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uTab), 1);
}

void OOXMLFastContextHandler::symbol()
{
    if (isForwardEvents())
        sendPropertiesWithId(NS_ooxml::LN_EG_RunInnerContent_sym);
}

void OOXMLFastContextHandler::cr()
{
    if (isForwardEvents())
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uCR), 1);
}

void OOXMLFastContextHandler::noBreakHyphen()
{
    if (isForwardEvents())
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uNoBreakHyphen), 1);
}

void OOXMLFastContextHandler::softHyphen()
{
    if (isForwardEvents())
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uSoftHyphen), 1);
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
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uCR), 1);

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

void OOXMLFastContextHandler::positionOffset(const OUString& rText)
{
    if (isForwardEvents())
        mpStream->positionOffset(rText, inPositionV);
}

void OOXMLFastContextHandler::ignore()
{
}

void OOXMLFastContextHandler::alignH(const OUString& rText)
{
    if (isForwardEvents())
        mpStream->align(rText, /*bVertical=*/false);
}

void OOXMLFastContextHandler::alignV(const OUString& rText)
{
    if (isForwardEvents())
        mpStream->align(rText, /*bVertical=*/true);
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

void OOXMLFastContextHandler::propagateCharacterPropertiesAsSet(Id nId)
{
    OOXMLValue::Pointer_t pValue(new OOXMLPropertySetValue(getPropertySet()));
    OOXMLPropertySet::Pointer_t pPropertySet(new OOXMLPropertySet);

    OOXMLProperty::Pointer_t pProp(new OOXMLProperty(nId, pValue, OOXMLProperty::SPRM));

    pPropertySet->add(pProp);
    mpParserState->setCharacterProperties(pPropertySet);
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
    mpParserState->setTableProperties(std::make_shared<OOXMLPropertySet>());
}

void OOXMLFastContextHandler::sendPropertiesWithId(Id nId)
{
    OOXMLValue::Pointer_t pValue(new OOXMLPropertySetValue(getPropertySet()));
    OOXMLPropertySet::Pointer_t pPropertySet(new OOXMLPropertySet);

    OOXMLProperty::Pointer_t pProp(new OOXMLProperty(nId, pValue, OOXMLProperty::SPRM));

    pPropertySet->add(pProp);
    mpStream->props(pPropertySet);
}

void OOXMLFastContextHandler::clearProps()
{
    setPropertySet(std::make_shared<OOXMLPropertySet>());
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

void OOXMLFastContextHandler::setXNoteId(const OOXMLValue::Pointer_t& pValue)
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

    OOXMLValue::Pointer_t aValue(new OOXMLInputStreamValue(xInputStream));

    newProperty(NS_ooxml::LN_inputstream, aValue);
}

OUString OOXMLFastContextHandler::getTargetForId
(const OUString & rId)
{
    return mpParserState->getDocument()->getTargetForId(rId);
}

void OOXMLFastContextHandler::sendPropertyToParent()
{
    if (mpParent != nullptr)
    {
        OOXMLPropertySet::Pointer_t pProps(mpParent->getPropertySet());

        if (pProps.get() != nullptr)
        {
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(mId, getValue(), OOXMLProperty::SPRM));
            pProps->add(pProp);
        }
    }
}

void OOXMLFastContextHandler::sendPropertiesToParent()
{
    if (mpParent != nullptr)
    {
        OOXMLPropertySet::Pointer_t pParentProps(mpParent->getPropertySet());

        if (pParentProps.get() != nullptr)
        {
            OOXMLPropertySet::Pointer_t pProps(getPropertySet());

            if (pProps.get() != nullptr)
            {
                OOXMLValue::Pointer_t pValue
                (new OOXMLPropertySetValue(getPropertySet()));

                OOXMLProperty::Pointer_t pProp(new OOXMLProperty(getId(), pValue, OOXMLProperty::SPRM));


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
  mpPropertySetAttrs(new OOXMLPropertySet)
{
}

OOXMLFastContextHandlerStream::~OOXMLFastContextHandlerStream()
{
}

void OOXMLFastContextHandlerStream::newProperty(Id nId,
                                                const OOXMLValue::Pointer_t& pVal)
{
    if (nId != 0x0)
    {
        OOXMLProperty::Pointer_t pProperty(new OOXMLProperty(nId, pVal, OOXMLProperty::ATTRIBUTE));

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
: OOXMLFastContextHandler(pContext), mpPropertySet(new OOXMLPropertySet),
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
{
    try
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
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const xml::sax::SAXException&)
    {
        throw;
    }
    catch (const uno::Exception& e)
    {
        auto a = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(e.Message, e.Context, a);
    }
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerProperties::getValue() const
{
    return OOXMLValue::Pointer_t(new OOXMLPropertySetValue(mpPropertySet));
}

void OOXMLFastContextHandlerProperties::newProperty
(Id nId, const OOXMLValue::Pointer_t& pVal)
{
    if (nId != 0x0)
    {
        OOXMLProperty::Pointer_t pProperty(new OOXMLProperty(nId, pVal, OOXMLProperty::ATTRIBUTE));

        mpPropertySet->add(pProperty);
    }
}

void OOXMLFastContextHandlerProperties::handleXNotes()
{
    switch (mnToken)
    {
    case W_TOKEN(footnoteReference):
        {
            OOXMLFootnoteHandler aFootnoteHandler(this);
            mpPropertySet->resolve(aFootnoteHandler);
        }
        break;
    case W_TOKEN(endnoteReference):
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
    case W_TOKEN(footerReference):
        {
            OOXMLFooterHandler aFooterHandler(this);
            mpPropertySet->resolve(aFooterHandler);
            aFooterHandler.finalize();
        }
        break;
    case W_TOKEN(headerReference):
        {
            OOXMLHeaderHandler aHeaderHandler(this);
            mpPropertySet->resolve(aHeaderHandler);
            aHeaderHandler.finalize();
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
    if(isForwardEvents())
    {
        OOXMLBreakHandler aBreakHandler(*mpStream);
        getPropertySet()->resolve(aBreakHandler);
    }
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

void OOXMLFastContextHandlerProperties::handleHyperlinkURL() {
    OOXMLHyperlinkURLHandler aHyperlinkURLHandler(this);
    getPropertySet()->resolve(aHyperlinkURLHandler);
}

void OOXMLFastContextHandlerProperties::setParent
(OOXMLFastContextHandler * pParent)
{
    OOXMLFastContextHandler::setParent(pParent);

    if (mpParent->getResource() == STREAM)
        mbResolve = true;
}

void OOXMLFastContextHandlerProperties::setPropertySet
(const OOXMLPropertySet::Pointer_t& pPropertySet)
{
    if (pPropertySet.get() != nullptr)
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
{
    OOXMLPropertySet::Pointer_t pPropSet(mpPropertySet->clone());
    OOXMLTable::ValuePointer_t pTmpVal
        (new OOXMLPropertySetValue(pPropSet));

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

void OOXMLFastContextHandlerValue::setValue(const OOXMLValue::Pointer_t& pValue)
{
    mpValue = pValue;
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerValue::getValue() const
{
    return mpValue;
}

void OOXMLFastContextHandlerValue::lcl_endFastElement
(Token_t Element)
{
    sendPropertyToParent();

    endAction(Element);
}

void OOXMLFastContextHandlerValue::setDefaultBooleanValue()
{
    if (mpValue.get() == nullptr)
    {
        OOXMLValue::Pointer_t pValue = OOXMLBooleanValue::Create(true);
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultIntegerValue()
{
    if (mpValue.get() == nullptr)
    {
        OOXMLValue::Pointer_t pValue = OOXMLIntegerValue::Create(0);
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultHexValue()
{
    if (mpValue.get() == nullptr)
    {
        OOXMLValue::Pointer_t pValue(new OOXMLHexValue(sal_uInt32(0)));
        setValue(pValue);
    }
}

void OOXMLFastContextHandlerValue::setDefaultStringValue()
{
    if (mpValue.get() == nullptr)
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
{
    addCurrentChild();

    mCurrentChild.set(OOXMLFastContextHandler::createFastChildContext(Element, Attribs));

    return mCurrentChild;
}

void OOXMLFastContextHandlerTable::lcl_endFastElement
(Token_t /*Element*/)
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
    if (pHandler != nullptr)
    {
        OOXMLValue::Pointer_t pValue(pHandler->getValue());

        if (pValue.get() != nullptr)
        {
            OOXMLTable::ValuePointer_t pTmpVal(pValue->clone());
            mTable.add(pTmpVal);
        }
    }
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
{
    mbForwardEventsSaved = isForwardEvents();

    // If this is the note we're looking for or this is the footnote separator one.
    if (mnMyXNoteId == getXNoteId() || static_cast<sal_uInt32>(mnMyXNoteType) == NS_ooxml::LN_Value_doc_ST_FtnEdn_separator)
        setForwardEvents(true);
    else
        setForwardEvents(false);

    startAction(Element);
}

void OOXMLFastContextHandlerXNote::lcl_endFastElement
(Token_t Element)
{
    endAction(Element);

    OOXMLFastContextHandlerProperties::lcl_endFastElement(Element);

    setForwardEvents(mbForwardEventsSaved);
}

void OOXMLFastContextHandlerXNote::checkId(const OOXMLValue::Pointer_t& pValue)
{
    mnMyXNoteId = sal_Int32(pValue->getInt());
}

void OOXMLFastContextHandlerXNote::checkType(const OOXMLValue::Pointer_t& pValue)
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
    if (isForwardEvents())
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySet;
        {
            OOXMLValue::Pointer_t pVal = OOXMLBooleanValue::Create(mnTableDepth > 0);
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_tcStart, pVal, OOXMLProperty::SPRM));
            pProps->add(pProp);
        }

        mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
    }
}

void OOXMLFastContextHandlerTextTableCell::endCell()
{
    if (isForwardEvents())
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySet;
        {
            OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(mnTableDepth);
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_tblDepth, pVal, OOXMLProperty::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(1);
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_inTbl, pVal, OOXMLProperty::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal = OOXMLBooleanValue::Create(mnTableDepth > 0);
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_tblCell, pVal, OOXMLProperty::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal = OOXMLBooleanValue::Create(mnTableDepth > 0);
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_tcEnd, pVal, OOXMLProperty::SPRM));
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
    if (mpGridAfter)
    {
        // Grid after is the same as grid before, the empty cells are just
        // inserted after the real ones, not before.
        handleGridBefore(mpGridAfter);
        mpGridAfter = nullptr;
    }

    startParagraphGroup();

    if (isForwardEvents())
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySet;
        {
            OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(mnTableDepth);
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_tblDepth, pVal, OOXMLProperty::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(1);
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_inTbl, pVal, OOXMLProperty::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(1);
            OOXMLProperty::Pointer_t pProp(new OOXMLProperty(NS_ooxml::LN_tblRow, pVal, OOXMLProperty::SPRM));
            pProps->add(pProp);
        }

        mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
    }

    startCharacterGroup();

    if (isForwardEvents())
        mpStream->utext(reinterpret_cast<const sal_uInt8*>(&uCR), 1);

    endCharacterGroup();
    endParagraphGroup();
}

void OOXMLFastContextHandlerTextTableRow::handleGridAfter(const OOXMLValue::Pointer_t& rValue)
{
    if (OOXMLFastContextHandler* pTableRowProperties = getParent())
    {
        if (OOXMLFastContextHandler* pTableRow = pTableRowProperties->getParent())
            // Save the value into the table row context, so it can be handled
            // right before the end of the row.
            pTableRow->setGridAfter(rValue);
    }
}

// Handle w:gridBefore here by faking necessary input that'll fake cells. I'm apparently
// not insane enough to find out how to add cells in dmapper.
void OOXMLFastContextHandlerTextTableRow::handleGridBefore( const OOXMLValue::Pointer_t& val )
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
            OOXMLPropertySet * pProps = new OOXMLPropertySet;
            {
                OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(mnTableDepth);
                OOXMLProperty::Pointer_t pProp
                    (new OOXMLProperty(NS_ooxml::LN_tblDepth, pVal, OOXMLProperty::SPRM));
                pProps->add(pProp);
            }
            {
                OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(1);
                OOXMLProperty::Pointer_t pProp
                    (new OOXMLProperty(NS_ooxml::LN_inTbl, pVal, OOXMLProperty::SPRM));
                pProps->add(pProp);
            }
            {
                OOXMLValue::Pointer_t pVal = OOXMLBooleanValue::Create(mnTableDepth > 0);
                OOXMLProperty::Pointer_t pProp
                    (new OOXMLProperty(NS_ooxml::LN_tblCell, pVal, OOXMLProperty::SPRM));
                pProps->add(pProp);
            }

            mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));

            // fake <w:tcBorders> with no border
            OOXMLPropertySet::Pointer_t pCellProps( new OOXMLPropertySet );
            {
                OOXMLPropertySet::Pointer_t pBorderProps( new OOXMLPropertySet );
                static Id borders[] = { NS_ooxml::LN_CT_TcBorders_top, NS_ooxml::LN_CT_TcBorders_bottom,
                    NS_ooxml::LN_CT_TcBorders_start, NS_ooxml::LN_CT_TcBorders_end };
                for(sal_uInt32 border : borders)
                    pBorderProps->add( fakeNoBorder( border ));
                OOXMLValue::Pointer_t pValue( new OOXMLPropertySetValue( pBorderProps ));
                OOXMLProperty::Pointer_t pProp
                    (new OOXMLProperty(NS_ooxml::LN_CT_TcPrBase_tcBorders, pValue, OOXMLProperty::SPRM));
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
    OOXMLPropertySet::Pointer_t pProps( new OOXMLPropertySet );
    OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(0);
    OOXMLProperty::Pointer_t pPropVal
        (new OOXMLProperty(NS_ooxml::LN_CT_Border_val, pVal, OOXMLProperty::ATTRIBUTE));
    pProps->add(pPropVal);
    OOXMLValue::Pointer_t pValue( new OOXMLPropertySetValue( pProps ));
    OOXMLProperty::Pointer_t pProp
        (new OOXMLProperty(id, pValue, OOXMLProperty::SPRM));
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
{
    mpParserState->startTable();
    mnTableDepth++;

    std::shared_ptr<OOXMLPropertySet> pProps( new OOXMLPropertySet );
    {
        OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(mnTableDepth);
        OOXMLProperty::Pointer_t pProp
            (new OOXMLProperty(NS_ooxml::LN_tblStart, pVal, OOXMLProperty::SPRM));
        pProps->add(pProp);
    }
    mpParserState->setCharacterProperties(pProps);

    startAction(Element);
}

void OOXMLFastContextHandlerTextTable::lcl_endFastElement
(Token_t Element)
{
    endAction(Element);

    std::shared_ptr<OOXMLPropertySet> pProps( new OOXMLPropertySet );
    {
        OOXMLValue::Pointer_t pVal = OOXMLIntegerValue::Create(mnTableDepth);
        OOXMLProperty::Pointer_t pProp
            (new OOXMLProperty(NS_ooxml::LN_tblEnd, pVal, OOXMLProperty::SPRM));
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
    uno::Reference<document::XDocumentPropertiesSupplier> xDocSupplier(getDocument()->getModel(), uno::UNO_QUERY_THROW);
    mrShapeContext->setDocumentProperties(xDocSupplier->getDocumentProperties());
    mrShapeContext->setDrawPage(getDocument()->getDrawPage());
    mrShapeContext->setMediaDescriptor(getDocument()->getMediaDescriptor());

    mrShapeContext->setRelationFragmentPath
        (mpParserState->getTarget());
}

OOXMLFastContextHandlerShape::~OOXMLFastContextHandlerShape()
{
}

void OOXMLFastContextHandlerShape::lcl_startFastElement
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
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
        awt::Point aPosition = mpStream->getPositionOffset();
        mrShapeContext->setPosition(aPosition);
        uno::Reference<drawing::XShape> xShape(mrShapeContext->getShape());
        if (xShape.is())
        {
            OOXMLValue::Pointer_t
                pValue(new OOXMLShapeValue(xShape));
            newProperty(NS_ooxml::LN_shape, pValue);
            m_bShapeSent = true;

            bool bIsPicture = Element == ( NMSP_dmlPicture | XML_pic );

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
{
    if (mrShapeContext.is())
    {
        mrShapeContext->endFastElement(Element);
        sendShape( Element );
    }

    OOXMLFastContextHandlerProperties::lcl_endFastElement(Element);

    // Ending the shape should be the last thing to do
    bool bIsPicture = Element == ( NMSP_dmlPicture | XML_pic );
    if ( !bIsPicture && m_bShapeStarted)
        mpStream->endShape( );
}

void SAL_CALL OOXMLFastContextHandlerShape::endUnknownElement
(const OUString & Namespace,
 const OUString & Name)
{
    if (mrShapeContext.is())
        mrShapeContext->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler >
OOXMLFastContextHandlerShape::lcl_createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    uno::Reference< xml::sax::XFastContextHandler > xContextHandler;

    bool bGroupShape = Element == Token_t(NMSP_vml | XML_group);
    // drawingML version also counts as a group shape.
    bGroupShape |= mrShapeContext->getStartToken() == Token_t(NMSP_wpg | XML_wgp);

    switch (oox::getNamespace(Element))
    {
        case NMSP_doc:
        case NMSP_vmlWord:
        case NMSP_vmlOffice:
            if (!bGroupShape)
                xContextHandler.set(OOXMLFactory::createFastChildContextFromStart(this, Element));
            SAL_FALLTHROUGH;
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
                        pWrapper->addNamespace(NMSP_doc);
                        pWrapper->addNamespace(NMSP_vmlWord);
                        pWrapper->addNamespace(NMSP_vmlOffice);
                        pWrapper->addToken( NMSP_vml|XML_textbox );
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
    if (Element == static_cast<sal_Int32>(NMSP_wps | XML_txbx) ||
        Element == static_cast<sal_Int32>(NMSP_wps | XML_linkedTxbx) )
        sendShape(Element);

    return xContextHandler;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandlerShape::createUnknownChildContext
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;

    if (mrShapeContext.is())
        xResult.set(mrShapeContext->createUnknownChildContext
            (Namespace, Name, Attribs));

    return xResult;
}

void OOXMLFastContextHandlerShape::lcl_characters
(const OUString & aChars)
{
    if (mrShapeContext.is())
        mrShapeContext->characters(aChars);
}

/*
  class OOXMLFastContextHandlerWrapper
*/

OOXMLFastContextHandlerWrapper::OOXMLFastContextHandlerWrapper
(OOXMLFastContextHandler * pParent,
 uno::Reference<XFastContextHandler> const & xContext)
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
{
    if (mxContext.is())
        mxContext->startUnknownElement(Namespace, Name, Attribs);
}

void SAL_CALL OOXMLFastContextHandlerWrapper::endUnknownElement
(const OUString & Namespace,
 const OUString & Name)
{
    if (mxContext.is())
        mxContext->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandlerWrapper::createUnknownChildContext
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
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
{
    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != nullptr)
            pHandler->attributes(Attribs);
    }
}

OOXMLFastContextHandler::ResourceEnum_t
OOXMLFastContextHandlerWrapper::getResource() const
{
    return UNKNOWN;
}

void OOXMLFastContextHandlerWrapper::addNamespace(Id nId)
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
{
    if (mxContext.is())
        mxContext->startFastElement(Element, Attribs);
}

void OOXMLFastContextHandlerWrapper::lcl_endFastElement
(Token_t Element)
{
    if (mxContext.is())
        mxContext->endFastElement(Element);
}

uno::Reference< xml::sax::XFastContextHandler >
OOXMLFastContextHandlerWrapper::lcl_createFastChildContext
(Token_t Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;

    bool bInNamespaces = mMyNamespaces.find(oox::getNamespace(Element)) != mMyNamespaces.end();
    bool bInTokens = mMyTokens.find( Element ) != mMyTokens.end( );

    // We have methods to _add_ individual tokens or whole namespaces to be
    // processed by writerfilter (instead of oox), but we have no method to
    // filter out a single token. Just hardwire the wrap token here until we
    // need a more generic solution.
    bool bIsWrap = Element == static_cast<sal_Int32>(NMSP_vmlWord | XML_wrap);
    bool bSkipImages = getDocument()->IsSkipImages() && oox::getNamespace(Element) == static_cast<sal_Int32>(NMSP_dml) &&
        !((oox::getBaseToken(Element) == XML_linkedTxbx) || (oox::getBaseToken(Element) == XML_txbx));

    if ( bInNamespaces && (!bIsWrap || static_cast<OOXMLFastContextHandlerShape*>(mpParent)->isShapeSent()) )
        xResult.set(OOXMLFactory::createFastChildContextFromStart(this, Element));
    else if (mxContext.is()  && !bSkipImages)
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
        static_cast<OOXMLFastContextHandlerShape*>(mpParent)->sendShape( Element );

    return xResult;
}

void OOXMLFastContextHandlerWrapper::lcl_characters
(const OUString & aChars)
{
    if (mxContext.is())
        mxContext->characters(aChars);
}

OOXMLFastContextHandler *
OOXMLFastContextHandlerWrapper::getFastContextHandler() const
{
    if (mxContext.is())
        return dynamic_cast<OOXMLFastContextHandler *>(mxContext.get());

    return nullptr;
}

void OOXMLFastContextHandlerWrapper::newProperty
(Id nId, const OOXMLValue::Pointer_t& pVal)
{
    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != nullptr)
            pHandler->newProperty(nId, pVal);
    }
}

void OOXMLFastContextHandlerWrapper::setPropertySet
(const OOXMLPropertySet::Pointer_t& pPropertySet)
{
    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != nullptr)
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
        if (pHandler != nullptr)
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
        if (pHandler != nullptr)
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
        if (pHandler != nullptr)
            pHandler->setId(rId);
    }
}

Id OOXMLFastContextHandlerWrapper::getId() const
{
    Id nResult = OOXMLFastContextHandler::getId();

    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != nullptr && pHandler->getId() != 0)
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
        if (pHandler != nullptr)
            pHandler->setToken(nToken);
    }
}

Token_t OOXMLFastContextHandlerWrapper::getToken() const
{
    Token_t nResult = OOXMLFastContextHandler::getToken();

    if (mxContext.is())
    {
        OOXMLFastContextHandler * pHandler = getFastContextHandler();
        if (pHandler != nullptr)
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
{
    buffer.appendOpeningTag( Element, Attribs );
    ++depthCount;
}

void OOXMLFastContextHandlerLinear::lcl_endFastElement(Token_t Element)
{
    buffer.appendClosingTag( Element );
    if( --depthCount == 0 )
        process();
}

uno::Reference< xml::sax::XFastContextHandler >
OOXMLFastContextHandlerLinear::lcl_createFastChildContext(Token_t,
    const uno::Reference< xml::sax::XFastAttributeList >&)
{
    uno::Reference< xml::sax::XFastContextHandler > xContextHandler;
    xContextHandler.set( this );
    return xContextHandler;
}

void OOXMLFastContextHandlerLinear::lcl_characters(const OUString& aChars)
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
    uno::Sequence<beans::PropertyValue> objArgs(1);
    objArgs[0].Name = "DefaultParentBaseURL";
    objArgs[0].Value <<= getDocument()->GetDocumentBaseURL();
    uno::Reference<embed::XEmbeddedObject> ref =
        container.CreateEmbeddedObject(name.GetByteSequence(), objArgs, aName);
    assert(ref.is());
    if (!ref.is())
        return;
    uno::Reference< uno::XInterface > component(ref->getComponent(), uno::UNO_QUERY_THROW);
// gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
// so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
// to RTLD_GLOBAL, so most probably a gcc bug.
    oox::FormulaImportBase& import = dynamic_cast<oox::FormulaImportBase&>(dynamic_cast<SfxBaseModel&>(*component.get()));
    import.readFormulaOoxml(buffer);
    if (isForwardEvents())
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySet;
        OOXMLValue::Pointer_t pVal( new OOXMLStarMathValue( ref ));
        OOXMLProperty::Pointer_t pProp( new OOXMLProperty( NS_ooxml::LN_starmath, pVal, OOXMLProperty::ATTRIBUTE ));
        pProps->add( pProp );
        mpStream->props( writerfilter::Reference< Properties >::Pointer_t( pProps ));
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
