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
#include "cppuhelper/implbase.hxx"
#include "com/sun/star/xml/sax/XFastContextHandler.hpp"
#include "OOXMLParserState.hxx"
#include "OOXMLPropertySetImpl.hxx"
#include "OOXMLDocumentImpl.hxx"
#include <oox/token/tokens.hxx>
#include <svtools/embedhlp.hxx>

#include <oox/mathml/import.hxx>
#include <oox/mathml/importutils.hxx>

namespace writerfilter {
namespace ooxml
{


class OOXMLFastContextHandler: public ::cppu::WeakImplHelper<css::xml::sax::XFastContextHandler>
{
public:
    typedef std::shared_ptr<OOXMLFastContextHandler> Pointer_t;

    enum ResourceEnum_t { UNKNOWN, STREAM, PROPERTIES, TABLE, SHAPE };

    OOXMLFastContextHandler();
    explicit OOXMLFastContextHandler(css::uno::Reference< css::uno::XComponentContext > const & context);

    explicit OOXMLFastContextHandler(OOXMLFastContextHandler * pContext);

    virtual ~OOXMLFastContextHandler();

    // css::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement (Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL startUnknownElement(const OUString & Namespace, const OUString & Name, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL endUnknownElement(const OUString & Namespace, const OUString & Name)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(Token_t Element,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext(const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL characters(const OUString & aChars)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

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
    virtual std::string getType() const { return "??"; }

    virtual ResourceEnum_t getResource() const { return STREAM; }

    virtual void attributes(const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs) throw (css::uno::RuntimeException, css::xml::sax::SAXException);

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

    css::uno::Reference < css::xml::sax::XFastContextHandler > createFromStart(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs);

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
    void setLastSectionGroup();
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
    void lockField();
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
    static void ignore();
    void alignH(const OUString & sText);
    void alignV(const OUString & sText);
    void positivePercentage(const OUString& rText);
    void startTxbxContent();
    void endTxbxContent();
    void propagateCharacterProperties();
    void propagateCharacterPropertiesAsSet(const Id & rId);
    void propagateTableProperties();
    void propagateRowProperties();
    void propagateCellProperties();
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

    virtual void lcl_startFastElement(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception);

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > lcl_createFastChildContext(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception);

    virtual void lcl_characters(const OUString & aChars)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception);

    void startAction(Token_t Element);
    void endAction(Token_t Element);

    virtual void resolvePropertySetAttrs();

    css::uno::Reference< css::uno::XComponentContext > getComponentContext() { return m_xContext;}

    sal_uInt32 mnInstanceNumber;

    bool inPositionV;

private:
    void operator =(OOXMLFastContextHandler &) SAL_DELETED_FUNCTION;
    /// Handles AlternateContent. Returns true, if children of the current element should be ignored.
    bool prepareMceContext(Token_t nElement, const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs);

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    bool m_bDiscardChildren;
    bool m_bTookChoice; ///< Did we take the Choice or want Fallback instead?

    static sal_uInt32 mnInstanceCount;

};

class OOXMLFastContextHandlerStream : public OOXMLFastContextHandler
{
public:
    explicit OOXMLFastContextHandlerStream(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerStream();

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return STREAM; }

    OOXMLPropertySet::Pointer_t getPropertySetAttrs() const { return mpPropertySetAttrs;}

    virtual void newProperty(const Id & rId, OOXMLValue::Pointer_t pVal) SAL_OVERRIDE;
    void sendProperty(Id nId);
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
    explicit OOXMLFastContextHandlerProperties(OOXMLFastContextHandler * pContext);
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

protected:
    /// the properties
    OOXMLPropertySet::Pointer_t mpPropertySet;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
    virtual void setParent(OOXMLFastContextHandler * pParent) SAL_OVERRIDE;

private:

    bool mbResolve;
};

class OOXMLFastContextHandlerPropertyTable :
    public OOXMLFastContextHandlerProperties
{
public:
    explicit OOXMLFastContextHandlerPropertyTable(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerPropertyTable();

protected:
    OOXMLTableImpl mTable;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
 };

class OOXMLFastContextHandlerValue :
    public OOXMLFastContextHandler
{
public:
    explicit OOXMLFastContextHandlerValue(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerValue();

    void setValue(OOXMLValue::Pointer_t pValue);
    virtual OOXMLValue::Pointer_t getValue() const SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual std::string getType() const SAL_OVERRIDE { return "Value"; }

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
    explicit OOXMLFastContextHandlerTable(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerTable();

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext (Token_t Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

protected:
    OOXMLTableImpl mTable;

    css::uno::Reference<css::xml::sax::XFastContextHandler> mCurrentChild;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return TABLE; }

    virtual std::string getType() const SAL_OVERRIDE { return "Table"; }

    void addCurrentChild();
};

class OOXMLFastContextHandlerXNote : public OOXMLFastContextHandlerProperties
{
public:
    explicit OOXMLFastContextHandlerXNote(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerXNote();

    void checkId(OOXMLValue::Pointer_t pValue);

    void checkType(OOXMLValue::Pointer_t pValue);

    virtual std::string getType() const SAL_OVERRIDE { return "XNote"; }

private:
    bool mbForwardEventsSaved;
    sal_Int32 mnMyXNoteId;
    sal_Int32 mnMyXNoteType;

    virtual void lcl_startFastElement(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return STREAM; }
};

class OOXMLFastContextHandlerTextTableCell : public OOXMLFastContextHandler
{
public:
    explicit OOXMLFastContextHandlerTextTableCell(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerTextTableCell();

    virtual std::string getType() const SAL_OVERRIDE { return "TextTableCell"; }

    static void startCell();
    void endCell();
};

class OOXMLFastContextHandlerTextTableRow : public OOXMLFastContextHandler
{
public:
    explicit OOXMLFastContextHandlerTextTableRow(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerTextTableRow();

    virtual std::string getType() const SAL_OVERRIDE { return "TextTableRow"; }

    static void startRow();
    void endRow();
    void handleGridBefore( OOXMLValue::Pointer_t val );
private:
    static OOXMLProperty::Pointer_t fakeNoBorder( Id id );
};

class OOXMLFastContextHandlerTextTable : public OOXMLFastContextHandler
{
public:
    explicit OOXMLFastContextHandlerTextTable(OOXMLFastContextHandler * pContext);

    virtual ~OOXMLFastContextHandlerTextTable();

    virtual std::string getType() const SAL_OVERRIDE { return "TextTable"; }

protected:
    virtual void lcl_startFastElement(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
};

class OOXMLFastContextHandlerShape: public OOXMLFastContextHandlerProperties
{
private:
    bool m_bShapeSent;
    bool m_bShapeStarted;

public:
    explicit OOXMLFastContextHandlerShape(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFastContextHandlerShape();

    virtual std::string getType() const SAL_OVERRIDE { return "Shape"; }

    // css::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startUnknownElement (const OUString & Namespace, const OUString & Name, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL endUnknownElement(const OUString & Namespace, const OUString & Name)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext(const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void setToken(Token_t nToken) SAL_OVERRIDE;

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE { return SHAPE; }

    void sendShape( Token_t Element );
    bool isShapeSent( ) { return m_bShapeSent; }

protected:
    css::uno::Reference<css::xml::sax::XFastShapeContextHandler> mrShapeContext;

    virtual void lcl_startFastElement(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > lcl_createFastChildContext (Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void lcl_characters(const OUString & aChars)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

};

/**
   OOXMLFastContextHandlerWrapper wraps an OOXMLFastContextHandler.

   The method calls for the interface css::xml::sax::XFastContextHandler are
   forwarded to the wrapped OOXMLFastContextHandler.
 */
class OOXMLFastContextHandlerWrapper : public OOXMLFastContextHandler
{
public:
    explicit OOXMLFastContextHandlerWrapper(OOXMLFastContextHandler * pParent, css::uno::Reference<css::xml::sax::XFastContextHandler> xContext);
    virtual ~OOXMLFastContextHandlerWrapper();

    // css::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startUnknownElement(const OUString & Namespace, const OUString & Name, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL endUnknownElement(const OUString & Namespace, const OUString & Name)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext (const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void attributes(const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;

    virtual ResourceEnum_t getResource() const SAL_OVERRIDE;

    void addNamespace(const Id & nId);
    void addToken( Token_t Element );

    virtual void newProperty(const Id & rId, OOXMLValue::Pointer_t pVal) SAL_OVERRIDE;
    virtual void setPropertySet(OOXMLPropertySet::Pointer_t pPropertySet) SAL_OVERRIDE;
    virtual OOXMLPropertySet::Pointer_t getPropertySet() const SAL_OVERRIDE;

    virtual std::string getType() const SAL_OVERRIDE;

protected:
    virtual void lcl_startFastElement(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > lcl_createFastChildContext(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void lcl_characters(const OUString & aChars)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void setId(Id nId) SAL_OVERRIDE;
    virtual Id getId() const SAL_OVERRIDE;

    virtual void setToken(Token_t nToken) SAL_OVERRIDE;
    virtual Token_t getToken() const SAL_OVERRIDE;

private:
    css::uno::Reference<css::xml::sax::XFastContextHandler> mxContext;
    std::set<Id> mMyNamespaces;
    std::set<Token_t> mMyTokens;
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
    virtual std::string getType() const SAL_OVERRIDE = 0;

protected:
    /**
     Called when the tokens for the element, its content and sub-elements have been linearized
     and should be processed. The data member @ref buffer contains the converted data.
    */
    virtual void process() = 0;

    virtual void lcl_startFastElement(Token_t Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void lcl_endFastElement(Token_t Element) throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > lcl_createFastChildContext(Token_t Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    virtual void lcl_characters(const OUString & aChars) throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;

    // should be private, but not much point in making deep copies of it
    oox::formulaimport::XmlStreamBuilder buffer;

private:
    int depthCount;
};

class OOXMLFastContextHandlerMath: public OOXMLFastContextHandlerLinear
{
public:
    explicit OOXMLFastContextHandlerMath(OOXMLFastContextHandler * pContext);
    virtual std::string getType() const SAL_OVERRIDE { return "Math"; }
protected:
    virtual void process() SAL_OVERRIDE;
};

}}
#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTCONTEXTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
