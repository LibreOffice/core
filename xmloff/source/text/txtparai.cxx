/*************************************************************************
 *
 *  $RCSfile: txtparai.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:07 $
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

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
//#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif


#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include "txtimp.hxx"
#endif
#ifndef _XMLOFF_TXTPARAI_HXX
#include "txtparai.hxx"
#endif
#ifndef _XMLOFF_TXTFLDI_HXX
#include "txtfldi.hxx"
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

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;


// ---------------------------------------------------------------------

class XMLHint_Impl
{
    OUString                 sStyleName;    // SfxItemSet aItemSet;
    Reference < XTextRange > xStart;    // xub_StrLen nStart;
    Reference < XTextRange > xEnd;      // xub_StrLen nEnd;

    sal_Bool                 bReference;

public:

    XMLHint_Impl( const OUString& rStyleName,
                  const Reference < XTextRange > & rPos,
                  sal_Bool bRef = sal_False ) :
        sStyleName( rStyleName ),
        xStart( rPos ),
        bReference( bRef )
    {
        if( !bReference )
            xEnd = rPos;
    }

    const OUString& GetStyleName() const { return sStyleName; }
    const Reference < XTextRange > & GetStart() const { return xStart; }
    const Reference < XTextRange > & GetEnd() const { return xEnd; }
    void SetEnd( const Reference < XTextRange > & rPos ) { xEnd = rPos; }
    sal_Bool IsReference() { return bReference; }
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
            XMLTextImportHelper& rTxtImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Unicode c,
            sal_Bool bCount );
    XMLImpCharContext_Impl(
            SvXMLImport& rImport,
            XMLTextImportHelper& rTxtImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Int16 nControl );

    virtual ~XMLImpCharContext_Impl();
};

TYPEINIT1( XMLImpCharContext_Impl, SvXMLImportContext );

XMLImpCharContext_Impl::XMLImpCharContext_Impl(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImport,
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
                aLocalName.compareToAscii( sXML_c ) == 0 )
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
        OUString sBuff( c );
        rTxtImport.InsertString( sBuff );
    }
    else
    {
        OUStringBuffer sBuff( nCount );
        while( nCount-- )
            sBuff.append( c );

        rTxtImport.InsertString( sBuff.makeStringAndClear() );
    }
}

XMLImpCharContext_Impl::XMLImpCharContext_Impl(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Int16 nControl ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    rTxtImport.InsertControlCharacter( nControl );
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
        const String& rLocalName,
        XMLHints_Impl& rHnts,
        const Reference<xml::sax::XAttributeList> & xAttrList);
};

TYPEINIT1( XMLStartReferenceContext_Impl, SvXMLImportContext );

XMLStartReferenceContext_Impl::XMLStartReferenceContext_Impl(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const String& rLocalName,
    XMLHints_Impl& rHints,
    const Reference<xml::sax::XAttributeList> & xAttrList) :
        SvXMLImportContext(rImport, nPrefix, rLocalName)
{
    OUString sName;

    // borrow FindName from XMLTextMarkImportContext, where bookmarks
    // and point references are handled.
    if (XMLTextMarkImportContext::FindName(GetImport(), xAttrList, sName))
    {
        XMLHint_Impl* pHint = new XMLHint_Impl(
            sName, rImport.GetTextImport()->GetCursor()->getStart(), sal_True);
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
        const String& rLocalName,
        XMLHints_Impl& rHnts,
        const Reference<xml::sax::XAttributeList> & xAttrList);
};

TYPEINIT1( XMLEndReferenceContext_Impl, SvXMLImportContext );

XMLEndReferenceContext_Impl::XMLEndReferenceContext_Impl(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const String& rLocalName,
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
            if ( rHints[nPos]->IsReference() &&
                 sName.equals(rHints[nPos]->GetStyleName()) )
            {
                // set end and stop searching
                rHints[nPos]->SetEnd(GetImport().GetTextImport()->
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
    XMLTextImportHelper& rTxtImport;
    XMLHints_Impl&  rHints;
    XMLHint_Impl    *pHint;

    sal_Bool&       rIgnoreLeadingSpace;

public:

    TYPEINFO();

    XMLImpSpanContext_Impl(
            SvXMLImport& rImport,
            XMLTextImportHelper& rTxtImp, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            XMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace );

    virtual ~XMLImpSpanContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void Characters( const OUString& rChars );
};

TYPEINIT1( XMLImpSpanContext_Impl, SvXMLImportContext );

XMLImpSpanContext_Impl::XMLImpSpanContext_Impl(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImp,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        XMLHints_Impl& rHnts,
        sal_Bool& rIgnLeadSpace ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rTxtImport( rTxtImp ),
    rHints( rHnts ),
    rIgnoreLeadingSpace( rIgnLeadSpace ),
    pHint( 0  )
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
            aLocalName.compareToAscii( sXML_style_name ) == 0 )
            aStyleName = xAttrList->getValueByIndex( i );
    }

    if( aStyleName.getLength() )
    {
        pHint = new XMLHint_Impl( aStyleName,
                                  rTxtImport.GetCursorAsRange()->getStart() );
            rHints.Insert( pHint, rHints.Count() );
    }
}

XMLImpSpanContext_Impl::~XMLImpSpanContext_Impl()
{
    if( pHint )
        pHint->SetEnd( rTxtImport.GetCursorAsRange()->getStart() );
}

SvXMLImportContext *XMLImpSpanContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = rTxtImport.GetTextPElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    switch( nToken )
    {
    case XML_TOK_TEXT_SPAN:
        pContext = new XMLImpSpanContext_Impl( GetImport(), rTxtImport, nPrefix,
                                               rLocalName, xAttrList,
                                               rHints,
                                               rIgnoreLeadingSpace );
        break;

    case XML_TOK_TEXT_TAB_STOP:
        pContext = new XMLImpCharContext_Impl( GetImport(), rTxtImport, nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x0009, sal_False );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_LINE_BREAK:
        pContext = new XMLImpCharContext_Impl( GetImport(), rTxtImport, nPrefix,
                                                 rLocalName, xAttrList,
                                                 ControlCharacter::LINE_BREAK );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_S:
        pContext = new XMLImpCharContext_Impl( GetImport(), rTxtImport, nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x0020, sal_True );
        break;

    case XML_TOK_TEXT_ENDNOTE:
    case XML_TOK_TEXT_FOOTNOTE:
        pContext = new XMLFootnoteImportContext( GetImport(), rTxtImport,
                                                 nPrefix, rLocalName );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_BOOKMARK:
    case XML_TOK_TEXT_BOOKMARK_START:
    case XML_TOK_TEXT_BOOKMARK_END:
    case XML_TOK_TEXT_REFERENCE:
        pContext = new XMLTextMarkImportContext( GetImport(), rTxtImport,
                                                 nPrefix, rLocalName );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_REFERENCE_START:
        pContext = new XMLStartReferenceContext_Impl( GetImport(),
                                                      nPrefix, rLocalName,
                                                      rHints, xAttrList );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_REFERENCE_END:
        pContext = new XMLEndReferenceContext_Impl( GetImport(),
                                                    nPrefix, rLocalName,
                                                    rHints, xAttrList );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_TEXTBOX:
        pContext = new XMLTextFrameContext( GetImport(), nPrefix,
                                            rLocalName, xAttrList,
                                            TextContentAnchorType_AS_CHARACTER,
                                            XML_TEXT_FRAME_TEXTBOX );
        break;

    case XML_TOK_TEXT_IMAGE:
        pContext = new XMLTextFrameContext( GetImport(), nPrefix,
                                            rLocalName, xAttrList,
                                            TextContentAnchorType_AS_CHARACTER,
                                            XML_TEXT_FRAME_GRAPHIC );
        break;

    default:
        // none of the above? then it's probably  a text field!
        pContext =
            XMLTextFieldImportContext::CreateTextFieldImportContext(
                GetImport(), rTxtImport, nPrefix, rLocalName, nToken);

        // ignore unknown content
        if (pContext == NULL)
        {
            pContext =
                new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
        }
        else
        {
            // text field found: white space!
            rIgnoreLeadingSpace = sal_False;
        }
        break;
    }

    return pContext;
}

void XMLImpSpanContext_Impl::Characters( const OUString& rChars )
{
    sal_Int32 nLen = rChars.getLength();
    OUStringBuffer sChars( nLen );

    for( sal_Int32 i=0; i < nLen; i++ )
    {
        sal_Unicode c = rChars[i];
        switch( c )
        {
            case 0x20:
            case 0x09:
            case 0x0a:
            case 0x0d:
                if( !rIgnoreLeadingSpace )
                    sChars.append( (sal_Unicode)0x20 );
                rIgnoreLeadingSpace = sal_True;
                break;
            default:
                rIgnoreLeadingSpace = sal_False;
                sChars.append( c );
                break;
        }
    }

    rTxtImport.InsertString( sChars.makeStringAndClear() );
}

// ---------------------------------------------------------------------

TYPEINIT1( XMLParaContext, SvXMLImportContext );

XMLParaContext::XMLParaContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImp,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bHead ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rTxtImport( rTxtImp ),
    xStart( rTxtImp.GetCursorAsRange()->getStart() ),
    nOutlineLevel( 1 ),
    pHints( 0 ),
    bIgnoreLeadingSpace( sal_True ),
    bHeading( bHead )
{
    const SvXMLTokenMap& rTokenMap = rTxtImport.GetTextPAttrTokenMap();

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
    Reference < XTextRange > xEnd = rTxtImport.GetCursorAsRange()->getStart();

    // insert a paragraph break
    rTxtImport.InsertControlCharacter( ControlCharacter::PARAGRAPH_BREAK );

    // create a cursor that select the whole last paragraph
    Reference < XTextCursor > xAttrCursor=
        rTxtImport.GetText()->createTextCursorByRange( xStart );
    xAttrCursor->gotoRange( xEnd, sal_True );

    // set style and hard attributes at the previous paragraph
    if( sStyleName.getLength() )
        sStyleName = rTxtImport.SetStyleAndAttrs( xAttrCursor,
                                                  sStyleName, sal_True );
    if( bHeading && !( rTxtImport.IsInsertMode() ||
                       rTxtImport.IsStylesOnlyMode() ))
        rTxtImport.SetOutlineStyle( nOutlineLevel, sStyleName );

    if( pHints && pHints->Count() )
    {
        for( sal_uInt16 i=0; i<pHints->Count(); i++ )
        {
            XMLHint_Impl *pHint = (*pHints)[i];
            const OUString& rStyleName = pHint->GetStyleName();
            if( rStyleName.getLength() )
            {
                // handle open references: truncate to paragraph end
                if (pHint->IsReference() && !pHint->GetEnd().is())
                {
                    pHint->SetEnd(xEnd);
                }

                xAttrCursor->gotoRange( pHint->GetStart(), sal_False );
                xAttrCursor->gotoRange( pHint->GetEnd(), sal_True );

                // handle styles and references
                if (pHint->IsReference())
                {
                    // convert XCursor to XTextRange
                    Reference<XTextRange> xRange(xAttrCursor, UNO_QUERY);

                    // reference name uses rStyleName member
                    // borrow from XMLTextMarkImportContext
                    XMLTextMarkImportContext::CreateAndInsertMark(
                        GetImport(),
                        OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.text.ReferenceMark")),
                        rStyleName,
                        xRange);
                }
                else
                {
                    rTxtImport.SetStyleAndAttrs( xAttrCursor, rStyleName,
                                                 sal_False );
                }
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

    const SvXMLTokenMap& rTokenMap = rTxtImport.GetTextPElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    switch( nToken )
    {
    case XML_TOK_TEXT_SPAN:
        if( !pHints )
            pHints = new XMLHints_Impl;
        pContext = new XMLImpSpanContext_Impl( GetImport(), rTxtImport,
                                                   nPrefix,
                                                 rLocalName, xAttrList,
                                                 *pHints, bIgnoreLeadingSpace );
        break;

    case XML_TOK_TEXT_TAB_STOP:
        pContext = new XMLImpCharContext_Impl( GetImport(), rTxtImport,
                                                nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x0009, sal_False );
        bIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_LINE_BREAK:
        pContext = new XMLImpCharContext_Impl( GetImport(), rTxtImport,
                                                nPrefix,
                                                 rLocalName, xAttrList,
                                                 ControlCharacter::LINE_BREAK );
        bIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_S:
        pContext = new XMLImpCharContext_Impl( GetImport(), rTxtImport,
                                                   nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x0020, sal_True );
        break;

    case XML_TOK_TEXT_ENDNOTE:
    case XML_TOK_TEXT_FOOTNOTE:
        pContext = new XMLFootnoteImportContext( GetImport(), rTxtImport,
                                                 nPrefix, rLocalName );
        bIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_BOOKMARK:
    case XML_TOK_TEXT_BOOKMARK_START:
    case XML_TOK_TEXT_BOOKMARK_END:
    case XML_TOK_TEXT_REFERENCE:
        pContext = new XMLTextMarkImportContext( GetImport(), rTxtImport,
                                                 nPrefix, rLocalName );
        bIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_REFERENCE_START:
        if( NULL == pHints )    // lazily create hints array
            pHints = new XMLHints_Impl;
        pContext = new XMLStartReferenceContext_Impl( GetImport(),
                                                      nPrefix, rLocalName,
                                                      *pHints, xAttrList );
        bIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_REFERENCE_END:
        if ( NULL != pHints )   // no hints, no start reference
        {
            pContext = new XMLEndReferenceContext_Impl( GetImport(),
                                                        nPrefix, rLocalName,
                                                        *pHints, xAttrList );
            bIgnoreLeadingSpace = sal_False;
        }
        break;

    case XML_TOK_TEXT_TEXTBOX:
        pContext = new XMLTextFrameContext( GetImport(), nPrefix,
                                            rLocalName, xAttrList,
                                            TextContentAnchorType_AS_CHARACTER,
                                            XML_TEXT_FRAME_TEXTBOX );
        break;

    case XML_TOK_TEXT_IMAGE:
        pContext = new XMLTextFrameContext( GetImport(), nPrefix,
                                            rLocalName, xAttrList,
                                            TextContentAnchorType_AS_CHARACTER,
                                            XML_TEXT_FRAME_GRAPHIC );
        break;


    default:
        // none of the above? then it's probably  a text field!
        pContext =
            XMLTextFieldImportContext::CreateTextFieldImportContext(
                GetImport(), rTxtImport, nPrefix, rLocalName, nToken);

        // ignore unknown content
        if (pContext == NULL)
        {
            pContext =
                new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
        }
        else
        {
            // text field found: white space!
            bIgnoreLeadingSpace = sal_False;
        }
        break;
    }

    return pContext;
}

void XMLParaContext::Characters( const OUString& rChars )
{
    sal_Int32 nLen = rChars.getLength();
    OUStringBuffer sChars( nLen );

    for( sal_Int32 i=0; i < nLen; i++ )
    {
        sal_Unicode c = rChars[ i ];
        switch( c )
        {
            case 0x20:
            case 0x09:
            case 0x0a:
            case 0x0d:
                if( !bIgnoreLeadingSpace )
                    sChars.append( (sal_Unicode)0x20 );
                bIgnoreLeadingSpace = sal_True;
                break;
            default:
                bIgnoreLeadingSpace = sal_False;
                sChars.append( c );
                break;
        }
    }

    rTxtImport.InsertString( sChars.makeStringAndClear() );
}


