/*************************************************************************
 *
 *  $RCSfile: xmlfmt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-18 11:20:44 $
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

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SVSTDARR_STRINGSSORT_DECL
#define _SVSTDARR_STRINGSSORT
#include <svtools/svstdarr.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _FORMAT_HXX //autogen wg. SwFmt
#include <format.hxx>
#endif
#ifndef _FMTCOL_HXX //autogen wg. SwTxtFmtColl
#include <fmtcol.hxx>
#endif
#ifndef _HINTS_HXX //autogen wg. SwFmtChg
#include <hints.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

#ifndef _POOLFMT_HXX //autogen wg. RES_POOL_CHRFMT_TYPE
#include <poolfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen wg. SwCharFmt
#include <charfmt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen wg. SwFrmFmt
#include <frmfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif

#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#include "docary.hxx"
#ifndef _UNOSTYLE_HXX
#include "unostyle.hxx"
#endif
#ifndef _FMTPDSC_HXX
#include "fmtpdsc.hxx"
#endif
#ifndef _PAGEDESC_HXX
#include "pagedesc.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include <xmloff/i18nmap.hxx>
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include <xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLOFF_XMLITEM_HXX
#include <xmloff/xmlitem.hxx>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <xmloff/xmlstyle.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_TXTSTYLI_HXX
#include <xmloff/txtstyli.hxx>
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTMASTERSTYLESCONTEXT_HXX
#include <xmloff/XMLTextMasterStylesContext.hxx>
#endif

#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLNUM_HXX
#include "xmlnum.hxx"
#endif
#ifndef _XMLTBLI_HXX
#include "xmltbli.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::rtl;

class SwXMLConditionParser_Impl
{
    OUString sInput;

    sal_uInt32 nCondition;
    sal_uInt32 nSubCondition;

    sal_Int32 nPos;
    sal_Int32 nLength;

    inline sal_Bool SkipWS();
    inline sal_Bool MatchChar( sal_Unicode c );
    inline sal_Bool MatchName( OUString& rName );
    inline sal_Bool MatchNumber( sal_uInt32& rNumber );

public:

    SwXMLConditionParser_Impl( const OUString& rInp );

    sal_Bool IsValid() const { return 0 != nCondition; }

    sal_uInt32 GetCondition() const { return nCondition; }
    sal_uInt32 GetSubCondition() const { return nSubCondition; }
};

inline sal_Bool SwXMLConditionParser_Impl::SkipWS()
{
    while( nPos < nLength && ' ' == sInput[nPos] )
        nPos++;
    return sal_True;
}

inline sal_Bool SwXMLConditionParser_Impl::MatchChar( sal_Unicode c )
{
    sal_Bool bRet = sal_False;
    if( nPos < nLength && c == sInput[nPos] )
    {
        nPos++;
        bRet = sal_True;
    }
    return bRet;
}

inline sal_Bool SwXMLConditionParser_Impl::MatchName( OUString& rName )
{
    OUStringBuffer sBuffer( nLength );
    while( nPos < nLength &&
           ( ('a' <= sInput[nPos] && sInput[nPos] <= 'z') ||
              '-' == sInput[nPos] ) )
    {
        sBuffer.append( sInput[nPos] );
        nPos++;
    }
    rName = sBuffer.makeStringAndClear();
    return rName.getLength() > 0;
}

inline sal_Bool SwXMLConditionParser_Impl::MatchNumber( sal_uInt32& rNumber )
{
    OUStringBuffer sBuffer( nLength );
    while( nPos < nLength && '0' <= sInput[nPos] && sInput[nPos] <= '9' )
    {
        sBuffer.append( sInput[nPos] );
        nPos++;
    }

    OUString sNum( sBuffer.makeStringAndClear() );
    if( sNum.getLength() )
        rNumber = sNum.toInt32();
    return sNum.getLength() > 0;
}

SwXMLConditionParser_Impl::SwXMLConditionParser_Impl( const OUString& rInp ) :
    sInput( rInp ),
    nPos( 0 ),
    nLength( rInp.getLength() ),
    nCondition( 0 ),
    nSubCondition( 0 )
{
    OUString sFunc;
    sal_Bool bHasSub = sal_False;
    sal_uInt32 nSub = 0;
    sal_Bool bOK = SkipWS() && MatchName( sFunc ) && SkipWS() &&
               MatchChar( '(' ) && SkipWS() && MatchChar( ')' ) && SkipWS();
    if( bOK && MatchChar( '=' ) )
    {
        bOK = SkipWS() && MatchNumber( nSub ) && SkipWS();
        bHasSub = sal_True;
    }

    bOK &= nPos == nLength;

    if( bOK )
    {
        switch( sFunc[0] )
        {
        case 'e':
            if( sFunc.compareToAscii( sXML_endnote ) == 0 && !bHasSub )
                nCondition = PARA_IN_ENDNOTE;
            break;

        case 'f':
            if( sFunc.compareToAscii( sXML_footer ) == 0 && !bHasSub )
                nCondition = PARA_IN_FOOTER;
            else if( sFunc.compareToAscii( sXML_table_header ) == 0 &&
                     !bHasSub )
                nCondition = PARA_IN_FOOTENOTE;
            break;

        case 'h':
            if( sFunc.compareToAscii( sXML_header ) == 0 && !bHasSub )
                nCondition = PARA_IN_HEADER;
            break;

        case 'l':
            if( sFunc.compareToAscii( sXML_list_level ) == 0 &&
                nSub >=1 && nSub < MAXLEVEL )
            {
                nCondition = PARA_IN_LIST;
                nSubCondition = nSub-1;
            }
            break;

        case 'o':
            if( sFunc.compareToAscii( sXML_outline_level ) == 0 &&
                nSub >=1 && nSub < MAXLEVEL )
            {
                nCondition = PARA_IN_OUTLINE;
                nSubCondition = nSub-1;
            }
            break;

        case 's':
            if( sFunc.compareToAscii( sXML_section ) == 0 && !bHasSub )
            {
                nCondition = PARA_IN_SECTION;
            }
            break;

        case 't':
            if( sFunc.compareToAscii( sXML_table ) == 0 && !bHasSub )
            {
                nCondition = PARA_IN_TABLEBODY;
            }
            else if( sFunc.compareToAscii( sXML_table_header ) == 0 &&
                     !bHasSub )
            {
                nCondition = PARA_IN_TABLEHEAD;
            }
            else if( sFunc.compareToAscii( sXML_text_box ) == 0 && !bHasSub )
            {
                nCondition = PARA_IN_FRAME;
            }
            break;
        }
    }
}

// ---------------------------------------------------------------------

class SwXMLConditionContext_Impl : public SvXMLImportContext
{
    sal_uInt32 nCondition;
    sal_uInt32 nSubCondition;

    OUString sApplyStyle;

    void ParseCondition( const OUString& rCond );

public:

    SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLConditionContext_Impl();

    sal_Bool IsValid() const { return 0 != nCondition; }

    sal_uInt32 GetCondition() const { return nCondition; }
    sal_uInt32 GetSubCondition() const { return nSubCondition; }
    const OUString& GetApplyStyle() const { return sApplyStyle; }
};

SwXMLConditionContext_Impl::SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nCondition( 0 ),
    nSubCondition( 0 )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        // TODO: use a map here
        if( XML_NAMESPACE_STYLE == nPrefix )
        {
            if( aLocalName.compareToAscii( sXML_condition ) == 0 )
            {
                SwXMLConditionParser_Impl aCondParser( rValue );
                if( aCondParser.IsValid() )
                {
                    nCondition = aCondParser.GetCondition();
                    nSubCondition = aCondParser.GetSubCondition();
                }
            }
            else if( aLocalName.compareToAscii( sXML_apply_style_name ) == 0 )
            {
                sApplyStyle = rValue;
            }
        }
    }
}

SwXMLConditionContext_Impl::~SwXMLConditionContext_Impl()
{
}

// ---------------------------------------------------------------------

typedef SwXMLConditionContext_Impl *SwXMLConditionContextPtr;
SV_DECL_PTRARR( SwXMLConditions_Impl, SwXMLConditionContextPtr, 5, 2 );

class SwXMLTextStyleContext_Impl : public XMLTextStyleContext
{
    SwXMLConditions_Impl    *pConditions;

protected:

    virtual Reference < XStyle > Create();

public:

    TYPEINFO();

    SwXMLTextStyleContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nFamily,
            SvXMLStylesContext& rStyles );
    virtual ~SwXMLTextStyleContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void Finish( sal_Bool bOverwrite );
};

TYPEINIT1( SwXMLTextStyleContext_Impl, XMLTextStyleContext );

Reference < XStyle > SwXMLTextStyleContext_Impl::Create()
{
    Reference < XStyle > xNewStyle;

    if( pConditions && XML_STYLE_FAMILY_TEXT_PARAGRAPH == GetFamily() )
    {
        Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    UNO_QUERY );
        if( xFactory.is() )
        {
            OUString sServiceName( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.style.ConditionalParagraphStyle" ) );
            Reference < XInterface > xIfc =
                xFactory->createInstance( sServiceName );
            if( xIfc.is() )
                xNewStyle = Reference < XStyle >( xIfc, UNO_QUERY );
        }
    }
    else
    {
        xNewStyle = XMLTextStyleContext::Create();
    }

    return xNewStyle;
}

SwXMLTextStyleContext_Impl::SwXMLTextStyleContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_uInt16 nFamily,
        SvXMLStylesContext& rStyles ) :
    XMLTextStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles ),
    pConditions( 0 )
{
    SetFamily( nFamily );
}

SwXMLTextStyleContext_Impl::~SwXMLTextStyleContext_Impl()
{
    if( pConditions )
    {
        while( pConditions->Count() )
        {
            SwXMLConditionContext_Impl *pCond = pConditions->GetObject(0);
            pConditions->Remove( 0UL );
            pCond->ReleaseRef();
        }
        delete pConditions;
    }
}

SvXMLImportContext *SwXMLTextStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        rLocalName.compareToAscii( sXML_map ) == 0 )
    {
        SwXMLConditionContext_Impl *pCond =
            new SwXMLConditionContext_Impl( GetImport(), nPrefix,
                                            rLocalName, xAttrList );
        if( pCond->IsValid() )
        {
            if( !pConditions )
               pConditions = new SwXMLConditions_Impl;
            pConditions->Insert( pCond, pConditions->Count() );
            pCond->AddRef();
        }
        pContext = pCond;
    }

    if( !pContext )
        pContext = XMLTextStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void SwXMLTextStyleContext_Impl::Finish( sal_Bool bOverwrite )
{
    XMLTextStyleContext::Finish( bOverwrite );

    if( !pConditions || XML_STYLE_FAMILY_TEXT_PARAGRAPH != GetFamily() )
        return;

    Reference < XStyle > xStyle = GetStyle();
    if( !xStyle.is() )
        return;

    const SwXStyle* pStyle = 0;
    Reference<XUnoTunnel> xStyleTunnel( xStyle, UNO_QUERY);
    if( xStyleTunnel.is() )
    {
        pStyle = (SwXStyle*)xStyleTunnel->getSomething(
                                            SwXStyle::getUnoTunnelId() );
    }
    if( !pStyle )
        return;

    const SwDoc *pDoc = pStyle->GetDoc();

    const OUString& rName =
                SwXStyleFamilies::GetUIName( GetName(), SFX_STYLE_FAMILY_PARA );
    SwTxtFmtColl *pColl = pDoc->FindTxtFmtCollByName( rName );
    ASSERT( pColl, "Text collection not found" );
    if( !pColl || RES_CONDTXTFMTCOLL != pColl->Which() )
        return;

    sal_uInt16 nCount = pConditions->Count();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        const SwXMLConditionContext_Impl *pCond = (*pConditions)[i];
        const OUString& rName =
                SwXStyleFamilies::GetUIName( pCond->GetApplyStyle(),
                                             SFX_STYLE_FAMILY_PARA  );
        SwTxtFmtColl* pCondColl = pDoc->FindTxtFmtCollByName( rName );
        ASSERT( pCondColl,
            "SwXMLItemSetStyleContext_Impl::ConnectConditions: cond coll missing" );
        if( pCondColl )
        {
            SwCollCondition aCond( pCondColl, pCond->GetCondition(),
                                              pCond->GetSubCondition() );
            ((SwConditionTxtFmtColl*)pColl)->InsertCondition( aCond );
        }
    }
}

// ---------------------------------------------------------------------

class SwXMLItemSetStyleContext_Impl : public SvXMLStyleContext
{
    OUString                sMasterPageName;
    SfxItemSet              *pItemSet;

    sal_Bool                bHasMasterPageName : 1;
    sal_Bool                bPageDescConnected : 1;

    SvXMLImportContext *CreateItemSetContext(
            sal_uInt16 nPrefix,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList);

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue );

    const SwXMLImport& GetSwImport() const
            { return (const SwXMLImport&)GetImport(); }
    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }

public:

    TYPEINFO();

    SwXMLItemSetStyleContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nFamily);
    virtual ~SwXMLItemSetStyleContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    // The item set may be empty!
    SfxItemSet *GetItemSet() { return pItemSet; }
    const SfxItemSet *GetItemSet() const { return pItemSet; }

    const OUString& GetMasterPageName() const { return sMasterPageName; }
    sal_Bool HasMasterPageName() const { bHasMasterPageName; }

    sal_Bool IsPageDescConnected() const { return bPageDescConnected; }
    void ConnectPageDesc();
};

void SwXMLItemSetStyleContext_Impl::SetAttribute( sal_uInt16 nPrefixKey,
                                           const OUString& rLocalName,
                                           const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey &&
        rLocalName.compareToAscii( sXML_master_page_name ) == 0 )
    {
        sMasterPageName = rValue;
        bHasMasterPageName = sal_True;
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

SvXMLImportContext *SwXMLItemSetStyleContext_Impl::CreateItemSetContext(
        sal_uInt16 nPrefix, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    ASSERT( !pItemSet,
            "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: item set exists" );

    SvXMLImportContext *pContext = 0;

    Reference<XUnoTunnel> xCrsrTunnel( GetImport().GetTextImport()->GetCursor(),
                                       UNO_QUERY);
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    SwXTextCursor *pTxtCrsr = (SwXTextCursor*)xCrsrTunnel->getSomething(
                                        SwXTextCursor::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SfxItemPool& rItemPool = pTxtCrsr->GetDoc()->GetAttrPool();
    switch( GetFamily() )
    {
    case XML_STYLE_FAMILY_TABLE_TABLE:
        pItemSet = new SfxItemSet( rItemPool, aTableSetRange );
        break;
    case XML_STYLE_FAMILY_TABLE_COLUMN:
        pItemSet = new SfxItemSet( rItemPool, RES_FRM_SIZE, RES_FRM_SIZE, 0 );
        break;
    case XML_STYLE_FAMILY_TABLE_ROW:
        pItemSet = new SfxItemSet( rItemPool, aTableLineSetRange );
        break;
    case XML_STYLE_FAMILY_TABLE_CELL:
        pItemSet = new SfxItemSet( rItemPool, aTableBoxSetRange );
        break;
    default:
        ASSERT( !this,
        "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: unknown family" );
        break;
    }
    if( pItemSet )
        pContext = GetSwImport().CreateTableItemImportContext(
                                nPrefix, rLName, xAttrList, GetFamily(),
                                *pItemSet );
    if( !pContext )
    {
        delete pItemSet;
        pItemSet = 0;
    }

    return pContext;
}

TYPEINIT1( SwXMLItemSetStyleContext_Impl, SvXMLStyleContext );

SwXMLItemSetStyleContext_Impl::SwXMLItemSetStyleContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_uInt16 nFamily ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
    pItemSet( 0 ),
    bHasMasterPageName( sal_False ),
    bPageDescConnected( sal_False )
{
    SetFamily( nFamily );
}

SwXMLItemSetStyleContext_Impl::~SwXMLItemSetStyleContext_Impl()
{
    delete pItemSet;
}

SvXMLImportContext *SwXMLItemSetStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        if( rLocalName.compareToAscii( sXML_properties ) == 0 )
        {
            pContext = CreateItemSetContext( nPrefix, rLocalName, xAttrList );
        }
    }

    if( !pContext )
        pContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void SwXMLItemSetStyleContext_Impl::ConnectPageDesc()
{
    if( bPageDescConnected || !HasMasterPageName() )
        return;
    bPageDescConnected = sal_True;

    Reference<XUnoTunnel> xCrsrTunnel( GetImport().GetTextImport()->GetCursor(),
                                       UNO_QUERY);
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    SwXTextCursor *pTxtCrsr = (SwXTextCursor*)xCrsrTunnel->getSomething(
                                        SwXTextCursor::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SwDoc *pDoc = pTxtCrsr->GetDoc();

    String sName( SwXStyleFamilies::GetUIName( GetMasterPageName(),
                                               SFX_STYLE_FAMILY_PAGE ) );
    SwPageDesc *pPageDesc = pDoc->FindPageDescByName( sName );
    if( !pPageDesc )
    {
        // If the page style is a pool style, then we maybe have to create it
        // first if it hasn't been used by now.
        sal_uInt16 nPoolId = pDoc->GetPoolId( sName, GET_POOLID_PAGEDESC );
        if( USHRT_MAX != nPoolId )
            pPageDesc = pDoc->GetPageDescFromPool( nPoolId );
    }

    if( !pPageDesc )
        return;

    if( !pItemSet )
    {
        SfxItemPool& rItemPool = pDoc->GetAttrPool();
        pItemSet = new SfxItemSet( rItemPool, aTableSetRange );
    }

    const SfxPoolItem *pItem;
    SwFmtPageDesc *pFmtPageDesc = 0;
    if( SFX_ITEM_SET == pItemSet->GetItemState( RES_PAGEDESC, sal_False,
                                                &pItem ) )
    {
         if( ((SwFmtPageDesc *)pItem)->GetPageDesc() != pPageDesc )
            pFmtPageDesc = new SwFmtPageDesc( *(SwFmtPageDesc *)pItem );
    }
    else
        pFmtPageDesc = new SwFmtPageDesc();

    if( pFmtPageDesc )
    {
        pPageDesc->Add( pFmtPageDesc );
        pItemSet->Put( *pFmtPageDesc );
        delete pFmtPageDesc;
    }
}

// ---------------------------------------------------------------------
//
class SwXMLStylesContext_Impl : public SvXMLStylesContext
{
    sal_Bool bAutoStyles;

    SwXMLItemSetStyleContext_Impl *GetSwStyle( sal_uInt16 i ) const;

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
    const SwXMLImport& GetSwImport() const
            { return (const SwXMLImport&)GetImport(); }

protected:

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList );
    // HACK
    virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const;

    virtual ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const;
    virtual ::rtl::OUString GetServiceName( sal_uInt16 nFamily ) const;
    // HACK

public:

    TYPEINFO();

    SwXMLStylesContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName ,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Bool bAuto );
    virtual ~SwXMLStylesContext_Impl();

    virtual sal_Bool InsertStyleFamily( sal_uInt16 nFamily ) const;

    virtual void EndElement();
};

TYPEINIT1( SwXMLStylesContext_Impl, SvXMLStylesContext );

inline SwXMLItemSetStyleContext_Impl *SwXMLStylesContext_Impl::GetSwStyle(
        sal_uInt16 i ) const
{
    return PTR_CAST( SwXMLItemSetStyleContext_Impl, GetStyle( i ) );
}

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = 0;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        pStyle = new SwXMLTextStyleContext_Impl( GetSwImport(), nPrefix,
                            rLocalName, xAttrList, nFamily, *this );
        break;
    case XML_STYLE_FAMILY_TABLE_TABLE:
    case XML_STYLE_FAMILY_TABLE_COLUMN:
    case XML_STYLE_FAMILY_TABLE_ROW:
    case XML_STYLE_FAMILY_TABLE_CELL:
        pStyle = new SwXMLItemSetStyleContext_Impl( GetSwImport(), nPrefix,
                            rLocalName, xAttrList, nFamily  );
        break;
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        // HACK until CL is back again
        pStyle = new XMLTextStyleContext( GetImport(), nPrefix,
                            rLocalName, xAttrList, *this );
        break;
    default:
        pStyle = SvXMLStylesContext::CreateStyleStyleChildContext( nFamily,
                                                                   nPrefix,
                                                              rLocalName,
                                                              xAttrList );
        break;
    }

    return pStyle;
}

SwXMLStylesContext_Impl::SwXMLStylesContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bAuto ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList ),
    bAutoStyles( bAuto )
{
}

SwXMLStylesContext_Impl::~SwXMLStylesContext_Impl()
{
}

sal_Bool SwXMLStylesContext_Impl::InsertStyleFamily( sal_uInt16 nFamily ) const
{
    const SwXMLImport& rSwImport = GetSwImport();
    sal_uInt16 nStyleFamilyMask = rSwImport.GetStyleFamilyMask();

    sal_Bool bIns = sal_True;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_PARA) != 0;
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_CHAR) != 0;
        break;
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_FRAME) != 0;
        break;
    case XML_STYLE_FAMILY_TEXT_LIST:
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_PSEUDO) != 0;
        break;
    case XML_STYLE_FAMILY_TEXT_OUTLINE:
        bIns = !(rSwImport.IsInsertMode() || rSwImport.IsStylesOnlyMode());
        break;
    default:
        bIns = SvXMLStylesContext::InsertStyleFamily( nFamily );
        break;
    }

    return bIns;
}

UniReference < SvXMLImportPropertyMapper > SwXMLStylesContext_Impl::GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper;
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        xMapper = ((SvXMLImport *)&GetImport())->GetTextImport()
                     ->GetFrameImportPropertySetMapper();
    else
        xMapper = SvXMLStylesContext::GetImportPropertyMapper( nFamily );

    return xMapper;
}

Reference < XNameContainer > SwXMLStylesContext_Impl::GetStylesContainer(
                                                sal_uInt16 nFamily ) const
{
    Reference < XNameContainer > xStyles;
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        xStyles = ((SvXMLImport *)&GetImport())->GetTextImport()->GetFrameStyles();
    else
        xStyles = SvXMLStylesContext::GetStylesContainer( nFamily );

    return xStyles;
}

OUString SwXMLStylesContext_Impl::GetServiceName( sal_uInt16 nFamily ) const
{
    String sServiceName;
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        sServiceName = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.FrameStyle") );
    else
        sServiceName = SvXMLStylesContext::GetServiceName( nFamily );

    return sServiceName;
}

void SwXMLStylesContext_Impl::EndElement()
{
    GetSwImport().InsertStyles( bAutoStyles );
}

// ---------------------------------------------------------------------
//
class SwXMLMasterStylesContext_Impl : public XMLTextMasterStylesContext
{
protected:
    virtual sal_Bool InsertStyleFamily( sal_uInt16 nFamily ) const;

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
    const SwXMLImport& GetSwImport() const
            { return (const SwXMLImport&)GetImport(); }

public:

    TYPEINFO();

    SwXMLMasterStylesContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName ,
            const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLMasterStylesContext_Impl();
    virtual void EndElement();
};

TYPEINIT1( SwXMLMasterStylesContext_Impl, XMLTextMasterStylesContext );

SwXMLMasterStylesContext_Impl::SwXMLMasterStylesContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName ,
        const Reference< xml::sax::XAttributeList > & xAttrList ) :
    XMLTextMasterStylesContext( rImport, nPrfx, rLName, xAttrList )
{
}

SwXMLMasterStylesContext_Impl::~SwXMLMasterStylesContext_Impl()
{
}

sal_Bool SwXMLMasterStylesContext_Impl::InsertStyleFamily( sal_uInt16 nFamily ) const
{
    sal_Bool bIns;

    const SwXMLImport& rSwImport = GetSwImport();
    sal_uInt16 nStyleFamilyMask = rSwImport.GetStyleFamilyMask();
    if( XML_STYLE_FAMILY_MASTER_PAGE == nFamily )
        bIns = (nStyleFamilyMask & SFX_STYLE_FAMILY_PAGE) != 0;
    else
        bIns = XMLTextMasterStylesContext::InsertStyleFamily( nFamily );

    return bIns;
}

void SwXMLMasterStylesContext_Impl::EndElement()
{
    FinishStyles( !GetSwImport().IsInsertMode() );
    GetSwImport().FinishStyles();
}
// ---------------------------------------------------------------------

SvXMLImportContext *SwXMLImport::CreateStylesContext(
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bAuto )
{
    SvXMLImportContext *pContext =
        new SwXMLStylesContext_Impl( *this, XML_NAMESPACE_OFFICE, rLocalName,
                                       xAttrList, bAuto );
    if( bAuto )
        xAutoStyles = pContext;
    else
        xStyles = pContext;

    return pContext;
}

SvXMLImportContext *SwXMLImport::CreateMasterStylesContext(
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext =
        new SwXMLMasterStylesContext_Impl( *this, XML_NAMESPACE_OFFICE, rLocalName,
                                          xAttrList );
    return pContext;
}

void SwXMLImport::InsertStyles( sal_Bool bAuto )
{
    if( !bAuto && xStyles.Is() )
        ((SwXMLStylesContext_Impl *)&xStyles)->CopyStylesToDoc(
                                                !IsInsertMode(),
                                                sal_False );

    if( bAuto && xAutoStyles.Is() )
        GetTextImport()->SetAutoStyles( (SwXMLStylesContext_Impl *)&xAutoStyles );
}

void SwXMLImport::FinishStyles()
{
    if( xStyles.Is() )
        ((SwXMLStylesContext_Impl *)&xStyles)->FinishStyles(
                                                !IsInsertMode() );
}

sal_Bool SwXMLImport::FindAutomaticStyle(
        sal_uInt16 nFamily,
        const OUString& rName,
        const SfxItemSet **ppItemSet,
        OUString *pParent ) const
{
    SwXMLItemSetStyleContext_Impl *pStyle = 0;
    if( xAutoStyles.Is() )
    {
        pStyle = PTR_CAST( SwXMLItemSetStyleContext_Impl,
              ((SwXMLStylesContext_Impl *)&xAutoStyles)->
                    FindStyleChildContext( nFamily, rName,
                                           sal_True ) );
        if( pStyle )
        {
            if( ppItemSet )
            {
                if( XML_STYLE_FAMILY_TABLE_TABLE == pStyle->GetFamily() &&
                    pStyle->HasMasterPageName() &&
                    !pStyle->IsPageDescConnected() )
                    pStyle->ConnectPageDesc();
                (*ppItemSet) = pStyle->GetItemSet();
            }

            if( pParent )
                *pParent = pStyle->GetParent();
        }
    }

    return pStyle != 0;
}
