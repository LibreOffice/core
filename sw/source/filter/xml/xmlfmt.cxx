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

#include <rtl/ustrbuf.hxx>

#include <xmloff/nmspmap.hxx>
#include <format.hxx>
#include <fmtcol.hxx>
#include <hints.hxx>
#include <poolfmt.hxx>
#include <charfmt.hxx>
#include <paratr.hxx>
#include <doc.hxx>
#include "docary.hxx"
#include <IDocumentStylePoolAccess.hxx>
#include "unostyle.hxx"
#include "fmtpdsc.hxx"
#include "pagedesc.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/i18nmap.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmlitem.hxx"
#include <xmloff/xmlstyle.hxx>
#include <xmloff/txtstyli.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/XMLTextMasterStylesContext.hxx>
#include <xmloff/XMLTextShapeStyleContext.hxx>
#include <xmloff/XMLGraphicsDefaultStyle.hxx>
#include "xmlimp.hxx"
#include "xmltbli.hxx"
#include "cellatr.hxx"
#include <SwStyleNameMapper.hxx>
#include <xmloff/attrlist.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

class SwXMLConditionParser_Impl
{
    OUString sInput;

    sal_uInt32 nCondition;
    sal_uInt32 nSubCondition;

    sal_Int32 nPos;
    sal_Int32 nLength;

    inline bool SkipWS();
    inline bool MatchChar( sal_Unicode c );
    inline bool MatchName( OUString& rName );
    inline bool MatchNumber( sal_uInt32& rNumber );

public:

    explicit SwXMLConditionParser_Impl( const OUString& rInp );

    bool IsValid() const { return 0 != nCondition; }

    sal_uInt32 GetCondition() const { return nCondition; }
    sal_uInt32 GetSubCondition() const { return nSubCondition; }
};

inline bool SwXMLConditionParser_Impl::SkipWS()
{
    while( nPos < nLength && ' ' == sInput[nPos] )
        nPos++;
    return true;
}

inline bool SwXMLConditionParser_Impl::MatchChar( sal_Unicode c )
{
    bool bRet = false;
    if( nPos < nLength && c == sInput[nPos] )
    {
        nPos++;
        bRet = true;
    }
    return bRet;
}

inline bool SwXMLConditionParser_Impl::MatchName( OUString& rName )
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
    return !rName.isEmpty();
}

inline bool SwXMLConditionParser_Impl::MatchNumber( sal_uInt32& rNumber )
{
    OUStringBuffer sBuffer( nLength );
    while( nPos < nLength && '0' <= sInput[nPos] && sInput[nPos] <= '9' )
    {
        sBuffer.append( sInput[nPos] );
        nPos++;
    }

    OUString sNum( sBuffer.makeStringAndClear() );
    if( !sNum.isEmpty() )
        rNumber = sNum.toInt32();
    return !sNum.isEmpty();
}

SwXMLConditionParser_Impl::SwXMLConditionParser_Impl( const OUString& rInp ) :
    sInput( rInp ),
    nCondition( 0 ),
    nSubCondition( 0 ),
    nPos( 0 ),
    nLength( rInp.getLength() )
{
    OUString sFunc;
    bool bHasSub = false;
    sal_uInt32 nSub = 0;
    bool bOK = SkipWS() && MatchName( sFunc ) && SkipWS() &&
               MatchChar( '(' ) && SkipWS() && MatchChar( ')' ) && SkipWS();
    if( bOK && MatchChar( '=' ) )
    {
        bOK = SkipWS() && MatchNumber( nSub ) && SkipWS();
        bHasSub = true;
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

class SwXMLConditionContext_Impl : public SvXMLImportContext
{
    sal_uInt32 nCondition;
    sal_uInt32 nSubCondition;

    OUString sApplyStyle;

public:

    SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLConditionContext_Impl();

    TYPEINFO_OVERRIDE();

    bool IsValid() const { return 0 != nCondition; }

    sal_uInt32 GetCondition() const { return nCondition; }
    sal_uInt32 GetSubCondition() const { return nSubCondition; }
    const OUString& GetApplyStyle() const { return sApplyStyle; }
};

SwXMLConditionContext_Impl::SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nCondition( 0 ),
    nSubCondition( 0 )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        const sal_uInt16 nPrefix =
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

TYPEINIT1( SwXMLConditionContext_Impl, XMLTextStyleContext );

typedef std::vector<SwXMLConditionContext_Impl*> SwXMLConditions_Impl;

class SwXMLTextStyleContext_Impl : public XMLTextStyleContext
{
    SwXMLConditions_Impl    *pConditions;

protected:

    virtual uno::Reference < style::XStyle > Create() SAL_OVERRIDE;

public:

    TYPEINFO_OVERRIDE();

    SwXMLTextStyleContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nFamily,
            SvXMLStylesContext& rStyles );
    virtual ~SwXMLTextStyleContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    virtual void Finish( bool bOverwrite ) SAL_OVERRIDE;
};

TYPEINIT1( SwXMLTextStyleContext_Impl, XMLTextStyleContext );

uno::Reference < style::XStyle > SwXMLTextStyleContext_Impl::Create()
{
    uno::Reference < style::XStyle > xNewStyle;

    if( pConditions && XML_STYLE_FAMILY_TEXT_PARAGRAPH == GetFamily() )
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    uno::UNO_QUERY );
        if( xFactory.is() )
        {
            uno::Reference < uno::XInterface > xIfc =
                xFactory->createInstance( "com.sun.star.style.ConditionalParagraphStyle" );
            if( xIfc.is() )
                xNewStyle = uno::Reference < style::XStyle >( xIfc, uno::UNO_QUERY );
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
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
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
        while( !pConditions->empty() )
        {
            SwXMLConditionContext_Impl *pCond = &*pConditions->back();
            pConditions->pop_back();
            pCond->ReleaseRef();
        }
        delete pConditions;
    }
}

SvXMLImportContext *SwXMLTextStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
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
            pConditions->push_back( pCond );
            pCond->AddFirstRef();
        }
        pContext = pCond;
    }

    if( !pContext )
        pContext = XMLTextStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void SwXMLTextStyleContext_Impl::Finish( bool bOverwrite )
{
    XMLTextStyleContext::Finish( bOverwrite );

    if( !pConditions || XML_STYLE_FAMILY_TEXT_PARAGRAPH != GetFamily() )
        return;

    uno::Reference < style::XStyle > xStyle = GetStyle();
    if( !xStyle.is() )
        return;

    const SwXStyle* pStyle = 0;
    uno::Reference<lang::XUnoTunnel> xStyleTunnel( xStyle, uno::UNO_QUERY);
    if( xStyleTunnel.is() )
    {
        pStyle = reinterpret_cast< SwXStyle * >(
                sal::static_int_cast< sal_IntPtr >( xStyleTunnel->getSomething( SwXStyle::getUnoTunnelId() )));
    }
    if( !pStyle )
        return;

    const SwDoc *pDoc = pStyle->GetDoc();

    SwTextFormatColl *pColl = pDoc->FindTextFormatCollByName( pStyle->GetStyleName() );
    OSL_ENSURE( pColl, "Text collection not found" );
    if( !pColl || RES_CONDTXTFMTCOLL != pColl->Which() )
        return;

    const size_t nCount = pConditions->size();
    OUString sName;
    for( size_t i = 0; i < nCount; i++ )
    {
        const SwXMLConditionContext_Impl *pCond = (*pConditions)[i];
        const OUString aDisplayName(
            GetImport().GetStyleDisplayName( XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                pCond->GetApplyStyle() ) );
        SwStyleNameMapper::FillUIName(aDisplayName,
                                      sName,
                                      nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL,
                                      true);
        SwTextFormatColl* pCondColl = pDoc->FindTextFormatCollByName( sName );
        OSL_ENSURE( pCondColl,
            "SwXMLItemSetStyleContext_Impl::ConnectConditions: cond coll missing" );
        if( pCondColl )
        {
            SwCollCondition aCond( pCondColl, pCond->GetCondition(),
                                              pCond->GetSubCondition() );
            static_cast<SwConditionTextFormatColl*>(pColl)->InsertCondition( aCond );
        }
    }
}

class SwXMLItemSetStyleContext_Impl : public SvXMLStyleContext
{
    OUString                sMasterPageName;
    SfxItemSet              *pItemSet;
    SwXMLTextStyleContext_Impl *pTextStyle;
    SvXMLStylesContext      &rStyles;

    OUString                sDataStyleName;

    bool                bHasMasterPageName : 1;
    bool                bPageDescConnected : 1;
    bool                bDataStyleIsResolved;

    SvXMLImportContext *CreateItemSetContext(
            sal_uInt16 nPrefix,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList);

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) SAL_OVERRIDE;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:

    TYPEINFO_OVERRIDE();

    SwXMLItemSetStyleContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStylesC,
            sal_uInt16 nFamily);
    virtual ~SwXMLItemSetStyleContext_Impl();

    virtual void CreateAndInsert( bool bOverwrite ) SAL_OVERRIDE;

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    // The item set may be empty!
    SfxItemSet *GetItemSet() { return pItemSet; }

    const OUString& GetMasterPageName() const { return sMasterPageName; }
    bool HasMasterPageName() const { return bHasMasterPageName; }

    bool IsPageDescConnected() const { return bPageDescConnected; }
    void ConnectPageDesc();

    bool ResolveDataStyleName();
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
            bHasMasterPageName = true;
        }
        else if ( IsXMLToken( rLocalName, XML_DATA_STYLE_NAME ) )
        {
            // if we have a valid data style name
            if (!rValue.isEmpty())
            {
                sDataStyleName = rValue;
                bDataStyleIsResolved = false;   // needs to be resolved
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
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    OSL_ENSURE( !pItemSet,
            "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: item set exists" );

    SvXMLImportContext *pContext = 0;

    SwDoc* pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );

    SfxItemPool& rItemPool = pDoc->GetAttrPool();
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
        OSL_ENSURE( false,
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
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        SvXMLStylesContext& rStylesC,
        sal_uInt16 nFamily ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, nFamily ),
    pItemSet( 0 ),
    pTextStyle( 0 ),
    rStyles( rStylesC ),
    bHasMasterPageName( false ),
    bPageDescConnected( false ),
    bDataStyleIsResolved( true )
{
}

SwXMLItemSetStyleContext_Impl::~SwXMLItemSetStyleContext_Impl()
{
    delete pItemSet;
}

void SwXMLItemSetStyleContext_Impl::CreateAndInsert( bool bOverwrite )
{
    if( pTextStyle )
        pTextStyle->CreateAndInsert( bOverwrite );
}

SvXMLImportContext *SwXMLItemSetStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_TABLE_PROPERTIES ) ||
            IsXMLToken( rLocalName, XML_TABLE_COLUMN_PROPERTIES ) ||
            IsXMLToken( rLocalName, XML_TABLE_ROW_PROPERTIES ) ||
            IsXMLToken( rLocalName, XML_TABLE_CELL_PROPERTIES ) )
        {
            pContext = CreateItemSetContext( nPrefix, rLocalName, xAttrList );
        }
        else if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) ||
                 IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES ))
        {
            if( !pTextStyle )
            {
                SvXMLAttributeList *pTmp = new SvXMLAttributeList;
                const OUString aStr = GetImport().GetNamespaceMap().GetQNameByKey(
                    nPrefix, GetXMLToken(XML_NAME) );
                pTmp->AddAttribute( aStr, GetName() );
                uno::Reference <xml::sax::XAttributeList> xTmpAttrList = pTmp;
                pTextStyle = new SwXMLTextStyleContext_Impl( GetSwImport(), nPrefix,
                                 rLocalName, xTmpAttrList, XML_STYLE_FAMILY_TEXT_PARAGRAPH, rStyles );
                pTextStyle->StartElement( xTmpAttrList );
                rStyles.AddStyle( *pTextStyle );
            }
            pContext = pTextStyle->CreateChildContext( nPrefix, rLocalName, xAttrList );
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
    bPageDescConnected = true;

    SwDoc *pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );

    // #i40788# - first determine the display name of the page style,
    // then map this name to the corresponding user interface name.
    OUString sName = GetImport().GetStyleDisplayName( XML_STYLE_FAMILY_MASTER_PAGE,
                                             GetMasterPageName() );
    SwStyleNameMapper::FillUIName( sName,
                                   sName,
                                   nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC,
                                   true);
    SwPageDesc *pPageDesc = pDoc->FindPageDesc(sName);
    if( !pPageDesc )
    {
        // If the page style is a pool style, then we maybe have to create it
        // first if it hasn't been used by now.
        const sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( sName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
        if( USHRT_MAX != nPoolId )
            pPageDesc = pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( nPoolId, false );
    }

    if( !pPageDesc )
        return;

    if( !pItemSet )
    {
        SfxItemPool& rItemPool = pDoc->GetAttrPool();
        pItemSet = new SfxItemSet( rItemPool, aTableSetRange );
    }

    const SfxPoolItem *pItem;
    SwFormatPageDesc *pFormatPageDesc = 0;
    if( SfxItemState::SET == pItemSet->GetItemState( RES_PAGEDESC, false,
                                                &pItem ) )
    {
         if( static_cast<const SwFormatPageDesc *>(pItem)->GetPageDesc() != pPageDesc )
            pFormatPageDesc = new SwFormatPageDesc( *static_cast<const SwFormatPageDesc *>(pItem) );
    }
    else
        pFormatPageDesc = new SwFormatPageDesc();

    if( pFormatPageDesc )
    {
        pFormatPageDesc->RegisterToPageDesc( *pPageDesc );
        pItemSet->Put( *pFormatPageDesc );
        delete pFormatPageDesc;
    }
}

bool SwXMLItemSetStyleContext_Impl::ResolveDataStyleName()
{
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
                SwDoc *pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );

                SfxItemPool& rItemPool = pDoc->GetAttrPool();
                pItemSet = new SfxItemSet( rItemPool, aTableBoxSetRange );
            }
            SwTableBoxNumFormat aNumFormatItem(nFormat);
            pItemSet->Put(aNumFormatItem);
        }

        // now resolved
        bDataStyleIsResolved = true;
        return true;
    }
    else
    {
        // was already resolved; nothing to do
        return false;
    }
}

class SwXMLStylesContext_Impl : public SvXMLStylesContext
{
    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
    const SwXMLImport& GetSwImport() const
            { return static_cast<const SwXMLImport&>(GetImport()); }

protected:

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;
    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;
    // HACK
    virtual rtl::Reference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
        sal_uInt16 nFamily ) const SAL_OVERRIDE;

    virtual uno::Reference < container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const SAL_OVERRIDE;
    virtual OUString GetServiceName( sal_uInt16 nFamily ) const SAL_OVERRIDE;
    // HACK

public:

    TYPEINFO_OVERRIDE();

    SwXMLStylesContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName ,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            bool bAuto );
    virtual ~SwXMLStylesContext_Impl();

    virtual bool InsertStyleFamily( sal_uInt16 nFamily ) const SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

TYPEINIT1( SwXMLStylesContext_Impl, SvXMLStylesContext );

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
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
                            rLocalName, xAttrList, *this, nFamily  );
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
    case XML_STYLE_FAMILY_TABLE_TABLE:
    case XML_STYLE_FAMILY_TABLE_ROW:
        pStyle = new XMLTextStyleContext( GetImport(), nPrefix, rLocalName,
                                          xAttrList, *this, nFamily,
                                          true );
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
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        bool bAuto ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList, bAuto )
{
}

SwXMLStylesContext_Impl::~SwXMLStylesContext_Impl()
{
}

bool SwXMLStylesContext_Impl::InsertStyleFamily( sal_uInt16 nFamily ) const
{
    const SwXMLImport& rSwImport = GetSwImport();
    const sal_uInt16 nStyleFamilyMask = rSwImport.GetStyleFamilyMask();

    bool bIns = true;
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

rtl::Reference < SvXMLImportPropertyMapper > SwXMLStylesContext_Impl::GetImportPropertyMapper(
        sal_uInt16 nFamily ) const
{
    rtl::Reference < SvXMLImportPropertyMapper > xMapper;
    if( nFamily == XML_STYLE_FAMILY_TABLE_TABLE )
        xMapper = XMLTextImportHelper::CreateTableDefaultExtPropMapper(
            const_cast<SwXMLStylesContext_Impl*>( this )->GetImport() );
    else if( nFamily == XML_STYLE_FAMILY_TABLE_ROW )
        xMapper = XMLTextImportHelper::CreateTableRowDefaultExtPropMapper(
            const_cast<SwXMLStylesContext_Impl*>( this )->GetImport() );
    else
        xMapper = SvXMLStylesContext::GetImportPropertyMapper( nFamily );
    return xMapper;
}

uno::Reference < container::XNameContainer > SwXMLStylesContext_Impl::GetStylesContainer(
                                                sal_uInt16 nFamily ) const
{
    uno::Reference < container::XNameContainer > xStyles;
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        xStyles = const_cast<SvXMLImport *>(&GetImport())->GetTextImport()->GetFrameStyles();
    else
        xStyles = SvXMLStylesContext::GetStylesContainer( nFamily );

    return xStyles;
}

OUString SwXMLStylesContext_Impl::GetServiceName( sal_uInt16 nFamily ) const
{
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        return OUString( "com.sun.star.style.FrameStyle" );

    return SvXMLStylesContext::GetServiceName( nFamily );
}

void SwXMLStylesContext_Impl::EndElement()
{
    GetSwImport().InsertStyles( IsAutomaticStyle() );
}

class SwXMLMasterStylesContext_Impl : public XMLTextMasterStylesContext
{
protected:
    virtual bool InsertStyleFamily( sal_uInt16 nFamily ) const SAL_OVERRIDE;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
    const SwXMLImport& GetSwImport() const
            { return static_cast<const SwXMLImport&>(GetImport()); }

public:

    TYPEINFO_OVERRIDE();

    SwXMLMasterStylesContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName ,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLMasterStylesContext_Impl();
    virtual void EndElement() SAL_OVERRIDE;
};

TYPEINIT1( SwXMLMasterStylesContext_Impl, XMLTextMasterStylesContext );

SwXMLMasterStylesContext_Impl::SwXMLMasterStylesContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName ,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) :
    XMLTextMasterStylesContext( rImport, nPrfx, rLName, xAttrList )
{
}

SwXMLMasterStylesContext_Impl::~SwXMLMasterStylesContext_Impl()
{
}

bool SwXMLMasterStylesContext_Impl::InsertStyleFamily( sal_uInt16 nFamily ) const
{
    bool bIns;

    const SwXMLImport& rSwImport = GetSwImport();
    const sal_uInt16 nStyleFamilyMask = rSwImport.GetStyleFamilyMask();
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

SvXMLImportContext *SwXMLImport::CreateStylesContext(
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        bool bAuto )
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
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStylesContext *pContext =
        new SwXMLMasterStylesContext_Impl( *this, XML_NAMESPACE_OFFICE, rLocalName,
                                          xAttrList );
    SetMasterStyles( pContext );

    return pContext;
}

void SwXMLImport::InsertStyles( bool bAuto )
{
    if( bAuto && GetAutoStyles() )
        GetAutoStyles()->CopyAutoStylesToDoc();
    if( !bAuto && GetStyles() )
        GetStyles()->CopyStylesToDoc( !IsInsertMode(), false );
}

void SwXMLImport::FinishStyles()
{
    if( GetStyles() )
        GetStyles()->FinishStyles( !IsInsertMode() );
}

void SwXMLImport::UpdateTextCollConditions( SwDoc *pDoc )
{
    if( !pDoc )
        pDoc = SwImport::GetDocFromXMLImport( *this );

    const SwTextFormatColls& rColls = *pDoc->GetTextFormatColls();
    const size_t nCount = rColls.size();
    for( size_t i=0; i < nCount; ++i )
    {
        SwTextFormatColl *pColl = rColls[i];
        if( pColl && RES_CONDTXTFMTCOLL == pColl->Which() )
        {
            const SwFormatCollConditions& rConditions =
                static_cast<const SwConditionTextFormatColl *>(pColl)->GetCondColls();
            bool bSendModify = false;
            for( size_t j=0; j < rConditions.size() && !bSendModify; ++j )
            {
                const SwCollCondition& rCond = *rConditions[j];
                switch( rCond.GetCondition() )
                {
                case PARA_IN_TABLEHEAD:
                case PARA_IN_TABLEBODY:
                case PARA_IN_FOOTER:
                case PARA_IN_HEADER:
                    bSendModify = true;
                    break;
                }
            }
            if( bSendModify )
            {
                SwCondCollCondChg aMsg( pColl );
                pColl->ModifyNotification( &aMsg, &aMsg );
            }
        }
    }
}

bool SwXMLImport::FindAutomaticStyle(
        sal_uInt16 nFamily,
        const OUString& rName,
        const SfxItemSet **ppItemSet,
        OUString *pParent ) const
{
    SwXMLItemSetStyleContext_Impl *pStyle = 0;
    if( GetAutoStyles() )
    {
        pStyle = const_cast<SwXMLItemSetStyleContext_Impl*>(dynamic_cast< const SwXMLItemSetStyleContext_Impl* >(
              GetAutoStyles()->
                    FindStyleChildContext( nFamily, rName,
                                           true ) ) );
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
                *pParent = pStyle->GetParentName();
        }
    }

    return pStyle != 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
