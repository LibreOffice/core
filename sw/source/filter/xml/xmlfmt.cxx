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
#include <xmloff/namespacemap.hxx>
#include <fmtcol.hxx>
#include <hints.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <unoprnms.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/txtstyli.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/XMLTextMasterStylesContext.hxx>
#include <xmloff/XMLTextShapeStyleContext.hxx>
#include <xmloff/XMLGraphicsDefaultStyle.hxx>
#include <xmloff/XMLDrawingPageStyleContext.hxx>
#include <xmloff/XMLTextMasterPageContext.hxx>
#include <xmloff/table/XMLTableImport.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "xmlimp.hxx"
#include <cellatr.hxx>
#include <SwStyleNameMapper.hxx>
#include <xmloff/attrlist.hxx>
#include <ccoll.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

namespace {

class SwXMLConditionParser_Impl
{
    OUString sInput;

    Master_CollCondition nCondition;
    sal_uInt32 nSubCondition;

    sal_Int32 nPos;
    sal_Int32 nLength;

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

}

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

    if( !bOK )
        return;

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

namespace {

class SwXMLConditionContext_Impl : public SvXMLImportContext
{
    Master_CollCondition nCondition;
    sal_uInt32 nSubCondition;

    OUString sApplyStyle;

public:

    SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_Int32 nElement,
            const uno::Reference< xml::sax::XFastAttributeList > & xAttrList );

    bool IsValid() const { return Master_CollCondition::NONE != nCondition; }

    Master_CollCondition getCondition() const { return nCondition; }
    sal_uInt32 getSubCondition() const { return nSubCondition; }
    OUString const &getApplyStyle() const { return sApplyStyle; }
};

}

SwXMLConditionContext_Impl::SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_Int32 /*nElement*/,
            const uno::Reference< xml::sax::XFastAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport ),
    nCondition( Master_CollCondition::NONE ),
    nSubCondition( 0 )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
            case XML_ELEMENT(STYLE, XML_CONDITION):
            {
                SwXMLConditionParser_Impl aCondParser( sValue );
                if( aCondParser.IsValid() )
                {
                    nCondition = aCondParser.GetCondition();
                    nSubCondition = aCondParser.GetSubCondition();
                }
                break;
            }
            case XML_ELEMENT(STYLE, XML_APPLY_STYLE_NAME):
                sApplyStyle = sValue;
                break;
            default:
                SAL_WARN("sw", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

typedef std::vector<rtl::Reference<SwXMLConditionContext_Impl>> SwXMLConditions_Impl;

namespace {

class SwXMLTextStyleContext_Impl : public XMLTextStyleContext
{
    std::unique_ptr<SwXMLConditions_Impl> pConditions;
    uno::Reference < style::XStyle > xNewStyle;

protected:

    virtual uno::Reference < style::XStyle > Create() override;
    virtual void Finish( bool bOverwrite ) override;

public:


    SwXMLTextStyleContext_Impl( SwXMLImport& rImport, sal_Int32 nElement,
            const uno::Reference< xml::sax::XFastAttributeList > & xAttrList,
            XmlStyleFamily nFamily,
            SvXMLStylesContext& rStyles );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

}

uno::Reference < style::XStyle > SwXMLTextStyleContext_Impl::Create()
{

    if( pConditions && XmlStyleFamily::TEXT_PARAGRAPH == GetFamily() )
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

    if( pConditions && XmlStyleFamily::TEXT_PARAGRAPH == GetFamily() && xNewStyle.is() )
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
            TOOLS_WARN_EXCEPTION("sw.xml", "exception when setting ParaStyleConditions");
        }
    }
    XMLTextStyleContext::Finish( bOverwrite );
}

SwXMLTextStyleContext_Impl::SwXMLTextStyleContext_Impl( SwXMLImport& rImport,
        sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList,
        XmlStyleFamily nFamily,
        SvXMLStylesContext& rStyles ) :
    XMLTextStyleContext( rImport, nElement, xAttrList, rStyles, nFamily )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SwXMLTextStyleContext_Impl::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(STYLE, XML_MAP) )
    {
        rtl::Reference<SwXMLConditionContext_Impl> xCond{
            new SwXMLConditionContext_Impl( GetImport(), nElement, xAttrList )};
        if( xCond->IsValid() )
        {
            if( !pConditions )
               pConditions = std::make_unique<SwXMLConditions_Impl>();
            pConditions->push_back( xCond );
        }
        return xCond.get();
    }

    return XMLTextStyleContext::createFastChildContext( nElement, xAttrList );
}

namespace {

class SwXMLCellStyleContext : public XMLPropStyleContext
{
    OUString m_sDataStyleName;
    void AddDataFormat();
public:
    using XMLPropStyleContext::XMLPropStyleContext;
    virtual void FillPropertySet(const css::uno::Reference<css::beans::XPropertySet>& rPropSet) override;
    virtual void SetAttribute(sal_uInt16 nPrefixKey, const OUString& rLocalName, const OUString& rValue) override;
};

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
            sal_Int32 nElement,
            const uno::Reference< xml::sax::XFastAttributeList > & xAttrList);

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:


    SwXMLItemSetStyleContext_Impl(
            SwXMLImport& rImport, sal_Int32 nElement,
            const uno::Reference< xml::sax::XFastAttributeList > & xAttrList,
            SvXMLStylesContext& rStylesC,
            XmlStyleFamily nFamily);

    virtual void CreateAndInsert( bool bOverwrite ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    // The item set may be empty!
    SfxItemSet *GetItemSet() { return pItemSet.get(); }

    bool HasMasterPageName() const { return bHasMasterPageName; }

    bool IsPageDescConnected() const { return bPageDescConnected; }
    void ConnectPageDesc();

    bool ResolveDataStyleName();
};

}

void SwXMLCellStyleContext::AddDataFormat()
{
    if (m_sDataStyleName.isEmpty() || IsDefaultStyle())
        return;

    const SvXMLNumFormatContext* pStyle = static_cast<const SvXMLNumFormatContext*>(
        GetStyles()->FindStyleChildContext(XmlStyleFamily::DATA_STYLE, m_sDataStyleName, true));

    if (!pStyle)
    {
        SAL_WARN("sw.xml", "not possible to get data style " << m_sDataStyleName);
        return;
    }

    sal_Int32 nNumberFormat = const_cast<SvXMLNumFormatContext*>(pStyle)->GetKey();
    if (nNumberFormat < 0)
        return;

    rtl::Reference<SvXMLImportPropertyMapper> xPropertyMapper(GetStyles()->GetImportPropertyMapper(GetFamily()));
    if (!xPropertyMapper.is())
    {
        SAL_WARN("sw.xml", "there is no import prop mapper");
        return;
    }

    const rtl::Reference<XMLPropertySetMapper>& xPropertySetMapper(xPropertyMapper->getPropertySetMapper());
    sal_Int32 nIndex = xPropertySetMapper->GetEntryIndex(XML_NAMESPACE_STYLE, GetXMLToken(XML_DATA_STYLE_NAME), 0);
    if (nIndex < 0)
    {
        SAL_WARN("sw.xml", "could not find id for " << GetXMLToken(XML_DATA_STYLE_NAME));
        return;
    }

    auto aIter = std::find_if(GetProperties().begin(), GetProperties().end(),
        [&nIndex](const XMLPropertyState& rProp) {
            return rProp.mnIndex == nIndex;
        });

    if (aIter != GetProperties().end())
        aIter->maValue <<= nNumberFormat;
    else
        GetProperties().push_back(XMLPropertyState(nIndex, makeAny(nNumberFormat)));
}

void SwXMLCellStyleContext::FillPropertySet(const css::uno::Reference<css::beans::XPropertySet>& rPropSet)
{
    AddDataFormat();
    XMLPropStyleContext::FillPropertySet(rPropSet);
}

void SwXMLCellStyleContext::SetAttribute(sal_uInt16 nPrefixKey, const OUString& rLocalName, const OUString& rValue)
{
    if (IsXMLToken(rLocalName, XML_DATA_STYLE_NAME))
        m_sDataStyleName = rValue;
    else
        XMLPropStyleContext::SetAttribute(nPrefixKey, rLocalName, rValue);
}

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
        sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    OSL_ENSURE( !pItemSet,
            "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: item set exists" );

    SvXMLImportContext *pContext = nullptr;

    SwDoc* pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );

    SfxItemPool& rItemPool = pDoc->GetAttrPool();
    switch( GetFamily() )
    {
    case XmlStyleFamily::TABLE_TABLE:
        pItemSet.reset( new SfxItemSet( rItemPool, aTableSetRange ) );
        break;
    case XmlStyleFamily::TABLE_COLUMN:
        pItemSet.reset( new SfxItemSet( rItemPool, svl::Items<RES_FRM_SIZE, RES_FRM_SIZE>{} ) );
        break;
    case XmlStyleFamily::TABLE_ROW:
        pItemSet.reset( new SfxItemSet( rItemPool, aTableLineSetRange ) );
        break;
    case XmlStyleFamily::TABLE_CELL:
        pItemSet.reset( new SfxItemSet( rItemPool, aTableBoxSetRange ) );
        break;
    default:
        OSL_ENSURE( false,
        "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: unknown family" );
        break;
    }
    if( pItemSet )
        pContext = GetSwImport().CreateTableItemImportContext(
                                nElement, xAttrList, GetFamily(),
                                *pItemSet );
    if( !pContext )
    {
        pItemSet.reset();
    }

    return pContext;
}


SwXMLItemSetStyleContext_Impl::SwXMLItemSetStyleContext_Impl( SwXMLImport& rImport,
        sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList,
        SvXMLStylesContext& rStylesC,
        XmlStyleFamily nFamily ) :
    SvXMLStyleContext( rImport, nElement, xAttrList, nFamily ),
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

css::uno::Reference< css::xml::sax::XFastContextHandler > SwXMLItemSetStyleContext_Impl::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    switch (nElement)
    {
        case XML_ELEMENT(STYLE, XML_TABLE_PROPERTIES):
        case XML_ELEMENT(STYLE, XML_TABLE_COLUMN_PROPERTIES):
        case XML_ELEMENT(STYLE, XML_TABLE_ROW_PROPERTIES):
        case XML_ELEMENT(STYLE, XML_TABLE_CELL_PROPERTIES):
            return CreateItemSetContext( nElement, xAttrList );
            break;
        case XML_ELEMENT(STYLE, XML_TEXT_PROPERTIES):
        case XML_ELEMENT(STYLE, XML_PARAGRAPH_PROPERTIES):
        {
            if( !pTextStyle )
            {
                rtl::Reference<sax_fastparser::FastAttributeList> xTmpAttrList = new sax_fastparser::FastAttributeList(nullptr);
                xTmpAttrList->add(XML_ELEMENT(STYLE, XML_NAME), GetName().toUtf8() );
                pTextStyle = new SwXMLTextStyleContext_Impl( GetSwImport(), nElement,
                                 xTmpAttrList.get(), XmlStyleFamily::TEXT_PARAGRAPH, rStyles );
                pTextStyle->startFastElement( nElement, xTmpAttrList.get() );
                rStyles.AddStyle( *pTextStyle );
            }
            return pTextStyle->createFastChildContext( nElement, xAttrList );
        }
        default:
            SAL_WARN("sw", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    }

    return nullptr;
}

void SwXMLItemSetStyleContext_Impl::ConnectPageDesc()
{
    if( bPageDescConnected || !HasMasterPageName() )
        return;
    bPageDescConnected = true;

    SwDoc *pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );

    // #i40788# - first determine the display name of the page style,
    // then map this name to the corresponding user interface name.
    OUString sName = GetImport().GetStyleDisplayName( XmlStyleFamily::MASTER_PAGE,
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

namespace {

class SwXMLStylesContext_Impl : public SvXMLStylesContext
{
    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
    const SwXMLImport& GetSwImport() const
            { return static_cast<const SwXMLImport&>(GetImport()); }

protected:

    using SvXMLStylesContext::CreateStyleChildContext;
    virtual SvXMLStyleContext *CreateStyleChildContext( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;

    using SvXMLStylesContext::CreateStyleStyleChildContext;
    virtual SvXMLStyleContext *CreateStyleStyleChildContext( XmlStyleFamily nFamily,
        sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList ) override;
    using SvXMLStylesContext::CreateDefaultStyleStyleChildContext;
    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList ) override;
    // HACK
    virtual rtl::Reference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
        XmlStyleFamily nFamily ) const override;

    virtual uno::Reference < container::XNameContainer >
        GetStylesContainer( XmlStyleFamily nFamily ) const override;
    virtual OUString GetServiceName( XmlStyleFamily nFamily ) const override;
    // HACK

public:


    SwXMLStylesContext_Impl(
            SwXMLImport& rImport,
            bool bAuto );

    virtual bool InsertStyleFamily( XmlStyleFamily nFamily ) const override;

    virtual void EndElement() override;
};

}

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateStyleChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList )
{
    SvXMLStyleContext* pContext = nullptr;

    if(nElement == XML_ELEMENT(TABLE, XML_TABLE_TEMPLATE))
    {
        rtl::Reference<XMLTableImport> xTableImport = GetImport().GetShapeImport()->GetShapeTableImport();
        pContext = xTableImport->CreateTableTemplateContext(nElement, xAttrList);
    }
    if (!pContext)
        pContext = SvXMLStylesContext::CreateStyleChildContext(nElement, xAttrList);

    return pContext;
}

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = nullptr;

    switch( nFamily )
    {
    case XmlStyleFamily::TEXT_PARAGRAPH:
        pStyle = new SwXMLTextStyleContext_Impl( GetSwImport(), nElement,
                            xAttrList, nFamily, *this );
        break;
    case XmlStyleFamily::TABLE_TABLE:
    case XmlStyleFamily::TABLE_COLUMN:
    case XmlStyleFamily::TABLE_ROW:
    case XmlStyleFamily::TABLE_CELL:
        // Distinguish real and automatic styles.
        if (IsAutomaticStyle())
            pStyle = new SwXMLItemSetStyleContext_Impl(GetSwImport(), nElement, xAttrList, *this, nFamily);
        else if (nFamily == XmlStyleFamily::TABLE_CELL) // Real cell styles are used for table-template import.
            pStyle = new SwXMLCellStyleContext(GetSwImport(), nElement, xAttrList, *this, nFamily);
        else
            SAL_WARN("sw.xml", "Context does not exists for non automatic table, column or row style.");
        break;
    case XmlStyleFamily::SD_GRAPHICS_ID:
        // As long as there are no element items, we can use the text
        // style class.
        pStyle = new XMLTextShapeStyleContext( GetImport(), nElement,
                            xAttrList, *this, nFamily );
        break;
    case XmlStyleFamily::SD_DRAWINGPAGE_ID:
        pStyle = new XMLDrawingPageStyleContext(GetImport(), nElement,
                xAttrList, *this, g_MasterPageContextIDs, g_MasterPageFamilies);
        break;
    default:
        pStyle = SvXMLStylesContext::CreateStyleStyleChildContext( nFamily,
                                                                   nElement,
                                                              xAttrList );
        break;
    }

    return pStyle;
}

SvXMLStyleContext *SwXMLStylesContext_Impl::CreateDefaultStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = nullptr;

    switch( nFamily )
    {
    case XmlStyleFamily::TEXT_PARAGRAPH:
    case XmlStyleFamily::TABLE_TABLE:
    case XmlStyleFamily::TABLE_ROW:
        pStyle = new XMLTextStyleContext( GetImport(), nElement,
                                          xAttrList, *this, nFamily,
                                          true );
        break;
    case XmlStyleFamily::SD_GRAPHICS_ID:
        // There are no writer specific defaults for graphic styles!
        pStyle = new XMLGraphicsDefaultStyle( GetImport(), nElement,
                            xAttrList, *this );
        break;
    default:
        pStyle = SvXMLStylesContext::CreateDefaultStyleStyleChildContext( nFamily,
                                                                   nElement,
                                                                   xAttrList );
        break;
    }

    return pStyle;
}

SwXMLStylesContext_Impl::SwXMLStylesContext_Impl(
        SwXMLImport& rImport,
        bool bAuto ) :
    SvXMLStylesContext( rImport, bAuto )
{
}

bool SwXMLStylesContext_Impl::InsertStyleFamily( XmlStyleFamily nFamily ) const
{
    const SwXMLImport& rSwImport = GetSwImport();
    const SfxStyleFamily nStyleFamilyMask = rSwImport.GetStyleFamilyMask();

    bool bIns = true;
    switch( nFamily )
    {
    case XmlStyleFamily::TEXT_PARAGRAPH:
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Para);
        break;
    case XmlStyleFamily::TEXT_TEXT:
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Char);
        break;
    case XmlStyleFamily::SD_GRAPHICS_ID:
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Frame);
        break;
    case XmlStyleFamily::TEXT_LIST:
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Pseudo);
        break;
    case XmlStyleFamily::TEXT_OUTLINE:
    case XmlStyleFamily::TEXT_FOOTNOTECONFIG:
    case XmlStyleFamily::TEXT_ENDNOTECONFIG:
    case XmlStyleFamily::TEXT_LINENUMBERINGCONFIG:
    case XmlStyleFamily::TEXT_BIBLIOGRAPHYCONFIG:
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
        XmlStyleFamily nFamily ) const
{
    rtl::Reference < SvXMLImportPropertyMapper > xMapper;
    if( nFamily == XmlStyleFamily::TABLE_TABLE )
        xMapper = XMLTextImportHelper::CreateTableDefaultExtPropMapper(
            const_cast<SwXMLStylesContext_Impl*>( this )->GetImport() );
    else if( nFamily == XmlStyleFamily::TABLE_ROW )
        xMapper = XMLTextImportHelper::CreateTableRowDefaultExtPropMapper(
            const_cast<SwXMLStylesContext_Impl*>( this )->GetImport() );
    else if( nFamily == XmlStyleFamily::TABLE_CELL )
        xMapper = XMLTextImportHelper::CreateTableCellExtPropMapper(
            const_cast<SwXMLStylesContext_Impl*>( this )->GetImport() );
    else if (nFamily == XmlStyleFamily::SD_DRAWINGPAGE_ID)
    {
        xMapper = XMLTextImportHelper::CreateDrawingPageExtPropMapper(
            const_cast<SwXMLStylesContext_Impl*>(this)->GetImport());
    }
    else
        xMapper = SvXMLStylesContext::GetImportPropertyMapper( nFamily );
    return xMapper;
}

uno::Reference < container::XNameContainer > SwXMLStylesContext_Impl::GetStylesContainer(
                                                XmlStyleFamily nFamily ) const
{
    uno::Reference < container::XNameContainer > xStyles;
    if( XmlStyleFamily::SD_GRAPHICS_ID == nFamily )
        xStyles = const_cast<SvXMLImport *>(&GetImport())->GetTextImport()->GetFrameStyles();
    else if( XmlStyleFamily::TABLE_CELL == nFamily )
        xStyles = const_cast<SvXMLImport *>(&GetImport())->GetTextImport()->GetCellStyles();

    if (!xStyles.is())
        xStyles = SvXMLStylesContext::GetStylesContainer( nFamily );

    return xStyles;
}

OUString SwXMLStylesContext_Impl::GetServiceName( XmlStyleFamily nFamily ) const
{
    if( XmlStyleFamily::SD_GRAPHICS_ID == nFamily )
        return "com.sun.star.style.FrameStyle";
    else if( XmlStyleFamily::TABLE_CELL == nFamily )
        return "com.sun.star.style.CellStyle";

    return SvXMLStylesContext::GetServiceName( nFamily );
}

void SwXMLStylesContext_Impl::EndElement()
{
    GetSwImport().InsertStyles( IsAutomaticStyle() );
}

namespace {

class SwXMLMasterStylesContext_Impl : public XMLTextMasterStylesContext
{
protected:
    virtual bool InsertStyleFamily( XmlStyleFamily nFamily ) const override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
    const SwXMLImport& GetSwImport() const
            { return static_cast<const SwXMLImport&>(GetImport()); }

public:


    SwXMLMasterStylesContext_Impl( SwXMLImport& rImport );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

}

SwXMLMasterStylesContext_Impl::SwXMLMasterStylesContext_Impl(
        SwXMLImport& rImport ) :
    XMLTextMasterStylesContext( rImport )
{
}

bool SwXMLMasterStylesContext_Impl::InsertStyleFamily( XmlStyleFamily nFamily ) const
{
    bool bIns;

    const SwXMLImport& rSwImport = GetSwImport();
    const SfxStyleFamily nStyleFamilyMask = rSwImport.GetStyleFamilyMask();
    if( XmlStyleFamily::MASTER_PAGE == nFamily )
        bIns = bool(nStyleFamilyMask & SfxStyleFamily::Page);
    else
        bIns = XMLTextMasterStylesContext::InsertStyleFamily( nFamily );

    return bIns;
}

void SwXMLMasterStylesContext_Impl::endFastElement(sal_Int32 )
{
    FinishStyles( !GetSwImport().IsInsertMode() );
    GetSwImport().FinishStyles();
}

SvXMLImportContext *SwXMLImport::CreateStylesContext(
        bool bAuto )
{
    SvXMLStylesContext *pContext = new SwXMLStylesContext_Impl( *this, bAuto );
    if( bAuto )
        SetAutoStyles( pContext );
    else
        SetStyles( pContext );

    return pContext;
}

SvXMLImportContext *SwXMLImport::CreateMasterStylesContext()
{
    SvXMLStylesContext *pContext =
        new SwXMLMasterStylesContext_Impl( *this );
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
        XmlStyleFamily nFamily,
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
                if( XmlStyleFamily::TABLE_TABLE == pStyle->GetFamily() &&
                    pStyle->HasMasterPageName() &&
                    !pStyle->IsPageDescConnected() )
                    pStyle->ConnectPageDesc();
                (*ppItemSet) = pStyle->GetItemSet();

                // resolve data style name late
                if( XmlStyleFamily::TABLE_CELL == pStyle->GetFamily() &&
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
