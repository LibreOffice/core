/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLFastContextHandler.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-24 16:01:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <iostream>
#include <set>
#include <rtl/uuid.h>
#include <com/sun/star/drawing/XShapes.hpp>
#include <resourcemodel/QNameToString.hxx>
#include <ooxml/resourceids.hxx>
#include "OOXMLFastContextHandler.hxx"
#include "OOXMLFastTokens.hxx"
#include "Handler.hxx"

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

static uno::Sequence< sal_Int8 >  CreateUnoTunnelId()
{
    static osl::Mutex aCreateMutex;
    osl::Guard<osl::Mutex> aGuard( aCreateMutex );
    uno::Sequence< sal_Int8 > aSeq( 16 );
    rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
    return aSeq;
}

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

set<OOXMLFastContextHandler *> aSetContexts;

/*
  class OOXMLFastContextHandler
 */

sal_uInt32 OOXMLFastContextHandler::mnInstanceCount = 0;

OOXMLFastContextHandler::OOXMLFastContextHandler
(uno::Reference< uno::XComponentContext > const & context)
: mpParent(NULL),
  mnToken(OOXML_FAST_TOKENS_END),
  mpStream(NULL),
  mnTableDepth(0),
  mnInstanceNumber(mnInstanceCount),
  mnRefCount(0),
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
  mnToken(OOXML_FAST_TOKENS_END),
  mpStream(NULL),
  mnTableDepth(0),
  mnInstanceNumber(mnInstanceCount),
  mnRefCount(0),
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

void OOXMLFastContextHandler::dumpOpenContexts()
{
    logger("DEBUG", "<open-contexts>");

    set<OOXMLFastContextHandler *>::iterator aIt(aSetContexts.begin());
    while (aIt != aSetContexts.end())
    {
        logger("DEBUG", "<open-context>" + (*aIt)->toString()
             + "</open-context>");
        aIt++;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%" SAL_PRI_SIZET "u", aSetContexts.size());

    logger("DEBUG", string("<count>") + buffer + "</count>");
    logger("DEBUG", "</open-contexts>");
}

#ifdef DEBUG_MEMORY
void SAL_CALL OOXMLFastContextHandler::acquire()
    throw ()
{
    mnRefCount++;

    clog << "MEMORY:" << mnInstanceNumber << ": " << getType() << ": acquire("
         << mnRefCount << ")" << endl;

    cppu::WeakImplHelper1<com::sun::star::xml::sax::XFastContextHandler>::acquire();
}

void SAL_CALL OOXMLFastContextHandler::release()
    throw ()
{
    clog << "MEMORY:" << mnInstanceNumber << ":" << getType() << ": release("
         << mnRefCount << ")" << endl;

    cppu::WeakImplHelper1<com::sun::star::xml::sax::XFastContextHandler>::release();
    mnRefCount--;
}
#endif

sal_uInt32 OOXMLFastContextHandler::getInstanceNumber() const
{
    return mnInstanceNumber;
}

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL OOXMLFastContextHandler::startFastElement
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<element>");
    logger("DEBUG", "<token>" + fastTokenToId(Element) + "</token>");
    logger("DEBUG", "<type>" + getType() + "</type>");
    logger("DEBUG", "<at-start>" + toString() + "</at-start>");
#endif
#ifdef DEBUG_MEMORY
    clog << "MEMORY:" << mnInstanceNumber << ":startFastElement" << endl;
#endif

    attributes(Attribs);
    lcl_startFastElement(Element, Attribs);
}

void SAL_CALL OOXMLFastContextHandler::startUnknownElement
(const ::rtl::OUString & Namespace, const ::rtl::OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
throw (uno::RuntimeException, xml::sax::SAXException)
{
    clog << "<element>"
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
}

void SAL_CALL OOXMLFastContextHandler::endFastElement(::sal_Int32 Element)
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<at-end>" + toString() + "</at-end>");
    logger("DEBUG", "</element>");
#endif
#ifdef DEBUG_MEMORY
    clog << "MEMORY:" << mnInstanceNumber << ":" << getType()
         << ":endFastElement" << endl;
#endif

    lcl_endFastElement(Element);
}

void OOXMLFastContextHandler::lcl_startFastElement
(::sal_Int32 /*Element*/,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    startAction();
}

void OOXMLFastContextHandler::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    endAction();
}

void SAL_CALL OOXMLFastContextHandler::endUnknownElement
(const ::rtl::OUString & Namespace, const ::rtl::OUString & Name)
throw (uno::RuntimeException, xml::sax::SAXException)
{
    clog << "end unknown element:"
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
 OOXMLFastContextHandler::createFastChildContext
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    logger("DEBUG", "<createFastChildContext>");
    logger("DEBUG", "<token>" + fastTokenToId(Element) + "</token>");
    logger("DEBUG", "<type>" + getType() + "</type>");
#endif

    uno::Reference< xml::sax::XFastContextHandler > xResult
        (lcl_createFastChildContext(Element, Attribs));

#ifdef DEBUG_CONTEXT_STACK
    logger("DEBUG", "</createFastChildContext>");
#endif

    return xResult;
}

uno::Reference< xml::sax::XFastContextHandler >
 OOXMLFastContextHandler::lcl_createFastChildContext
(::sal_Int32 /*Element*/,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    return uno::Reference< xml::sax::XFastContextHandler >
        (new OOXMLFastContextHandler(this));
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandler::createUnknownChildContext
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    clog << "createUnknownChildContext:"
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;

    return uno::Reference< xml::sax::XFastContextHandler >
        (new OOXMLFastContextHandler(*const_cast<const OOXMLFastContextHandler *>(this)));
}

void SAL_CALL OOXMLFastContextHandler::characters
(const ::rtl::OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    lcl_characters(aChars);
}

void OOXMLFastContextHandler::lcl_characters
(const ::rtl::OUString & /*aChars*/)
throw (uno::RuntimeException, xml::sax::SAXException)
{
}

const uno::Sequence< sal_Int8 > & OOXMLFastContextHandler::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = CreateUnoTunnelId();
    return aSeq;
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
(const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
 throw (uno::RuntimeException, xml::sax::SAXException)
{
}

void OOXMLFastContextHandler::startAction()
{
}

void OOXMLFastContextHandler::endAction()
{
}

string OOXMLFastContextHandler::toString() const
{
    string sResult = "(";

    char sBuffer[128];
    snprintf(sBuffer, sizeof(sBuffer), "%p(%" SAL_PRIuUINT32 ", %" SAL_PRIuUINT32 ")", this, mnInstanceNumber,
             mnRefCount);
    sResult += sBuffer;
    snprintf(sBuffer, sizeof(sBuffer), ", p:%p, ", mpParent);
    sResult += sBuffer;
    sResult += getType();
    sResult += ", ";
    sResult += getResourceString();
    sResult += ",";

    OOXMLValue::Pointer_t pVal(getValue());

    if (pVal.get() != NULL)
        sResult += pVal->toString();
    else
        sResult += "(null)";

    sResult += mpParserState->toString();

    sResult += ")";

    return sResult;
}

string OOXMLFastContextHandler::getResourceString() const
{
    return resourceToString(getResource());
}

void OOXMLFastContextHandler::setId(Id rId)
{
    mId = rId;
}

void OOXMLFastContextHandler::setToken(Token_t nToken)
{
    mnToken = nToken;
}

void OOXMLFastContextHandler::setParent
(OOXMLFastContextHandler * pParent)
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<setParent>OOXMLFastContextHandler</setParent>");
#endif

    mpParent = pParent;
}

OOXMLPropertySet * OOXMLFastContextHandler::getPicturePropSet
(const ::rtl::OUString & rId)
{
    return dynamic_cast<OOXMLDocumentImpl *>(mpParserState->getDocument())->
        getPicturePropSet(rId);
}

void OOXMLFastContextHandler::sendTableDepth() const
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<sendTableDepth/>");
#endif

    if (mnTableDepth > 0)
    {
        OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(mnTableDepth));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(0x6649, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(1));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(0x2416, pVal, OOXMLPropertyImpl::SPRM));
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
#ifdef DEBUG_ELEMENT
            logger("DEBUG", "<startCharacterGroup/>");
#endif

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
#ifdef DEBUG_ELEMENT
        logger("DEBUG", "<endCharacterGroup/>");
#endif

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

        if (mpParserState->isLastParagraphInSection())
        {
            startSectionGroup();
            mpParserState->setLastParagraphInSection(false);
        }

        if (! mpParserState->isInSectionGroup())
            startSectionGroup();

        if (! mpParserState->isInParagraphGroup())
        {
#ifdef DEBUG_ELEMENT
            logger("DEBUG", "<startParagraphGroup/>");
#endif

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
#ifdef DEBUG_ELEMENT
            logger("DEBUG", "<endParagraphGroup/>");
#endif
            mpStream->endParagraphGroup();
            mpParserState->setInParagraphGroup(false);
        }
        /*
              if (mpParserState->isLastParagraphInSection())
              endSectionGroup(); */
    }
}

void OOXMLFastContextHandler::startSectionGroup()
{
    if (isForwardEvents())
    {
        if (mpParserState->isInSectionGroup())
            endSectionGroup();

        if (! mpParserState->isInSectionGroup())
        {
#ifdef DEBUG_ELEMENT
            logger("DEBUG", "<startSectionGroup/>");
#endif
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
#ifdef DEBUG_ELEMENT
        logger("DEBUG", "<endSectionGroup/>");
#endif
            mpStream->endSectionGroup();
            mpParserState->setInSectionGroup(false);
        }
    }
}

void OOXMLFastContextHandler::setLastParagraphInSection()
{
    mpParserState->setLastParagraphInSection(true);
}

void OOXMLFastContextHandler::newProperty
(const Id & /*nId*/, OOXMLValue::Pointer_t /*pVal*/)
{
#ifdef DEBUG_PROPERTIES
    logger("DEBUG", "<property/>");
#endif
}

void OOXMLFastContextHandler::setPropertySet
(OOXMLPropertySet::Pointer_t /* pPropertySet */)
{
}

OOXMLPropertySet::Pointer_t OOXMLFastContextHandler::getPropertySet()
{
    return OOXMLPropertySet::Pointer_t();
}

void OOXMLFastContextHandler::startField()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<startField/>");
#endif
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(sFieldStart, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::fieldSeparator()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<fieldSeparator/>");
#endif
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(sFieldSep, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::endField()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<endField/>");
#endif
    startCharacterGroup();
    if (isForwardEvents())
        mpStream->text(sFieldEnd, 1);
    endCharacterGroup();
}

void OOXMLFastContextHandler::ftnednref()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<ftnednref/>");
#endif
    if (isForwardEvents())
        mpStream->utext(sFtnEdnRef, 1);
}

void OOXMLFastContextHandler::ftnednsep()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "ftnednsep/>");
#endif
    if (isForwardEvents())
        mpStream->utext(sFtnEdnSep, 1);
}

void OOXMLFastContextHandler::ftnedncont()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<ftnedncont/>");
#endif
    if (isForwardEvents())
        mpStream->text(sFtnEdnCont, 1);
}

void OOXMLFastContextHandler::pgNum()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<pgNum/>");
#endif
    if (isForwardEvents())
        mpStream->utext(sPgNum, 1);
}

void OOXMLFastContextHandler::tab()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<tab/>");
#endif
    if (isForwardEvents())
        mpStream->utext(sTab, 1);
}

void OOXMLFastContextHandler::cr()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<cr/>");
#endif
    if (isForwardEvents())
        mpStream->utext(sCR, 1);
}

void OOXMLFastContextHandler::noBreak()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<noBreak/>");
#endif
    if (isForwardEvents())
        mpStream->utext(sNoBreakHyphen, 1);
}

void OOXMLFastContextHandler::softHyphen()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<softHyphen/>");
#endif
    if (isForwardEvents())
        mpStream->utext(sSoftHyphen, 1);
}

void OOXMLFastContextHandler::endOfParagraph()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<endOfParagraph/>");
#endif
    if (! mpParserState->isInCharacterGroup())
        startCharacterGroup();
    if (isForwardEvents())
        mpStream->utext(sCR, 1);
}

void OOXMLFastContextHandler::text(const ::rtl::OUString & sText)
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", string("<text>")
           + OUStringToOString(sText, RTL_TEXTENCODING_ASCII_US).getStr()
           + "</text>");
#endif
    mpStream->utext(reinterpret_cast < const sal_uInt8 * >
                    (sText.getStr()),
                    sText.getLength());
}

void OOXMLFastContextHandler::propagateCharacterProperties()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<propagateCharacterProperties/>");
#endif

    mpParserState->setCharacterProperties(getPropertySet());
}

bool OOXMLFastContextHandler::propagatesCharacterProperties() const
{
    return false;
}

void OOXMLFastContextHandler::clearProps()
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<clearProps/>");
#endif

    setPropertySet(OOXMLPropertySet::Pointer_t(new OOXMLPropertySetImpl()));
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
    mpParserState->setForwardEvents(bForwardEvents);
}

bool OOXMLFastContextHandler::isForwardEvents() const
{
    return mpParserState->isForwardEvents();
}

void OOXMLFastContextHandler::setXNoteId(const rtl::OUString & rId)
{
    mpParserState->setXNoteId(rId);
}

const rtl::OUString & OOXMLFastContextHandler::getXNoteId() const
{
    return mpParserState->getXNoteId();
}

void OOXMLFastContextHandler::resolveFootnote
(const rtl::OUString & rId)
{
    mpParserState->getDocument()->resolveFootnote
        (*mpStream, rId);
}

void OOXMLFastContextHandler::resolveEndnote(const rtl::OUString & rId)
{
    mpParserState->getDocument()->resolveEndnote
        (*mpStream, rId);
}

void OOXMLFastContextHandler::resolveComment(const rtl::OUString & rId)
{
    mpParserState->getDocument()->resolveComment(*mpStream, rId);
}

void OOXMLFastContextHandler::resolvePicture(const rtl::OUString & rId)
{
    mpParserState->getDocument()->resolvePicture(*mpStream, rId);
}

void OOXMLFastContextHandler::resolveHeader
(const sal_Int32 type, const rtl::OUString & rId)
{
    mpParserState->getDocument()->resolveHeader(*mpStream, type, rId);
}

void OOXMLFastContextHandler::resolveFooter
(const sal_Int32 type, const rtl::OUString & rId)
{
    mpParserState->getDocument()->resolveFooter(*mpStream, type, rId);
}

::rtl::OUString OOXMLFastContextHandler::getTargetForId
(const ::rtl::OUString & rId)
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
                pProp(new OOXMLPropertyImpl(mId, getValue(),
                                            OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
    }
}

uno::Reference< uno::XComponentContext >
OOXMLFastContextHandler::getComponentContext()
{
    return m_xContext;
}

/*
  class OOXMLFastContextHandlerNoResource
 */
OOXMLFastContextHandlerNoResource::OOXMLFastContextHandlerNoResource
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerNoResource::~OOXMLFastContextHandlerNoResource()
{
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

#ifdef DEBUG_PROPERTIES
        logger("DEBUG", "<property>"
               + xmlify(pProperty->toString()) + "</property>");
#endif

        mpPropertySetAttrs->add(pProperty);
    }
}

void OOXMLFastContextHandlerStream::sendProperty(Id nId)
{
#ifdef DEBUG_PROPERTIES
    logger("DEBUG", "<sendProperty id='" + (*QNameToString::Instance())(nId)
           + "'>" + xmlify(getPropertySetAttrs()->toString())
           + "</sendProperty>");
#endif

    OOXMLPropertySetEntryToString aHandler(nId);
    getPropertySetAttrs()->resolve(aHandler);
    const ::rtl::OUString & sText = aHandler.getString();
    mpStream->utext(reinterpret_cast < const sal_uInt8 * >
                    (sText.getStr()),
                    sText.getLength());
}

void OOXMLFastContextHandlerStream::setPropertySetAttrs
(OOXMLPropertySet::Pointer_t pPropertySetAttrs)
{
    mpPropertySetAttrs = pPropertySetAttrs;
}

OOXMLPropertySet::Pointer_t
OOXMLFastContextHandlerStream::getPropertySetAttrs() const
{
    return mpPropertySetAttrs;
}

void OOXMLFastContextHandlerStream::resolvePropertySetAttrs()
{
    mpStream->props(mpPropertySetAttrs);
}

void OOXMLFastContextHandlerStream::handleHyperlink()
{
    OOXMLHyperlinkHandler aHyperlinkHandler(this);
    getPropertySetAttrs()->resolve(aHyperlinkHandler);
}

void OOXMLFastContextHandlerStream::lcl_characters
(const ::rtl::OUString & rChars)
throw (uno::RuntimeException, xml::sax::SAXException)
{
    text(rChars);
}

string OOXMLFastContextHandlerStream::toString() const
{
    string sResult = "(";
    char sBuffer[128];
    snprintf(sBuffer, sizeof(sBuffer), "%p(%" SAL_PRIuUINT32 ", %" SAL_PRIuUINT32 ")", this, mnInstanceNumber,
             mnRefCount);
    sResult += sBuffer;
    snprintf(sBuffer, sizeof(sBuffer), ", p:%p, ", mpParent);
    sResult += sBuffer;
    sResult += getType();
    sResult += ", ";
    sResult += getResourceString();

    if (mpPropertySetAttrs.get() != NULL)
    {
        sResult += ", ";

        sResult += xmlify(mpPropertySetAttrs->toString());
    }

    sResult += ")";

    return sResult;
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
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mbResolve)
    {
        if (isForwardEvents())
            mpStream->props(mpPropertySet);
    }
    else
    {
        OOXMLValue::Pointer_t pVal
            (new OOXMLPropertySetValue(mpPropertySet));

        OOXMLPropertyImpl::Pointer_t pProperty
            (new OOXMLPropertyImpl(mId, pVal, OOXMLPropertyImpl::SPRM));

        OOXMLPropertySet::Pointer_t pProperties = (*mpParent).getPropertySet();

        if (pProperties.get() != NULL)
            pProperties->add(pProperty);
#ifdef DEBUG_PROPERTIES
        else if (! propagatesCharacterProperties())
            logger("DEBUG", "<warning>properties lost</warning>");
#endif
    }

    endAction();
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerProperties::getValue() const
{
    return OOXMLValue::Pointer_t(new OOXMLPropertySetValue(mpPropertySet));
}

string OOXMLFastContextHandlerProperties::toString() const
{
    string sResult = "(";
    char sBuffer[128];
    snprintf(sBuffer, sizeof(sBuffer), "%p(%" SAL_PRIuUINT32 ", %" SAL_PRIuUINT32 ")", this, mnInstanceNumber,
             mnRefCount);
    sResult += sBuffer;
    snprintf(sBuffer, sizeof(sBuffer), ", p:%p, ", mpParent);
    sResult += sBuffer;
    sResult += getType();
    sResult += ", ";
    sResult += getResourceString();
    sResult += ",";
    sResult += mbResolve ? "resolve" : "noResolve";

    if (mpPropertySet.get() != NULL)
    {
        sResult += ", ";

        sResult += xmlify(mpPropertySet->toString());
    }

    sResult += ")";

    return sResult;
}

void OOXMLFastContextHandlerProperties::newProperty
(const Id & rId, OOXMLValue::Pointer_t pVal)
{
    if (rId != 0x0)
    {
        OOXMLPropertyImpl::Pointer_t pProperty
            (new OOXMLPropertyImpl(rId, pVal, OOXMLPropertyImpl::ATTRIBUTE));

#ifdef DEBUG_PROPERTIES
        logger("DEBUG", string("<property>") +
               xmlify(pProperty->toString()) + "</property>");
#endif

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
#ifdef DEBUG_PICTURE
    logger("DEBUG", "handle picture");
#endif

    OOXMLPictureHandler aPictureHandler(this);
    getPropertySet()->resolve(aPictureHandler);
}

void OOXMLFastContextHandlerProperties::handleBreak()
{
    OOXMLBreakHandler aBreakHandler(*mpStream, this);
    getPropertySet()->resolve(aBreakHandler);
}

void OOXMLFastContextHandlerProperties::setParent
(OOXMLFastContextHandler * pParent)
{
#ifdef DEBUG_ELEMENT
    logger("DEBUG", "<setParent>OOXMLFastContextHandlerProperties"
           + resourceToString(mpParent->getResource())
           + "</setParent>");
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
OOXMLFastContextHandlerProperties::getPropertySet()
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

void OOXMLFastContextHandlerPropertyTable::setId(Id nId)
{
    mId = nId;
}

void OOXMLFastContextHandlerPropertyTable::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    OOXMLPropertySet::Pointer_t pPropSet(mpPropertySet->clone());
    OOXMLTableImpl::ValuePointer_t pTmpVal
        (new OOXMLPropertySetValue(pPropSet));

    mTable.add(pTmpVal);

    writerfilter::Reference<Table>::Pointer_t pTable(mTable.clone());
    mpStream->table(mId, pTable);

    endAction();
}

/*
  class OOXMLFastContextHandlerBooleanValue
 */

OOXMLFastContextHandlerBooleanValue::OOXMLFastContextHandlerBooleanValue
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext), mbValue(true)
{
}

OOXMLFastContextHandlerBooleanValue::~OOXMLFastContextHandlerBooleanValue()
{
}

void OOXMLFastContextHandlerBooleanValue::attributes
(const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
 throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (Attribs->hasAttribute(NS_wordprocessingml|OOXML_val))
        mbValue = Attribs->getValue(NS_wordprocessingml|OOXML_val).toBoolean();
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerBooleanValue::getValue() const
{
    return OOXMLValue::Pointer_t(new OOXMLBooleanValue(mbValue));
}

void OOXMLFastContextHandlerBooleanValue::setValue
(const ::rtl::OUString & rString)
{
    static rtl::OUString sOn(RTL_CONSTASCII_USTRINGPARAM("on"));
    static rtl::OUString sOff(RTL_CONSTASCII_USTRINGPARAM("off"));
    static rtl::OUString sTrue(RTL_CONSTASCII_USTRINGPARAM("true"));
    static rtl::OUString sFalse(RTL_CONSTASCII_USTRINGPARAM("false"));

    if (rString == sOn || rString == sTrue)
        mbValue = true;
    else if (rString == sOff || rString == sFalse)
        mbValue = false;
}

void OOXMLFastContextHandlerBooleanValue::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    sendPropertyToParent();

    endAction();
}

/*
  class OOXMLFastContextHandlerIntegerValue
 */

OOXMLFastContextHandlerIntegerValue::OOXMLFastContextHandlerIntegerValue
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext), mnValue(0)
{
}

OOXMLFastContextHandlerIntegerValue::~OOXMLFastContextHandlerIntegerValue()
{
}

void OOXMLFastContextHandlerIntegerValue::attributes
(const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
 throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (Attribs->hasAttribute(NS_wordprocessingml|OOXML_val))
        mnValue = Attribs->getValue(NS_wordprocessingml|OOXML_val).toInt32();
}

void OOXMLFastContextHandlerIntegerValue::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    sendPropertyToParent();

    endAction();
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerIntegerValue::getValue() const
{
    return OOXMLValue::Pointer_t(new OOXMLIntegerValue(mnValue));
}

/*
  class OOXMLFastContextHandlerStringValue
 */

OOXMLFastContextHandlerStringValue::OOXMLFastContextHandlerStringValue
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerStringValue::~OOXMLFastContextHandlerStringValue()
{
}

void OOXMLFastContextHandlerStringValue::attributes
(const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
 throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (Attribs->hasAttribute(NS_wordprocessingml|OOXML_val))
        msValue = Attribs->getValue(NS_wordprocessingml|OOXML_val);
}

void OOXMLFastContextHandlerStringValue::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    sendPropertyToParent();

    endAction();
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerStringValue::getValue() const
{
    return OOXMLValue::Pointer_t(new OOXMLStringValue(msValue));
}

/*
  class OOXMLFastContextHandlerHexValue
 */

OOXMLFastContextHandlerHexValue::OOXMLFastContextHandlerHexValue
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext), mnValue(0)
{
}

OOXMLFastContextHandlerHexValue::~OOXMLFastContextHandlerHexValue()
{
}

void OOXMLFastContextHandlerHexValue::attributes
(const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
 throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (Attribs->hasAttribute(NS_wordprocessingml|OOXML_val))
        mnValue = Attribs->getValue(NS_wordprocessingml|OOXML_val).toInt32(16);
}

void OOXMLFastContextHandlerHexValue::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    sendPropertyToParent();

    endAction();
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerHexValue::getValue() const
{
    return OOXMLValue::Pointer_t(new OOXMLIntegerValue(mnValue));
}

/*
  class OOXMLFastContextHandlerListValue
 */

OOXMLFastContextHandlerListValue::OOXMLFastContextHandlerListValue
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerListValue::~OOXMLFastContextHandlerListValue()
{
}

void OOXMLFastContextHandlerListValue::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    sendPropertyToParent();

    endAction();
}

OOXMLValue::Pointer_t OOXMLFastContextHandlerListValue::getValue() const
{
    return mpValue;
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
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    addCurrentChild();

    mCurrentChild.set
        (OOXMLFastContextHandler::createFastChildContext(Element, Attribs));

    return mCurrentChild;
}

void OOXMLFastContextHandlerTable::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    addCurrentChild();

    writerfilter::Reference<Table>::Pointer_t pTable(mTable.clone());
    if (isForwardEvents() && mId != OOXML_FAST_TOKENS_END)
        mpStream->table(mId, pTable);
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

void OOXMLFastContextHandlerTable::setId(Id nId)
{
    mId = nId;
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
: OOXMLFastContextHandler(pContext)
{
}

OOXMLFastContextHandlerXNote::~OOXMLFastContextHandlerXNote()
{
}

void OOXMLFastContextHandlerXNote::lcl_startFastElement
(::sal_Int32 /*Element*/,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    mbForwardEventsSaved = isForwardEvents();
    setForwardEvents(false);

    startAction();
}

void OOXMLFastContextHandlerXNote::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    endAction();

    setForwardEvents(mbForwardEventsSaved);
}

void OOXMLFastContextHandlerXNote::checkId(const rtl::OUString & rId)
{
    if (rId.compareTo(getXNoteId()) == 0)
        setForwardEvents(true);
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
                (new OOXMLPropertyImpl(0x6649, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(1));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(0x2416, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(mnTableDepth));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(0x244b, pVal, OOXMLPropertyImpl::SPRM));
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
    startRow();
}

OOXMLFastContextHandlerTextTableRow::~OOXMLFastContextHandlerTextTableRow()
{
    endRow();
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
                (new OOXMLPropertyImpl(0x6649, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(1));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(0x2416, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }
        {
            OOXMLValue::Pointer_t pVal
                (new OOXMLIntegerValue(1));
            OOXMLProperty::Pointer_t pProp
                (new OOXMLPropertyImpl(0x244c, pVal, OOXMLPropertyImpl::SPRM));
            pProps->add(pProp);
        }

        mpStream->props(writerfilter::Reference<Properties>::Pointer_t(pProps));
    }

    startCharacterGroup();

    if (isForwardEvents())
        mpStream->utext(s0xd, 1);

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
}

void OOXMLFastContextHandlerTextTable::lcl_startFastElement
(::sal_Int32 /*Element*/,
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    mnTableDepth++;

    startAction();
}

void OOXMLFastContextHandlerTextTable::lcl_endFastElement
(::sal_Int32 /*Element*/)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    endAction();

    mnTableDepth--;
}

/*
  class OOXMLFastContextHandlerShape
 */

class ShapesNoAdd:
    public ::cppu::WeakImplHelper1<
        drawing::XShapes>
{
public:
    explicit ShapesNoAdd(uno::Reference< uno::XComponentContext > const & context, uno::Reference< drawing::XShapes> const & xShapes);

    // container::XElementAccess:
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException);

    // container::XIndexAccess:
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException);
    virtual uno::Any SAL_CALL getByIndex(::sal_Int32 Index) throw (uno::RuntimeException, lang::IndexOutOfBoundsException, lang::WrappedTargetException);

    // drawing::XShapes:
    virtual void SAL_CALL add(const uno::Reference< drawing::XShape > & xShape) throw (uno::RuntimeException);
    virtual void SAL_CALL remove(const uno::Reference< drawing::XShape > & xShape) throw (uno::RuntimeException);

private:
    ShapesNoAdd(ShapesNoAdd &); // not defined
    void operator =(ShapesNoAdd &); // not defined

    virtual ~ShapesNoAdd() {}

    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< drawing::XShapes > m_xShapes;
};

ShapesNoAdd::ShapesNoAdd(uno::Reference< uno::XComponentContext > const & context, uno::Reference< drawing::XShapes> const & xShapes) :
m_xContext(context), m_xShapes(xShapes)
{}

// container::XElementAccess:
uno::Type SAL_CALL ShapesNoAdd::getElementType() throw (uno::RuntimeException)
{
    return m_xShapes->getElementType();
}

::sal_Bool SAL_CALL ShapesNoAdd::hasElements() throw (uno::RuntimeException)
{
    return m_xShapes->hasElements();
}

// container::XIndexAccess:
::sal_Int32 SAL_CALL ShapesNoAdd::getCount() throw (uno::RuntimeException)
{
    return m_xShapes->getCount();
}

uno::Any SAL_CALL ShapesNoAdd::getByIndex(::sal_Int32 Index) throw (uno::RuntimeException, lang::IndexOutOfBoundsException, lang::WrappedTargetException)
{
    return m_xShapes->getByIndex(Index);
}

// drawing::XShapes:
void SAL_CALL ShapesNoAdd::add(const uno::Reference< drawing::XShape > & /*xShape*/) throw (uno::RuntimeException)
{
    // noop!
}

void SAL_CALL ShapesNoAdd::remove(const uno::Reference< drawing::XShape > & xShape) throw (uno::RuntimeException)
{
    m_xShapes->remove(xShape);
}

OOXMLFastContextHandlerShape::OOXMLFastContextHandlerShape
(OOXMLFastContextHandler * pContext)
: OOXMLFastContextHandlerProperties(pContext)
{
    uno::Reference<uno::XComponentContext> xContext(getComponentContext());

    if (xContext.is())
    {
        uno::Reference<XMultiComponentFactory> rServiceManager
            (xContext->getServiceManager());

        mrShapeContext.set
            (rServiceManager->
              createInstanceWithContext
              (::rtl::OUString
               (RTL_CONSTASCII_USTRINGPARAM
                ("com.sun.star.xml.sax.FastShapeContextHandler")), xContext),
              uno::UNO_QUERY);

        if (mrShapeContext.is())
        {
            mrShapeContext->setModel(getDocument()->getModel());

            uno::Reference<drawing::XShapes> xShapes(new ShapesNoAdd(xContext, getDocument()->getShapes()));
            mrShapeContext->setShapes(xShapes);
        }
    }
}

OOXMLFastContextHandlerShape::~OOXMLFastContextHandlerShape()
{
}

void OOXMLFastContextHandlerShape::lcl_startFastElement
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    logger("DEBUG", "<info>OOXMLFastContextHandlerShape</info>");
#endif
    if (mrShapeContext.is())
        mrShapeContext->startFastElement(Element, Attribs);
}

void SAL_CALL OOXMLFastContextHandlerShape::startUnknownElement
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mrShapeContext.is())
        mrShapeContext->startUnknownElement(Namespace, Name, Attribs);
}

void OOXMLFastContextHandlerShape::lcl_endFastElement
(::sal_Int32 Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    logger("DEBUG", "<info>OOXMLFastContextHandlerShape</info>");
#endif
    if (mrShapeContext.is())
    {
        mrShapeContext->endFastElement(Element);

        uno::Reference<drawing::XShape> xShape(mrShapeContext->getShape());

#ifdef DEBUG_ELEMENT
        uno::Reference<beans::XPropertySet> xPropSet
            (xShape, uno::UNO_QUERY_THROW);
        logger("DEBUG", propertysetToString(xPropSet));
#endif

        OOXMLValue::Pointer_t
            pValue(new OOXMLShapeValue(xShape));
        newProperty(NS_ooxml::LN_shape, pValue);
    }

    OOXMLFastContextHandlerProperties::lcl_endFastElement(Element);
}

void SAL_CALL OOXMLFastContextHandlerShape::endUnknownElement
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mrShapeContext.is())
        mrShapeContext->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler >
OOXMLFastContextHandlerShape::lcl_createFastChildContext
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    logger("DEBUG", "<info>OOXMLFastContextHandlerShape</info>");
#endif
    uno::Reference< xml::sax::XFastContextHandler > xContextHandler;

    if (mrShapeContext.is())
        xContextHandler.set
            (mrShapeContext->createFastChildContext(Element, Attribs));
    else
        xContextHandler.set(new OOXMLFastContextHandlerShape(this));

    return xContextHandler;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
OOXMLFastContextHandlerShape::createUnknownChildContext
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name,
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
(const ::rtl::OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mrShapeContext.is())
        mrShapeContext->characters(aChars);
}

}}
