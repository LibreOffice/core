/*************************************************************************
 *
 *  $RCSfile: txtparaimphint.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-14 12:15:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _XMLOFF_TXTPARAIMPHINT_HXX
#define _XMLOFF_TXTPARAIMPHINT_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLTEXTFRAMECONTEXT_HXX
#include "XMLTextFrameContext.hxx"
#endif
#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

// ---------------------------------------------------------------------

#define XML_HINT_STYLE 1
#define XML_HINT_REFERENCE 2
#define XML_HINT_HYPERLINK 3
#define XML_HINT_RUBY 4
#define XML_HINT_INDEX_MARK 5
#define XML_HINT_TEXT_FRAME 6
// --> DVO, OD 2004-07-14 #i26791#
#define XML_HINT_DRAW 7
// <--

class XMLHint_Impl
{
    Reference < XTextRange > xStart;
    Reference < XTextRange > xEnd;

    sal_uInt8 nType;

public:

    XMLHint_Impl( sal_uInt8 nTyp,
                  const Reference < XTextRange > & rS,
                  const Reference < XTextRange > & rE ) :
        nType( nTyp ),
        xStart( rS ),
        xEnd( rE )
    {
    }

    XMLHint_Impl( sal_uInt8 nTyp,
                  const Reference < XTextRange > & rS ) :
        nType( nTyp ),
        xStart( rS )
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
    XMLEventsImportContext* GetEventsContext() const;
    void SetEventsContext( XMLEventsImportContext* pCtxt );
};

XMLEventsImportContext* XMLHyperlinkHint_Impl::GetEventsContext() const
{
    return pEvents;
}

void XMLHyperlinkHint_Impl::SetEventsContext( XMLEventsImportContext* pCtxt )
{
    pEvents = pCtxt;
    if (pEvents != NULL)
        pEvents->AddRef();
}


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

class XMLRubyHint_Impl : public XMLHint_Impl
{
    OUString                 sStyleName;
    OUString                 sTextStyleName;
    OUString                 sText;

public:

    XMLRubyHint_Impl( const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_RUBY, rPos, rPos )
    {
    }

    virtual ~XMLRubyHint_Impl() {}

    void SetStyleName( const OUString& s ) { sStyleName = s; }
    const OUString& GetStyleName() const { return sStyleName; }
    void SetTextStyleName( const OUString& s ) { sTextStyleName = s; }
    const OUString& GetTextStyleName() const { return sTextStyleName; }
    void AppendText( const OUString& s ) { sText += s; }
    const OUString& GetText() const { return sText; }
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

// --> DVO, OD 2004-07-14 #i26791#
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

    SvXMLShapeContext* GetShapeContext() const
    {
    return static_cast<SvXMLShapeContext*>(&xContext);
    }
};
// <--
#endif
