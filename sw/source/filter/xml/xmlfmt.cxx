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
#include <comphelper/diagnose_ex.hxx>
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
#include <ccoll.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

namespace {

class SwXMLConditionParser_Impl
{
    OUString m_sInput;

    Master_CollCondition m_nCondition;
    sal_uInt32 m_nSubCondition;

    sal_Int32 m_nPos;
    sal_Int32 m_nLength;

    inline bool SkipWS();
    inline bool MatchChar( sal_Unicode c );
    inline bool MatchName( OUString& rName );
    inline bool MatchNumber( sal_uInt32& rNumber );

public:

    explicit SwXMLConditionParser_Impl( const OUString& rInp );

    bool IsValid() const { return Master_CollCondition::NONE != m_nCondition; }

    Master_CollCondition GetCondition() const { return m_nCondition; }
    sal_uInt32 GetSubCondition() const { return m_nSubCondition; }
};

}

inline bool SwXMLConditionParser_Impl::SkipWS()
{
    while( m_nPos < m_nLength && ' ' == m_sInput[m_nPos] )
        m_nPos++;
    return true;
}

inline bool SwXMLConditionParser_Impl::MatchChar( sal_Unicode c )
{
    bool bRet = false;
    if( m_nPos < m_nLength && c == m_sInput[m_nPos] )
    {
        m_nPos++;
        bRet = true;
    }
    return bRet;
}

inline bool SwXMLConditionParser_Impl::MatchName( OUString& rName )
{
    OUStringBuffer sBuffer( m_nLength );
    while( m_nPos < m_nLength &&
           ( ('a' <= m_sInput[m_nPos] && m_sInput[m_nPos] <= 'z') ||
              '-' == m_sInput[m_nPos] ) )
    {
        sBuffer.append( m_sInput[m_nPos] );
        m_nPos++;
    }
    rName = sBuffer.makeStringAndClear();
    return !rName.isEmpty();
}

inline bool SwXMLConditionParser_Impl::MatchNumber( sal_uInt32& rNumber )
{
    OUStringBuffer sBuffer( m_nLength );
    while( m_nPos < m_nLength && '0' <= m_sInput[m_nPos] && m_sInput[m_nPos] <= '9' )
    {
        sBuffer.append( m_sInput[m_nPos] );
        m_nPos++;
    }

    OUString sNum( sBuffer.makeStringAndClear() );
    if( !sNum.isEmpty() )
        rNumber = sNum.toInt32();
    return !sNum.isEmpty();
}

SwXMLConditionParser_Impl::SwXMLConditionParser_Impl( const OUString& rInp ) :
    m_sInput( rInp ),
    m_nCondition( Master_CollCondition::NONE ),
    m_nSubCondition( 0 ),
    m_nPos( 0 ),
    m_nLength( rInp.getLength() )
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

    bOK &= m_nPos == m_nLength;

    if( !bOK )
        return;

    if( IsXMLToken( sFunc, XML_ENDNOTE ) && !bHasSub )
        m_nCondition = Master_CollCondition::PARA_IN_ENDNOTE;
    else if( IsXMLToken( sFunc, XML_FOOTER ) && !bHasSub )
        m_nCondition = Master_CollCondition::PARA_IN_FOOTER;
    else if( IsXMLToken( sFunc, XML_FOOTNOTE ) && !bHasSub )
        m_nCondition = Master_CollCondition::PARA_IN_FOOTNOTE;
    else if( IsXMLToken( sFunc, XML_HEADER ) && !bHasSub )
        m_nCondition = Master_CollCondition::PARA_IN_HEADER;
    else if( IsXMLToken( sFunc, XML_LIST_LEVEL) &&
            nSub >=1 && nSub <= MAXLEVEL )
    {
        m_nCondition = Master_CollCondition::PARA_IN_LIST;
        m_nSubCondition = nSub-1;
    }
    else if( IsXMLToken( sFunc, XML_OUTLINE_LEVEL) &&
             nSub >=1 && nSub <= MAXLEVEL )
    {
        m_nCondition = Master_CollCondition::PARA_IN_OUTLINE;
        m_nSubCondition = nSub-1;
    }
    else if( IsXMLToken( sFunc, XML_SECTION ) && !bHasSub )
    {
        m_nCondition = Master_CollCondition::PARA_IN_SECTION;
    }
    else if( IsXMLToken( sFunc, XML_TABLE ) && !bHasSub )
    {
        m_nCondition = Master_CollCondition::PARA_IN_TABLEBODY;
    }
    else if( IsXMLToken( sFunc, XML_TABLE_HEADER ) && !bHasSub )
    {
        m_nCondition = Master_CollCondition::PARA_IN_TABLEHEAD;
    }
    else if( IsXMLToken( sFunc, XML_TEXT_BOX ) && !bHasSub )
    {
        m_nCondition = Master_CollCondition::PARA_IN_FRAME;
    }
}

namespace {

class SwXMLConditionContext_Impl : public SvXMLImportContext
{
    Master_CollCondition m_nCondition;
    sal_uInt32 m_nSubCondition;

    OUString m_sApplyStyle;

public:

    SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_Int32 nElement,
            const uno::Reference< xml::sax::XFastAttributeList > & xAttrList );

    bool IsValid() const { return Master_CollCondition::NONE != m_nCondition; }

    Master_CollCondition getCondition() const { return m_nCondition; }
    sal_uInt32 getSubCondition() const { return m_nSubCondition; }
    OUString const &getApplyStyle() const { return m_sApplyStyle; }
};

}

SwXMLConditionContext_Impl::SwXMLConditionContext_Impl(
            SvXMLImport& rImport, sal_Int32 /*nElement*/,
            const uno::Reference< xml::sax::XFastAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport ),
    m_nCondition( Master_CollCondition::NONE ),
    m_nSubCondition( 0 )
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
                    m_nCondition = aCondParser.GetCondition();
                    m_nSubCondition = aCondParser.GetSubCondition();
                }
                break;
            }
            case XML_ELEMENT(STYLE, XML_APPLY_STYLE_NAME):
                m_sApplyStyle = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("sw", aIter);
        }
    }
}

typedef std::vector<rtl::Reference<SwXMLConditionContext_Impl>> SwXMLConditions_Impl;

namespace {

class SwXMLTextStyleContext_Impl : public XMLTextStyleContext
{
    std::unique_ptr<SwXMLConditions_Impl> m_pConditions;

protected:

    virtual uno::Reference < style::XStyle > Create() override;
    virtual void Finish( bool bOverwrite ) override;

public:


    SwXMLTextStyleContext_Impl( SwXMLImport& rImport,
            XmlStyleFamily nFamily,
            SvXMLStylesContext& rStyles );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

}

uno::Reference < style::XStyle > SwXMLTextStyleContext_Impl::Create()
{
    uno::Reference < style::XStyle > xNewStyle;
    if( m_pConditions && XmlStyleFamily::TEXT_PARAGRAPH == GetFamily() )
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    uno::UNO_QUERY );
        if( xFactory.is() )
        {
            uno::Reference < uno::XInterface > xIfc =
                xFactory->createInstance( u"com.sun.star.style.ConditionalParagraphStyle"_ustr );
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
    if( m_pConditions && XmlStyleFamily::TEXT_PARAGRAPH == GetFamily() && GetStyle().is() )
    {
        CommandStruct const*const pCommands = SwCondCollItem::GetCmds();

        Reference< XPropertySet > xPropSet( GetStyle(), UNO_QUERY );

        uno::Sequence< beans::NamedValue > aSeq( m_pConditions->size() );
        auto aSeqRange = asNonConstRange(aSeq);

        for (std::vector<rtl::Reference<SwXMLConditionContext_Impl>>::size_type i = 0;
            i < m_pConditions->size(); ++i)
        {
            assert((*m_pConditions)[i]->IsValid()); // checked before inserting
            Master_CollCondition nCond = (*m_pConditions)[i]->getCondition();
            sal_uInt32 nSubCond = (*m_pConditions)[i]->getSubCondition();

            for (size_t j = 0; j < COND_COMMAND_COUNT; ++j)
            {
                if (pCommands[j].nCnd == nCond &&
                    pCommands[j].nSubCond == nSubCond)
                {
                    aSeqRange[i].Name = GetCommandContextByIndex( j );
                    aSeqRange[i].Value <<= GetImport().GetStyleDisplayName(
                            GetFamily(), (*m_pConditions)[i]->getApplyStyle() );
                    break;
                }
            }
        }

        try
        {
            xPropSet->setPropertyValue(UNO_NAME_PARA_STYLE_CONDITIONS, uno::Any(aSeq));
        }
        catch (uno::Exception const&)
        {
            TOOLS_WARN_EXCEPTION("sw.xml", "exception when setting ParaStyleConditions");
        }
    }
    XMLTextStyleContext::Finish( bOverwrite );
}

SwXMLTextStyleContext_Impl::SwXMLTextStyleContext_Impl( SwXMLImport& rImport,
        XmlStyleFamily nFamily,
        SvXMLStylesContext& rStyles ) :
    XMLTextStyleContext( rImport, rStyles, nFamily )
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
            if( !m_pConditions )
               m_pConditions = std::make_unique<SwXMLConditions_Impl>();
            m_pConditions->push_back( xCond );
        }
        return xCond;
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
    virtual void SetAttribute(sal_Int32 nElement, const OUString& rValue) override;
};

class SwXMLItemSetStyleContext_Impl : public SvXMLStyleContext
{
    OUString                    m_sMasterPageName;
    std::optional<SfxItemSet>   m_oItemSet;
    SwXMLTextStyleContext_Impl *m_pTextStyle;
    SvXMLStylesContext          &m_rStyles;

    OUString                m_sDataStyleName;

    bool                m_bHasMasterPageName : 1;
    bool                m_bPageDescConnected : 1;
    bool                m_bDataStyleIsResolved;

    SvXMLImportContext *CreateItemSetContext(
            sal_Int32 nElement,
            const uno::Reference< xml::sax::XFastAttributeList > & xAttrList);

protected:

    virtual void SetAttribute( sal_Int32 nElement,
                               const OUString& rValue ) override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

public:


    SwXMLItemSetStyleContext_Impl(
            SwXMLImport& rImport,
            SvXMLStylesContext& rStylesC,
            XmlStyleFamily nFamily);

    virtual void CreateAndInsert( bool bOverwrite ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    // The item set may be empty!
    SfxItemSet *GetItemSet() { return m_oItemSet ? &*m_oItemSet : nullptr; }

    bool HasMasterPageName() const { return m_bHasMasterPageName; }

    bool IsPageDescConnected() const { return m_bPageDescConnected; }
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
        GetProperties().emplace_back(nIndex, Any(nNumberFormat));
}

void SwXMLCellStyleContext::FillPropertySet(const css::uno::Reference<css::beans::XPropertySet>& rPropSet)
{
    AddDataFormat();
    XMLPropStyleContext::FillPropertySet(rPropSet);
}

void SwXMLCellStyleContext::SetAttribute(sal_Int32 nElement, const OUString& rValue)
{
    if ((nElement & TOKEN_MASK) == XML_DATA_STYLE_NAME)
        m_sDataStyleName = rValue;
    else
        XMLPropStyleContext::SetAttribute(nElement, rValue);
}

void SwXMLItemSetStyleContext_Impl::SetAttribute( sal_Int32 nElement,
                                           const OUString& rValue )
{
    switch(nElement)
    {
        case XML_ELEMENT(STYLE, XML_MASTER_PAGE_NAME):
        {
            m_sMasterPageName = rValue;
            m_bHasMasterPageName = true;
            break;
        }
        case XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME):
        {
            // if we have a valid data style name
            if (!rValue.isEmpty())
            {
                m_sDataStyleName = rValue;
                m_bDataStyleIsResolved = false;   // needs to be resolved
            }
            break;
        }
        default:
            SvXMLStyleContext::SetAttribute( nElement, rValue );
    }
}

SvXMLImportContext *SwXMLItemSetStyleContext_Impl::CreateItemSetContext(
        sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    OSL_ENSURE( !m_oItemSet,
            "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: item set exists" );

    SvXMLImportContext *pContext = nullptr;

    SwDoc* pDoc = GetSwImport().getDoc();

    SfxItemPool& rItemPool = pDoc->GetAttrPool();
    switch( GetFamily() )
    {
    case XmlStyleFamily::TABLE_TABLE:
        m_oItemSet.emplace( rItemPool, aTableSetRange );
        break;
    case XmlStyleFamily::TABLE_COLUMN:
        m_oItemSet.emplace( rItemPool, svl::Items<RES_FRM_SIZE, RES_FRM_SIZE> );
        break;
    case XmlStyleFamily::TABLE_ROW:
        m_oItemSet.emplace( rItemPool, aTableLineSetRange );
        break;
    case XmlStyleFamily::TABLE_CELL:
        m_oItemSet.emplace( rItemPool, aTableBoxSetRange );
        break;
    default:
        OSL_ENSURE( false,
        "SwXMLItemSetStyleContext_Impl::CreateItemSetContext: unknown family" );
        break;
    }
    if( m_oItemSet )
        pContext = GetSwImport().CreateTableItemImportContext(
                                nElement, xAttrList, GetFamily(),
                                *m_oItemSet );
    if( !pContext )
    {
        m_oItemSet.reset();
    }

    return pContext;
}


SwXMLItemSetStyleContext_Impl::SwXMLItemSetStyleContext_Impl( SwXMLImport& rImport,
        SvXMLStylesContext& rStylesC,
        XmlStyleFamily nFamily ) :
    SvXMLStyleContext( rImport, nFamily ),
    m_pTextStyle( nullptr ),
    m_rStyles( rStylesC ),
    m_bHasMasterPageName( false ),
    m_bPageDescConnected( false ),
    m_bDataStyleIsResolved( true )
{
}

void SwXMLItemSetStyleContext_Impl::CreateAndInsert( bool bOverwrite )
{
    if( m_pTextStyle )
        m_pTextStyle->CreateAndInsert( bOverwrite );
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
        case XML_ELEMENT(STYLE, XML_TEXT_PROPERTIES):
        case XML_ELEMENT(STYLE, XML_PARAGRAPH_PROPERTIES):
        {
            if( !m_pTextStyle )
            {
                m_pTextStyle = new SwXMLTextStyleContext_Impl( GetSwImport(), XmlStyleFamily::TEXT_PARAGRAPH, m_rStyles );
                rtl::Reference<sax_fastparser::FastAttributeList> xTmpAttrList = new sax_fastparser::FastAttributeList(nullptr);
                xTmpAttrList->add(XML_ELEMENT(STYLE, XML_NAME), GetName().toUtf8() );
                m_pTextStyle->startFastElement( nElement, xTmpAttrList );
                m_rStyles.AddStyle( *m_pTextStyle );
            }
            return m_pTextStyle->createFastChildContext( nElement, xAttrList );
        }
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("sw", nElement);
    }

    return nullptr;
}

void SwXMLItemSetStyleContext_Impl::ConnectPageDesc()
{
    if( m_bPageDescConnected || !HasMasterPageName() )
        return;
    m_bPageDescConnected = true;

    SwDoc *pDoc = GetSwImport().getDoc();

    // #i40788# - first determine the display name of the page style,
    // then map this name to the corresponding user interface name.
    OUString sName = GetImport().GetStyleDisplayName( XmlStyleFamily::MASTER_PAGE,
                                             m_sMasterPageName );
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

    if( !m_oItemSet )
    {
        SfxItemPool& rItemPool = pDoc->GetAttrPool();
        m_oItemSet.emplace( rItemPool, aTableSetRange );
    }

    std::unique_ptr<SwFormatPageDesc> pFormatPageDesc;
    if( const SwFormatPageDesc* pItem = m_oItemSet->GetItemIfSet( RES_PAGEDESC, false ) )
    {
         if( pItem->GetPageDesc() != pPageDesc )
            pFormatPageDesc.reset(new SwFormatPageDesc( *pItem ));
    }
    else
        pFormatPageDesc.reset(new SwFormatPageDesc());

    if( pFormatPageDesc )
    {
        pFormatPageDesc->RegisterToPageDesc( *pPageDesc );
        m_oItemSet->Put( std::move(pFormatPageDesc) );
    }
}

bool SwXMLItemSetStyleContext_Impl::ResolveDataStyleName()
{
    // resolve, if not already done
    if (! m_bDataStyleIsResolved)
    {
        // get the format key
        sal_Int32 nFormat =
            GetImport().GetTextImport()->GetDataStyleKey(m_sDataStyleName);

        // if the key is valid, insert Item into ItemSet
        if( -1 != nFormat )
        {
            if( !m_oItemSet )
            {
                SwDoc *pDoc = GetSwImport().getDoc();

                SfxItemPool& rItemPool = pDoc->GetAttrPool();
                m_oItemSet.emplace( rItemPool, aTableBoxSetRange );
            }
            SwTableBoxNumFormat aNumFormatItem(nFormat);
            m_oItemSet->Put(aNumFormatItem);
        }

        // now resolved
        m_bDataStyleIsResolved = true;
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

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
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
        pStyle = new SwXMLTextStyleContext_Impl( GetSwImport(), nFamily, *this );
        break;
    case XmlStyleFamily::TABLE_TABLE:
    case XmlStyleFamily::TABLE_COLUMN:
    case XmlStyleFamily::TABLE_ROW:
    case XmlStyleFamily::TABLE_CELL:
        // Distinguish real and automatic styles.
        if (IsAutomaticStyle())
            pStyle = new SwXMLItemSetStyleContext_Impl(GetSwImport(), *this, nFamily);
        else if (nFamily == XmlStyleFamily::TABLE_CELL) // Real cell styles are used for table-template import.
            pStyle = new SwXMLCellStyleContext(GetSwImport(), *this, nFamily);
        else
            SAL_WARN("sw.xml", "Context does not exists for non automatic table, column or row style.");
        break;
    case XmlStyleFamily::SD_GRAPHICS_ID:
        // As long as there are no element items, we can use the text
        // style class.
        pStyle = new XMLTextShapeStyleContext( GetImport(), *this, nFamily );
        break;
    case XmlStyleFamily::SD_DRAWINGPAGE_ID:
        pStyle = new XMLDrawingPageStyleContext(GetImport(),
                *this, g_MasterPageContextIDs, g_MasterPageFamilies);
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
        pStyle = new XMLTextStyleContext( GetImport(),
                                          *this, nFamily,
                                          true );
        break;
    case XmlStyleFamily::SD_GRAPHICS_ID:
        // There are no writer specific defaults for graphic styles!
        pStyle = new XMLGraphicsDefaultStyle( GetImport(), *this );
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
        return u"com.sun.star.style.FrameStyle"_ustr;
    else if( XmlStyleFamily::TABLE_CELL == nFamily )
        return u"com.sun.star.style.CellStyle"_ustr;

    return SvXMLStylesContext::GetServiceName( nFamily );
}

void SwXMLStylesContext_Impl::endFastElement(sal_Int32 )
{
    GetSwImport().InsertStyles( IsAutomaticStyle() );
    if (!IsAutomaticStyle())
        GetImport().GetShapeImport()->GetShapeTableImport()->finishStyles();
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
        pDoc = getDoc();

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
            if(bSendModify)
                pColl->GetNotifier().Broadcast(sw::CondCollCondChg(*pColl));
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
