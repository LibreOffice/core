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
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <xmloff/nmspmap.hxx>
#include <format.hxx>
#include <fmtcol.hxx>
#include <hints.hxx>
#include <poolfmt.hxx>
#include <charfmt.hxx>
#include <paratr.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <unostyle.hxx>
#include <unoprnms.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
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
#include <xmloff/table/XMLTableImport.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "xmlimp.hxx"
#include "xmltbli.hxx"
#include <cellatr.hxx>
#include <SwStyleNameMapper.hxx>
#include <xmloff/attrlist.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <ccoll.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

class SwXMLConditionParser_Impl
{
    OUString const sInput;

    Master_CollCondition nCondition;
    sal_uInt32 nSubCondition;

    sal_Int32 nPos;
    sal_Int32 const nLength;

    inline bool SkipWS();
    inline bool MatchChar( sal_Unicode c );
    inline bool MatchName( OUString& rName );
    inline bool MatchNumber( sal_uInt32& rNumber );

public:

    explicit SwXMLConditionParser_Impl( const OUString& rInp );

    bool IsValid() const { return Master_CollCondition::NONE != nCondition; }

    Master_CollCondition GetCondition() const { return nCondition; }
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
    nCondition( Master_CollCondition::NONE ),
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
            nCondition = Master_CollCondition::PARA_IN_ENDNOTE;
        else if( IsXMLToken( sFunc, XML_FOOTER ) && !bHasSub )
            nCondition = Master_CollCondition::PARA_IN_FOOTER;
        else if( IsXMLToken( sFunc, XML_FOOTNOTE ) && !bHasSub )
            nCondition = Master_CollCondition::PARA_IN_FOOTNOTE;
        else if( IsXMLToken( sFunc, XML_HEADER ) && !bHasSub )
            nCondition = Master_CollCondition::PARA_IN_HEADER;
        else if( IsXMLToken( sFunc, XML_LIST_LEVEL) &&
                nSub >=1 && nSub <= MAXLEVEL )
        {
            nCondition = Master_CollCondition::PARA_IN_LIST;
            nSubCondition = nSub-1;
        }
        else if( IsXMLToken( sFunc, XML_OUTLINE_LEVEL) &&
                 nSub >=1 && nSub <= MAXLEVEL )
        {
            nCondition = Master_CollCondition::PARA_IN_OUTLINE;
            nSubCondition = nSub-1;
        }
        else if( IsXMLToken( sFunc, XML_SECTION ) && !bHasSub )
        {
            nCondition = Master_CollCondition::PARA_IN_SECTION;
        }
        else if( IsXMLToken( sFunc, XML_TABLE ) && !bHasSub )
        {
            nCondition = Master_CollCondition::PARA_IN_TABLEBODY;
        }
        else if( IsXMLToken( sFunc, XML_TABLE_HEADER ) && !bHasSub )
        {
            nCondition = Master_CollCondition::PARA_IN_TABLEHEAD;
        }
        else if( IsXMLToken( sFunc, XML_TEXT_BOX ) && !bHasSub )
        {
            nCondition = Master_CollCondition::PARA_IN_FRAME;
        }
    }
}

class SwXMLConditionContext_Impl : public SvXMLImportContext
{
    Master_CollCondition nCondition;
    sal_uInt32 nSubCondition;

    OUString sApplyStyle;

public:

    SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList );

    bool IsValid() const { return Master_CollCondition::NONE != nCondition; }

    Master_CollCondition getCondition() const { return nCondition; }
    sal_uInt32 getSubCondition() const { return nSubCondition; }
    OUString const &getApplyStyle() const { return sApplyStyle; }
};

SwXMLConditionContext_Impl::SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nCondition( Master_CollCondition::NONE ),
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

typedef std::vector<rtl::Reference<SwXMLConditionContext_Impl>> SwXMLConditions_Impl;

class SwXMLTextStyleContext_Impl : public XMLTextStyleContext
{
    std::unique_ptr<SwXMLConditions_Impl> pConditions;
    uno::Reference < style::XStyle > xNewStyle;

protected:

    virtual uno::Reference < style::XStyle > Create() override;
    virtual void Finish( bool bOverwrite ) override;

public:


    SwXMLTextStyleContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nFamily,
            SvXMLStylesContext& rStyles );

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override;
};


uno::Reference < style::XStyle > SwXMLTextStyleContext_Impl::Create()
{

    if( pConditions && XML_STYLE_FAMILY_TEXT_PARAGRAPH == GetFamily() )
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    uno::UNO_QUERY );
        if( xFactory.is() )
        {
            uno::Reference < uno::XInterface > xIfc =
                xFactory->createInstance( "com.sun.star.style.ConditionalParagraphStyle" );
            if( xIfc.is() )
                xNewStyle.set( xIfc, uno::UNO_QUERY );
        }
    }
    else
    {
        xNewStyle = XMLTextStyleContext::Create();
    }

    return xNewStyle;
}

void
SwXMLTextStyleContext_Impl::Finish( bool bOverwrite )
{

    if( pConditions && XML_STYLE_FAMILY_TEXT_PARAGRAPH == GetFamily() && xNewStyle.is() )
    {
        CommandStruct const*const pCommands = SwCondCollItem::GetCmds();

        Reference< XPropertySet > xPropSet( xNewStyle, UNO_QUERY );

        uno::Sequence< beans::NamedValue > aSeq( pConditions->size() );

        for (std::vector<rtl::Reference<SwXMLConditionContext_Impl>>::size_type i = 0;
            i < pConditions->size(); ++i)
        {
            assert((*pConditions)[i]->IsValid()); // checked before inserting
            Master_CollCondition nCond = (*pConditions)[i]->getCondition();
            sal_uInt32 nSubCond = (*pConditions)[i]->getSubCondition();

            for (size_t j = 0; j < COND_COMMAND_COUNT; ++j)
            {
                if (pCommands[j].nCnd == nCond &&
                    pCommands[j].nSubCond == nSubCond)
                {
                    aSeq[i].Name = GetCommandContextByIndex( j );
                    aSeq[i].Value <<= GetImport().GetStyleDisplayName(
                            GetFamily(), (*pConditions)[i]->getApplyStyle() );
                    break;
                }
            }
        }

        try
        {
            xPropSet->setPropertyValue(UNO_NAME_PARA_STYLE_CONDITIONS, uno::makeAny(aSeq));
        }
        catch (uno::Exception const&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("sw.xml", "exception when setting ParaStyleConditions: " << exceptionToString(ex));
        }
    }
    XMLTextStyleContext::Finish( bOverwrite );
}

SwXMLTextStyleContext_Impl::SwXMLTextStyleContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        sal_uInt16 nFamily,
        SvXMLStylesContext& rStyles ) :
    XMLTextStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily )
{
}

SvXMLImportContextRef SwXMLTextStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext;

    if( XML_NAMESPACE_STYLE == nPrefix && IsXMLToken( rLocalName, XML_MAP ) )
    {
        rtl::Reference<SwXMLConditionContext_Impl> xCond{
            new SwXMLConditionContext_Impl( GetImport(), nPrefix,
                                            rLocalName, xAttrList )};
        if( xCond->IsValid() )
        {
            if( !pConditions )
               pConditions = std::make_unique<SwXMLConditions_Impl>();
            pConditions->push_back( xCond );
        }
        xContext = xCond.get();
    }

    if (!xContext)
        xContext = XMLTextStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return xContext;
}

class SwXMLItemSetStyleContext_Impl : public SvXMLStyleContext
{
    OUString                    sMasterPageName;
    std::unique_ptr<SfxItemSet> pItemSet;
    SwXMLTextStyleContext_Impl *pTextStyle;
    SvXMLStylesContext          &rStyles;

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
                               const OUString& rValue ) override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:


    SwXMLItemSetStyleContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStylesC,
            sal_uInt16 nFamily);

    virtual void CreateAndInsert( bool bOverwrite ) override;

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override;

    // The item set may be empty!
    SfxItemSet *GetItemSet() { return pItemSet.get(); }

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

    SvXMLImportContext *pContext = nullptr;

    SwDoc* pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );

    SfxItemPool& rItemPool = pDoc->GetAttrPool();
    switch( GetFamily() )
    {
    case XML_STYLE_FAMILY_TABLE_TABLE:
        pItemSet.reset( new SfxItemSet( rItemPool, aTableSetRange ) );
        break;
    case XML_STYLE_FAMILY_TABLE_COLUMN:
        pItemSet.reset( new SfxItemSet( rItemPool, svl::Items<RES_FRM_SIZE, RES_FRM_SIZE>{} ) );
        break;
    case XML_STYLE_FAMILY_TABLE_ROW:
        pItemSet.reset( new SfxItemSet( rItemPool, aTableLineSetRange ) );
        break;
    case XML_STYLE_FAMILY_TABLE_CELL:
        pItemSet.reset( new SfxItemSet( rItemPool, aTableBoxSetRange ) );
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
        pItemSet.reset();
    }

    return pContext;
}


SwXMLItemSetStyleContext_Impl::SwXMLItemSetStyleContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        SvXMLStylesContext& rStylesC,
        sal_uInt16 nFamily ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, nFamily ),
    pTextStyle( nullptr ),
    rStyles( rStylesC ),
    bHasMasterPageName( false ),
    bPageDescConnected( false ),
    bDataStyleIsResolved( true )
{
}

void SwXMLItemSetStyleContext_Impl::CreateAndInsert( bool bOverwrite )
{
    if( pTextStyle )
        pTextStyle->CreateAndInsert( bOverwrite );
}

SvXMLImportContextRef SwXMLItemSetStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_TABLE_PROPERTIES ) ||
            IsXMLToken( rLocalName, XML_TABLE_COLUMN_PROPERTIES ) ||
            IsXMLToken( rLocalName, XML_TABLE_ROW_PROPERTIES ) ||
            IsXMLToken( rLocalName, XML_TABLE_CELL_PROPERTIES ) )
        {
            xContext = CreateItemSetContext( nPrefix, rLocalName, xAttrList );
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
            xContext = pTextStyle->CreateChildContext( nPrefix, rLocalName, xAttrList );
        }
    }

    if (!xContext)
        xContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return xContext;
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
                                             sMasterPageName );
    SwStyleNameMapper::FillUIName( sName,
                                   sName,
                                   SwGetPoolIdFromName::PageDesc);
    SwPageDesc *pPageDesc = pDoc->FindPageDesc(sName);
    if( !pPageDesc )
    {
        // If the page style is a pool style, then we maybe have to create it
        // first if it hasn't been used by now.
        const sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( sName, SwGetPoolIdFromName::PageDesc );
        if( USHRT_MAX != nPoolId )
            pPageDesc = pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( nPoolId, false );
    }

    if( !pPageDesc )
        return;

    if( !pItemSet )
    {
        SfxItemPool& rItemPool = pDoc->GetAttrPool();
        pItemSet.reset( new SfxItemSet( rItemPool, aTableSetRange ) );
    }

    const SfxPoolItem *pItem;
    std::unique_ptr<SwFormatPageDesc> pFormatPageDesc;
    if( SfxItemState::SET == pItemSet->GetItemState( RES_PAGEDESC, false,
                                                &pItem ) )
    {
         if( static_cast<const SwFormatPageDesc *>(pItem)->GetPageDesc() != pPageDesc )
            pFormatPageDesc.reset(new SwFormatPageDesc( *static_cast<const SwFormatPageDesc *>(pItem) ));
    }
    else
        pFormatPageDesc.reset(new SwFormatPageDesc());

    if( pFormatPageDesc )
    {
        pFormatPageDesc->RegisterToPageDesc( *pPageDesc );
        pItemSet->Put( *pFormatPageDesc );
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
                pItemSet.reset( new SfxItemSet( rItemPool, aTableBoxSetRange ) );
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

    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override;
    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override;
    // HACK
    virtual rtl::Reference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
        sal_uInt16 nFamily ) const override;

    virtual uno::Reference < container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const override;
    virtual OUString GetServiceName( sal_uInt16 nFamily ) const override;
    // HACK

public:


    SwXMLStylesContext_Impl(
            SwXMLImport& rImport,
            const OUString& rLName ,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            bool bAuto );

    virtual bool InsertStyleFamily( sal_uInt16 nFamily ) const override;

    virtual void EndElement() override;
};

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateStyleChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext* pContext = nullptr;

    if(nPrefix == XML_NAMESPACE_TABLE && IsXMLToken(rLocalName, XML_TABLE_TEMPLATE))
    {
        rtl::Reference<XMLTableImport> xTableImport = GetImport().GetShapeImport()->GetShapeTableImport();
        pContext = xTableImport->CreateTableTemplateContext(nPrefix, rLocalName, xAttrList);
    }
    if (!pContext)
        pContext = SvXMLStylesContext::CreateStyleChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = nullptr;

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
        // Distinguish real and automatic styles.
        if (IsAutomaticStyle())
            pStyle = new SwXMLItemSetStyleContext_Impl(GetSwImport(), nPrefix, rLocalName, xAttrList, *this, nFamily);
        else if (nFamily == XML_STYLE_FAMILY_TABLE_CELL) // Real cell styles are used for table-template import.
            pStyle = new XMLPropStyleContext(GetSwImport(), nPrefix, rLocalName, xAttrList, *this, nFamily);
        else
            SAL_WARN("sw.xml", "Context does not exists for non automatic table, column or row style.");
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
    SvXMLStyleContext *pStyle = nullptr;

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
        SwXMLImport& rImport, const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        bool bAuto ) :
    SvXMLStylesContext( rImport, XML_NAMESPACE_OFFICE, rLName, xAttrList, bAuto )
{
}

bool SwXMLStylesContext_Impl::InsertStyleFamily( sal_uInt16 nFamily ) const
{
    const SwXMLImport& rSwImport = GetSwImport();
    const SfxStyleFamily nStyleFamilyMask = rSwImport.GetStyleFamilyMask();

    bool bIns = true;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Para);
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Char);
        break;
    case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Frame);
        break;
    case XML_STYLE_FAMILY_TEXT_LIST:
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Pseudo);
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
    else if( nFamily == XML_STYLE_FAMILY_TABLE_CELL )
        xMapper = XMLTextImportHelper::CreateTableCellExtPropMapper(
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
    else if( XML_STYLE_FAMILY_TABLE_CELL == nFamily )
        xStyles = const_cast<SvXMLImport *>(&GetImport())->GetTextImport()->GetCellStyles();

    if (!xStyles.is())
        xStyles = SvXMLStylesContext::GetStylesContainer( nFamily );

    return xStyles;
}

OUString SwXMLStylesContext_Impl::GetServiceName( sal_uInt16 nFamily ) const
{
    if( XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily )
        return OUString( "com.sun.star.style.FrameStyle" );
    else if( XML_STYLE_FAMILY_TABLE_CELL == nFamily )
        return OUString( "com.sun.star.style.CellStyle" );

    return SvXMLStylesContext::GetServiceName( nFamily );
}

void SwXMLStylesContext_Impl::EndElement()
{
    GetSwImport().InsertStyles( IsAutomaticStyle() );
}

class SwXMLMasterStylesContext_Impl : public XMLTextMasterStylesContext
{
protected:
    virtual bool InsertStyleFamily( sal_uInt16 nFamily ) const override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
    const SwXMLImport& GetSwImport() const
            { return static_cast<const SwXMLImport&>(GetImport()); }

public:


    SwXMLMasterStylesContext_Impl(
            SwXMLImport& rImport,
            const OUString& rLName ,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement() override;
};


SwXMLMasterStylesContext_Impl::SwXMLMasterStylesContext_Impl(
        SwXMLImport& rImport,
        const OUString& rLName ,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) :
    XMLTextMasterStylesContext( rImport, XML_NAMESPACE_OFFICE, rLName, xAttrList )
{
}

bool SwXMLMasterStylesContext_Impl::InsertStyleFamily( sal_uInt16 nFamily ) const
{
    bool bIns;

    const SwXMLImport& rSwImport = GetSwImport();
    const SfxStyleFamily nStyleFamilyMask = rSwImport.GetStyleFamilyMask();
    if( XML_STYLE_FAMILY_MASTER_PAGE == nFamily )
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Page);
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
        new SwXMLStylesContext_Impl( *this, rLocalName,
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
        new SwXMLMasterStylesContext_Impl( *this, rLocalName, xAttrList );
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
                case Master_CollCondition::PARA_IN_TABLEHEAD:
                case Master_CollCondition::PARA_IN_TABLEBODY:
                case Master_CollCondition::PARA_IN_FOOTER:
                case Master_CollCondition::PARA_IN_HEADER:
                    bSendModify = true;
                    break;
                default: break;
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
        const SfxItemSet **ppItemSet ) const
{
    SwXMLItemSetStyleContext_Impl *pStyle = nullptr;
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
        }
    }

    return pStyle != nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
