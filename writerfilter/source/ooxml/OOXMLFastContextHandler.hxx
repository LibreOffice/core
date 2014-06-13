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

#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTCONTEXTHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTCONTEXTHANDLER_HXX

#include <com/sun/star/xml/sax/XFastShapeContextHandler.hpp>

#include <string>
#include <set>
#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/xml/sax/XFastContextHandler.hpp"
#include "OOXMLParserState.hxx"
#include "OOXMLPropertySetImpl.hxx"
#include "OOXMLDocumentImpl.hxx"
#include <ooxml/OOXMLFastTokens.hxx>
#include <svtools/embedhlp.hxx>

#include <oox/mathml/import.hxx>
#include <oox/mathml/importutils.hxx>

namespace writerfilter {
namespace ooxml
{

typedef boost::shared_ptr<Stream> StreamPointer_t;

/**
 * Struct to store our 'alternate state'. If multiple mc:AlternateContent
 * elements arrive, then while the inner ones are active, the original state is
 * saved away, and once they inner goes out of scope, the original state is
 * restored.
 */
struct SavedAlternateState
{
    bool m_bDiscardChildren;
    bool m_bTookChoice; ///< Did we take the Choice or want Fallback instead?
};

class OOXMLFastContextHandler:
    public ::cppu::WeakImplHelper1<
        xml::sax::XFastContextHandler>
{
public:
    typedef boost::shared_ptr<OOXMLFastContextHandler> Pointer_t;

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
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL startUnknownElement
    (const OUString & Namespace,
     const OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL endUnknownElement
    (const OUString & Namespace, const OUString & Name)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const OUString & Namespace,
     const OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL characters(const OUString & aChars)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

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

    virtual ResourceEnum_t getResource() const { return STREAM; }

    virtual void attributes
    (const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException);

    virtual void newProperty(const Id & rId, OOXMLValue::Pointer_t pVal);
    virtual void setPropertySet(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual OOXMLPropertySet::Pointer_t getPropertySet() const;

    virtual void setToken(Token_t nToken);
    virtual Token_t getToken() const;

    void resolveFootnote(const sal_Int32 nId);
    void resolveEndnote(const sal_Int32 nId);
    void resolveComment(const sal_Int32 nId);
    void resolvePicture(const OUString & rId);
    void resolveHeader(const sal_Int32 type,
                                const OUString & rId);
    void resolveFooter(const sal_Int32 type,
                                const OUString & rId);
    void resolveData(const OUString & rId);

    OUString getTargetForId(const OUString & rId);

    uno::Reference < xml::sax::XFastContextHandler >
    createFromStart
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs);

    void setDocument(OOXMLDocumentImpl* pDocument);
    OOXMLDocumentImpl* getDocument();
    void setXNoteId(OOXMLValue::Pointer_t pValue);
    void setXNoteId(const sal_Int32 nId);
    sal_Int32 getXNoteId() const;
    void setForwardEvents(bool bForwardEvents);
    bool isForwardEvents() const;
    virtual void setParent(OOXMLFastContextHandler * pParent);
    virtual void setId(Id nId);
    virtual Id getId() const;

    void setDefine(Id nDefine);
    Id getDefine() const { return mnDefine;}

    OOXMLParserState::Pointer_t getParserState() const { return mpParserState;}

    void sendTableDepth() const;
    void setHandle();

    void startSectionGroup();
    void setLastParagraphInSection();
    void endSectionGroup();
    void startParagraphGroup();
    void endParagraphGroup();
    void startCharacterGroup();
    void endCharacterGroup();
    void startSdt();
    void endSdt();

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
    void handleLastParagraphInSection();
    void endOfParagraph();
    void text(const OUString & sText);
    void positionOffset(const OUString & sText);
    void alignH(const OUString & sText);
    void alignV(const OUString & sText);
    void positivePercentage(const OUString& rText);
    void startTxbxContent();
    void endTxbxContent();
    virtual void propagateCharacterProperties();
    virtual void propagateCharacterPropertiesAsSet(const Id & rId);
    virtual void propagateTableProperties();
    virtual void propagateRowProperties();
    virtual void propagateCellProperties();
    virtual bool propagatesProperties() const;
    void sendPropertiesWithId(const Id & rId);
    void sendPropertiesToParent();
    void sendCellProperties();
    void sendRowProperties();
    void sendTableProperties();
    void clearTableProps();
    void clearProps();

    virtual void setDefaultBooleanValue();
    virtual void setDefaultIntegerValue();
    virtual void setDefaultHexValue();
    virtual void setDefaultStringValue();

    void sendPropertyToParent();

#if OSL_DEBUG_LEVEL > 1
    virtual void dumpXml( const TagLogger::Pointer_t pLogger ) const;
#endif

    sal_uInt32 getInstanceNumber() { return mnInstanceNumber; }
protected:
    OOXMLFastContextHandler * mpParent;
    Id mId;
    Id mnDefine;
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

    virtual void lcl_characters(const OUString & aChars)
        throw (uno::RuntimeException, xml::sax::SAXException);

    void startAction(Token_t Element);
    void endAction(Token_t Element);

    virtual OOXMLPropertySet * getPicturePropSet
    (const OUString & rId);
    virtual void resolvePropertySetAttrs();

    uno::Reference< uno::XComponentContext > getComponentContext() { return m_xContext;}

    sal_uInt32 mnInstanceNumber;

    bool inPositionV;

private:
    void operator =(OOXMLFastContextHandler &); // not defined
    /// Handles AlternateContent. Returns true, if children of the current element should be ignored.
    bool prepareMceContext(Token_t nElement, const uno::Reference<xml::sax::XFastAttributeList>& Attribs);

    uno::Reference< uno::XComponentContext > m_xContext;
    bool m_bDiscardChildren;
    bool m_bTookChoice; ///< Did we take the Choice or want Fallback instead?
    std::stack<SavedAlternateState> m_aSavedAlternateStates;

    static sal_uInt32 mnInstanceCount;

};

class OOXMLFastContextHandlerStream : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerStream(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerStream();

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return STREAM; }

    OOXMLPropertySet::Pointer_t getPropertySetAttrs() const { return mpPropertySetAttrs;}

    virtual void newProperty(const Id & rId, OOXMLValue::Pointer_t pVal) SAL_OVERRIDE;
    virtual void sendProperty(Id nId);
    virtual OOXMLPropertySet::Pointer_t getPropertySet() const SAL_OVERRIDE;

    void handleHyperlink();

protected:
    virtual void resolvePropertySetAttrs() SAL_OVERRIDE;

private:
    mutable OOXMLPropertySet::Pointer_t mpPropertySetAttrs;
};

class OOXMLFastContextHandlerProperties : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerProperties(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerProperties();

    virtual OOXMLValue::Pointer_t getValue() const SAL_OVERRIDE;
    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return PROPERTIES; }

    virtual void newProperty(const Id & nId, OOXMLValue::Pointer_t pVal) SAL_OVERRIDE;

    void handleXNotes();
    void handleHdrFtr();
    void handleComment();
    void handlePicture();
    void handleBreak();
    void handleOLE();
    void handleFontRel();

    virtual void setPropertySet(OOXMLPropertySet::Pointer_t pPropertySet) SAL_OVERRIDE;
    virtual OOXMLPropertySet::Pointer_t getPropertySet() const SAL_OVERRIDE;

#if OSL_DEBUG_LEVEL > 1
    virtual void dumpXml( const TagLogger::Pointer_t pLogger ) const;
#endif

protected:
    /// the properties
    OOXMLPropertySet::Pointer_t mpPropertySet;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;
    virtual void setParent(OOXMLFastContextHandler * pParent) SAL_OVERRIDE;

private:

    bool mbResolve;
};

class OOXMLFastContextHandlerPropertyTable :
    public OOXMLFastContextHandlerProperties
{
public:
    OOXMLFastContextHandlerPropertyTable(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerPropertyTable();

protected:
    OOXMLTableImpl mTable;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;
 };

class OOXMLFastContextHandlerValue :
    public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerValue
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerValue();

    virtual void setValue(OOXMLValue::Pointer_t pValue);
    virtual OOXMLValue::Pointer_t getValue() const SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
    throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual string getType() const SAL_OVERRIDE { return "Value"; }

    virtual void setDefaultBooleanValue() SAL_OVERRIDE;
    virtual void setDefaultIntegerValue() SAL_OVERRIDE;
    virtual void setDefaultHexValue() SAL_OVERRIDE;
    virtual void setDefaultStringValue() SAL_OVERRIDE;

protected:
    OOXMLValue::Pointer_t mpValue;
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
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void newPropertySet(OOXMLPropertySet::Pointer_t pPropertySet);
protected:
    OOXMLTableImpl mTable;

    uno::Reference<XFastContextHandler> mCurrentChild;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return TABLE; }

    virtual string getType() const SAL_OVERRIDE { return "Table"; }

    void addCurrentChild();
};

class OOXMLFastContextHandlerXNote : public OOXMLFastContextHandlerProperties
{
public:
    OOXMLFastContextHandlerXNote(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerXNote();

    void checkId(OOXMLValue::Pointer_t pValue);

    void checkType(OOXMLValue::Pointer_t pValue);

    virtual string getType() const SAL_OVERRIDE { return "XNote"; }

private:
    bool mbForwardEventsSaved;
    sal_Int32 mnMyXNoteId;
    sal_Int32 mnMyXNoteType;

    virtual void lcl_startFastElement
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return STREAM; }
};

class OOXMLFastContextHandlerTextTableCell : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerTextTableCell
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerTextTableCell();

    virtual string getType() const SAL_OVERRIDE { return "TextTableCell"; }

    void startCell();
    void endCell();
};

class OOXMLFastContextHandlerTextTableRow : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerTextTableRow
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerTextTableRow();

    virtual string getType() const SAL_OVERRIDE { return "TextTableRow"; }

    void startRow();
    void endRow();
    void handleGridBefore( OOXMLValue::Pointer_t val );
private:
    OOXMLProperty::Pointer_t fakeNoBorder( Id id );
};

class OOXMLFastContextHandlerTextTable : public OOXMLFastContextHandler
{
public:
    OOXMLFastContextHandlerTextTable
    (OOXMLFastContextHandler * pContext);

    virtual ~OOXMLFastContextHandlerTextTable();

    virtual string getType() const SAL_OVERRIDE { return "TextTable"; }

protected:
    virtual void lcl_startFastElement
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;
};

class OOXMLFastContextHandlerShape: public OOXMLFastContextHandlerProperties
{
private:
    bool m_bShapeSent;
    bool m_bShapeStarted;

public:
    explicit OOXMLFastContextHandlerShape
    (OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerShape();

    virtual string getType() const SAL_OVERRIDE { return "Shape"; }

    // ::com::sun::star::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startUnknownElement
    (const OUString & Namespace,
     const OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL endUnknownElement
    (const OUString & Namespace,
     const OUString & Name)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const OUString & Namespace,
     const OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void setToken(Token_t nToken) SAL_OVERRIDE;

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return SHAPE; }

    void sendShape( Token_t Element );
    bool isShapeSent( ) { return m_bShapeSent; }

protected:
    typedef css::uno::Reference<css::xml::sax::XFastShapeContextHandler> ShapeContextRef;
    ShapeContextRef mrShapeContext;

    virtual void lcl_startFastElement
    (Token_t Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;

    virtual uno::Reference< xml::sax::XFastContextHandler >
    lcl_createFastChildContext
    (Token_t Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;

    virtual void lcl_characters(const OUString & aChars)
                throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;

};

/**
   OOXMLFastContextHandlerWrapper wraps an OOXMLFastContextHandler.

   The method calls for the interface
   ::com::sun::star::xml::sax::XFastContextHandler are forwarded to the wrapped
   OOXMLFastContextHandler.
 */
class OOXMLFastContextHandlerWrapper : public OOXMLFastContextHandler
{
public:
    explicit OOXMLFastContextHandlerWrapper
    (OOXMLFastContextHandler * pParent,
     uno::Reference<XFastContextHandler>  xContext);
    virtual ~OOXMLFastContextHandlerWrapper();

    // ::com::sun::star::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startUnknownElement
    (const OUString & Namespace,
     const OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL endUnknownElement
    (const OUString & Namespace,
     const OUString & Name)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const OUString & Namespace,
     const OUString & Name,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void attributes
    (const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE;

    void addNamespace(const Id & nId);
    void addToken( Token_t Element );

    virtual void newProperty(const Id & rId, OOXMLValue::Pointer_t pVal) SAL_OVERRIDE;
    virtual void setPropertySet(OOXMLPropertySet::Pointer_t pPropertySet) SAL_OVERRIDE;
    virtual OOXMLPropertySet::Pointer_t getPropertySet() const SAL_OVERRIDE;

    virtual string getType() const SAL_OVERRIDE;

protected:
    virtual void lcl_startFastElement
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual uno::Reference< xml::sax::XFastContextHandler >
    lcl_createFastChildContext
    (Token_t Element,
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual void lcl_characters(const OUString & aChars)
                throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual void setId(Id nId) SAL_OVERRIDE;
    virtual Id getId() const SAL_OVERRIDE;

    virtual void setToken(Token_t nToken) SAL_OVERRIDE;
    virtual Token_t getToken() const SAL_OVERRIDE;

private:
    uno::Reference<XFastContextHandler> mxContext;
    set<Id> mMyNamespaces;
    set<Token_t> mMyTokens;
    OOXMLPropertySet::Pointer_t mpPropertySet;

    OOXMLFastContextHandler * getFastContextHandler() const;
};

/**
 A class that converts from XFastParser/XFastContextHandler usage to a liner XML stream of data.

 The purpose of this class is to convert the rather complex XFastContextHandler-based XML
 processing that requires context subclasses, callbacks, etc. into a linear stream of XML tokens
 that can be handled simply by reading the tokens one by one and directly processing them.
 See the oox::formulaimport::XmlStream class documentation for more information.

 Usage: Create a subclass of OOXMLFastContextHandlerLinear, reimplemented getType() to provide
 type of the subclass and process() to actually process the XML stream. Also make sure to
 add a line like the following to model.xml (for class OOXMLFastContextHandlerMath):

 <resource name="CT_OMath" resource="Math"/>

 @since 3.5
*/
class OOXMLFastContextHandlerLinear: public OOXMLFastContextHandlerProperties
{
public:
    explicit OOXMLFastContextHandlerLinear(OOXMLFastContextHandler * pContext);
    /**
     Return the type of the class, as written in model.xml .
     */
    virtual string getType() const SAL_OVERRIDE = 0;

protected:
    /**
     Called when the tokens for the element, its content and sub-elements have been linearized
     and should be processed. The data member @ref buffer contains the converted data.
    */
    virtual void process() = 0;

    virtual void lcl_startFastElement(Token_t Element, const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element) throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual uno::Reference< xml::sax::XFastContextHandler > lcl_createFastChildContext(Token_t Element,
        const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
        throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    virtual void lcl_characters(const OUString & aChars) throw (uno::RuntimeException, xml::sax::SAXException) SAL_OVERRIDE;

    // should be private, but not much point in making deep copies of it
    oox::formulaimport::XmlStreamBuilder buffer;

private:
    int depthCount;
};

class OOXMLFastContextHandlerMath: public OOXMLFastContextHandlerLinear
{
public:
    explicit OOXMLFastContextHandlerMath(OOXMLFastContextHandler * pContext);
    virtual string getType() const SAL_OVERRIDE { return "Math"; }
protected:
    virtual void process() SAL_OVERRIDE;
};

}}
#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTCONTEXTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
