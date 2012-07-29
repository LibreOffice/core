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
#ifndef _XMLOFF_TXTPARAIMPHINT_HXX
#define _XMLOFF_TXTPARAIMPHINT_HXX

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlimp.hxx>
#include "XMLTextFrameContext.hxx"
#include <xmloff/XMLEventsImportContext.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

// ---------------------------------------------------------------------

#define XML_HINT_STYLE 1
#define XML_HINT_REFERENCE 2
#define XML_HINT_HYPERLINK 3
#define XML_HINT_INDEX_MARK 5
#define XML_HINT_TEXT_FRAME 6
// Core impl. of the unification of drawing objects and Writer fly frames (#i26791#)
#define XML_HINT_DRAW 7

class XMLHint_Impl
{
    Reference < XTextRange > xStart;
    Reference < XTextRange > xEnd;

    sal_uInt8 nType;

public:

    XMLHint_Impl( sal_uInt8 nTyp,
                  const Reference < XTextRange > & rS,
                  const Reference < XTextRange > & rE ) :
        xStart( rS ),
        xEnd( rE ),
        nType( nTyp )
    {
    }

    XMLHint_Impl( sal_uInt8 nTyp,
                  const Reference < XTextRange > & rS ) :
        xStart( rS ),
        nType( nTyp )
    {
    }

    virtual ~XMLHint_Impl() {}

    const Reference < XTextRange > & GetStart() const { return xStart; }
    const Reference < XTextRange > & GetEnd() const { return xEnd; }
    void SetEnd( const Reference < XTextRange > & rPos ) { xEnd = rPos; }

    // We don't use virtual methods to differ between the sub classes,
    // because this seems to be to expensive if compared to inline methods.
    sal_uInt8 GetType() const { return nType; }
    sal_Bool IsStyle() { return XML_HINT_STYLE==nType; }
    sal_Bool IsReference() { return XML_HINT_REFERENCE==nType; }
    sal_Bool IsHyperlink() { return XML_HINT_HYPERLINK==nType; }
    sal_Bool IsIndexMark() { return XML_HINT_INDEX_MARK==nType; }
};

class XMLStyleHint_Impl : public XMLHint_Impl
{
    OUString                 sStyleName;

public:

    XMLStyleHint_Impl( const OUString& rStyleName,
                         const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_STYLE, rPos, rPos ),
        sStyleName( rStyleName )
    {
    }
    virtual ~XMLStyleHint_Impl() {}

    const OUString& GetStyleName() const { return sStyleName; }
};

class XMLReferenceHint_Impl : public XMLHint_Impl
{
    OUString                 sRefName;

public:

    XMLReferenceHint_Impl( const OUString& rRefName,
                             const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_REFERENCE, rPos, rPos ),
        sRefName( rRefName )
    {
    }

    virtual ~XMLReferenceHint_Impl() {}

    const OUString& GetRefName() const { return sRefName; }
};

class XMLHyperlinkHint_Impl : public XMLHint_Impl
{
    OUString                 sHRef;
    OUString                 sName;
    OUString                 sTargetFrameName;
    OUString                 sStyleName;
    OUString                 sVisitedStyleName;
    XMLEventsImportContext*  pEvents;

public:

    XMLHyperlinkHint_Impl( const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_HYPERLINK, rPos, rPos ),
        pEvents( NULL )
    {
    }

    virtual ~XMLHyperlinkHint_Impl()
    {
        if (NULL != pEvents)
            pEvents->ReleaseRef();
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
        return pEvents;
    }
    void SetEventsContext( XMLEventsImportContext* pCtxt )
    {
        pEvents = pCtxt;
        if (pEvents != NULL)
            pEvents->AddRef();
    }
};


class XMLIndexMarkHint_Impl : public XMLHint_Impl
{
    const Reference<beans::XPropertySet> xIndexMarkPropSet;

    const OUString sID;

public:

    XMLIndexMarkHint_Impl( const Reference < beans::XPropertySet > & rPropSet,
                           const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_INDEX_MARK, rPos, rPos ),
        xIndexMarkPropSet( rPropSet ),
        sID()
    {
    }

    XMLIndexMarkHint_Impl( const Reference < beans::XPropertySet > & rPropSet,
                           const Reference < XTextRange > & rPos,
                           OUString sIDString) :
        XMLHint_Impl( XML_HINT_INDEX_MARK, rPos, rPos ),
        xIndexMarkPropSet( rPropSet ),
        sID(sIDString)
    {
    }

    virtual ~XMLIndexMarkHint_Impl() {}

    const Reference<beans::XPropertySet> & GetMark() const
        { return xIndexMarkPropSet; }
    const OUString& GetID() const { return sID; }
};


class XMLTextFrameHint_Impl : public XMLHint_Impl
{
    // OD 2004-04-20 #i26791#
    SvXMLImportContextRef xContext;

public:

    XMLTextFrameHint_Impl( SvXMLImportContext* pContext,
                           const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_TEXT_FRAME, rPos, rPos ),
        xContext( pContext )
    {
    }

    virtual ~XMLTextFrameHint_Impl()
    {
    }

    Reference < XTextContent > GetTextContent() const
    {
        Reference <XTextContent > xTxt;
        SvXMLImportContext *pContext = &xContext;
        if( pContext->ISA( XMLTextFrameContext ) )
            xTxt = PTR_CAST( XMLTextFrameContext, pContext )->GetTextContent();
        else if( pContext->ISA( XMLTextFrameHyperlinkContext ) )
            xTxt = PTR_CAST( XMLTextFrameHyperlinkContext, pContext )
                        ->GetTextContent();

        return xTxt;
    }

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    Reference < drawing::XShape > GetShape() const
    {
        Reference < drawing::XShape > xShape;
        SvXMLImportContext *pContext = &xContext;
        if( pContext->ISA( XMLTextFrameContext ) )
            xShape = PTR_CAST( XMLTextFrameContext, pContext )->GetShape();
        else if( pContext->ISA( XMLTextFrameHyperlinkContext ) )
            xShape = PTR_CAST( XMLTextFrameHyperlinkContext, pContext )->GetShape();

        return xShape;
    }

    sal_Bool IsBoundAtChar() const
    {
        sal_Bool bRet = sal_False;
        SvXMLImportContext *pContext = &xContext;
        if( pContext->ISA( XMLTextFrameContext ) )
            bRet = TextContentAnchorType_AT_CHARACTER ==
                PTR_CAST( XMLTextFrameContext, pContext )
                    ->GetAnchorType();
        else if( pContext->ISA( XMLTextFrameHyperlinkContext ) )
            bRet = TextContentAnchorType_AT_CHARACTER ==
                PTR_CAST( XMLTextFrameHyperlinkContext, pContext )
                    ->GetAnchorType();
        return bRet;
    }
};

// Core impl. of the unification of drawing objects and Writer fly frames (#i26791#)
class XMLDrawHint_Impl : public XMLHint_Impl
{
    SvXMLImportContextRef xContext;

public:

    XMLDrawHint_Impl( SvXMLShapeContext* pContext,
                      const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_DRAW, rPos, rPos ),
        xContext( pContext )
    {
    }

    virtual ~XMLDrawHint_Impl()
    {
    }

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    Reference < drawing::XShape > GetShape() const
    {
        return static_cast<SvXMLShapeContext*>(&xContext)->getShape();
    }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
