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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SVSTDARR_STRINGSSORT_DECL
#define _SVSTDARR_STRINGSSORT
#include <bf_svtools/svstdarr.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <bf_xmloff/nmspmap.hxx>
#endif

#ifndef _FORMAT_HXX //autogen wg. SwFmt
#include <format.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
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

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
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
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include <bf_xmloff/i18nmap.hxx>
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include <bf_xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLITEM_HXX
#include "xmlitem.hxx"
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <bf_xmloff/xmlstyle.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_TXTSTYLI_HXX
#include <bf_xmloff/txtstyli.hxx>
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include <bf_xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX
#include <bf_xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTMASTERSTYLESCONTEXT_HXX
#include <bf_xmloff/XMLTextMasterStylesContext.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTSHAPESTYLECONTEXT_HXX
#include <bf_xmloff/XMLTextShapeStyleContext.hxx>
#endif
#ifndef _XMLOFF_XMLGRAPHICSDEFAULTSTYLE_HXX
#include <bf_xmloff/XMLGraphicsDefaultStyle.hxx>
#endif

#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLTBLI_HXX
#include "xmltbli.hxx"
#endif
#ifndef _CELLATR_HXX
#include "cellatr.hxx"
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
namespace binfilter {


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::rtl;
using namespace ::binfilter::xmloff::token;

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
    if(	nPos < nLength && c == sInput[nPos] )
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
        if( IsXMLToken( sFunc, XML_ENDNOTE ) && !bHasSub )
            nCondition = PARA_IN_ENDNOTE;
        else if( IsXMLToken( sFunc, XML_FOOTER ) && !bHasSub )
            nCondition = PARA_IN_FOOTER;
        else if( IsXMLToken( sFunc, XML_FOOTNOTE ) && !bHasSub )
            nCondition = PARA_IN_FOOTENOTE;
        else if( IsXMLToken( sFunc, XML_HEADER ) && !bHasSub )
            nCondition = PARA_IN_HEADER;
        else if( IsXMLToken( sFunc, XML_LIST_LEVEL) &&
                nSub >=1 && nSub <= MAXLEVEL )
        { 
            nCondition = PARA_IN_LIST;
            nSubCondition = nSub-1;
        }
        else if( IsXMLToken( sFunc, XML_OUTLINE_LEVEL) && 
                 nSub >=1 && nSub <= MAXLEVEL )
        { 
            nCondition = PARA_IN_OUTLINE;
            nSubCondition = nSub-1;
        }
        else if( IsXMLToken( sFunc, XML_SECTION ) && !bHasSub )
        {
            nCondition = PARA_IN_SECTION;
        }
        else if( IsXMLToken( sFunc, XML_TABLE ) && !bHasSub )
        {
            nCondition = PARA_IN_TABLEBODY;
        }
        else if( IsXMLToken( sFunc, XML_TABLE_HEADER ) && !bHasSub )
        {
            nCondition = PARA_IN_TABLEHEAD;
        }
        else if( IsXMLToken( sFunc, XML_TEXT_BOX ) && !bHasSub )
        {
            nCondition = PARA_IN_FRAME;
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
            if( IsXMLToken( aLocalName, XML_CONDITION ) )
            {
                SwXMLConditionParser_Impl aCondParser( rValue );
                if( aCondParser.IsValid() )
                {
                    nCondition = aCondParser.GetCondition();
                    nSubCondition = aCondParser.GetSubCondition();
                }
            }
            else if( IsXMLToken( aLocalName, XML_APPLY_STYLE_NAME ) )
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
SV_DECL_PTRARR( SwXMLConditions_Impl, SwXMLConditionContextPtr, 5, 2 )//STRIP008 ;

class SwXMLTextStyleContext_Impl : public XMLTextStyleContext
{
    SwXMLConditions_Impl	*pConditions;

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
    XMLTextStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily ),
    pConditions( 0 )
{
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

    if( XML_NAMESPACE_STYLE == nPrefix && IsXMLToken( rLocalName, XML_MAP ) )
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

    SwTxtFmtColl *pColl = pDoc->FindTxtFmtCollByName( pStyle->GetStyleName() );
    ASSERT( pColl, "Text collection not found" );
    if( !pColl || RES_CONDTXTFMTCOLL != pColl->Which() )
        return;

    sal_uInt16 nCount = pConditions->Count();
    String aString;
    OUString sName;
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        const SwXMLConditionContext_Impl *pCond = (*pConditions)[i];
        SwStyleNameMapper::FillUIName( pCond->GetApplyStyle(),
                                      aString,
                                      GET_POOLID_TXTCOLL,
                                      sal_True);
        sName = aString;
        SwTxtFmtColl* pCondColl = pDoc->FindTxtFmtCollByName( sName );
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
    OUString				sMasterPageName;
    SfxItemSet  			*pItemSet;

    OUString 				sDataStyleName;

    sal_Bool				bHasMasterPageName : 1;
    sal_Bool 				bPageDescConnected : 1;
    sal_Bool				bDataStyleIsResolved;

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
    sal_Bool HasMasterPageName() const { return bHasMasterPageName; }

    sal_Bool IsPageDescConnected() const { return bPageDescConnected; }
    void ConnectPageDesc();

    sal_Bool ResolveDataStyleName();
};

void SwXMLItemSetStyleContext_Impl::SetAttribute( sal_uInt16 nPrefixKey,
                                           const OUString& rLocalName,
                                           const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey )
    {
        if ( IsXMLToken( rLocalName, XML_MASTER_PAGE_NAME ) )
        {
            sMasterPageName = rValue;
            bHasMasterPageName = sal_True;
        }
        else if ( IsXMLToken( rLocalName, XML_DATA_STYLE_NAME ) )
        {
            // if we have a valid data style name
            if (rValue.getLength() > 0)
            {
                sDataStyleName = rValue;
                bDataStyleIsResolved = sal_False;	// needs to be resolved
            }
        }
        else
        {
            SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
        }
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
    OTextCursorHelper *pTxtCrsr = (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() );
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
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, nFamily ),
    pItemSet( 0 ),
    bHasMasterPageName( sal_False ),
    bPageDescConnected( sal_False ),
    sDataStyleName(),
    bDataStyleIsResolved( sal_True )
{
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
        if( IsXMLToken( rLocalName, XML_PROPERTIES ) )
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
    OTextCursorHelper *pTxtCrsr = (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SwDoc *pDoc = pTxtCrsr->GetDoc();

    String sName;
    SwStyleNameMapper::FillUIName( GetMasterPageName(),
                                   sName,
                                   GET_POOLID_PAGEDESC,
                                   sal_True);
    SwPageDesc *pPageDesc = pDoc->FindPageDescByName( sName );
    if( !pPageDesc )
    {
        // If the page style is a pool style, then we maybe have to create it
        // first if it hasn't been used by now.
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( sName, GET_POOLID_PAGEDESC );
        if( USHRT_MAX != nPoolId )
            pPageDesc = pDoc->GetPageDescFromPoolSimple( nPoolId, FALSE );
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

sal_Bool SwXMLItemSetStyleContext_Impl::ResolveDataStyleName()
{
    sal_Bool bTmp = bDataStyleIsResolved;

    // resolve, if not already done
    if (! bDataStyleIsResolved)
    {
        // get the format key
        sal_Int32 nFormat = 
            GetImport().GetTextImport()->GetDataStyleKey(sDataStyleName);

        // if the key is valid, insert Item into ItemSet
        if( -1 != nFormat )
        {
            if( !pItemSet )
            {
                Reference<XUnoTunnel> xCrsrTunnel( GetImport().GetTextImport()->GetCursor(),
                                                   UNO_QUERY);
                ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
                OTextCursorHelper *pTxtCrsr = (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                                    OTextCursorHelper::getUnoTunnelId() );
                ASSERT( pTxtCrsr, "SwXTextCursor missing" );
                SwDoc *pDoc = pTxtCrsr->GetDoc();

                SfxItemPool& rItemPool = pDoc->GetAttrPool();
                pItemSet = new SfxItemSet( rItemPool, aTableBoxSetRange );
            }
            SwTblBoxNumFormat aNumFormatItem(nFormat);
            pItemSet->Put(aNumFormatItem);
        }

        // now resolved
        bDataStyleIsResolved = sal_True;
        return sal_True;
    }
    else
    {
        // was already resolved; nothing to do
        return sal_False;
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
    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
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
        // As long as there are no element items, we can use the text
        // style class. 
        pStyle = new XMLTextShapeStyleContext( GetImport(), nPrefix,
                            rLocalName, xAttrList, *this, nFamily );
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

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = 0;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        pStyle = new XMLTextStyleContext( GetImport(), nPrefix, rLocalName,
                                          xAttrList, *this, nFamily, 
                                          sal_True );
        break;
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        // There are no writer specific defaults for graphic styles!
        pStyle = new XMLGraphicsDefaultStyle( GetImport(), nPrefix,
                            rLocalName, xAttrList, *this );
        break;
    default:
        pStyle = SvXMLStylesContext::CreateDefaultStyleStyleChildContext( nFamily,
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
    case XML_STYLE_FAMILY_TEXT_FOOTNOTECONFIG:
    case XML_STYLE_FAMILY_TEXT_ENDNOTECONFIG:
    case XML_STYLE_FAMILY_TEXT_LINENUMBERINGCONFIG:
    case XML_STYLE_FAMILY_TEXT_BIBLIOGRAPHYCONFIG:
        bIns = !(rSwImport.IsInsertMode() || rSwImport.IsStylesOnlyMode() ||
                 rSwImport.IsBlockMode());
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
//	if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
//		xMapper = ((SvXMLImport *)&GetImport())->GetTextImport()
//					 ->GetFrameImportPropertySetMapper();
//	else
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
    SvXMLStylesContext *pContext =
        new SwXMLStylesContext_Impl( *this, XML_NAMESPACE_OFFICE, rLocalName,
                                       xAttrList, bAuto );
    if( bAuto )
        SetAutoStyles( pContext );
    else
        SetStyles( pContext );

    return pContext;
}

SvXMLImportContext *SwXMLImport::CreateMasterStylesContext(
        const OUString& rLocalName,   
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStylesContext *pContext =
        new SwXMLMasterStylesContext_Impl( *this, XML_NAMESPACE_OFFICE, rLocalName,
                                          xAttrList );
    SetMasterStyles( pContext );

    return pContext;
}

void SwXMLImport::InsertStyles( sal_Bool bAuto )
{
    if( !bAuto && GetStyles() )
        GetStyles()->CopyStylesToDoc( !IsInsertMode(), sal_False );
}

void SwXMLImport::FinishStyles()
{
    if( GetStyles() )
        GetStyles()->FinishStyles( !IsInsertMode() );
}

void SwXMLImport::UpdateTxtCollConditions( SwDoc *pDoc )
{
    if( !pDoc )
    {
        Reference<XUnoTunnel> xCrsrTunnel( GetTextImport()->GetCursor(),
                                              UNO_QUERY);
        ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
        OTextCursorHelper *pTxtCrsr =
                (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                            OTextCursorHelper::getUnoTunnelId() );
        ASSERT( pTxtCrsr, "SwXTextCursor missing" );
        pDoc = pTxtCrsr->GetDoc();

        ASSERT( pDoc, "document missing" );
    }

    const SwTxtFmtColls& rColls = *pDoc->GetTxtFmtColls();
    sal_uInt16 nCount = rColls.Count();
    for( sal_uInt16 i=0; i < nCount; i++ )
    {
        SwTxtFmtColl *pColl = rColls[i];
        if( pColl && RES_CONDTXTFMTCOLL == pColl->Which() )
        {
            const SwFmtCollConditions& rConditions =
                ((const SwConditionTxtFmtColl *)pColl)->GetCondColls();
            sal_Bool bSendModify = sal_False;
            for( sal_uInt16 j=0; j < rConditions.Count() && !bSendModify; j++ )
            {
                const SwCollCondition& rCond = *rConditions[j];
                switch( rCond.GetCondition() )
                {
                case PARA_IN_TABLEHEAD:
                case PARA_IN_TABLEBODY:
                case PARA_IN_FOOTER:
                case PARA_IN_HEADER:
                    bSendModify = sal_True;
                    break;
                }
            }
            if( bSendModify )
            {
                SwCondCollCondChg aMsg( pColl );
                pColl->Modify( &aMsg, &aMsg );
            }
        }
    }
}

sal_Bool SwXMLImport::FindAutomaticStyle(
        sal_uInt16 nFamily,
        const OUString& rName,
        const SfxItemSet **ppItemSet,
        OUString *pParent ) const
{
    SwXMLItemSetStyleContext_Impl *pStyle = 0;
    if( GetAutoStyles() )
    {
        pStyle = PTR_CAST( SwXMLItemSetStyleContext_Impl,
              GetAutoStyles()->
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

                // resolve data style name late
                if( XML_STYLE_FAMILY_TABLE_CELL == pStyle->GetFamily() &&
                    pStyle->ResolveDataStyleName() )
                {
                    (*ppItemSet) = pStyle->GetItemSet();
                }
                    
            }

            if( pParent )
                *pParent = pStyle->GetParent();
        }
    }
    
    return pStyle != 0;
}
}
