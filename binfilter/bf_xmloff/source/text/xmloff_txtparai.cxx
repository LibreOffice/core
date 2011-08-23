/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/




#ifndef _SVARRAY_HXX 
#include <bf_svtools/svarray.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_ 
#include <com/sun/star/text/ControlCharacter.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif


#ifndef _XMLOFF_XMLIMP_HXX 
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX 
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX 
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_TXTPARAI_HXX 
#include "txtparai.hxx"
#endif
#ifndef _XMLOFF_TXTFLDI_HXX
#include "txtfldi.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLFOOTNOTEIMPORTCONTEXT_HXX
#include "XMLFootnoteImportContext.hxx"
#endif
#ifndef _XMLOFF_XMLTEXTMARKIMPORTCONTEXT_HXX
#include "XMLTextMarkImportContext.hxx"
#endif
#ifndef _XMLTEXTFRAMECONTEXT_HXX
#include "XMLTextFrameContext.hxx"
#endif
#ifndef _XMLTEXTFRAMEHYPERLINKCONTEXT_HXX
#include "XMLTextFrameHyperlinkContext.hxx"
#endif
#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
#endif
#ifndef _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX
#include "XMLChangeImportContext.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::binfilter::xmloff::token;


// ---------------------------------------------------------------------

#define XML_HINT_STYLE 1
#define XML_HINT_REFERENCE 2
#define XML_HINT_HYPERLINK 3
#define XML_HINT_RUBY 4
#define XML_HINT_INDEX_MARK 5
#define XML_HINT_TEXT_FRAME 6


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
    OUString				 sStyleName;
    
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
    OUString				 sRefName;
    
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
    OUString				 sHRef;
    OUString				 sName;
    OUString				 sTargetFrameName;
    OUString				 sStyleName;
    OUString				 sVisitedStyleName;
    XMLEventsImportContext*	 pEvents;
    
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

void XMLHyperlinkHint_Impl::SetEventsContext( XMLEventsImportContext* pCtxt )
{ 
    pEvents = pCtxt; 
    if (pEvents != NULL) 
        pEvents->AddRef();
}
    
XMLEventsImportContext* XMLHyperlinkHint_Impl::GetEventsContext() const 
{
    return pEvents; 
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
    OUString				 sStyleName;
    OUString				 sTextStyleName;
    OUString				 sText;
    
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
    Reference < XTextContent > xTextContent;
    TextContentAnchorType eAnchorType;

public:

    XMLTextFrameHint_Impl( const Reference < XTextContent > & rTxtCntnt,
                              const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_TEXT_FRAME, rPos, rPos ),
        xTextContent( rTxtCntnt ),
        eAnchorType( TextContentAnchorType_AT_CHARACTER )
    {
    }

    XMLTextFrameHint_Impl( const Reference < XTextRange > & rPos ) :
        XMLHint_Impl( XML_HINT_TEXT_FRAME, rPos, rPos ),
        eAnchorType( TextContentAnchorType_AS_CHARACTER )
    {
    }

    virtual ~XMLTextFrameHint_Impl() {}

    Reference < XTextContent >& GetTextContentRef() { return xTextContent; }
    TextContentAnchorType& GetAnchorTypeRef() { return eAnchorType; }

    Reference < XTextContent > GetTextContent() const { return xTextContent; }
    sal_Bool IsBoundAtChar() const { return TextContentAnchorType_AT_CHARACTER == eAnchorType; }
};

typedef XMLHint_Impl *XMLHint_ImplPtr;
SV_DECL_PTRARR_DEL( XMLHints_Impl, XMLHint_ImplPtr, 5, 5 )
SV_IMPL_PTRARR( XMLHints_Impl, XMLHint_ImplPtr )

// ---------------------------------------------------------------------

class XMLImpCharContext_Impl : public SvXMLImportContext
{
public:

    TYPEINFO();

    XMLImpCharContext_Impl(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Unicode c,
            sal_Bool bCount );
    XMLImpCharContext_Impl(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Int16 nControl );

    virtual ~XMLImpCharContext_Impl();
};

TYPEINIT1( XMLImpCharContext_Impl, SvXMLImportContext );

XMLImpCharContext_Impl::XMLImpCharContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Unicode c,
        sal_Bool bCount ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt16 nCount = 1;

    if( bCount )
    {
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );

            OUString aLocalName;
            sal_uInt16 nPrefix =
                GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
            if( XML_NAMESPACE_TEXT == nPrefix &&
                IsXMLToken( aLocalName, XML_C ) )
            {
                sal_Int32 nTmp = xAttrList->getValueByIndex(i).toInt32();
                if( nTmp > 0L )
                {
                    if( nTmp > USHRT_MAX )
                        nCount = USHRT_MAX;
                    else
                        nCount = (sal_uInt16)nTmp;
                }
            }
        }
    }

    if( 1U == nCount )
    {		
        OUString sBuff( &c, 1 );
        GetImport().GetTextImport()->InsertString( sBuff );
    }
    else
    {
        OUStringBuffer sBuff( nCount );
        while( nCount-- )
            sBuff.append( &c, 1 );

        GetImport().GetTextImport()->InsertString( sBuff.makeStringAndClear() );
    }
}

XMLImpCharContext_Impl::XMLImpCharContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Int16 nControl ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    GetImport().GetTextImport()->InsertControlCharacter( nControl );
}

XMLImpCharContext_Impl::~XMLImpCharContext_Impl()
{
}

// ---------------------------------------------------------------------

/** import start of reference (<text:reference-start>) */
class XMLStartReferenceContext_Impl : public SvXMLImportContext
{
public:
    TYPEINFO();

    // Do everything in constructor. Well ...
    XMLStartReferenceContext_Impl (
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        XMLHints_Impl& rHnts,
        const Reference<xml::sax::XAttributeList> & xAttrList);
};

TYPEINIT1( XMLStartReferenceContext_Impl, SvXMLImportContext );

XMLStartReferenceContext_Impl::XMLStartReferenceContext_Impl(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    XMLHints_Impl& rHints,
    const Reference<xml::sax::XAttributeList> & xAttrList) :
        SvXMLImportContext(rImport, nPrefix, rLocalName)
{
    OUString sName;

    // borrow FindName from XMLTextMarkImportContext, where bookmarks
    // and point references are handled.
    if (XMLTextMarkImportContext::FindName(GetImport(), xAttrList, sName))
    {
        XMLHint_Impl* pHint = new XMLReferenceHint_Impl(
            sName, rImport.GetTextImport()->GetCursor()->getStart() );

        // degenerates to point reference, if no end is found!
        pHint->SetEnd(rImport.GetTextImport()->GetCursor()->getStart() );

        rHints.Insert(pHint, rHints.Count());
    }
}

// ---------------------------------------------------------------------

/** import end of reference (<text:reference-end>) */
class XMLEndReferenceContext_Impl : public SvXMLImportContext
{
public:
    TYPEINFO();

    // Do everything in constructor. Well ...
    XMLEndReferenceContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        XMLHints_Impl& rHnts,
        const Reference<xml::sax::XAttributeList> & xAttrList);
};

TYPEINIT1( XMLEndReferenceContext_Impl, SvXMLImportContext );

XMLEndReferenceContext_Impl::XMLEndReferenceContext_Impl(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    XMLHints_Impl& rHints,
    const Reference<xml::sax::XAttributeList> & xAttrList) :
        SvXMLImportContext(rImport, nPrefix, rLocalName)
{
    OUString sName;

    // borrow from XMLTextMarkImportContext
    if (XMLTextMarkImportContext::FindName(GetImport(), xAttrList, sName))
    {
        // search for reference start
        sal_uInt16 nCount = rHints.Count();
        for(sal_uInt16 nPos = 0; nPos < nCount; nPos++)
        {
            XMLHint_Impl *pHint = rHints[nPos];
            if ( pHint->IsReference() && 
                 sName.equals( ((XMLReferenceHint_Impl *)pHint)->GetRefName()) )
            {
                // set end and stop searching
                pHint->SetEnd(GetImport().GetTextImport()->
                                     GetCursor()->getStart() );
                break;
            }
        }
        // else: no start (in this paragraph) -> ignore
    }
}

// ---------------------------------------------------------------------

class XMLImpSpanContext_Impl : public SvXMLImportContext
{
    const OUString sTextFrame;

    XMLHints_Impl&	rHints;
    XMLStyleHint_Impl	*pHint;
    
    sal_Bool&		rIgnoreLeadingSpace;

#ifdef CONV_STAR_FONTS
    sal_uInt8				nStarFontsConvFlags;
#endif

public:

    TYPEINFO();

    XMLImpSpanContext_Impl(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            XMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace
#ifdef CONV_STAR_FONTS
    ,sal_uInt8				nSFConvFlags
#endif
                          );

    virtual ~XMLImpSpanContext_Impl();

    static SvXMLImportContext *CreateChildContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nToken, XMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace
#ifdef CONV_STAR_FONTS
    ,sal_uInt8				nStarFontsConvFlags = 0
#endif
             );
    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void Characters( const OUString& rChars );
};
// ---------------------------------------------------------------------

class XMLImpHyperlinkContext_Impl : public SvXMLImportContext
{
    XMLHints_Impl&	rHints;
    XMLHyperlinkHint_Impl	*pHint;
    
    sal_Bool&		rIgnoreLeadingSpace;

public:

    TYPEINFO();

    XMLImpHyperlinkContext_Impl(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            XMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace );

    virtual ~XMLImpHyperlinkContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void Characters( const OUString& rChars );
};

TYPEINIT1( XMLImpHyperlinkContext_Impl, SvXMLImportContext );

XMLImpHyperlinkContext_Impl::XMLImpHyperlinkContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        XMLHints_Impl& rHnts,
        sal_Bool& rIgnLeadSpace ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rHints( rHnts ),
    rIgnoreLeadingSpace( rIgnLeadSpace ),
    pHint( new XMLHyperlinkHint_Impl( 
              GetImport().GetTextImport()->GetCursorAsRange()->getStart() ) )
{
    OUString sShow;
    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextHyperlinkAttrTokenMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_HYPERLINK_HREF:
            pHint->SetHRef( GetImport().GetAbsoluteReference( rValue ) );
            break;
        case XML_TOK_TEXT_HYPERLINK_NAME:
            pHint->SetName( rValue );
            break;
        case XML_TOK_TEXT_HYPERLINK_TARGET_FRAME:
            pHint->SetTargetFrameName( rValue );
            break;
        case XML_TOK_TEXT_HYPERLINK_SHOW:
            sShow = rValue;
            break;
        case XML_TOK_TEXT_HYPERLINK_STYLE_NAME:
            pHint->SetStyleName( rValue );
            break;
        case XML_TOK_TEXT_HYPERLINK_VIS_STYLE_NAME:
            pHint->SetVisitedStyleName( rValue );
            break;
        }
    }

    if( sShow.getLength() && !pHint->GetTargetFrameName().getLength() )
    {
        if( IsXMLToken( sShow, XML_NEW ) )
            pHint->SetTargetFrameName(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("_blank" ) ) );
        else if( IsXMLToken( sShow, XML_REPLACE ) )
            pHint->SetTargetFrameName(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("_self" ) ) );
    }
    rHints.Insert( pHint, rHints.Count() );
}

XMLImpHyperlinkContext_Impl::~XMLImpHyperlinkContext_Impl()
{
    if( pHint )
        pHint->SetEnd( GetImport().GetTextImport()
                            ->GetCursorAsRange()->getStart() );
}

SvXMLImportContext *XMLImpHyperlinkContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    if ( (nPrefix == XML_NAMESPACE_OFFICE) && 
         IsXMLToken(rLocalName, XML_EVENTS) )
    {
        XMLEventsImportContext* pCtxt = new XMLEventsImportContext(
            GetImport(), nPrefix, rLocalName);
        pHint->SetEventsContext(pCtxt);
        return pCtxt;
    }
    else
    {
        const SvXMLTokenMap& rTokenMap =
            GetImport().GetTextImport()->GetTextPElemTokenMap();
        sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );

        return XMLImpSpanContext_Impl::CreateChildContext( 
            GetImport(), nPrefix, rLocalName, xAttrList, 
            nToken, rHints, rIgnoreLeadingSpace );
    }
}

void XMLImpHyperlinkContext_Impl::Characters( const OUString& rChars )
{
    GetImport().GetTextImport()->InsertString( rChars, rIgnoreLeadingSpace );
}

// ---------------------------------------------------------------------

class XMLImpRubyBaseContext_Impl : public SvXMLImportContext
{
    XMLHints_Impl&	rHints;
    
    sal_Bool&		rIgnoreLeadingSpace;

public:

    TYPEINFO();

    XMLImpRubyBaseContext_Impl(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            XMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace );

    virtual ~XMLImpRubyBaseContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void Characters( const OUString& rChars );
};

TYPEINIT1( XMLImpRubyBaseContext_Impl, SvXMLImportContext );

XMLImpRubyBaseContext_Impl::XMLImpRubyBaseContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        XMLHints_Impl& rHnts,
        sal_Bool& rIgnLeadSpace ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rHints( rHnts ),
    rIgnoreLeadingSpace( rIgnLeadSpace )
{
}

XMLImpRubyBaseContext_Impl::~XMLImpRubyBaseContext_Impl()
{
}

SvXMLImportContext *XMLImpRubyBaseContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextPElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    
    return XMLImpSpanContext_Impl::CreateChildContext( GetImport(), nPrefix,
                                                       rLocalName, xAttrList,
                               nToken, rHints, rIgnoreLeadingSpace );
}

void XMLImpRubyBaseContext_Impl::Characters( const OUString& rChars )
{
    GetImport().GetTextImport()->InsertString( rChars, rIgnoreLeadingSpace );
}

// ---------------------------------------------------------------------

class XMLImpRubyTextContext_Impl : public SvXMLImportContext
{
    XMLRubyHint_Impl	*pHint;

public:

    TYPEINFO();

    XMLImpRubyTextContext_Impl(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            XMLRubyHint_Impl *pHint );

    virtual ~XMLImpRubyTextContext_Impl();

    virtual void Characters( const OUString& rChars );
};

TYPEINIT1( XMLImpRubyTextContext_Impl, SvXMLImportContext );

XMLImpRubyTextContext_Impl::XMLImpRubyTextContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        XMLRubyHint_Impl *pHt ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pHint( pHt )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        if( XML_NAMESPACE_TEXT == nPrefix &&
            IsXMLToken( aLocalName, XML_STYLE_NAME ) )
        {
            pHint->SetTextStyleName( rValue );
            break;
        }
    }
}

XMLImpRubyTextContext_Impl::~XMLImpRubyTextContext_Impl()
{
}

void XMLImpRubyTextContext_Impl::Characters( const OUString& rChars )
{
    pHint->AppendText( rChars );
}

// ---------------------------------------------------------------------

class XMLImpRubyContext_Impl : public SvXMLImportContext
{
    XMLHints_Impl&	rHints;
    XMLRubyHint_Impl	*pHint;
    
    sal_Bool&		rIgnoreLeadingSpace;

public:

    TYPEINFO();

    XMLImpRubyContext_Impl(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            XMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace );

    virtual ~XMLImpRubyContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
};

TYPEINIT1( XMLImpRubyContext_Impl, SvXMLImportContext );

XMLImpRubyContext_Impl::XMLImpRubyContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        XMLHints_Impl& rHnts,
        sal_Bool& rIgnLeadSpace ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rHints( rHnts ),
    rIgnoreLeadingSpace( rIgnLeadSpace ),
    pHint( new XMLRubyHint_Impl( 
              GetImport().GetTextImport()->GetCursorAsRange()->getStart() ) )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        if( XML_NAMESPACE_TEXT == nPrefix &&
            IsXMLToken( aLocalName, XML_STYLE_NAME ) )
        {
            pHint->SetStyleName( rValue );
            break;
        }
    }
    rHints.Insert( pHint, rHints.Count() );
}

XMLImpRubyContext_Impl::~XMLImpRubyContext_Impl()
{
    if( pHint )
        pHint->SetEnd( GetImport().GetTextImport()
                            ->GetCursorAsRange()->getStart() );
}

SvXMLImportContext *XMLImpRubyContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext;
    if( XML_NAMESPACE_TEXT == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_RUBY_BASE ) )
            pContext = new XMLImpRubyBaseContext_Impl( GetImport(), nPrefix,
                                                       rLocalName,
                                                       xAttrList,
                                                       rHints,
                                                       rIgnoreLeadingSpace );
        else if( IsXMLToken( rLocalName, XML_RUBY_TEXT ) )
            pContext = new XMLImpRubyTextContext_Impl( GetImport(), nPrefix,
                                                       rLocalName,
                                                       xAttrList,
                                                       pHint );
        else
                pContext = 0; //  TODO What value should this be?

    }
    else
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList );

    return pContext;
}

// ---------------------------------------------------------------------


/**
 * Process index marks.
 *
 * All *-mark-end index marks should instantiate *this* class (because
 * it doesn't process attributes other than ID), while the *-mark and
 * *-mark-start classes should instantiate the apporpiate subclasses.
 */
class XMLIndexMarkImportContext_Impl : public SvXMLImportContext 
{
    const OUString sAlternativeText;

    XMLHints_Impl& rHints;
    const enum XMLTextPElemTokens eToken;
    OUString sID;

public:
    TYPEINFO();

    XMLIndexMarkImportContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        enum XMLTextPElemTokens nTok,
        XMLHints_Impl& rHnts);

    void StartElement(const Reference<xml::sax::XAttributeList> & xAttrList);

protected:

    /// process all attributes
    void ProcessAttributes(const Reference<xml::sax::XAttributeList> & xAttrList,
                           Reference<beans::XPropertySet>& rPropSet);

    /** 
     * All marks can be created immediatly. Since we don't care about
     * the element content, ProcessAttribute should set the properties
     * immediatly.
     *
     * This method tolerates an empty PropertySet; subclasses however
     * are not expected to.
     */
    virtual void ProcessAttribute(sal_uInt16 nNamespace,
                                  OUString sLocalName,
                                  OUString sValue,
                                  Reference<beans::XPropertySet>& rPropSet);

    static void GetServiceName(OUString& sServiceName,
                               enum XMLTextPElemTokens nToken);

    sal_Bool CreateMark(Reference<beans::XPropertySet>& rPropSet,
                        const OUString& rServiceName);
};


TYPEINIT1( XMLIndexMarkImportContext_Impl, SvXMLImportContext );

XMLIndexMarkImportContext_Impl::XMLIndexMarkImportContext_Impl(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    enum XMLTextPElemTokens eTok,
    XMLHints_Impl& rHnts) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rHints(rHnts),
        eToken(eTok),
        sAlternativeText(RTL_CONSTASCII_USTRINGPARAM("AlternativeText")),
        sID()
{
}

void XMLIndexMarkImportContext_Impl::StartElement(
    const Reference<xml::sax::XAttributeList> & xAttrList)
{
    // get Cursor position (needed for all cases)
    Reference<XTextRange> xPos = 
        GetImport().GetTextImport()->GetCursor()->getStart();
    Reference<beans::XPropertySet> xMark;

    switch (eToken)
    {
        case XML_TOK_TEXT_TOC_MARK:
        case XML_TOK_TEXT_USER_INDEX_MARK:
        case XML_TOK_TEXT_ALPHA_INDEX_MARK:
        {
            // single mark: create mark and insert
            OUString sService;
            GetServiceName(sService, eToken);
            if (CreateMark(xMark, sService))
            {
                ProcessAttributes(xAttrList, xMark);
                XMLHint_Impl* pHint = new XMLIndexMarkHint_Impl(xMark, xPos);
                rHints.Insert(pHint, rHints.Count());
            }
            // else: can't create mark -> ignore
            break;
        }

        case XML_TOK_TEXT_TOC_MARK_START:
        case XML_TOK_TEXT_USER_INDEX_MARK_START:
        case XML_TOK_TEXT_ALPHA_INDEX_MARK_START:
        {
            // start: create mark and insert (if ID is found)
            OUString sService;
            GetServiceName(sService, eToken);
            if (CreateMark(xMark, sService))
            {
                ProcessAttributes(xAttrList, xMark);
                if (sID.getLength() > 0)
                {
                    // process only if we find an ID
                    XMLHint_Impl* pHint = 
                        new XMLIndexMarkHint_Impl(xMark, xPos, sID);
                    rHints.Insert(pHint, rHints.Count());
                }
                // else: no ID -> we'll never find the end -> ignore
            }
            // else: can't create mark -> ignore
            break;
        }

        case XML_TOK_TEXT_TOC_MARK_END:
        case XML_TOK_TEXT_USER_INDEX_MARK_END:
        case XML_TOK_TEXT_ALPHA_INDEX_MARK_END:
        {
            // end: search for ID and set end of mark

            // call process attributes with empty XPropertySet:
            ProcessAttributes(xAttrList, xMark);
            if (sID.getLength() > 0)
            {
                // if we have an ID, find the hint and set the end position
                sal_uInt16 nCount = rHints.Count();
                for(sal_uInt16 nPos = 0; nPos < nCount; nPos++)
                {
                    XMLHint_Impl *pHint = rHints[nPos];
                    if ( pHint->IsIndexMark() && 
                         sID.equals( 
                             ((XMLIndexMarkHint_Impl *)pHint)->GetID()) )
                    {
                        // set end and stop searching
                        pHint->SetEnd(xPos);
                        break;
                    }
                }
            }
            // else: no ID -> ignore
            break;
        }

        default:
            DBG_ERROR("unknown index mark type!");
            break;
    }
}
    
void XMLIndexMarkImportContext_Impl::ProcessAttributes(
    const Reference<xml::sax::XAttributeList> & xAttrList,
    Reference<beans::XPropertySet>& rPropSet)
{
    // process attributes
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++) 
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        ProcessAttribute(nPrefix, sLocalName,
                         xAttrList->getValueByIndex(i),
                         rPropSet);
    }
}

void XMLIndexMarkImportContext_Impl::ProcessAttribute(
    sal_uInt16 nNamespace,
    OUString sLocalName,
    OUString sValue,
    Reference<beans::XPropertySet>& rPropSet)
{
    // we only know ID + string-value attribute; 
    // (former: marks, latter: -start + -end-marks)
    // the remainder is handled in sub-classes
    switch (eToken)
    {
        case XML_TOK_TEXT_TOC_MARK:
        case XML_TOK_TEXT_USER_INDEX_MARK:
        case XML_TOK_TEXT_ALPHA_INDEX_MARK:
            if ( (XML_NAMESPACE_TEXT == nNamespace) &&
                 IsXMLToken( sLocalName, XML_STRING_VALUE ) )
            {
                Any aAny;
                aAny <<= sValue;
                rPropSet->setPropertyValue(sAlternativeText, aAny);
            }
            // else: ignore!
            break;

        case XML_TOK_TEXT_TOC_MARK_START:
        case XML_TOK_TEXT_USER_INDEX_MARK_START:
        case XML_TOK_TEXT_ALPHA_INDEX_MARK_START:
        case XML_TOK_TEXT_TOC_MARK_END:
        case XML_TOK_TEXT_USER_INDEX_MARK_END:
        case XML_TOK_TEXT_ALPHA_INDEX_MARK_END:
            if ( (XML_NAMESPACE_TEXT == nNamespace) &&
                 IsXMLToken( sLocalName, XML_ID ) )
            {
                sID = sValue;
            }
            // else: ignore
            break;

        default:
            DBG_ERROR("unknown index mark type!");
            break;
    }
}

static const sal_Char sAPI_com_sun_star_text_ContentIndexMark[] = 
        "com.sun.star.text.ContentIndexMark";
static const sal_Char sAPI_com_sun_star_text_UserIndexMark[] = 
        "com.sun.star.text.UserIndexMark";
static const sal_Char sAPI_com_sun_star_text_DocumentIndexMark[] =
        "com.sun.star.text.DocumentIndexMark";


void XMLIndexMarkImportContext_Impl::GetServiceName(
    OUString& sServiceName,
    enum XMLTextPElemTokens eToken)
{
    switch (eToken)
    {
        case XML_TOK_TEXT_TOC_MARK:
        case XML_TOK_TEXT_TOC_MARK_START:
        case XML_TOK_TEXT_TOC_MARK_END:
        {
            OUString sTmp(RTL_CONSTASCII_USTRINGPARAM(
                sAPI_com_sun_star_text_ContentIndexMark));
            sServiceName = sTmp;
            break;
        }

        case XML_TOK_TEXT_USER_INDEX_MARK:
        case XML_TOK_TEXT_USER_INDEX_MARK_START:
        case XML_TOK_TEXT_USER_INDEX_MARK_END:
        {
            OUString sTmp(RTL_CONSTASCII_USTRINGPARAM(
                sAPI_com_sun_star_text_UserIndexMark));
            sServiceName = sTmp;
            break;
        }

        case XML_TOK_TEXT_ALPHA_INDEX_MARK:
        case XML_TOK_TEXT_ALPHA_INDEX_MARK_START:
        case XML_TOK_TEXT_ALPHA_INDEX_MARK_END:
        {
            OUString sTmp(RTL_CONSTASCII_USTRINGPARAM(
                sAPI_com_sun_star_text_DocumentIndexMark));
            sServiceName = sTmp;
            break;
        }

        default:
        {
            DBG_ERROR("unknown index mark type!");
            OUString sTmp;
            sServiceName = sTmp;
            break;
        }
    }
}


sal_Bool XMLIndexMarkImportContext_Impl::CreateMark(
    Reference<beans::XPropertySet>& rPropSet,
    const OUString& rServiceName)
{
    Reference<lang::XMultiServiceFactory> 
        xFactory(GetImport().GetModel(), UNO_QUERY);

    if( xFactory.is() )
    {
        Reference<XInterface> xIfc = xFactory->createInstance(rServiceName);
        if( xIfc.is() )
        {
            Reference<beans::XPropertySet> xPropSet( xIfc, UNO_QUERY );
            if (xPropSet.is())
                rPropSet = xPropSet;	
            return sal_True;
        }
    }

    return sal_False;
}


class XMLTOCMarkImportContext_Impl : public XMLIndexMarkImportContext_Impl
{
    const OUString sLevel;

public:
    TYPEINFO();

    XMLTOCMarkImportContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        enum XMLTextPElemTokens nTok,
        XMLHints_Impl& rHnts);

protected:

    /** process outline level */
    virtual void ProcessAttribute(sal_uInt16 nNamespace,
                                  OUString sLocalName,
                                  OUString sValue,
                                  Reference<beans::XPropertySet>& rPropSet);
};

TYPEINIT1( XMLTOCMarkImportContext_Impl, XMLIndexMarkImportContext_Impl );


XMLTOCMarkImportContext_Impl::XMLTOCMarkImportContext_Impl(
    SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName,
    enum XMLTextPElemTokens nTok, XMLHints_Impl& rHnts) :
        XMLIndexMarkImportContext_Impl(rImport, nPrefix, rLocalName, 
                                       nTok, rHnts),
        sLevel(RTL_CONSTASCII_USTRINGPARAM("Level"))
{
}

void XMLTOCMarkImportContext_Impl::ProcessAttribute(
    sal_uInt16 nNamespace,
    OUString sLocalName,
    OUString sValue,
    Reference<beans::XPropertySet>& rPropSet)
{
    DBG_ASSERT(rPropSet.is(), "need PropertySet");

    if ((XML_NAMESPACE_TEXT == nNamespace) &&
        IsXMLToken( sLocalName, XML_OUTLINE_LEVEL ) )
    {
        // ouline level: set Level property
        sal_Int32 nTmp;
        if (SvXMLUnitConverter::convertNumber(
            nTmp, sValue, 0, 
            GetImport().GetTextImport()->GetChapterNumbering()->getCount()))
        {
            Any aAny;
            aAny <<= (sal_Int16)nTmp;
            rPropSet->setPropertyValue(sLevel, aAny);
        }
        // else: value out of range -> ignore
    }
    else
    {
        // else: delegate to superclass
        XMLIndexMarkImportContext_Impl::ProcessAttribute(
            nNamespace, sLocalName, sValue, rPropSet);
    }
}

class XMLUserIndexMarkImportContext_Impl : public XMLIndexMarkImportContext_Impl
{
    const OUString sUserIndexName;
    const OUString sLevel;

public:
    TYPEINFO();

    XMLUserIndexMarkImportContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        enum XMLTextPElemTokens nTok,
        XMLHints_Impl& rHnts);

protected:

    /** process index name */
    virtual void ProcessAttribute(sal_uInt16 nNamespace,
                                  OUString sLocalName,
                                  OUString sValue,
                                  Reference<beans::XPropertySet>& rPropSet);
};

TYPEINIT1( XMLUserIndexMarkImportContext_Impl, XMLIndexMarkImportContext_Impl);

XMLUserIndexMarkImportContext_Impl::XMLUserIndexMarkImportContext_Impl(
    SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName,
    enum XMLTextPElemTokens nTok, XMLHints_Impl& rHnts) :
        XMLIndexMarkImportContext_Impl(rImport, nPrefix, rLocalName, 
                                       nTok, rHnts),
        sUserIndexName(RTL_CONSTASCII_USTRINGPARAM("UserIndexName")),
        sLevel(RTL_CONSTASCII_USTRINGPARAM("Level"))
{
}

void XMLUserIndexMarkImportContext_Impl::ProcessAttribute(
    sal_uInt16 nNamespace, OUString sLocalName, OUString sValue,
    Reference<beans::XPropertySet>& rPropSet)
{
    if ( XML_NAMESPACE_TEXT == nNamespace )
    {
        if ( IsXMLToken( sLocalName, XML_INDEX_NAME ) )
        {
            Any aAny;
            aAny <<= sValue;
            rPropSet->setPropertyValue(sUserIndexName, aAny);
        }
        else if ( IsXMLToken( sLocalName, XML_OUTLINE_LEVEL ) )
        {
            // ouline level: set Level property
            sal_Int32 nTmp;
            if (SvXMLUnitConverter::convertNumber(
                nTmp, sValue, 0, 
               GetImport().GetTextImport()->GetChapterNumbering()->getCount()))
            {
                Any aAny;
                aAny <<= (sal_Int16)nTmp;
                rPropSet->setPropertyValue(sLevel, aAny);
            }
            // else: value out of range -> ignore
        }
        else
        {
            // else: unknown text property: delegate to super class
            XMLIndexMarkImportContext_Impl::ProcessAttribute(
                nNamespace, sLocalName, sValue, rPropSet);
        }
    }
    else
    {
        // else: unknown namespace: delegate to super class
        XMLIndexMarkImportContext_Impl::ProcessAttribute(
            nNamespace, sLocalName, sValue, rPropSet);
    }
}


class XMLAlphaIndexMarkImportContext_Impl : public XMLIndexMarkImportContext_Impl
{
    const OUString sPrimaryKey;
    const OUString sSecondaryKey;
    const OUString sTextReading;
    const OUString sPrimaryKeyReading;
    const OUString sSecondaryKeyReading;
    const OUString sMainEntry;

public:
    TYPEINFO();

    XMLAlphaIndexMarkImportContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        enum XMLTextPElemTokens nTok,
        XMLHints_Impl& rHnts);

protected:

    /** process primary + secondary keys */
    virtual void ProcessAttribute(sal_uInt16 nNamespace,
                                  OUString sLocalName,
                                  OUString sValue,
                                  Reference<beans::XPropertySet>& rPropSet);
};

TYPEINIT1( XMLAlphaIndexMarkImportContext_Impl, 
           XMLIndexMarkImportContext_Impl );

XMLAlphaIndexMarkImportContext_Impl::XMLAlphaIndexMarkImportContext_Impl(
    SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName,
    enum XMLTextPElemTokens nTok, XMLHints_Impl& rHnts) :
        XMLIndexMarkImportContext_Impl(rImport, nPrefix, rLocalName, 
                                       nTok, rHnts),
        sPrimaryKey(RTL_CONSTASCII_USTRINGPARAM("PrimaryKey")),
        sSecondaryKey(RTL_CONSTASCII_USTRINGPARAM("SecondaryKey")),
        sTextReading(RTL_CONSTASCII_USTRINGPARAM("TextReading")),
        sPrimaryKeyReading(RTL_CONSTASCII_USTRINGPARAM("PrimaryKeyReading")),
        sSecondaryKeyReading(RTL_CONSTASCII_USTRINGPARAM("SecondaryKeyReading")),
        sMainEntry(RTL_CONSTASCII_USTRINGPARAM("IsMainEntry"))
{
}

void XMLAlphaIndexMarkImportContext_Impl::ProcessAttribute(
    sal_uInt16 nNamespace, OUString sLocalName, OUString sValue,
    Reference<beans::XPropertySet>& rPropSet)
{
    if (XML_NAMESPACE_TEXT == nNamespace)
    {
        if ( IsXMLToken( sLocalName, XML_KEY1 ) )
        {
            Any aAny;
            aAny <<= sValue;
            rPropSet->setPropertyValue(sPrimaryKey, aAny);
        }
        else if ( IsXMLToken( sLocalName, XML_KEY2 ) )
        {
            Any aAny;
            aAny <<= sValue;
            rPropSet->setPropertyValue(sSecondaryKey, aAny);
        }
        else if ( IsXMLToken( sLocalName, XML_KEY1_PHONETIC ) )
        {
            Any aAny;
            aAny <<= sValue;
            rPropSet->setPropertyValue(sPrimaryKeyReading, aAny);
        }
        else if ( IsXMLToken( sLocalName, XML_KEY2_PHONETIC ) )
        {
            Any aAny;
            aAny <<= sValue;
            rPropSet->setPropertyValue(sSecondaryKeyReading, aAny);
        }
        else if ( IsXMLToken( sLocalName, XML_STRING_VALUE_PHONETIC ) )
        {
            Any aAny;
            aAny <<= sValue;
            rPropSet->setPropertyValue(sTextReading, aAny);
        }
        else if ( IsXMLToken( sLocalName, XML_MAIN_ENTRY ) )
        {
            sal_Bool bMainEntry = sal_False, bTmp;

            if (SvXMLUnitConverter::convertBool(bTmp, sValue))
                bMainEntry = bTmp;

            Any aAny;
            aAny.setValue(&bMainEntry, ::getBooleanCppuType());
            rPropSet->setPropertyValue(sMainEntry, aAny);
        }
        else
        {
            XMLIndexMarkImportContext_Impl::ProcessAttribute(
                nNamespace, sLocalName, sValue, rPropSet);
        }
    }
    else
    {
        XMLIndexMarkImportContext_Impl::ProcessAttribute(
            nNamespace, sLocalName, sValue, rPropSet);
    }
}


// ---------------------------------------------------------------------

TYPEINIT1( XMLImpSpanContext_Impl, SvXMLImportContext );

XMLImpSpanContext_Impl::XMLImpSpanContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        XMLHints_Impl& rHnts,
        sal_Bool& rIgnLeadSpace
#ifdef CONV_STAR_FONTS
    ,sal_uInt8				nSFConvFlags
#endif
                                              ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rHints( rHnts ),
    rIgnoreLeadingSpace( rIgnLeadSpace ),
    pHint( 0  ),
    sTextFrame(RTL_CONSTASCII_USTRINGPARAM("TextFrame"))
#ifdef CONV_STAR_FONTS
    ,nStarFontsConvFlags( nSFConvFlags & (CONV_FROM_STAR_BATS|CONV_FROM_STAR_MATH) )
#endif
{
    OUString aStyleName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        if( XML_NAMESPACE_TEXT == nPrefix &&
            IsXMLToken( aLocalName, XML_STYLE_NAME ) )
            aStyleName = xAttrList->getValueByIndex( i );
    }

    if( aStyleName.getLength() )
    {
        pHint = new XMLStyleHint_Impl( aStyleName,
                  GetImport().GetTextImport()->GetCursorAsRange()->getStart() );
            rHints.Insert( pHint, rHints.Count() );
    }
}

XMLImpSpanContext_Impl::~XMLImpSpanContext_Impl()
{
    if( pHint )
        pHint->SetEnd( GetImport().GetTextImport()
                            ->GetCursorAsRange()->getStart() );
}

SvXMLImportContext *XMLImpSpanContext_Impl::CreateChildContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_uInt16 nToken,
        XMLHints_Impl& rHints,
        sal_Bool& rIgnoreLeadingSpace
#ifdef CONV_STAR_FONTS
    ,sal_uInt8				nStarFontsConvFlags
#endif
     )
{
    SvXMLImportContext *pContext = 0;
    sal_Bool bInsertTextFrame = sal_False;
    sal_uInt16 nTextFrameType = 0;

    sal_Bool bObjectOLE = sal_False;
    switch( nToken )
    {
    case XML_TOK_TEXT_SPAN:
        pContext = new XMLImpSpanContext_Impl( rImport, nPrefix,
                                               rLocalName, xAttrList, 
                                               rHints,
                                               rIgnoreLeadingSpace
#ifdef CONV_STAR_FONTS
                                               ,nStarFontsConvFlags
#endif
                                             );
        break;

    case XML_TOK_TEXT_TAB_STOP:
        pContext = new XMLImpCharContext_Impl( rImport, nPrefix,
                                               rLocalName, xAttrList, 
                                               0x0009, sal_False );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_LINE_BREAK:
        pContext = new XMLImpCharContext_Impl( rImport, nPrefix,
                                               rLocalName, xAttrList, 
                                               ControlCharacter::LINE_BREAK );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_S:
        pContext = new XMLImpCharContext_Impl( rImport, nPrefix,
                                               rLocalName, xAttrList, 
                                               0x0020, sal_True );
        break;

    case XML_TOK_TEXT_HYPERLINK:
    {
        // test for HyperLinkURL property. If present, insert link as
        // text property (StarWriter), else try to insert as text
        // field (StarCalc, StarDraw, ...)
        Reference<beans::XPropertySet> xPropSet( 
            rImport.GetTextImport()->GetCursor(), 
            UNO_QUERY );

        const OUString sHyperLinkURL(
            RTL_CONSTASCII_USTRINGPARAM("HyperLinkURL"));

        if (xPropSet->getPropertySetInfo()->hasPropertyByName(sHyperLinkURL))
        {
            pContext = new XMLImpHyperlinkContext_Impl( rImport, nPrefix,
                                                        rLocalName, xAttrList, 
                                                        rHints,
                                                        rIgnoreLeadingSpace );
        }
        else
        {
            pContext = new XMLUrlFieldImportContext( rImport, 
                                              *rImport.GetTextImport().get(),
                                                     nPrefix, rLocalName);
            //whitespace handling like other fields
            rIgnoreLeadingSpace = sal_False;

        }
        break;
    }

    case XML_TOK_TEXT_RUBY:
        pContext = new XMLImpRubyContext_Impl( rImport, nPrefix,
                                               rLocalName, xAttrList, 
                                               rHints,
                                               rIgnoreLeadingSpace );
        break;

    case XML_TOK_TEXT_ENDNOTE:
    case XML_TOK_TEXT_FOOTNOTE:
#ifndef SVX_LIGHT
        if (rImport.GetTextImport()->IsInFrame())
        {
            // we must not insert footnotes into text frames
            pContext = new SvXMLImportContext( rImport, nPrefix, 
                                               rLocalName );
        }
        else
        {
            pContext = new XMLFootnoteImportContext( rImport,
                                                     *rImport.GetTextImport().get(),
                                                     nPrefix, rLocalName );
        }
#else
        // create default context to skip content
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_REFERENCE:
    case XML_TOK_TEXT_BOOKMARK:
    case XML_TOK_TEXT_BOOKMARK_START:
    case XML_TOK_TEXT_BOOKMARK_END:
        pContext = new XMLTextMarkImportContext( rImport,
                                                 *rImport.GetTextImport().get(),
                                                 nPrefix, rLocalName );
        break;

    case XML_TOK_TEXT_REFERENCE_START:
        pContext = new XMLStartReferenceContext_Impl( rImport,
                                                      nPrefix, rLocalName,
                                                      rHints, xAttrList );
        break;

    case XML_TOK_TEXT_REFERENCE_END:
        pContext = new XMLEndReferenceContext_Impl( rImport,
                                                    nPrefix, rLocalName,
                                                    rHints, xAttrList );
        break;

    case XML_TOK_TEXT_TEXTBOX:
        bInsertTextFrame = sal_True;
        nTextFrameType = XML_TEXT_FRAME_TEXTBOX;
        break;

    case XML_TOK_TEXT_IMAGE:
        bInsertTextFrame = sal_True;
        nTextFrameType = XML_TEXT_FRAME_GRAPHIC;
        break;
    case XML_TOK_TEXT_APPLET:
        bInsertTextFrame = sal_True;
        nTextFrameType = XML_TEXT_FRAME_APPLET;
        break;
    case XML_TOK_TEXT_FLOATING_FRAME:
        bInsertTextFrame = sal_True;
        nTextFrameType = XML_TEXT_FRAME_FLOATING_FRAME;
        break;
    case XML_TOK_TEXT_PLUGIN:
        bInsertTextFrame = sal_True;
        nTextFrameType = XML_TEXT_FRAME_PLUGIN;
        break;
        
    case XML_TOK_TEXT_OBJECT_OLE:
        bInsertTextFrame = sal_True;
        nTextFrameType = XML_TEXT_FRAME_OBJECT_OLE;
        break;
    case XML_TOK_TEXT_OBJECT:
        bInsertTextFrame = sal_True;
        nTextFrameType = XML_TEXT_FRAME_OBJECT;
        break;
        
    case XML_TOK_DRAW_A:
        {
            Reference < XTextRange > xAnchorPos =
                rImport.GetTextImport()->GetCursor()->getStart();
            XMLTextFrameHint_Impl *pHint =
                new XMLTextFrameHint_Impl( xAnchorPos );
            XMLTextFrameHyperlinkContext *pLinkContext = 
                new XMLTextFrameHyperlinkContext( rImport, nPrefix,
                                        rLocalName, xAttrList, 
                                        TextContentAnchorType_AS_CHARACTER,
                                        &pHint->GetTextContentRef(),
                                        &pHint->GetAnchorTypeRef() );
            rHints.Insert( pHint, rHints.Count() );
            pContext = pLinkContext;
        }
        break;
        
    case XML_TOK_TEXT_TOC_MARK:
    case XML_TOK_TEXT_TOC_MARK_START:
        pContext = new XMLTOCMarkImportContext_Impl(
            rImport, nPrefix, rLocalName, 
            (enum XMLTextPElemTokens)nToken, rHints);
        break;

    case XML_TOK_TEXT_USER_INDEX_MARK:
    case XML_TOK_TEXT_USER_INDEX_MARK_START:
        pContext = new XMLUserIndexMarkImportContext_Impl(
            rImport, nPrefix, rLocalName, 
            (enum XMLTextPElemTokens)nToken, rHints);
        break;

    case XML_TOK_TEXT_ALPHA_INDEX_MARK:
    case XML_TOK_TEXT_ALPHA_INDEX_MARK_START:
        pContext = new XMLAlphaIndexMarkImportContext_Impl(
            rImport, nPrefix, rLocalName, 
            (enum XMLTextPElemTokens)nToken, rHints);
        break;

    case XML_TOK_TEXT_TOC_MARK_END:
    case XML_TOK_TEXT_USER_INDEX_MARK_END:
    case XML_TOK_TEXT_ALPHA_INDEX_MARK_END:
        pContext = new XMLIndexMarkImportContext_Impl( 
            rImport, nPrefix, rLocalName, (enum XMLTextPElemTokens)nToken, 
            rHints);
        break;

    case XML_TOK_TEXTP_CHANGE_START:
    case XML_TOK_TEXTP_CHANGE_END:
    case XML_TOK_TEXTP_CHANGE:
        pContext = new XMLChangeImportContext(
            rImport, nPrefix, rLocalName,
            (nToken != XML_TOK_TEXTP_CHANGE_END),
            (nToken != XML_TOK_TEXTP_CHANGE_START),
            sal_False);
        break;

    default:
        // none of the above? then it's probably  a text field!
        pContext = 
            XMLTextFieldImportContext::CreateTextFieldImportContext(
                rImport, *rImport.GetTextImport().get(), nPrefix, rLocalName,
                nToken);
        // #108784# import draw elements (except control shapes in headers)
        if( pContext == NULL &&
            !( rImport.GetTextImport()->IsInHeaderFooter() &&
               nPrefix == XML_NAMESPACE_DRAW && 
               IsXMLToken( rLocalName, XML_CONTROL ) ) )
        {
            Reference < XShapes > xShapes;
            pContext = rImport.GetShapeImport()->CreateGroupChildContext(
                    rImport, nPrefix, rLocalName, xAttrList, xShapes );
        }
        if( !pContext )
        {
            // ignore unknown content
            pContext = 
                new SvXMLImportContext( rImport, nPrefix, rLocalName );
        }
        // Behind fields, shapes and any unknown content blanks aren't ignored
        rIgnoreLeadingSpace = sal_False;
    }

    if( bInsertTextFrame )
    {
        if( XMLTextImportHelper::HasDrawNameAttribute( xAttrList, rImport.GetNamespaceMap() ) )
        {
            Reference < XTextRange > xAnchorPos =
                rImport.GetTextImport()->GetCursor()->getStart();
            XMLTextFrameContext *pTextFrameContext =
                new XMLTextFrameContext( rImport, nPrefix,
                                         rLocalName, xAttrList, 
                                         TextContentAnchorType_AS_CHARACTER,
                                         nTextFrameType );
            if( TextContentAnchorType_AT_CHARACTER ==
                    pTextFrameContext->GetAnchorType() &&
                pTextFrameContext->GetTextContent().is() )	
            {
                rHints.Insert( new XMLTextFrameHint_Impl( 
                    pTextFrameContext->GetTextContent(), xAnchorPos ),
                    rHints.Count() );
            }
            pContext = pTextFrameContext;
        }
        else
        {
            Reference < XShapes > xShapes;
            pContext = rImport.GetShapeImport()->CreateGroupChildContext(
                    rImport, nPrefix, rLocalName, xAttrList, xShapes );
        }

        rIgnoreLeadingSpace = sal_False;
    }

    return pContext;
}

SvXMLImportContext *XMLImpSpanContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextPElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    
    return CreateChildContext( GetImport(), nPrefix, rLocalName, xAttrList,
                               nToken, rHints, rIgnoreLeadingSpace
#ifdef CONV_STAR_FONTS
                               ,nStarFontsConvFlags
#endif
                             );
}

void XMLImpSpanContext_Impl::Characters( const OUString& rChars )
{
#ifdef CONV_STAR_FONTS
    OUString sStyleName;
    if( pHint )
        sStyleName = pHint->GetStyleName();
    OUString sChars = 
        GetImport().GetTextImport()->ConvertStarFonts( rChars, sStyleName,
                                                       nStarFontsConvFlags,
                                                       sal_False, GetImport() );
    GetImport().GetTextImport()->InsertString( sChars, rIgnoreLeadingSpace );
#else
    GetImport().GetTextImport()->InsertString( rChars, rIgnoreLeadingSpace );
#endif
}

// ---------------------------------------------------------------------

TYPEINIT1( XMLParaContext, SvXMLImportContext );

XMLParaContext::XMLParaContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bHead ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xStart( rImport.GetTextImport()->GetCursorAsRange()->getStart() ),
    nOutlineLevel( 1 ),
    pHints( 0 ),
    bIgnoreLeadingSpace( sal_True ),
    bHeading( bHead )
#ifdef CONV_STAR_FONTS
    ,nStarFontsConvFlags( 0 )
#endif
{
    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextPAttrTokenMap();

    OUString aCondStyleName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_P_STYLE_NAME:
            sStyleName = rValue;
            break;
        case XML_TOK_TEXT_P_COND_STYLE_NAME:
            aCondStyleName = rValue;
            break;
        case XML_TOK_TEXT_P_LEVEL:
            {
                sal_Int32 nTmp = rValue.toInt32();
                if( nTmp > 0L )
                {
                    if( nTmp > 127 )
                        nTmp = 127;
                    nOutlineLevel = (sal_Int8)nTmp;
                }
            }

        }
    }

    if( aCondStyleName.getLength() )
        sStyleName = aCondStyleName;
}

XMLParaContext::~XMLParaContext()
{
    UniReference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    Reference < XTextRange > xEnd = xTxtImport->GetCursorAsRange()->getStart();

    // insert a paragraph break
    xTxtImport->InsertControlCharacter( ControlCharacter::APPEND_PARAGRAPH );

    // create a cursor that select the whole last paragraph
    Reference < XTextCursor > xAttrCursor=
        xTxtImport->GetText()->createTextCursorByRange( xStart );
    xAttrCursor->gotoRange( xEnd, sal_True );

    // #103445# for headings without style name, find the proper style
    if( bHeading && (sStyleName.getLength() == 0) )
        xTxtImport->FindOutlineStyleName( sStyleName, nOutlineLevel );

    // set style and hard attributes at the previous paragraph
    sStyleName = xTxtImport->SetStyleAndAttrs( xAttrCursor, sStyleName, sal_True );

    if( bHeading )
        xTxtImport->SetOutlineStyle( nOutlineLevel, sStyleName );

    if( pHints && pHints->Count() )
    {
        for( sal_uInt16 i=0; i<pHints->Count(); i++ )
        {
            XMLHint_Impl *pHint = (*pHints)[i];
            xAttrCursor->gotoRange( pHint->GetStart(), sal_False );
            xAttrCursor->gotoRange( pHint->GetEnd(), sal_True );
            switch( pHint->GetType() )
            {
            case XML_HINT_STYLE:
                {
                    const OUString& rStyleName =
                            ((XMLStyleHint_Impl *)pHint)->GetStyleName();
                    if( rStyleName.getLength() )
                        xTxtImport->SetStyleAndAttrs( xAttrCursor, rStyleName,
                                                      sal_False );
                }
                break;
            case XML_HINT_REFERENCE:
                {
                    const OUString& rRefName =
                            ((XMLReferenceHint_Impl *)pHint)->GetRefName();
                    if( rRefName.getLength() )
                    {
                        if( !pHint->GetEnd().is() )
                            pHint->SetEnd(xEnd);

                        // convert XCursor to XTextRange
                        Reference<XTextRange> xRange(xAttrCursor, UNO_QUERY);

                        // reference name uses rStyleName member
                        // borrow from XMLTextMarkImportContext
                        XMLTextMarkImportContext::CreateAndInsertMark(
                            GetImport(),
                            OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.text.ReferenceMark")),
                            rRefName,
                            xRange);
                    }
                }
                break;
            case XML_HINT_HYPERLINK:
                {
                    const XMLHyperlinkHint_Impl *pHHint = 
                        (const XMLHyperlinkHint_Impl *)pHint;
                    xTxtImport->SetHyperlink( xAttrCursor,
                                              pHHint->GetHRef(),
                                              pHHint->GetName(),
                                              pHHint->GetTargetFrameName(),
                                              pHHint->GetStyleName(),
                                              pHHint->GetVisitedStyleName(),
                                              pHHint->GetEventsContext() );
                }
                break;
            case XML_HINT_RUBY:
                {
                    const XMLRubyHint_Impl *pRHint = 
                        (const XMLRubyHint_Impl *)pHint;
                    xTxtImport->SetRuby( xAttrCursor,
                                         pRHint->GetStyleName(),
                                         pRHint->GetTextStyleName(),
                                         pRHint->GetText() );
                }
                break;
            case XML_HINT_INDEX_MARK:
                {
                    Reference<beans::XPropertySet> xMark(
                        ((const XMLIndexMarkHint_Impl *)pHint)->GetMark());
                    Reference<XTextContent> xContent(xMark,	UNO_QUERY);
                    Reference<XTextRange> xRange(xAttrCursor, UNO_QUERY);
                    xTxtImport->GetText()->insertTextContent( 
                        xRange, xContent, sal_True );
                }
                break;
            case XML_HINT_TEXT_FRAME:
                {
                    const XMLTextFrameHint_Impl *pFHint = 
                        (const XMLTextFrameHint_Impl *)pHint;
                    if( pFHint->IsBoundAtChar() )
                    {
                        Reference<XTextRange> xRange(xAttrCursor, UNO_QUERY);
                        pFHint->GetTextContent()->attach( xRange );
                    }
                }
                break;
            default:
                DBG_ASSERT( !this, "What's this" );
                break;
            }
        }
    }
    delete pHints;
}

SvXMLImportContext *XMLParaContext::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextPElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    if( !pHints )
        pHints = new XMLHints_Impl;
    return XMLImpSpanContext_Impl::CreateChildContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                   nToken, *pHints, bIgnoreLeadingSpace
#ifdef CONV_STAR_FONTS
                                , nStarFontsConvFlags
#endif
                                                     );
}

void XMLParaContext::Characters( const OUString& rChars )
{
#ifdef CONV_STAR_FONTS
    OUString sChars =
        GetImport().GetTextImport()->ConvertStarFonts( rChars, sStyleName,
                                                       nStarFontsConvFlags,
                                                       sal_True, GetImport() );
    GetImport().GetTextImport()->InsertString( sChars, bIgnoreLeadingSpace );
#else
    GetImport().GetTextImport()->InsertString( rChars, bIgnoreLeadingSpace );
#endif
}

}//end of namespace binfilter
