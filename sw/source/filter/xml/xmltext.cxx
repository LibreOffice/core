/*************************************************************************
 *
 *  $RCSfile: xmltext.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:15:00 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_I18NMAP_HXX
#include <xmloff/i18nmap.hxx>
#endif

#ifndef _XMLOFF_XMLITEM_HXX
#include <xmloff/xmlitem.hxx>
#endif

#ifndef _XMLOFF_XMLIMPIT_HXX
#include <xmloff/xmlimpit.hxx>
#endif

#ifndef _XMLOFF_XMLITMPR_HXX
#include <xmloff/xmlexpit.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _XMLOFF_XMLASTPL_HXX
#include <xmloff/xmlastpl.hxx>
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include <xmloff/txtimp.hxx>
#endif

#ifndef _XMLOFF_TXTFLDE_HXX
#include <xmloff/txtflde.hxx>
#endif

#ifndef _XMLOFF_TXTVFLDI_HXX
//#include <xmloff/txtvfldi.hxx>
#endif

#ifndef _UNOFIELD_HXX
#include <unofield.hxx>
#endif

#ifndef _SVX_FONTITEM_HXX //autogen wg. SvxFontItem
#include <svx/fontitem.hxx>
#endif

#ifndef _NDTXT_HXX //autogen wg. SwTxtNode
#include <ndtxt.hxx>
#endif
#ifndef _SECTION_HXX //autogen wg. SwSection
#include <section.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include "unostyle.hxx"
#endif

#ifndef _XMLECTXT_HXX
#include "xmlectxt.hxx"
#endif
#ifndef _HINTLIST_HXX
#include "hintlist.hxx"
#endif
#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLNUM_HXX
#include "xmlnum.hxx"
#endif
#ifndef _XMLTBLI_HXX
#include "xmltbli.hxx"
#endif
#ifndef _XMLFMTE_HXX
#include "xmlfmte.hxx"
#endif
#ifndef _XMLTEXTI_HXX
#include "xmltexti.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

#ifdef XML_CORE_API
enum SwXMLTextPElemTokens
{
    XML_TOK_SW_SPAN,
    XML_TOK_SW_TAB_STOP,
    XML_TOK_SW_LINE_BREAK,
    XML_TOK_SW_S,
    XML_TOK_SW_P_ELEM_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aTextPElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_span, XML_TOK_SW_SPAN },
    { XML_NAMESPACE_TEXT, sXML_tab_stop, XML_TOK_SW_TAB_STOP },
    { XML_NAMESPACE_TEXT, sXML_line_break, XML_TOK_SW_LINE_BREAK },
    { XML_NAMESPACE_TEXT, sXML_s, XML_TOK_SW_S },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& SwXMLImport::GetTextPElemTokenMap()
{
    if( !pTextPElemTokenMap )
        pTextPElemTokenMap = new SvXMLTokenMap( aTextPElemTokenMap );

    return *pTextPElemTokenMap;
}

// ---------------------------------------------------------------------

class SwXMLHint_Impl
{
    SfxItemSet aItemSet;
    xub_StrLen nStart;
    xub_StrLen nEnd;

public:

    SwXMLHint_Impl( SfxItemPool& rPool, xub_StrLen nPos ) :
        aItemSet( rPool, RES_CHRATR_BEGIN,      RES_CHRATR_END - 1,
                            RES_TXTATR_BEGIN,       RES_TXTATR_END - 1,
                         RES_UNKNOWNATR_BEGIN,  RES_UNKNOWNATR_END - 1,  0 ),
        nStart( nPos ),
        nEnd( nPos )
    {}

    SfxItemSet& GetItemSet() { return aItemSet; }
    xub_StrLen GetStart() const { return nStart; }
    xub_StrLen GetEnd() const { return nEnd; }
    void SetEnd( xub_StrLen nPos ) { nEnd = nPos; }
};

typedef SwXMLHint_Impl *SwXMLHint_ImplPtr;
SV_DECL_PTRARR_DEL( SwXMLHints_Impl, SwXMLHint_ImplPtr, 5, 5 )
SV_IMPL_PTRARR( SwXMLHints_Impl, SwXMLHint_ImplPtr )

// ---------------------------------------------------------------------

class SwXMLImpCharContext_Impl : public SvXMLImportContext
{
public:

    SwXMLImpCharContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Unicode c,
            sal_Bool bCount );

    virtual ~SwXMLImpCharContext_Impl();

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
};

SwXMLImpCharContext_Impl::SwXMLImpCharContext_Impl(
        SwXMLImport& rImport,
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
        GetSwImport().GetDoc().Insert( GetSwImport().GetPaM(), c );
    }
    else
    {
        OUStringBuffer sBuff( nCount );
        while( nCount-- )
            sBuff.append( c );

        GetSwImport().GetDoc().Insert( GetSwImport().GetPaM(),
                                       sBuff.makeStringAndClear() );
    }
}

SwXMLImpCharContext_Impl::~SwXMLImpCharContext_Impl()
{
}

// ---------------------------------------------------------------------

class SwXMLImpSpanContext_Impl : public SvXMLImportContext
{
    SwXMLHints_Impl&    rHints;
    SwXMLHint_Impl      *pHint;

    sal_Bool&           rIgnoreLeadingSpace;

public:

    SwXMLImpSpanContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLHints_Impl& rHnts,
            sal_Bool& rIgnLeadSpace );

    virtual ~SwXMLImpSpanContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void Characters( const OUString& rChars );

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
};

SwXMLImpSpanContext_Impl::SwXMLImpSpanContext_Impl(
        SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SwXMLHints_Impl& rHnts,
        sal_Bool& rIgnLeadSpace ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
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
        SwDoc& rDoc = GetSwImport().GetDoc();

        // try to find a matching automatic style
        const SfxItemSet *pAutoItemSet = 0;
        OUString sParentStyle;
        if( GetSwImport().FindAutomaticStyle( SFX_STYLE_FAMILY_CHAR,
                                              aStyleName, &pAutoItemSet,
                                              &sParentStyle ) )
        {
            aStyleName = sParentStyle;
        }

        // if an automatic style has not been found or has been found and
        // has a parent style, try to find this one.
        SwCharFmt *pCharFmt = 0;
        if( aStyleName.getLength() )
        {
            const SvI18NMap& rI18NMap = GetSwImport().GetI18NMap();
            String sName( rI18NMap.Get( SFX_STYLE_FAMILY_CHAR,
                                             aStyleName) );
            pCharFmt = rDoc.FindCharFmtByName( sName );
        }

        if( pAutoItemSet || pCharFmt )
        {
            xub_StrLen nPos =
                GetSwImport().GetPaM().GetPoint()->nContent.GetIndex();
            pHint = new SwXMLHint_Impl( rDoc.GetAttrPool(), nPos );
            rHints.Insert( pHint, rHints.Count() );
            if( pAutoItemSet )
                pHint->GetItemSet().Put( *pAutoItemSet );
            if( pCharFmt )
            {
                SwFmtCharFmt aCharFmt( pCharFmt );
                pHint->GetItemSet().Put( aCharFmt );
            }
        }
    }
}

SwXMLImpSpanContext_Impl::~SwXMLImpSpanContext_Impl()
{
    if( pHint )
    {
        xub_StrLen nPos =
            GetSwImport().GetPaM().GetPoint()->nContent.GetIndex();
        pHint->SetEnd( nPos );
    }
}

SvXMLImportContext *SwXMLImpSpanContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetSwImport().GetTextPElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_SW_SPAN:
        pContext = new SwXMLImpSpanContext_Impl( GetSwImport(), nPrefix,
                                                 rLocalName, xAttrList,
                                                 rHints,
                                                 rIgnoreLeadingSpace );
        break;

    case XML_TOK_SW_TAB_STOP:
        pContext = new SwXMLImpCharContext_Impl( GetSwImport(), nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x0009, sal_False );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_SW_LINE_BREAK:
        pContext = new SwXMLImpCharContext_Impl( GetSwImport(), nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x000A, sal_False );
        rIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_SW_S:
        pContext = new SwXMLImpCharContext_Impl( GetSwImport(), nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x0020, sal_True );
        break;

    default:
        // ignore unknown content
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
        break;
    }

    return pContext;
}

void SwXMLImpSpanContext_Impl::Characters( const OUString& rChars )
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

    GetSwImport().GetDoc().Insert( GetSwImport().GetPaM(),
                                   sChars.makeStringAndClear() );
}

// ---------------------------------------------------------------------

enum SwXMLTextPAttrTokens
{
    XML_TOK_SW_P_STYLE_NAME,
    XML_TOK_SW_P_COND_STYLE_NAME,
    XML_TOK_SW_P_LEVEL,
    XML_TOK_SW_P_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aTextPAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_style_name,  XML_TOK_SW_P_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_cond_style_name,
                                            XML_TOK_SW_P_COND_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_level,       XML_TOK_SW_P_LEVEL },
    XML_TOKEN_MAP_END
};

SwXMLParaContext::SwXMLParaContext(
        SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bHead ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pItemSet( 0 ),
    pHints( 0 ),
    bIgnoreLeadingSpace( sal_True ),
    bHeading( bHead )
{
    const SvXMLTokenMap& rTokenMap = GetSwImport().GetTextPAttrTokenMap();

    OUString aStyleName, aCondStyleName;
    sal_uInt8 nOutlineLevel = NO_NUMBERING;

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
        case XML_TOK_SW_P_STYLE_NAME:
            aStyleName = rValue;
            break;
        case XML_TOK_SW_P_COND_STYLE_NAME:
            aCondStyleName = rValue;
            break;
        case XML_TOK_SW_P_LEVEL:
            {
                sal_Int32 nTmp = rValue.toInt32() - 1L;
                if( nTmp >= 0L )
                {
                    if( nTmp > MAXLEVEL )
                        nTmp = MAXLEVEL;
                    nOutlineLevel = (sal_uInt8)nTmp;
                }
            }

        }
    }

    if( aCondStyleName.getLength() )
    {
        OUString sTmp( aStyleName );
        aStyleName = aCondStyleName;
        aCondStyleName = sTmp;
    }

    if( aStyleName.getLength() )
    {
        const SfxItemSet *pAutoItemSet = 0;
        OUString sParentStyle;
        if( GetSwImport().FindAutomaticStyle( SFX_STYLE_FAMILY_PARA,
                                              aStyleName, &pAutoItemSet,
                                                 &sParentStyle ) )
        {
            aStyleName = sParentStyle;
        }

        if( pAutoItemSet )
        {
            if( !pItemSet )
                pItemSet = new SfxItemSet( GetSwImport().GetDoc().GetAttrPool(),
                                           aTxtNodeSetRange );
            pItemSet->Put( *pAutoItemSet );
        }

        if( aStyleName.getLength() )
        {
            const SvI18NMap& rI18NMap = GetSwImport().GetI18NMap();
            String sName( rI18NMap.Get( SFX_STYLE_FAMILY_PARA, aStyleName ) );
            SwDoc& rDoc = GetSwImport().GetDoc();
            SwTxtFmtColl *pColl = rDoc.FindTxtFmtCollByName( sName );
            if( pColl )
            {
                sal_uInt8 nCollOutlineLevel = pColl->GetOutlineLevel();
                if( bHeading && !( GetSwImport().IsInsertMode() ||
                                      GetSwImport().IsStylesOnlyMode() ) &&
                    NO_NUMBERING != nOutlineLevel &&
                    ( NO_NUMBERING == nCollOutlineLevel ||
                      GetRealLevel(nCollOutlineLevel) != nOutlineLevel ) )
                {
                    sal_uInt16 nArrLen = rDoc.GetTxtFmtColls()->Count();
                    for( sal_uInt16 i=0; i<nArrLen; i++ )
                    {
                        SwTxtFmtColl* pCur = (*rDoc.GetTxtFmtColls())[i];
                        sal_uInt8 nCurLevel = pCur->GetOutlineLevel();
                        if( nCurLevel != NO_NUMBERING &&
                            GetRealLevel(nCurLevel) == nOutlineLevel )
                        {
                            if( pColl != pCur )
                                pCur->SetOutlineLevel( NO_NUMBERING );
                            break;
                        }
                    }
                    pColl->SetOutlineLevel( nOutlineLevel );
                }
                rDoc.SetTxtFmtColl( GetSwImport().GetPaM(), pColl );
            }
        }
    }
}

SwXMLParaContext::~SwXMLParaContext()
{
#ifndef PRODUCT
    SwCntntNode *pTTTNd = GetSwImport().GetPaM().GetNode()->GetCntntNode();
#endif

    // Start a new node.
    SwDoc& rDoc = GetSwImport().GetDoc();
    rDoc.AppendTxtNode( *GetSwImport().GetPaM().GetPoint() );

    // Create a "attribute" PaM and move it to the previous node.
    SwPaM aPaM( GetSwImport().GetPaM() );
    aPaM.Move( fnMoveBackward, fnGoCntnt );
    aPaM.SetMark();
    SwCntntNode *pCNd = aPaM.GetNode()->GetCntntNode();
    ASSERT( pCNd,
       "SwXMLParaContext::~SwXMLParaContext: no content node" );
#ifndef PRODUCT
    ASSERT( pTTTNd == pCNd,
       "SwXMLParaContext::~SwXMLParaContext: wrong content node" );
    ASSERT( (const SwIndexReg*)pTTTNd == aPaM.GetPoint()->nContent.GetIdxReg(),
       "SwXMLParaContext::~SwXMLParaContext: wrong content node" );
#endif

    aPaM.GetPoint()->nContent.Assign( pCNd, 0 );

    // Set paragraph attributes.
    if( pItemSet )
        rDoc.Insert( aPaM, *pItemSet );
    delete pItemSet;

    // Set numbering rules
    SwTxtNode *pTxtNd = pCNd->GetTxtNode();
    const SwNumRule *pNumRule = pTxtNd->GetNumRule();
    if( GetSwImport().IsInList() )
    {
        SwXMLListBlockContext *pListBlock = GetSwImport().GetListBlock();
        String sStyleName( GetSwImport().GetI18NMap().Get(
                        SFX_STYLE_FAMILY_PSEUDO,pListBlock->GetStyleName()) );
        if( !pNumRule ||
            ( !pListBlock->HasGeneratedStyle() &&
              pNumRule->GetName() != sStyleName ) )
        {
            rDoc.Insert( aPaM, SwNumRuleItem( sStyleName ) );
            pNumRule = pTxtNd->GetNumRule();
        }

        ASSERT( pNumRule, "SwXMLParaContext::~SwXMLParaContext: no num rule" );
        GetSwImport().SetUsed( *pNumRule );

        SwXMLListItemContext *pListItem = GetSwImport().GetListItem();

        sal_uInt8 nLevel = pListBlock->GetLevel() > MAXLEVEL
                            ? MAXLEVEL
                            : (sal_uInt8)pListBlock->GetLevel();
        if( !pListItem )
            nLevel |= NO_NUMLEVEL;

        SwNodeNum aNodeNum( nLevel );
        if( pListBlock->IsRestartNumbering() )
        {
            aNodeNum.SetStart();
            pListBlock->ResetRestartNumbering();
        }
        if( pListItem && pListItem->HasStartValue() )
        {
            aNodeNum.SetSetValue( pListItem->GetStartValue() );
        }

        pTxtNd->UpdateNum( aNodeNum );

        GetSwImport().SetListItem( 0 );
    }
    else
    {
        // If the paragraph is not in a list but its style, remove it from
        // the list.
        if( pNumRule )
            rDoc.Insert( aPaM, SwNumRuleItem() );
    }

    if( pHints && pHints->Count() )
    {
        for( sal_uInt16 i=0; i<pHints->Count(); i++ )
        {
            SwXMLHint_Impl *pHint = (*pHints)[i];
            xub_StrLen nStt = pHint->GetStart();
            xub_StrLen nEnd = pHint->GetEnd();
            if( nStt != nEnd )
            {
                aPaM.GetPoint()->nContent.Assign( pCNd, nEnd );
                aPaM.SetMark();
                aPaM.GetPoint()->nContent.Assign( pCNd, nStt );
                rDoc.Insert( aPaM, pHint->GetItemSet() );
            }
        }
    }
    delete pHints;
}

SvXMLImportContext *SwXMLParaContext::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetSwImport().GetTextPElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_SW_SPAN:
        if( !pHints )
            pHints = new SwXMLHints_Impl;
        pContext = new SwXMLImpSpanContext_Impl( GetSwImport(), nPrefix,
                                                 rLocalName, xAttrList,
                                                 *pHints, bIgnoreLeadingSpace );
        break;

    case XML_TOK_SW_TAB_STOP:
        pContext = new SwXMLImpCharContext_Impl( GetSwImport(), nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x0009, sal_False );
        bIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_SW_LINE_BREAK:
        pContext = new SwXMLImpCharContext_Impl( GetSwImport(), nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x000A, sal_False );
        bIgnoreLeadingSpace = sal_False;
        break;

    case XML_TOK_SW_S:
        pContext = new SwXMLImpCharContext_Impl( GetSwImport(), nPrefix,
                                                 rLocalName, xAttrList,
                                                 0x0020, sal_True );
        break;

    default:
        // ignore unknown content
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
        break;
    }

    return pContext;
}

void SwXMLParaContext::Characters( const OUString& rChars )
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

    GetSwImport().GetDoc().Insert( GetSwImport().GetPaM(),
                                   sChars.makeStringAndClear() );
}

const SvXMLTokenMap& SwXMLImport::GetTextPAttrTokenMap()
{
    if( !pTextPAttrTokenMap )
        pTextPAttrTokenMap = new SvXMLTokenMap( aTextPAttrTokenMap );

    return *pTextPAttrTokenMap;
}
#endif

// ---------------------------------------------------------------------

#ifdef XML_CORE_API
static __FAR_DATA SvXMLTokenMapEntry aBodyElemTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_p,               XML_TOK_SW_P                },
    { XML_NAMESPACE_TEXT, sXML_h,               XML_TOK_SW_H                },
    { XML_NAMESPACE_TEXT, sXML_ordered_list,    XML_TOK_SW_ORDERED_LIST },
    { XML_NAMESPACE_TEXT, sXML_unordered_list,  XML_TOK_SW_UNORDERED_LIST },
    { XML_NAMESPACE_TABLE,sXML_table,           XML_TOK_TABLE_TABLE         },
    { XML_NAMESPACE_TABLE,sXML_sub_table,       XML_TOK_TABLE_SUBTABLE      },
    XML_TOKEN_MAP_END
};
#endif

class SwXMLBodyContext_Impl : public SvXMLImportContext
{
#ifdef XML_CORE_API
    const SwXMLImport& GetSwImport() const
            { return (const SwXMLImport&)GetImport(); }
#endif
    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }

public:

    SwXMLBodyContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                             const OUString& rLName );
    virtual ~SwXMLBodyContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
};

SwXMLBodyContext_Impl::SwXMLBodyContext_Impl( SwXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SwXMLBodyContext_Impl::~SwXMLBodyContext_Impl()
{
}

SvXMLImportContext *SwXMLBodyContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

#ifndef XML_CORE_API
    pContext = GetSwImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList,
               XML_TEXT_TYPE_BODY );
#else
    const SvXMLTokenMap& rTokenMap = GetSwImport().GetBodyElemTokenMap();
    sal_Bool bOrdered = sal_False;
    sal_Bool bHeading = sal_False;

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_SW_H:
        bHeading = sal_True;
    case XML_TOK_SW_P:
        pContext = new SwXMLParaContext( GetSwImport(),nPrefix, rLocalName,
                                         xAttrList, bHeading );
        break;
    case XML_TOK_SW_ORDERED_LIST:
        bOrdered = sal_True;
    case XML_TOK_SW_UNORDERED_LIST:
        pContext = new SwXMLListBlockContext( GetSwImport(),nPrefix, rLocalName,
                                              xAttrList, bOrdered );
        break;
    case XML_TOK_TABLE_TABLE:
        if( !GetSwImport().GetPaM().GetNode()->FindTableNode() )
            pContext = new SwXMLTableContext( GetSwImport(),nPrefix, rLocalName,
                                              xAttrList );
        break;
    }
#endif

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

#ifdef XML_CORE_API
const SvXMLTokenMap& SwXMLImport::GetBodyElemTokenMap()
{
    if( !pBodyElemTokenMap )
        pBodyElemTokenMap = new SvXMLTokenMap( aBodyElemTokenMap );

    return *pBodyElemTokenMap;
}
#endif

SvXMLImportContext *SwXMLImport::CreateBodyContext(
                                       const OUString& rLocalName )
{
    // styles nun einfuegen!
    InsertStyles();

    SvXMLImportContext *pContext = 0;

    if( !IsStylesOnlyMode() )
         pContext = new SwXMLBodyContext_Impl( *this, XML_NAMESPACE_OFFICE,
                                              rLocalName );
    else
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
                                           rLocalName );

    return pContext;
}

// ---------------------------------------------------------------------

#ifdef XML_CORE_API
class SwXMLHintEndPosList : public SwHintEndPosList
{
    OUString sQSpan;
    SwXMLExport& rExport;

public:

    SwXMLHintEndPosList( SwXMLExport& rExp );
    ~SwXMLHintEndPosList();

    void Insert( const SfxPoolItem& rItem, xub_StrLen nStart, xub_StrLen nEnd );

    sal_Bool HasStartingOrEndingHints( xub_StrLen nPos );

    sal_Bool ExportStartingHints( xub_StrLen nPos );
    sal_Bool ExportEndingHints( xub_StrLen nPos );
};

SwXMLHintEndPosList::SwXMLHintEndPosList( SwXMLExport& rExp ) :
    rExport( rExp )
{
    OUString sLName( OUString::createFromAscii( sXML_span ) );
    sQSpan = rExp.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TEXT, sLName );
}

SwXMLHintEndPosList::~SwXMLHintEndPosList()
{
}

void SwXMLHintEndPosList::Insert( const SfxPoolItem& rItem,
                                  xub_StrLen nStart, xub_StrLen nEnd )
{
    // empty hints will be ignored
    if( nStart != nEnd )
    {
        InsertHint( rItem, nStart, nEnd );
    }
}

sal_Bool SwXMLHintEndPosList::ExportStartingHints( xub_StrLen nPos )
{
    sal_Bool bRet = sal_False;

    // Hints in the start list are sorted ascending.

    sal_uInt16 nCount = GetStartingCount();
    for( sal_uInt16 i=0; i< nCount; i++ )
    {
        SwHintSttEndPos *pPos = GetStartingHint( i );
        xub_StrLen nStart = pPos->GetStart();
        if( nStart > nPos )
        {
            // This and all following hint will be started later
            break;
        }
        else if( nStart == nPos )
        {
            // export hint's start
//          if( bIWSOutside )
//              rExport.GetDocHandler()->ignorableWhitespace( rExport.sWS );
            const SfxPoolItem& rItem = pPos->GetItem();
            if( (rItem.Which() >= RES_CHRATR_BEGIN &&
                                        rItem.Which() < RES_CHRATR_END) ||
                (rItem.Which() >= RES_UNKNOWNATR_BEGIN &&
                                        rItem.Which() < RES_UNKNOWNATR_END) )
            {
                SfxItemSet aItemSet( rExport.GetDoc().GetAttrPool(),
                                 RES_CHRATR_BEGIN,      RES_CHRATR_END - 1,
                                 RES_UNKNOWNATR_BEGIN,  RES_UNKNOWNATR_END - 1,
                                 0 );
                aItemSet.Put( rItem );
                OUString sName( rExport.GetItemSetAutoStylePool().Find(
                                    SFX_STYLE_FAMILY_CHAR, aItemSet ) );
                ASSERT( sName.getLength(), "no automatic style found" );
                if( sName.getLength() )
                    rExport.AddAttribute( XML_NAMESPACE_TEXT, sXML_style_name,
                                          sName );
            }
            else
            {
                rExport.GetParaItemMapper().exportXML(
                            rExport.GetAttrList(), rItem,
                            rExport.GetTwipUnitConverter(),
                            rExport.GetNamespaceMap() );
            }

            rExport.GetDocHandler()->startElement( sQSpan,
                                                   rExport.GetXAttrList() );
            rExport.ClearAttrList();

            bRet = sal_True;
        }
    }

    return bRet;
}

sal_Bool SwXMLHintEndPosList::ExportEndingHints( xub_StrLen nPos )
{
    sal_Bool bRet = sal_False;

    // Hints in the end list are sorted ascending
    sal_uInt16 i=0;
    while( i < GetEndingCount() )
    {
        SwHintSttEndPos *pPos = GetEndingHint( i );
        xub_StrLen nEnd = pPos->GetEnd();

        if( STRING_LEN==nPos || nEnd == nPos )
        {
//          if( bIgnWS )
//              rExport.GetDocHandler()->ignorableWhitespace( rExport.sWS );
            rExport.GetDocHandler()->endElement( sQSpan );
            _RemoveHint( i );

            bRet = sal_True;
        }
        else if( nEnd > nPos )
        {
            // This and all hints that are following will be closed later
            break;
        }
        else
        {
            // This hint should have been exported already!
            ASSERT( nEnd >= nPos,
                    "SwXMLHintEndPosLst::OutEndAttrs: hint overseen" );
            i++;
        }
    }

    return bRet;
}

sal_Bool SwXMLHintEndPosList::HasStartingOrEndingHints( xub_StrLen nPos )
{
    sal_Bool bRet = sal_False;

    // Hints in the start list are sorted ascending.

    sal_uInt16 nCount = GetStartingCount();
    for( sal_uInt16 i=0; i< nCount; i++ )
    {
        SwHintSttEndPos *pPos = GetStartingHint( i );
        xub_StrLen nStart = pPos->GetStart();
        if( nStart == nPos )
        {
            bRet = sal_True;
            break;
        }
        else if( nStart > nPos )
        {
            // This and all following hint will be started later
            break;
        }
    }

    if( !bRet )
    {
        nCount = GetEndingCount();
        for( i=0; i < nCount; i++ )
        {
            SwHintSttEndPos *pPos = GetEndingHint( i );
            xub_StrLen nEnd = pPos->GetEnd();

            if( nEnd == nPos )
            {
                bRet = sal_True;
                break;
            }
            else if( nEnd > nPos )
            {
                // This and all hints that are following will be closed later
                break;
            }
        }
    }

    return bRet;
}

void SwXMLExport::ExportTxtNode( const SwTxtNode& rTxtNd, xub_StrLen nStart,
                                 xub_StrLen nEnd, sal_Bool bExportWholeNode )
{
    // <text:p ...>
    CheckAttrList();

    const SfxItemSet *pItemSet = rTxtNd.GetpSwAttrSet();

    // text:style-name="..."
    // If the text collection is a conditional style, the current style
    // is written as style-name rather than the master style.
    const SwFmtColl& rColl = rTxtNd.GetAnyFmtColl();
    OUString sStyleName( rColl.GetName() );
    if( pItemSet )
        sStyleName = FindParaAutoStyle( sStyleName, *pItemSet );
    else
        sStyleName = SwXStyleFamilies::GetProgrammaticName( sStyleName,
                                                    SFX_STYLE_FAMILY_PARA );
    ASSERT( sStyleName.getLength(), "auto para style not found" );
    AddAttribute( XML_NAMESPACE_TEXT, sXML_style_name, sStyleName );

    sal_uInt8 nOutlineLevel = ((const SwTxtFmtColl&)rColl).GetOutlineLevel();

    // text:cond-style-name="..."
    if( rTxtNd.GetCondFmtColl() )
    {
        // master style
        const SwFmtColl *pColl = rTxtNd.GetFmtColl();
        OUString sCondStyleName;
        if( &rColl == pColl )
        {
            sCondStyleName = sStyleName;
        }
        else
        {
            sCondStyleName = pColl->GetName();
            if( pItemSet )
                sCondStyleName = FindParaAutoStyle( sCondStyleName, *pItemSet );
            else
                sCondStyleName = SwXStyleFamilies::GetProgrammaticName(
                                                    sCondStyleName,
                                                    SFX_STYLE_FAMILY_PARA );
            ASSERT( sCondStyleName.getLength(), "auto para style not found" );
            nOutlineLevel = ((const SwTxtFmtColl *)pColl)->GetOutlineLevel();
        }
        AddAttribute( XML_NAMESPACE_TEXT, sXML_cond_style_name,
                      sCondStyleName );
    }

    if( NO_NUMBERING != nOutlineLevel )
    {
        OUStringBuffer sTmp;
        sTmp.append( GetRealLevel(nOutlineLevel)+1L );
        AddAttribute( XML_NAMESPACE_TEXT, sXML_level,
                      sTmp.makeStringAndClear() );
    }

    {
        const sal_Char *pElem =
            NO_NUMBERING == nOutlineLevel ? sXML_p : sXML_h;
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT, pElem, sal_True,
                                  sal_False );
        xub_StrLen nOffset = 0;

        const String& rText = rTxtNd.GetTxt();
        xub_StrLen nPos = pCurPaM->GetPoint()->nContent.GetIndex();
        xub_StrLen nEndPos = rText.Len();
        if( pCurPaM->GetPoint()->nNode == pCurPaM->GetMark()->nNode )
            nEndPos = pCurPaM->GetMark()->nContent.GetIndex();

        const SwpHints *pHints = rTxtNd.GetpSwpHints();
        sal_uInt16 nHintCount = pHints ? pHints->Count() : 0;
        sal_uInt16 nHintPos = 0;
        SwXMLHintEndPosList aHintLst( *this );

        // export all hints before the current text position
        const SwTxtAttr * pTxtAttr = 0;
        if( nHintCount && nPos > *( pTxtAttr = (*pHints)[0] )->GetStart() )
        {
            do
            {
                nHintPos++;

                if( pTxtAttr->GetEnd() )
                {
                    DBG_ASSERT( RES_TXTATR_FIELD == pTxtAttr->Which(),
                                "ignore fields" );
                    xub_StrLen nHtEnd = *pTxtAttr->GetEnd(),
                           nHtStt = *pTxtAttr->GetStart();
                    if( (!bExportWholeNode && nHtEnd <= nPos) ||
                         nHtEnd == nHtStt )
                        continue;

                    // add hint to hint list
                    if( bExportWholeNode )
                    {
                        aHintLst.Insert( pTxtAttr->GetAttr(), nHtStt + nOffset,
                                         nHtEnd + nOffset );
                    }
                    else
                    {
                        xub_StrLen nTmpStt = nHtStt < nPos ? nPos : nHtStt;
                        xub_StrLen nTmpEnd = nHtEnd < nEndPos ? nHtEnd : nEndPos;
                        aHintLst.Insert( pTxtAttr->GetAttr(), nTmpStt + nOffset,
                                            nTmpEnd + nOffset );
                    }
                }

            }
            while( nHintPos < nHintCount &&
                   nPos > *( pTxtAttr = (*pHints)[nHintPos ] )->GetStart() );
        }

        xub_StrLen nExpStartPos = nPos;
        sal_Bool bCurrCharIsSpace = sal_False;
        xub_StrLen nSpaceChars = 0;
        for( ; nPos < nEndPos; nPos++ )
        {
            sal_Bool bExpCharAsText = sal_True;
            const SwTxtAttr * pTxtAttrWOEnd = 0;
            if( nHintPos < nHintCount && *pTxtAttr->GetStart() == nPos
                && nPos != nEndPos )
            {
                do
                {
                    if( pTxtAttr->GetEnd() )
                    {
                        if( *pTxtAttr->GetEnd() != nPos )
                        {
                            // insert hint into the hint list if its not
                            // empty
                            aHintLst.Insert( pTxtAttr->GetAttr(),
                                             nPos + nOffset,
                                                *pTxtAttr->GetEnd() + nOffset );
                        }
                    }
                    else
                    {
                        // hints without end must be exported after all
                        // other hints that start at this position
                        ASSERT( !pTxtAttrWOEnd,
                        "SwXMLExport::ExportTxtNode: There is a hint already" );
                        pTxtAttrWOEnd = pTxtAttr;
                        bExpCharAsText = sal_False;     // ignore 0xff
                    }
                }
                while( ++nHintPos < nHintCount &&
                      nPos == *( pTxtAttr = (*pHints)[nHintPos] )->GetStart() );
            }

            // Some characters require some extra work ...
            sal_Bool bExpCharAsElement = sal_False;
            sal_Bool bPrevCharIsSpace = bCurrCharIsSpace;
            bCurrCharIsSpace = sal_False;
            if( bExpCharAsText )
            {
                switch( rText.GetChar(nPos) )
                {
                case 0x0009:    // Tab
                case 0x000A:    // LF
                    // These characters are exported as text.
                    bExpCharAsElement = sal_True;
                    bExpCharAsText = sal_False;
                    break;
                case 0x0020:    // Blank
                    if( bPrevCharIsSpace )
                    {
                        // If the previous character is a space character,
                        // too, export a special space element.
                        bExpCharAsText = sal_False;
                    }
                    bCurrCharIsSpace = sal_True;
                    break;
                }
            }

            // If the current character is not exported as text or if there
            // are hints starting or ending before the current position,
            // the text that has not been exported by now has to be exported
            // now.
            if( nPos > nExpStartPos &&
                ( !bExpCharAsText ||
                  aHintLst.HasStartingOrEndingHints( nPos+nOffset ) ) )
            {
                ASSERT( 0==nSpaceChars,
                        "SwXMLExport::ExportTxtNode: pending spaces" );
                String sExp( rText.Copy( nExpStartPos,
                                         nPos - nExpStartPos ) );
                GetDocHandler()->characters( sExp );
                nExpStartPos = nPos;
            }

            // If there are spaces left that have not been exported and the
            // current chracter is either not a space or there are some
            // hints starting or ending, the pending spaces have to be
            // exported now.
            if( nSpaceChars > 0 &&
                ( !bCurrCharIsSpace ||
                  aHintLst.HasStartingOrEndingHints( nPos+nOffset ) ) )
            {
                ASSERT( nExpStartPos == nPos,
                        "SwXMLExport::ExportTxtNode: pending characters" );

                if( nSpaceChars > 1 )
                {
                    OUStringBuffer sTmp;
                    sTmp.append( (sal_Int32)nSpaceChars );
                    AddAttribute( XML_NAMESPACE_TEXT, sXML_c,
                                  sTmp.makeStringAndClear() );
                }

                SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT,
                                        sXML_s, sal_False, sal_False );

                nSpaceChars = 0;
            }

            // Before the text or tag of the current element, ending and
            // starting hints have to be exported.
            aHintLst.ExportEndingHints( nPos + nOffset );
            aHintLst.ExportStartingHints( nPos + nOffset );

            // If the current character has to be exported as a special
            // element, the elemnt will be exported now.
            if( bExpCharAsElement )
            {
                switch( rText.GetChar(nPos) )
                {
                case 0x0009:    // Tab
                    {
                        SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT,
                                                  sXML_tab_stop, sal_False,
                                                  sal_False );
                    }
                    break;
                case 0x000A:    // LF
                    {
                        SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT,
                                                  sXML_line_break, sal_False,
                                                  sal_False );
                    }
                    break;
                }
            }
            if( pTxtAttrWOEnd  )
            {
                // TODO: This is a hack as long as if there is no field
                // export.
                if( RES_TXTATR_FIELD == pTxtAttrWOEnd->Which() )
                {
                    const SwField *pFld =
                        ((const SwFmtFld&)pTxtAttrWOEnd->GetAttr()).GetFld();
                    if( pFld )
                    {
                        GetDocHandler()->characters( pFld->Expand() );
                    }
#ifdef XML_CORE_API
//                  Reference < XTextField > xFld = new SwXTextField(
//                      ((const SwFmtFld&)pTxtAttrWOEnd->GetAttr()),
//                      &GetDoc() );
//
//                  GetTextFieldExport().ExportField( xFld );
#endif
                }
            }

            // If the current character is a space, and the previous one
            // is a space, too, the number of pending spaces is incremented
            // only.
            if( bCurrCharIsSpace && bPrevCharIsSpace )
                nSpaceChars++;

            // If the currect character is not exported as text, the start
            // position for text is the position behind the current position.
            if( !bExpCharAsText )
            {
                ASSERT( nExpStartPos == nPos, "wrong export start pos" );
                nExpStartPos = nPos+1;
            }
        }

        // If there is some text left, it has to be exported now.
        if( nExpStartPos < nEndPos )
        {
            ASSERT( 0==nSpaceChars,
                    "SwXMLExport::ExportTxtNode: pending spaces " );
            String sExp( rText.Copy( nExpStartPos, nEndPos - nExpStartPos ) );
            GetDocHandler()->characters( sExp );
        }

        // If there are some spaces left, they have to be exported now.
        if( nSpaceChars > 0 )
        {
            if( nSpaceChars > 1 )
            {
                OUStringBuffer sTmp;
                sTmp.append( (sal_Int32)nSpaceChars );
                AddAttribute( XML_NAMESPACE_TEXT, sXML_c,
                              sTmp.makeStringAndClear() );
            }

            SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT, sXML_s,
                                      sal_False, sal_False );
        }

        // The same applies to all hints that are currently open.
        aHintLst.ExportEndingHints( STRING_LEN );
    }
}

void SwXMLExport::ExportSection( const SwSectionNode& rSectNd )
{
#if 0
    const SwSection& rSection = rSectNd.GetSection();
    const SwSectionFmt *pSectFmt = rSection.GetFmt();
    ASSERT( pSectFmt, "SwXMLExport::ExportSection: section without a format?" );

    // <text:section-desc ...>
    CheckAttrList();

    // text:name="..."
    AddAttribute( XML_NAMESPACE_TEXT, sXML_name, S2WS(rSection.GetName()) );

    // text:hidden="..."
    if( rSection.IsHidden() )
    {
        AddAttributeASCII( XML_NAMESPACE_TEXT, sXML_hidden, sXML_true );

        // text:condition="..."
        if( rSection.IsCondHidden() )
        {
            AddAttribute( XML_NAMESPACE_TEXT, sXML_condition,
                          S2WS(rSection.GetCondition()) );
        }
    }

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT, sXML_section_desc,
                                sal_True, sal_True );
//      TODO
//      pSectionItemExport->Export( pSectFmt->GetAttrSet(), XML_ITEM_IGN_WS );
    }

    // <text:section ...>
    CheckAttrList();

    // text:name="..."
    AddAttribute( XML_NAMESPACE_TEXT, sXML_section_name,
                  S2WS(rSection.GetName()) );
#endif

    SwNode *pStartNd = pDoc->GetNodes()[rSectNd.GetIndex() + 1];
    SwNode *pEndNd = pDoc->GetNodes()[rSectNd.EndOfSectionIndex() - 1];

    {
#if 0
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT, sXML_section,
                                sal_True, sal_True );
#endif
        SwXMLExpContext aContext( *this, *pStartNd, *pEndNd, 0, STRING_LEN );
        ExportCurPaM();
    }

    pCurPaM->GetPoint()->nNode = *rSectNd.EndOfSectionNode();
}
#endif

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmltext.cxx,v 1.1.1.1 2000-09-18 17:15:00 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.48  2000/09/18 16:05:08  willem.vandorp
      OpenOffice header added.

      Revision 1.47  2000/09/18 11:58:03  mib
      text frames/graphics import and export continued

      Revision 1.46  2000/08/24 11:16:42  mib
      text import continued

      Revision 1.45  2000/08/10 10:22:16  mib
      #74404#: Adeptions to new XSL/XLink working draft

      Revision 1.44  2000/08/04 11:39:54  dvo
      - field declarations import removed from CORE API import

      Revision 1.43  2000/08/03 18:03:58  dvo
      - variable field declarations import added

      Revision 1.42  2000/07/31 09:42:35  mib
      text export continued

      Revision 1.41  2000/07/24 10:19:02  dvo
      - textfield export for XML_CORE_API

      Revision 1.40  2000/07/21 12:55:15  mib
      text import/export using StarOffice API

      Revision 1.39  2000/07/11 11:17:59  dvo
      text field export added

      Revision 1.38  2000/06/08 09:45:55  aw
      changed to use functionality from xmloff project now

      Revision 1.37  2000/05/24 12:08:11  mib
      unicode bug fix

      Revision 1.36  2000/05/03 12:08:05  mib
      unicode

      Revision 1.35  2000/03/13 14:33:45  mib
      UNO3

      Revision 1.34  2000/03/06 10:46:11  mib
      #72585#: toInt32

      Revision 1.33  2000/02/21 12:51:07  mib
      #70271#: Export of section element removed

      Revision 1.32  2000/02/17 14:40:30  mib
      #70271#: XML table import

      Revision 1.30  2000/01/27 08:59:02  mib
      #70271#: outline numbering

      Revision 1.29  2000/01/20 10:03:16  mib
      #70271#: Lists reworked

      Revision 1.28  2000/01/12 15:00:23  mib
      #70271#: lists

      Revision 1.27  2000/01/06 15:08:28  mib
      #70271#:separation of text/layout, cond. styles, adaptions to wd-xlink-19991229

      Revision 1.26  1999/12/14 09:47:51  mib
      #70271#: Export field content as text

      Revision 1.25  1999/12/13 08:29:07  mib
      #70271#: Support for element items added, background and brush item

      Revision 1.24  1999/12/08 10:44:21  cl
      #70271# added first support for importing tab-stops

      Revision 1.23  1999/12/06 11:41:33  mib
      #70258#: Container item for unkown attributes

      Revision 1.22  1999/11/26 11:11:49  mib
      export-flags, loading of styles only

      Revision 1.21  1999/11/19 15:27:59  mib
      Opt: using C++ arrays instead of string buffers

      Revision 1.20  1999/11/12 11:43:03  mib
      using item mapper, part iii

      Revision 1.19  1999/11/10 15:08:09  mib
      Import now uses XMLItemMapper

      Revision 1.18  1999/11/09 15:40:08  mib
      Using XMLItemMapper for export

      Revision 1.17  1999/11/03 11:13:50  mib
      list style import continued

      Revision 1.16  1999/11/01 11:38:50  mib
      List style import

      Revision 1.15  1999/10/26 13:33:50  mib
      list styles continued

      Revision 1.14  1999/10/25 10:41:48  mib
      Using new OUString ASCII methods

      Revision 1.13  1999/10/15 12:38:01  mib
      XML_TOK_SW_LINE_FEED -> XML_TOK_SW_LINE_BREAK

      Revision 1.12  1999/10/08 11:47:56  mib
      moved some file to SVTOOLS/SVX

      Revision 1.11  1999/10/06 06:48:06  mib
      WS stripping

      Revision 1.10  1999/10/01 14:12:02  mib
      tab, line break and space

      Revision 1.9  1999/09/28 10:47:05  mib
      memory leak

      Revision 1.8  1999/09/28 08:31:47  mib
      char fmts, hints

      Revision 1.7  1999/09/23 11:54:28  mib
      i18n, token maps and hard paragraph attributes

      Revision 1.6  1999/09/22 11:57:46  mib
      string -> wstring

      Revision 1.5  1999/08/18 15:03:38  MIB
      Style import


      Rev 1.4   18 Aug 1999 17:03:38   MIB
   Style import

      Rev 1.3   17 Aug 1999 16:29:24   MIB
   import of text nodes started

      Rev 1.2   13 Aug 1999 16:19:42   MIB
   styles and sections

      Rev 1.1   12 Aug 1999 18:06:22   MIB
   Export ofSvxFontItem, SvxFontHeightItem and SvxLRSpaceItem

      Rev 1.0   12 Aug 1999 12:29:28   MIB
   Initial revision.

*************************************************************************/

