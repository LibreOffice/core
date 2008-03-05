/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLFastContextHandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:04:26 $
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

#ifndef INCLUDED_OOXML_FAST_CONTEXT_HANDLER_HXX
#define INCLUDED_OOXML_FAST_CONTEXT_HANDLER_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XFASTSHAPECONTEXTHANDLER_HPP_
#include <com/sun/star/xml/sax/XFastShapeContextHandler.hpp>
#endif

#include <string>
#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/xml/sax/XFastContextHandler.hpp"
#include "OOXMLParserState.hxx"
#include "OOXMLPropertySetImpl.hxx"
#include "OOXMLDocumentImpl.hxx"
#include "RefAndPointer.hxx"
#include <ooxml/OOXMLFastTokens.hxx>

namespace writerfilter {
namespace ooxml
{
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;

typedef boost::shared_ptr<Stream> StreamPointer_t;

class OOXMLFastContextHandler:
    public ::cppu::WeakImplHelper1<
        xml::sax::XFastContextHandler>
{
public:
    typedef RefAndPointer<XFastContextHandler, OOXMLFastContextHandler>
    RefAndPointer_t;

    enum ResourceEnum_t { UNKNOWN, STREAM, PROPERTIES, TABLE, SHAPE };

    OOXMLFastContextHandler();
    explicit OOXMLFastContextHandler
    (uno::Reference< uno::XComponentContext > const & context);

    explicit OOXMLFastContextHandler
    (OOXMLFastContextHandler * pContext);

    virtual ~OOXMLFastContextHandler();

    // ::com::sun::star::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void SAL_CALL startUnknownElement
    (const ::rtl::OUString & Namespace,
     const ::rtl::OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void SAL_CALL endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void SAL_CALL endUnknownElement
    (const ::rtl::OUString & Namespace, const ::rtl::OUString & Name)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const ::rtl::OUString & Namespace,
     const ::rtl::OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void SAL_CALL characters(const ::rtl::OUString & aChars)
        throw (uno::RuntimeException, xml::sax::SAXException);

    static const uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual sal_Int64 SAL_CALL getSomething(const uno::Sequence<sal_Int8> & rId)
        throw (uno::RuntimeException);

    // local

    void setStream(Stream * pStream);

    /**
       Return value of this context(element).

       @return  the value
     */
    virtual OOXMLValue::Pointer_t getValue() const;

    /**
       Returns a string describing the type of the context.

       This is the name of the define normally.

       @return type string
     */
    virtual string getType() const { return "??"; }

    virtual ResourceEnum_t getResource() const { return UNKNOWN; }

    virtual string toString() const;

    virtual void attributes
    (const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void newProperty(const Id & rId, OOXMLValue::Pointer_t pVal);
    virtual void setPropertySet(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual OOXMLPropertySet::Pointer_t getPropertySet();

    void setToken(Token_t nToken);
    Token_t getToken() const;

    void resolveFootnote(const rtl::OUString & rId);
    void resolveEndnote(const rtl::OUString & rId);
    void resolveComment(const rtl::OUString & rId);
    void resolvePicture(const rtl::OUString & rId);
    void resolveHeader(const sal_Int32 type,
                                const rtl::OUString & rId);
    void resolveFooter(const sal_Int32 type,
                                const rtl::OUString & rId);
    void resolveOLE(const rtl::OUString & rId);

    ::rtl::OUString getTargetForId(const ::rtl::OUString & rId);

    uno::Reference < xml::sax::XFastContextHandler >
    createFromStart
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs);

    void setDocument(OOXMLDocument * pDocument);
    OOXMLDocument * getDocument();
    void setXNoteId(const ::rtl::OUString & rId);
    const rtl::OUString & getXNoteId() const;
    void setForwardEvents(bool bForwardEvents);
    bool isForwardEvents() const;
    virtual void setParent(OOXMLFastContextHandler * pParent);
    virtual void setId(Id nId);
    virtual Id getId() const;

    void setFallback(bool bFallbac);
    bool isFallback() const;

#ifdef DEBUG_MEMORY
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();
#endif

    sal_uInt32 getInstanceNumber() const;

    static void dumpOpenContexts();

protected:
    OOXMLFastContextHandler * mpParent;
    Token_t mId;
    Token_t mnToken;

    // the stream to send the stream events to.
    Stream * mpStream;

    // the current global parser state
    OOXMLParserState::Pointer_t mpParserState;

    // the table depth of this context
    unsigned int mnTableDepth;

    virtual void lcl_startFastElement
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual uno::Reference< xml::sax::XFastContextHandler >
    lcl_createFastChildContext
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void lcl_characters(const ::rtl::OUString & aChars)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void startAction(Token_t Element);
    virtual void endAction(Token_t Element);


    // Returns string for resource of this context. (debug)
    string getResourceString() const;

    virtual OOXMLPropertySet * getPicturePropSet
    (const ::rtl::OUString & rId);
    virtual void resolvePropertySetAttrs();

    void sendTableDepth() const;
    void setHandle();

    void startSectionGroup();
    void setLastParagraphInSection();
    void endSectionGroup();
    void startParagraphGroup();
    void endParagraphGroup();
    void startCharacterGroup();
    void endCharacterGroup();

    void startField();
    void fieldSeparator();
    void endField();
    void ftnednref();
    void ftnedncont();
    void ftnednsep();
    void pgNum();
    void tab();
    void cr();
    void noBreakHyphen();
    void softHyphen();
    void endOfParagraph();
    void text(const ::rtl::OUString & sText);
    virtual void propagateCharacterProperties();
    virtual bool propagatesProperties() const;
    void propagateTableProperties();
    void clearProps();

    void sendPropertyToParent();

    uno::Reference< uno::XComponentContext > getComponentContext();

    sal_uInt32 mnInstanceNumber;
    sal_uInt32 mnRefCount;
private:
    void operator =(OOXMLFastContextHandler &); // not defined

    uno::Reference< uno::XComponentContext > m_xContext;

    static sal_uInt32 mnInstanceCount;

    bool mbFallback;
};

class OOXMLFastContextHandlerNoResource : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerNoResource(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerNoResource();
};

class OOXMLFastContextHandlerStream : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerStream(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerStream();

    virtual ResourceEnum_t getResource() const { return STREAM; }

    OOXMLPropertySet::Pointer_t getPropertySetAttrs() const;

    virtual void newProperty(const Id & rId, OOXMLValue::Pointer_t pVal);
    virtual void sendProperty(Id nId);
    virtual OOXMLPropertySet::Pointer_t getPropertySet();

    virtual string toString() const;

    void handleHyperlink();

protected:
    void setPropertySetAttrs(OOXMLPropertySet::Pointer_t pPropertySetAttrs);
    virtual void resolvePropertySetAttrs();
    virtual void lcl_characters(const ::rtl::OUString & aChars)
                throw (uno::RuntimeException, xml::sax::SAXException);
private:
    mutable OOXMLPropertySet::Pointer_t mpPropertySetAttrs;
};

class OOXMLFastContextHandlerProperties : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerProperties(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerProperties();

    virtual OOXMLValue::Pointer_t getValue() const;
    virtual ResourceEnum_t getResource() const { return PROPERTIES; }

    virtual string toString() const;

    virtual void newProperty(const Id & nId, OOXMLValue::Pointer_t pVal);

    void handleXNotes();
    void handleHdrFtr();
    void handleComment();
    void handlePicture();
    void handleBreak();
    void handleOLE();

    virtual void setPropertySet(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual OOXMLPropertySet::Pointer_t getPropertySet();

protected:
    /// the properties
    OOXMLPropertySet::Pointer_t mpPropertySet;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void setParent(OOXMLFastContextHandler * pParent);

private:

    bool mbResolve;
};

class OOXMLFastContextHandlerPropertyTable :
    public OOXMLFastContextHandlerProperties
{
public:
    OOXMLFastContextHandlerPropertyTable(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerPropertyTable();

    virtual void setId(Id nId);
protected:
    Token_t mId;
    OOXMLTableImpl mTable;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);
 };

class OOXMLFastContextHandlerBooleanValue :
    public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerBooleanValue
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerBooleanValue();

protected:
    bool mbValue;

    virtual void attributes
    (const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual OOXMLValue::Pointer_t getValue() const;
    void setValue(const ::rtl::OUString & rString);
};

class OOXMLFastContextHandlerIntegerValue :
    public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerIntegerValue
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerIntegerValue();

protected:
    sal_Int32 mnValue;

    void attributes
    (const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual OOXMLValue::Pointer_t getValue() const;
};

class OOXMLFastContextHandlerStringValue :
    public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerStringValue
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerStringValue();

protected:
    ::rtl::OUString  msValue;

    void attributes
    (const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual OOXMLValue::Pointer_t getValue() const;
};

class OOXMLFastContextHandlerHexValue :
    public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerHexValue
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerHexValue();

protected:
    sal_Int32  mnValue;

    void attributes
    (const uno::Reference < xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual OOXMLValue::Pointer_t getValue() const;
};

class OOXMLFastContextHandlerListValue :
    public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerListValue
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerListValue();

protected:
    mutable OOXMLValue::Pointer_t mpValue;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual OOXMLValue::Pointer_t getValue() const;
};

class OOXMLFastContextHandlerTable : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerTable(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerTable();

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void setId(Id nId);
    virtual void newPropertySet(OOXMLPropertySet::Pointer_t pPropertySet);
protected:
    Token_t mId;
    OOXMLTableImpl mTable;

    RefAndPointer_t mCurrentChild;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual ResourceEnum_t getResource() const { return TABLE; }

    void addCurrentChild();
};

class OOXMLFastContextHandlerXNote : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerXNote(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerXNote();

    void checkId(const rtl::OUString & rId);
private:
    bool mbForwardEventsSaved;
    ::rtl::OUString msMyXNoteId;

    virtual void lcl_startFastElement
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual ResourceEnum_t getResource() const { return STREAM; }
};

class OOXMLFastContextHandlerTextTableCell : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerTextTableCell
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerTextTableCell();

    void startCell();
    void endCell();
};

class OOXMLFastContextHandlerTextTableRow : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerTextTableRow
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerTextTableRow();

    void startRow();
    void endRow();
};

class OOXMLFastContextHandlerTextTable : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerTextTable
    (OOXMLFastContextHandler * pContext);

    virtual ~OOXMLFastContextHandlerTextTable();

protected:
    virtual void lcl_startFastElement
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);
};

class OOXMLFastContextHandlerShape: public OOXMLFastContextHandlerProperties
{
public:
    explicit OOXMLFastContextHandlerShape
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerShape();

    // ::com::sun::star::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startUnknownElement
    (const ::rtl::OUString & Namespace,
     const ::rtl::OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void SAL_CALL endUnknownElement
    (const ::rtl::OUString & Namespace,
     const ::rtl::OUString & Name)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const ::rtl::OUString & Namespace,
     const ::rtl::OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual ResourceEnum_t getResource() const { return SHAPE; }
protected:
    typedef uno::Reference<XFastShapeContextHandler> ShapeContextRef;
    ShapeContextRef mrShapeContext;

    virtual void lcl_startFastElement
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual uno::Reference< xml::sax::XFastContextHandler >
    lcl_createFastChildContext
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void lcl_characters(const ::rtl::OUString & aChars)
                throw (uno::RuntimeException, xml::sax::SAXException);

};
}}
#endif // INCLUDED_OOXML_FAST_CONTEXT_HANDLER_HXX
