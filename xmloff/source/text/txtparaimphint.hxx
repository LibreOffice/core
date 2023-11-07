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
#pragma once

#include <rtl/ustring.hxx>
#include "XMLTextFrameContext.hxx"
#include "XMLTextFrameHyperlinkContext.hxx"
#include <utility>
#include <xmloff/XMLEventsImportContext.hxx>

enum class XMLHintType
{
    XML_HINT_STYLE      = 1,
    XML_HINT_REFERENCE  = 2,
    XML_HINT_HYPERLINK  = 3,
    // There is no 4 defined here
    XML_HINT_INDEX_MARK = 5,
    XML_HINT_TEXT_FRAME = 6,
    // Core impl. of the unification of drawing objects and Writer fly frames (#i26791#)
    XML_HINT_DRAW       = 7
};

class XMLHint_Impl
{
    css::uno::Reference < css::text::XTextRange > xStart;
    css::uno::Reference < css::text::XTextRange > xEnd;

    XMLHintType nType;

public:

    XMLHint_Impl( XMLHintType nTyp,
                  css::uno::Reference < css::text::XTextRange > xS,
                  css::uno::Reference < css::text::XTextRange > xE ) :
        xStart(std::move( xS )),
        xEnd(std::move( xE )),
        nType( nTyp )
    {
    }

    virtual ~XMLHint_Impl() {}

    const css::uno::Reference < css::text::XTextRange > & GetStart() const { return xStart; }
    const css::uno::Reference < css::text::XTextRange > & GetEnd() const { return xEnd; }
    void SetEnd( const css::uno::Reference < css::text::XTextRange > & rPos ) { xEnd = rPos; }

    // We don't use virtual methods to differ between the sub classes,
    // because this seems to be too expensive if compared to inline methods.
    XMLHintType GetType() const { return nType; }
    bool IsReference() const { return XMLHintType::XML_HINT_REFERENCE==nType; }
};

class XMLStyleHint_Impl : public XMLHint_Impl
{
    OUString                 sStyleName;

public:

    XMLStyleHint_Impl( OUString aStyleName,
                         const css::uno::Reference < css::text::XTextRange > & rPos ) :
        XMLHint_Impl( XMLHintType::XML_HINT_STYLE, rPos, rPos ),
        sStyleName(std::move( aStyleName ))
    {
    }

    const OUString& GetStyleName() const { return sStyleName; }
};

class XMLReferenceHint_Impl : public XMLHint_Impl
{
    OUString                 sRefName;

public:

    XMLReferenceHint_Impl( OUString aRefName,
                             const css::uno::Reference < css::text::XTextRange > & rPos ) :
        XMLHint_Impl( XMLHintType::XML_HINT_REFERENCE, rPos, rPos ),
        sRefName(std::move( aRefName ))
    {
    }

    const OUString& GetRefName() const { return sRefName; }
};

class XMLHyperlinkHint_Impl : public XMLHint_Impl
{
    OUString                 sHRef;
    OUString                 sName;
    OUString                 sTargetFrameName;
    OUString                 sStyleName;
    OUString                 sVisitedStyleName;
    rtl::Reference<XMLEventsImportContext> mxEvents;

public:

    XMLHyperlinkHint_Impl( const css::uno::Reference < css::text::XTextRange > & rPos ) :
        XMLHint_Impl( XMLHintType::XML_HINT_HYPERLINK, rPos, rPos )
    {
    }

    void SetHRef( const OUString& s ) { sHRef = s; }
    const OUString& GetHRef() const { return sHRef; }
    void SetName( const OUString& s ) { sName = s; }
    const OUString& GetName() const { return sName; }
    void SetTargetFrameName( const OUString& s ) { sTargetFrameName = s; }
    const OUString& GetTargetFrameName() const { return sTargetFrameName; }
    void SetStyleName( const OUString& s ) { sStyleName = s; }
    const OUString& GetStyleName() const { return sStyleName; }
    void SetVisitedStyleName( const OUString& s ) { sVisitedStyleName = s; }
    const OUString& GetVisitedStyleName() const { return sVisitedStyleName; }
    XMLEventsImportContext* GetEventsContext() const
    {
        return mxEvents.get();
    }
    void SetEventsContext( XMLEventsImportContext* pCtxt )
    {
        mxEvents.set(pCtxt);
    }
};

class XMLIndexMarkHint_Impl : public XMLHint_Impl
{
    const css::uno::Reference<css::beans::XPropertySet> xIndexMarkPropSet;

    const OUString sID;

public:

    XMLIndexMarkHint_Impl( css::uno::Reference < css::beans::XPropertySet > xPropSet,
                           const css::uno::Reference < css::text::XTextRange > & rPos ) :
        XMLHint_Impl( XMLHintType::XML_HINT_INDEX_MARK, rPos, rPos ),
        xIndexMarkPropSet(std::move( xPropSet )),
        sID()
    {
    }

    XMLIndexMarkHint_Impl( css::uno::Reference < css::beans::XPropertySet > xPropSet,
                           const css::uno::Reference < css::text::XTextRange > & rPos,
                           OUString sIDString) :
        XMLHint_Impl( XMLHintType::XML_HINT_INDEX_MARK, rPos, rPos ),
        xIndexMarkPropSet(std::move( xPropSet )),
        sID(std::move(sIDString))
    {
    }

    const css::uno::Reference<css::beans::XPropertySet> & GetMark() const
        { return xIndexMarkPropSet; }
    const OUString& GetID() const { return sID; }
};

class XMLTextFrameHint_Impl : public XMLHint_Impl
{
    // OD 2004-04-20 #i26791#
    SvXMLImportContextRef xContext;

public:

    XMLTextFrameHint_Impl( SvXMLImportContext* pContext,
                           const css::uno::Reference < css::text::XTextRange > & rPos ) :
        XMLHint_Impl( XMLHintType::XML_HINT_TEXT_FRAME, rPos, rPos ),
        xContext( pContext )
    {
    }

    css::uno::Reference < css::text::XTextContent > GetTextContent() const
    {
        css::uno::Reference < css::text::XTextContent > xTxt;
        SvXMLImportContext *pContext = xContext.get();
        if (XMLTextFrameContext *pFrameContext =  dynamic_cast<XMLTextFrameContext*>(pContext))
            xTxt = pFrameContext->GetTextContent();
        else if (XMLTextFrameHyperlinkContext *pLinkContext = dynamic_cast<XMLTextFrameHyperlinkContext*>(pContext))
            xTxt = pLinkContext->GetTextContent();

        return xTxt;
    }

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    css::uno::Reference < css::drawing::XShape > GetShape() const
    {
        css::uno::Reference < css::drawing::XShape > xShape;
        SvXMLImportContext *pContext = xContext.get();
        if (XMLTextFrameContext *pFrameContext = dynamic_cast<XMLTextFrameContext*>(pContext))
            xShape = pFrameContext->GetShape();
        else if(XMLTextFrameHyperlinkContext *pLinkContext =  dynamic_cast<XMLTextFrameHyperlinkContext*>(pContext))
            xShape = pLinkContext->GetShape();

        return xShape;
    }

    bool IsBoundAtChar() const
    {
        bool bRet = false;
        SvXMLImportContext *pContext = xContext.get();
        if (XMLTextFrameContext *pFrameContext = dynamic_cast<XMLTextFrameContext*>(pContext))
            bRet = css::text::TextContentAnchorType_AT_CHARACTER ==
                pFrameContext->GetAnchorType();
        else if (XMLTextFrameHyperlinkContext *pLinkContext = dynamic_cast<XMLTextFrameHyperlinkContext*>(pContext))
            bRet = css::text::TextContentAnchorType_AT_CHARACTER ==
                pLinkContext->GetAnchorType();
        return bRet;
    }
};

// Core impl. of the unification of drawing objects and Writer fly frames (#i26791#)
class XMLDrawHint_Impl : public XMLHint_Impl
{
    rtl::Reference<SvXMLShapeContext> xContext;

public:

    XMLDrawHint_Impl( SvXMLShapeContext* pContext,
                      const css::uno::Reference < css::text::XTextRange > & rPos ) :
        XMLHint_Impl( XMLHintType::XML_HINT_DRAW, rPos, rPos ),
        xContext( pContext )
    {
    }

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    css::uno::Reference < css::drawing::XShape > const & GetShape() const
    {
        return xContext->getShape();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
