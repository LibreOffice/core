/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtparai.cxx,v $
 *
 *  $Revision: 1.66 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:11:05 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef __COMPHELPER_UNOINTERFACETOUNIQUEIDENTIFIERMAPPER__
#include "unointerfacetouniqueidentifiermapper.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _TOOLS_STRINGS_HXX
#include <tools/string.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif


#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_TXTPARAI_HXX
#include "txtparai.hxx"
#endif
#ifndef _XMLOFF_TXTFLDI_HXX
#include "txtfldi.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
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
#include <xmloff/XMLCharContext.hxx>
#ifndef _XMLTEXTFRAMEHYPERLINKCONTEXT_HXX
#include "XMLTextFrameHyperlinkContext.hxx"
#endif
#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include <xmloff/XMLEventsImportContext.hxx>
#endif
#ifndef _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX
#include "XMLChangeImportContext.hxx"
#endif

// OD 2004-04-21 #i26791#
#ifndef _XMLOFF_TXTPARAIMPHINT_HXX
#include <txtparaimphint.hxx>
#endif
typedef XMLHint_Impl *XMLHint_ImplPtr;
SV_DECL_PTRARR_DEL( XMLHints_Impl, XMLHint_ImplPtr, 5, 5 )
SV_IMPL_PTRARR( XMLHints_Impl, XMLHint_ImplPtr )
// OD 2004-04-21 #i26791#
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;


TYPEINIT1( XMLCharContext, SvXMLImportContext );

XMLCharContext::XMLCharContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Unicode c,
        sal_Bool bCount ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
    ,m_nControl(0)
    ,m_nCount(1)
    ,m_c(c)
{
    if( bCount )
    {
        const SvXMLNamespaceMap& rMap = GetImport().GetNamespaceMap();
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );

            OUString aLocalName;
            sal_uInt16 nPrefix =rMap.GetKeyByAttrName( rAttrName,&aLocalName );
            if( XML_NAMESPACE_TEXT == nPrefix &&
                IsXMLToken( aLocalName, XML_C ) )
            {
                sal_Int32 nTmp = xAttrList->getValueByIndex(i).toInt32();
                if( nTmp > 0L )
                {
                    if( nTmp > USHRT_MAX )
                        m_nCount = USHRT_MAX;
                    else
                        m_nCount = (sal_uInt16)nTmp;
                }
            }
        }
    }
}

XMLCharContext::XMLCharContext(
        SvXMLImport& rImp,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > &,
        sal_Int16 nControl ) :
    SvXMLImportContext( rImp, nPrfx, rLName )
    ,m_nControl(nControl)
    ,m_nCount(0)
{
}

XMLCharContext::~XMLCharContext()
{
}
// -----------------------------------------------------------------------------
void XMLCharContext::EndElement()
{
    if ( !m_nCount )
        InsertControlCharacter( m_nControl );
    else
    {
        if( 1U == m_nCount )
        {
            OUString sBuff( &m_c, 1 );
            InsertString(sBuff);
        }
        else
        {
            OUStringBuffer sBuff( m_nCount );
            while( m_nCount-- )
                sBuff.append( &m_c, 1 );

            InsertString(sBuff.makeStringAndClear() );
        }
    }
}
// -----------------------------------------------------------------------------
void XMLCharContext::InsertControlCharacter(sal_Int16   _nControl)
{
    GetImport().GetTextImport()->InsertControlCharacter( _nControl );
}
void XMLCharContext::InsertString(const ::rtl::OUString& _sString)
{
    GetImport().GetTextImport()->InsertString( _sString );
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

    XMLHints_Impl&  rHints;
    XMLStyleHint_Impl   *pHint;

    sal_Bool&       rIgnoreLeadingSpace;

    sal_uInt8               nStarFontsConvFlags;

public:

    TYPEINFO();

    XMLImpSpanContext_Impl(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            XMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace
    ,sal_uInt8              nSFConvFlags
                          );

    virtual ~XMLImpSpanContext_Impl();

    static SvXMLImportContext *CreateChildContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nToken, XMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace
    ,sal_uInt8              nStarFontsConvFlags = 0
             );
    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void Characters( const OUString& rChars );
};
// ---------------------------------------------------------------------

class XMLImpHyperlinkContext_Impl : public SvXMLImportContext
{
    XMLHints_Impl&  rHints;
    XMLHyperlinkHint_Impl   *pHint;

    sal_Bool&       rIgnoreLeadingSpace;

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
    pHint( new XMLHyperlinkHint_Impl(
              GetImport().GetTextImport()->GetCursorAsRange()->getStart() ) ),
    rIgnoreLeadingSpace( rIgnLeadSpace )
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
         IsXMLToken(rLocalName, XML_EVENT_LISTENERS) )
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
    XMLHints_Impl&  rHints;

    sal_Bool&       rIgnoreLeadingSpace;

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
        const Reference< xml::sax::XAttributeList > &,
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
    XMLRubyHint_Impl    *pHint;

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
    XMLHints_Impl&  rHints;
    XMLRubyHint_Impl    *pHint;

    sal_Bool&       rIgnoreLeadingSpace;

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
    pHint( new XMLRubyHint_Impl(
              GetImport().GetTextImport()->GetCursorAsRange()->getStart() ) ),
    rIgnoreLeadingSpace( rIgnLeadSpace )
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
        sAlternativeText(RTL_CONSTASCII_USTRINGPARAM("AlternativeText")),
        rHints(rHnts),
        eToken(eTok)
{
}

void XMLIndexMarkImportContext_Impl::StartElement(
    const Reference<xml::sax::XAttributeList> & xAttrList)
{
    // get Cursor position (needed for all cases)
    Reference<XTextRange> xPos(
        GetImport().GetTextImport()->GetCursor()->getStart());
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
                rPropSet->setPropertyValue(sAlternativeText, uno::makeAny(sValue));
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
        Reference<beans::XPropertySet> xPropSet( xFactory->createInstance(rServiceName), UNO_QUERY );
        if (xPropSet.is())
            rPropSet = xPropSet;
        return sal_True;
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
        if ( SvXMLUnitConverter::convertNumber( nTmp, sValue )
             && nTmp >= 1
             && nTmp < GetImport().GetTextImport()->
                              GetChapterNumbering()->getCount() )
        {
            rPropSet->setPropertyValue(sLevel, uno::makeAny((sal_Int16)(nTmp - 1)));
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
            rPropSet->setPropertyValue(sUserIndexName, uno::makeAny(sValue));
        }
        else if ( IsXMLToken( sLocalName, XML_OUTLINE_LEVEL ) )
        {
            // ouline level: set Level property
            sal_Int32 nTmp;
            if (SvXMLUnitConverter::convertNumber(
                nTmp, sValue, 0,
               GetImport().GetTextImport()->GetChapterNumbering()->getCount()))
            {
                rPropSet->setPropertyValue(sLevel, uno::makeAny(static_cast<sal_Int16>(nTmp - 1)));
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
            rPropSet->setPropertyValue(sPrimaryKey, uno::makeAny(sValue));
        }
        else if ( IsXMLToken( sLocalName, XML_KEY2 ) )
        {
            rPropSet->setPropertyValue(sSecondaryKey, uno::makeAny(sValue));
        }
        else if ( IsXMLToken( sLocalName, XML_KEY1_PHONETIC ) )
        {
            rPropSet->setPropertyValue(sPrimaryKeyReading, uno::makeAny(sValue));
        }
        else if ( IsXMLToken( sLocalName, XML_KEY2_PHONETIC ) )
        {
            rPropSet->setPropertyValue(sSecondaryKeyReading, uno::makeAny(sValue));
        }
        else if ( IsXMLToken( sLocalName, XML_STRING_VALUE_PHONETIC ) )
        {
            rPropSet->setPropertyValue(sTextReading, uno::makeAny(sValue));
        }
        else if ( IsXMLToken( sLocalName, XML_MAIN_ENTRY ) )
        {
            sal_Bool bMainEntry = sal_False, bTmp;

            if (SvXMLUnitConverter::convertBool(bTmp, sValue))
                bMainEntry = bTmp;

            rPropSet->setPropertyValue(sMainEntry, uno::makeAny(bMainEntry));
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
    ,sal_uInt8              nSFConvFlags
                                              )
:   SvXMLImportContext( rImport, nPrfx, rLName )
,   sTextFrame(RTL_CONSTASCII_USTRINGPARAM("TextFrame"))
,   rHints( rHnts )
,   pHint( 0  )
,   rIgnoreLeadingSpace( rIgnLeadSpace )
,   nStarFontsConvFlags( nSFConvFlags & (CONV_FROM_STAR_BATS|CONV_FROM_STAR_MATH) )
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
    ,sal_uInt8              nStarFontsConvFlags
     )
{
    SvXMLImportContext *pContext = 0;

    switch( nToken )
    {
    case XML_TOK_TEXT_SPAN:
        pContext = new XMLImpSpanContext_Impl( rImport, nPrefix,
                                               rLocalName, xAttrList,
                                               rHints,
                                               rIgnoreLeadingSpace
                                               ,nStarFontsConvFlags
                                             );
        break;

    case XML_TOK_TEXT_TAB_STOP:
        pContext = new XMLCharContext( rImport, nPrefix,
                                               rLocalName, xAttrList,
                                               0x0009, sal_False );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_LINE_BREAK:
        pContext = new XMLCharContext( rImport, nPrefix,
                                               rLocalName, xAttrList,
                                               ControlCharacter::LINE_BREAK );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_TEXT_S:
        pContext = new XMLCharContext( rImport, nPrefix,
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

    case XML_TOK_TEXT_NOTE:
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

    case XML_TOK_TEXT_FRAME:
        {
            Reference < XTextRange > xAnchorPos =
                rImport.GetTextImport()->GetCursor()->getStart();
            XMLTextFrameContext *pTextFrameContext =
                new XMLTextFrameContext( rImport, nPrefix,
                                         rLocalName, xAttrList,
                                         TextContentAnchorType_AS_CHARACTER );
            // --> OD 2004-08-24 #i33242# - remove check for text content.
            // Check for text content is done on the processing of the hint
            if( TextContentAnchorType_AT_CHARACTER ==
                                            pTextFrameContext->GetAnchorType() )
            // <--
            {
                rHints.Insert( new XMLTextFrameHint_Impl(
                    pTextFrameContext, xAnchorPos ),
                    rHints.Count() );
            }
            pContext = pTextFrameContext;
            rIgnoreLeadingSpace = sal_False;
        }
        break;
    case XML_TOK_DRAW_A:
        {
            Reference < XTextRange > xAnchorPos(rImport.GetTextImport()->GetCursor()->getStart());
            pContext =
                new XMLTextFrameHyperlinkContext( rImport, nPrefix,
                                        rLocalName, xAttrList,
                                        TextContentAnchorType_AS_CHARACTER );
            XMLTextFrameHint_Impl *pHint =
                new XMLTextFrameHint_Impl( pContext, xAnchorPos);
            rHints.Insert( pHint, rHints.Count() );
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
            SvXMLShapeContext* pShapeContext = rImport.GetShapeImport()->CreateGroupChildContext(
                rImport, nPrefix, rLocalName, xAttrList, xShapes );
            pContext = pShapeContext;
            // OD 2004-04-20 #i26791# - keep shape in a text frame hint to
            // adjust its anchor position, if its at-character anchored
            Reference < XTextRange > xAnchorPos =
                rImport.GetTextImport()->GetCursor()->getStart();
            rHints.Insert( new XMLDrawHint_Impl( pShapeContext, xAnchorPos ),
                           rHints.Count() );
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
                               ,nStarFontsConvFlags
                             );
}

void XMLImpSpanContext_Impl::Characters( const OUString& rChars )
{
    OUString sStyleName;
    if( pHint )
        sStyleName = pHint->GetStyleName();
    OUString sChars =
        GetImport().GetTextImport()->ConvertStarFonts( rChars, sStyleName,
                                                       nStarFontsConvFlags,
                                                       sal_False, GetImport() );
    GetImport().GetTextImport()->InsertString( sChars, rIgnoreLeadingSpace );
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
    nOutlineLevel( IsXMLToken( rLName, XML_H ) ? 1 : -1 ),
    pHints( 0 ),
    // --> OD 2007-07-25 #i73509#
    mbOutlineLevelAttrFound( sal_False ),
    // <--
    bIgnoreLeadingSpace( sal_True ),
    bHeading( bHead ),
    bIsListHeader( false ),
    bIsRestart (false),
    nStartValue(0)
    ,nStarFontsConvFlags( 0 )
{
    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextPAttrTokenMap();

    OUString aCondStyleName, sClassNames;

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
        case XML_TOK_TEXT_P_CLASS_NAMES:
            sClassNames = rValue;
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
                // --> OD 2007-07-25 #i73509#
                mbOutlineLevelAttrFound = sal_True;
                // <--
            }
            break;
        case XML_TOK_TEXT_P_IS_LIST_HEADER:
            {
                sal_Bool bBool;
                if( SvXMLUnitConverter::convertBool( bBool, rValue ) )
                {
                    bIsListHeader = bBool;
                }
            }
            break;
        case XML_TOK_TEXT_P_ID:
            sId = rValue;
            break;
        case XML_TOK_TEXT_P_RESTART_NUMBERING:
            {
                sal_Bool bBool;
                if (SvXMLUnitConverter::convertBool(bBool, rValue))
                {
                    bIsRestart = bBool;
                }
            }
            break;
        case XML_TOK_TEXT_P_START_VALUE:
            {
                nStartValue = sal::static_int_cast< sal_Int16 >(
                    rValue.toInt32());
            }
            break;
        }
    }

    if( aCondStyleName.getLength() )
        sStyleName = aCondStyleName;
    else if( sClassNames.getLength() )
    {
        sal_Int32 nDummy = 0;
        sStyleName = sClassNames.getToken( 0, ' ', nDummy );
    }
}

XMLParaContext::~XMLParaContext()
{
    UniReference < XMLTextImportHelper > xTxtImport(
        GetImport().GetTextImport());
    Reference < XTextRange > xCrsrRange( xTxtImport->GetCursorAsRange() );
    if( !xCrsrRange.is() )
        return; // Robust (defect file)
    Reference < XTextRange > xEnd(xCrsrRange->getStart());

    // if we have an id set for this paragraph, get a cursor for this
    // paragraph and register it with the given identifier
    if( sId.getLength() )
    {
        Reference < XTextCursor > xIdCursor( xTxtImport->GetText()->createTextCursorByRange( xStart ) );
        if( xIdCursor.is() )
        {
            xIdCursor->gotoRange( xEnd, sal_True );
            Reference< XInterface > xRef( xIdCursor, UNO_QUERY );
            GetImport().getInterfaceToIdentifierMapper().registerReference( sId, xRef );
        }
    }

    // insert a paragraph break
    xTxtImport->InsertControlCharacter( ControlCharacter::APPEND_PARAGRAPH );

    // create a cursor that select the whole last paragraph
    Reference < XTextCursor > xAttrCursor(
        xTxtImport->GetText()->createTextCursorByRange( xStart ));
    if( !xAttrCursor.is() )
        return; // Robust (defect file)
    xAttrCursor->gotoRange( xEnd, sal_True );

    OUString sCellParaStyleName = xTxtImport->sCellParaStyleDefault;
    if( sCellParaStyleName.getLength() > 0 )
    {
        // --> OD 2007-08-16 #i80724#
        // suppress handling of outline and list attributes,
        // because of side effects of method <SetStyleAndAttrs(..)>
        xTxtImport->SetStyleAndAttrs( GetImport(), xAttrCursor,
                                      sCellParaStyleName,
                                      sal_True,
                                      sal_False, -1, // suppress outline handling
                                      sal_False );   // suppress list attributes handling
        // <--
    }

    // #103445# for headings without style name, find the proper style
    if( bHeading && (sStyleName.getLength() == 0) )
        xTxtImport->FindOutlineStyleName( sStyleName, nOutlineLevel );

    // set style and hard attributes at the previous paragraph
    // --> OD 2007-07-25 #i73509# - add paramter <mbOutlineLevelAttrFound>
    sStyleName = xTxtImport->SetStyleAndAttrs( GetImport(), xAttrCursor,
                                               sStyleName,
                                               sal_True,
                                               mbOutlineLevelAttrFound,
                                               bHeading ? nOutlineLevel : -1 );
    // <--

    // handle list style header
    if (bHeading && (bIsListHeader || bIsRestart))
    {
        Reference<XPropertySet> xPropSet( xAttrCursor, UNO_QUERY );

        if (xPropSet.is())
        {
            if (bIsListHeader)
            {
                OUString sNumberingIsNumber
                    (RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber"));
                if(xPropSet->getPropertySetInfo()->
                   hasPropertyByName(sNumberingIsNumber))
                {
                    xPropSet->setPropertyValue
                        (sNumberingIsNumber, makeAny( false ) );
                }
            }
            if (bIsRestart)
            {
                OUString sParaIsNumberingRestart
                    (RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart"));
                OUString sNumberingStartValue
                    (RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue"));
                if (xPropSet->getPropertySetInfo()->
                    hasPropertyByName(sParaIsNumberingRestart))
                {
                    xPropSet->setPropertyValue
                        (sParaIsNumberingRestart, makeAny(true));
                }

                if (xPropSet->getPropertySetInfo()->
                    hasPropertyByName(sNumberingStartValue))
                {
                    xPropSet->setPropertyValue
                        (sNumberingStartValue, makeAny(nStartValue));
                }
            }

        }
    }

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
                        xTxtImport->SetStyleAndAttrs( GetImport(),
                                                      xAttrCursor, rStyleName,
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
                    xTxtImport->SetHyperlink( GetImport(),
                                              xAttrCursor,
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
                    xTxtImport->SetRuby( GetImport(),
                                         xAttrCursor,
                                         pRHint->GetStyleName(),
                                         pRHint->GetTextStyleName(),
                                         pRHint->GetText() );
                }
                break;
            case XML_HINT_INDEX_MARK:
                {
                    Reference<beans::XPropertySet> xMark(
                        ((const XMLIndexMarkHint_Impl *)pHint)->GetMark());
                    Reference<XTextContent> xContent(xMark, UNO_QUERY);
                    Reference<XTextRange> xRange(xAttrCursor, UNO_QUERY);
                    xTxtImport->GetText()->insertTextContent(
                        xRange, xContent, sal_True );
                }
                break;
            case XML_HINT_TEXT_FRAME:
                {
                    const XMLTextFrameHint_Impl *pFHint =
                        (const XMLTextFrameHint_Impl *)pHint;
                    // --> OD 2004-08-24 #i33242# - check for text content
                    Reference < XTextContent > xTextContent =
                                                    pFHint->GetTextContent();
                    if ( xTextContent.is() )
                    {
                        // OD 2004-04-20 #i26791#
                        Reference<XTextRange> xRange(xAttrCursor, UNO_QUERY);
                        if ( pFHint->IsBoundAtChar() )
                        {
                            xTextContent->attach( xRange );
                        }
                    }
                    // <--
                    // --> OD 2004-08-24 #i33242# - consider, that hint can
                    // also contain a shape - e.g. drawing object of type 'Text'.
                    else
                    {
                        Reference < XShape > xShape = pFHint->GetShape();
                        if ( xShape.is() )
                        {
                            // determine anchor type
                            Reference < XPropertySet > xPropSet( xShape, UNO_QUERY );
                            TextContentAnchorType eAnchorType =
                                            TextContentAnchorType_AT_PARAGRAPH;
                            {
                                OUString sAnchorType( RTL_CONSTASCII_USTRINGPARAM( "AnchorType" ) );
                                Any aAny = xPropSet->getPropertyValue( sAnchorType );
                                aAny >>= eAnchorType;
                            }
                            if ( TextContentAnchorType_AT_CHARACTER == eAnchorType )
                            {
                                // set anchor position for at-character anchored objects
                                Reference<XTextRange> xRange(xAttrCursor, UNO_QUERY);
                                Any aPos;
                                aPos <<= xRange;
                                OUString sTextRange( RTL_CONSTASCII_USTRINGPARAM( "TextRange" ) );
                                xPropSet->setPropertyValue(sTextRange, aPos);
                            }
                        }
                    }
                    // <--
                }
                break;
            // --> DVO, OD 2004-07-14 #i26791#
            case XML_HINT_DRAW:
                {
                    const XMLDrawHint_Impl *pDHint =
                        static_cast<const XMLDrawHint_Impl*>(pHint);
                    // --> OD 2004-08-24 #i33242# - improvement: hint directly
                    // provides the shape.
                    Reference < XShape > xShape = pDHint->GetShape();
                    if ( xShape.is() )
                    {
                        // determine anchor type
                        Reference < XPropertySet > xPropSet( xShape, UNO_QUERY );
                        TextContentAnchorType eAnchorType = TextContentAnchorType_AT_PARAGRAPH;
                        {
                            OUString sAnchorType( RTL_CONSTASCII_USTRINGPARAM( "AnchorType" ) );
                            Any aAny = xPropSet->getPropertyValue( sAnchorType );
                            aAny >>= eAnchorType;
                        }
                        if ( TextContentAnchorType_AT_CHARACTER == eAnchorType )
                        {
                            // set anchor position for at-character anchored objects
                            Reference<XTextRange> xRange(xAttrCursor, UNO_QUERY);
                            Any aPos;
                            aPos <<= xRange;
                            OUString sTextRange( RTL_CONSTASCII_USTRINGPARAM( "TextRange" ) );
                            xPropSet->setPropertyValue(sTextRange, aPos);
                        }
                    }
                    // <--
                }
                break;
            // <--

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
    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextPElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    if( !pHints )
        pHints = new XMLHints_Impl;
    return XMLImpSpanContext_Impl::CreateChildContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                   nToken, *pHints, bIgnoreLeadingSpace
                                , nStarFontsConvFlags
                                                     );
}

void XMLParaContext::Characters( const OUString& rChars )
{
    OUString sChars =
        GetImport().GetTextImport()->ConvertStarFonts( rChars, sStyleName,
                                                       nStarFontsConvFlags,
                                                       sal_True, GetImport() );
    GetImport().GetTextImport()->InsertString( sChars, bIgnoreLeadingSpace );
}



TYPEINIT1( XMLNumberedParaContext, SvXMLImportContext );

XMLNumberedParaContext::XMLNumberedParaContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName ) :
     SvXMLImportContext( rImport, nPrfx, rLName )
 {
 }

 XMLNumberedParaContext::~XMLNumberedParaContext()
 {
 }

 SvXMLImportContext *XMLNumberedParaContext::CreateChildContext(
         sal_uInt16 nPrefix, const OUString& rLocalName,
         const Reference< xml::sax::XAttributeList > & xAttrList )
 {
    //FIXME #i52127#: currently <text:numbered-paragraph> is not implemented!
     SvXMLImportContext *pContext = GetImport().GetTextImport()->CreateTextChildContext(
             GetImport(), nPrefix, rLocalName, xAttrList, XML_TEXT_TYPE_BODY );
     return pContext;
 }

 void XMLNumberedParaContext::Characters( const OUString& )
 {
 }

