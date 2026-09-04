/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <comphelper/classids.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <xmloff/families.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlexppr.hxx>

#include <ndole.hxx>
#include <fmtcntnt.hxx>
#include <unoframe.hxx>
#include "xmlexp.hxx"
#include "xmltexte.hxx"
#include <ndindex.hxx>

#include <osl/diagnose.h>
#include <sot/exchange.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/frmdescr.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <fmtcol.hxx>
#include <ndtxt.hxx>
#include <unomap.hxx>
#include <unoparagraph.hxx>
#include <set>

using namespace ::com::sun::star;
using namespace ::cpo;
using namespace ::cpo::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::xmloff::token;

namespace {

enum SvEmbeddedObjectTypes
{
    SV_EMBEDDED_OWN,
    SV_EMBEDDED_OUTPLACE,
    SV_EMBEDDED_FRAME
};

}

SwNoTextNode *SwXMLTextParagraphExport::GetNoTextNode(
    const Reference < XPropertySet >& rPropSet )
{
    SwXFrame* pFrame = dynamic_cast<SwXFrame*>(rPropSet.get());
    assert(pFrame && "SwXFrame missing");
    SwFrameFormat *pFrameFormat = pFrame->GetFrameFormat();
    const SwFormatContent& rContent = pFrameFormat->GetContent();
    const SwNodeIndex *pNdIdx = rContent.GetContentIdx();
    return  pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetNoTextNode();
}

constexpr OUString gsEmbeddedObjectProtocol( u"vnd.sun.star.EmbeddedObject:"_ustr );

SwXMLTextParagraphExport::SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& _rAutoStylePool ) :
    XMLTextParagraphExport( rExp, _rAutoStylePool ),
    m_aIFrameClassId( SO3_IFRAME_CLASSID )
{
}

SwXMLTextParagraphExport::~SwXMLTextParagraphExport()
{
}

namespace {

/// A paragraph in a cell of a live styled table, seen with the text formatting its table style
/// role gives it as the paragraph's own. The paragraph itself reports that formatting as
/// inherited, since it is not the paragraph's; the file has to carry it all the same, because a
/// reader without table styles has nothing to resolve it from.
class SwTableStyleRoleParagraph final
    : public cppu::WeakImplHelper<css::beans::XPropertySet, css::beans::XPropertyState>
{
    uno::Reference<css::beans::XPropertySet> m_xParagraph;
    uno::Reference<css::beans::XPropertyState> m_xParagraphState;
    /// The properties the role gives the paragraph and the paragraph does not set itself.
    std::set<OUString> m_aRoleProperties;

public:
    SwTableStyleRoleParagraph(uno::Reference<css::beans::XPropertySet> xParagraph,
                              std::set<OUString> aRoleProperties)
        : m_xParagraph(std::move(xParagraph))
        , m_xParagraphState(m_xParagraph, uno::UNO_QUERY_THROW)
        , m_aRoleProperties(std::move(aRoleProperties))
    {
    }

    // XPropertySet
    virtual uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override
    {
        return m_xParagraph->getPropertySetInfo();
    }
    virtual void SAL_CALL setPropertyValue(const OUString& rName, const uno::Any& rValue) override
    {
        m_xParagraph->setPropertyValue(rName, rValue);
    }
    virtual uno::Any SAL_CALL getPropertyValue(const OUString& rName) override
    {
        return m_xParagraph->getPropertyValue(rName);
    }
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString& rName,
        const uno::Reference<css::beans::XPropertyChangeListener>& xListener) override
    {
        m_xParagraph->addPropertyChangeListener(rName, xListener);
    }
    virtual void SAL_CALL removePropertyChangeListener(
        const OUString& rName,
        const uno::Reference<css::beans::XPropertyChangeListener>& xListener) override
    {
        m_xParagraph->removePropertyChangeListener(rName, xListener);
    }
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString& rName,
        const uno::Reference<css::beans::XVetoableChangeListener>& xListener) override
    {
        m_xParagraph->addVetoableChangeListener(rName, xListener);
    }
    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString& rName,
        const uno::Reference<css::beans::XVetoableChangeListener>& xListener) override
    {
        m_xParagraph->removeVetoableChangeListener(rName, xListener);
    }

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState(const OUString& rName) override
    {
        if (m_aRoleProperties.count(rName))
            return css::beans::PropertyState_DIRECT_VALUE;
        return m_xParagraphState->getPropertyState(rName);
    }
    virtual uno::Sequence<css::beans::PropertyState> SAL_CALL
    getPropertyStates(const uno::Sequence<OUString>& rNames) override
    {
        uno::Sequence<css::beans::PropertyState> aStates
            = m_xParagraphState->getPropertyStates(rNames);
        css::beans::PropertyState* pStates = aStates.getArray();
        for (sal_Int32 i = 0; i < rNames.getLength(); ++i)
            if (m_aRoleProperties.count(rNames[i]))
                pStates[i] = css::beans::PropertyState_DIRECT_VALUE;
        return aStates;
    }
    virtual void SAL_CALL setPropertyToDefault(const OUString& rName) override
    {
        m_xParagraphState->setPropertyToDefault(rName);
    }
    virtual uno::Any SAL_CALL getPropertyDefault(const OUString& rName) override
    {
        return m_xParagraphState->getPropertyDefault(rName);
    }
};

}

uno::Reference<css::beans::XPropertySet>
SwXMLTextParagraphExport::getParagraphAutoStylePropertySet(
    const uno::Reference<css::beans::XPropertySet>& rPropSet) const
{
    const SwXParagraph* pParagraph = dynamic_cast<SwXParagraph*>(rPropSet.get());
    const SwTextNode* pNode = pParagraph ? pParagraph->GetTextNode() : nullptr;
    const SwTextFormatColl* pRoleColl = pNode ? pNode->GetTableStyleRoleColl() : nullptr;
    if (!pRoleColl)
        return rPropSet;

    const SfxItemSet& rRoleSet = pRoleColl->GetAttrSet();
    const SwAttrSet* pOwnSet = pNode->GetpSwAttrSet();
    std::set<OUString> aRoleProperties;
    for (const SfxItemPropertyMapEntry* pEntry :
         aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH)->getPropertyMap().getPropertyEntries())
    {
        if (SfxItemState::SET != rRoleSet.GetItemState(pEntry->nWID, false))
            continue;
        if (pOwnSet && SfxItemState::SET == pOwnSet->GetItemState(pEntry->nWID, false))
            continue;
        aRoleProperties.insert(pEntry->aName);
    }
    if (aRoleProperties.empty())
        return rPropSet;
    return new SwTableStyleRoleParagraph(rPropSet, std::move(aRoleProperties));
}

static void lcl_addURL ( SvXMLExport &rExport, const OUString &rURL,
                         bool bToRel = true )
{
    const OUString sRelURL = ( bToRel && !rURL.isEmpty() )
        ? URIHelper::simpleNormalizedMakeRelative(rExport.GetOrigFileName(), rURL)
        : rURL;

    if (!sRelURL.isEmpty())
    {
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, sRelURL );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
    }
}

static void lcl_addAspect(
        const svt::EmbeddedObjectRef& rObj,
        std::vector<XMLPropertyState>& rStates,
        const rtl::Reference < XMLPropertySetMapper >& rMapper )
{
    sal_Int64 nAspect = rObj.GetViewAspect();
    if ( nAspect )
        rStates.emplace_back( rMapper->FindEntryIndex( CTF_OLE_DRAW_ASPECT ), cpo::uno::Any( nAspect ) );
}

static void lcl_addOutplaceProperties(
        const svt::EmbeddedObjectRef& rObj,
        std::vector<XMLPropertyState>& rStates,
        const rtl::Reference < XMLPropertySetMapper >& rMapper )
{
    MapMode aMode( MapUnit::Map100thMM ); // the API expects this map mode for the embedded objects
    Size aSize = rObj.GetSize( &aMode ); // get the size in the requested map mode

    if( !(aSize.Width() && aSize.Height()) )
        return;

    rStates.emplace_back( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_LEFT ), Any(sal_Int32(0)) );
    rStates.emplace_back( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_TOP ), Any(sal_Int32(0)) );
    rStates.emplace_back( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_WIDTH ), Any(static_cast<sal_Int32>(aSize.Width())) );
    rStates.emplace_back( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_HEIGHT ), Any(static_cast<sal_Int32>(aSize.Height())) );
}

static void lcl_addFrameProperties(
        const uno::Reference < embed::XEmbeddedObject >& xObj,
        std::vector<XMLPropertyState>& rStates,
        const rtl::Reference < XMLPropertySetMapper >& rMapper )
{
    if ( !::svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        return;

    uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
    if ( !xSet.is() )
        return;

    bool bIsAutoScroll = false, bIsScrollingMode = false;
    Any aAny = xSet->getPropertyValue(u"FrameIsAutoScroll"_ustr);
    aAny >>= bIsAutoScroll;
    if ( !bIsAutoScroll )
    {
        aAny = xSet->getPropertyValue(u"FrameIsScrollingMode"_ustr);
        aAny >>= bIsScrollingMode;
    }

    bool bIsBorderSet = false, bIsAutoBorder = false;
    aAny = xSet->getPropertyValue(u"FrameIsAutoBorder"_ustr);
    aAny >>= bIsAutoBorder;
    if ( !bIsAutoBorder )
    {
        aAny = xSet->getPropertyValue(u"FrameIsBorder"_ustr);
        aAny >>= bIsBorderSet;
    }

    sal_Int32 nWidth, nHeight;
    aAny = xSet->getPropertyValue(u"FrameMarginWidth"_ustr);
    aAny >>= nWidth;
    aAny = xSet->getPropertyValue(u"FrameMarginHeight"_ustr);
    aAny >>= nHeight;

    if( !bIsAutoScroll )
        rStates.emplace_back( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_SCROLLBAR ), Any(bIsScrollingMode) );
    if( !bIsAutoBorder )
        rStates.emplace_back( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_BORDER ), Any(bIsBorderSet) );
    if( SIZE_NOT_SET != nWidth )
        rStates.emplace_back( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_HORI ), Any(nWidth) );
    if( SIZE_NOT_SET != nHeight )
        rStates.emplace_back( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_VERT ), Any(nHeight) );
}

void SwXMLTextParagraphExport::_collectTextEmbeddedAutoStyles(
        const Reference < XPropertySet > & rPropSet )
{
    SwOLENode *pOLENd = GetNoTextNode( rPropSet )->GetOLENode();
    svt::EmbeddedObjectRef& rObjRef = pOLENd->GetOLEObj().GetObject();
    if( !rObjRef.is() )
        return;

    std::vector<XMLPropertyState> aStates;
    aStates.reserve(8);
    SvGlobalName aClassId( rObjRef->getClassID() );

    if( m_aIFrameClassId == aClassId )
    {
        lcl_addFrameProperties( rObjRef.GetObject(), aStates,
               GetAutoFramePropMapper()->getPropertySetMapper() );
    }
    else if ( !SotExchange::IsInternal( aClassId ) )
    {
        lcl_addOutplaceProperties( rObjRef, aStates,
               GetAutoFramePropMapper()->getPropertySetMapper() );
    }

    lcl_addAspect( rObjRef, aStates,
           GetAutoFramePropMapper()->getPropertySetMapper() );

    Add( XmlStyleFamily::TEXT_FRAME, rPropSet, aStates );
}

void SwXMLTextParagraphExport::_exportTextEmbedded(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    SwOLENode *pOLENd = GetNoTextNode( rPropSet )->GetOLENode();
    SwOLEObj& rOLEObj = pOLENd->GetOLEObj();
    svt::EmbeddedObjectRef& rObjRef = rOLEObj.GetObject();
    if( !rObjRef.is() )
        return;

    SvGlobalName aClassId( rObjRef->getClassID() );

    SvEmbeddedObjectTypes nType = SV_EMBEDDED_OWN;
    if( m_aIFrameClassId == aClassId )
    {
        nType = SV_EMBEDDED_FRAME;
    }
    else if ( !SotExchange::IsInternal( aClassId ) )
    {
        nType = SV_EMBEDDED_OUTPLACE;
    }

    enum XMLTokenEnum eElementName = XML__UNKNOWN_;
    SvXMLExport &rXMLExport = GetExport();

    // First the stuff common to Floating Frame
    OUString sStyle;
    Any aAny;
    if( rPropSetInfo->hasPropertyByName( gsFrameStyleName ) )
    {
        aAny = rPropSet->getPropertyValue( gsFrameStyleName );
        aAny >>= sStyle;
    }

    std::vector<XMLPropertyState> aStates;
    aStates.reserve(8);
    switch( nType )
    {
    case SV_EMBEDDED_FRAME:
        lcl_addFrameProperties( rObjRef.GetObject(), aStates,
            GetAutoFramePropMapper()->getPropertySetMapper() );
        break;
    case SV_EMBEDDED_OUTPLACE:
        lcl_addOutplaceProperties( rObjRef, aStates,
            GetAutoFramePropMapper()->getPropertySetMapper() );
        break;
    default:
        ;
    }

    lcl_addAspect( rObjRef, aStates,
        GetAutoFramePropMapper()->getPropertySetMapper() );

    const OUString sAutoStyle = Find( XmlStyleFamily::TEXT_FRAME,
                                      rPropSet, sStyle, aStates );
    aStates.clear();

    if( !sAutoStyle.isEmpty() )
        rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE_NAME, sAutoStyle );
    addTextFrameAttributes( rPropSet, false );

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              XML_FRAME, false, true );

    switch (nType)
    {
    case SV_EMBEDDED_OUTPLACE:
    case SV_EMBEDDED_OWN:
        if( !(rXMLExport.getExportFlags() & SvXMLExportFlags::EMBEDDED) )
        {
            OUString sURL;

            bool bIsOwnLink = false;
            if( SV_EMBEDDED_OWN == nType )
            {
                try
                {
                    uno::Reference< embed::XLinkageSupport > xLinkage( rObjRef.GetObject(), uno::UNO_QUERY );
                    bIsOwnLink = xLinkage.is() && xLinkage->isLink();
                    if ( bIsOwnLink )
                        sURL = xLinkage->getLinkURL();
                }
                catch(const cpo::uno::Exception&)
                {
                    // TODO/LATER: error handling
                    OSL_FAIL( "Link detection or retrieving of the URL of OOo link is failed!" );
                }
            }

            if ( !bIsOwnLink )
            {
                sURL = gsEmbeddedObjectProtocol + rOLEObj.GetCurrentPersistName();
            }

            sURL = GetExport().AddEmbeddedObject( sURL );
            lcl_addURL( rXMLExport, sURL, false );
        }
        if( SV_EMBEDDED_OWN == nType && !pOLENd->GetChartTableName().isEmpty() )
        {
            OUString sRange( pOLENd->GetChartTableName().toString() );
            OUStringBuffer aBuffer( sRange.getLength() + 2 );
            for( sal_Int32 i=0; i < sRange.getLength(); i++ )
            {
                sal_Unicode c = sRange[i];
                switch( c  )
                {
                    case ' ':
                    case '.':
                    case '\'':
                    case '\\':
                        if( aBuffer.isEmpty() )
                        {
                            aBuffer.append( OUString::Concat("\'") + sRange.subView(0, i) );
                        }
                        if( '\'' == c || '\\' == c )
                            aBuffer.append( '\\' );
                        [[fallthrough]];
                    default:
                        if( !aBuffer.isEmpty() )
                            aBuffer.append( c );
                }
            }
            if( !aBuffer.isEmpty() )
            {
                aBuffer.append( '\'' );
                sRange = aBuffer.makeStringAndClear();
            }

            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES,
            sRange );
        }
        eElementName = SV_EMBEDDED_OUTPLACE==nType ? XML_OBJECT_OLE
                                                   : XML_OBJECT;
        break;
    case SV_EMBEDDED_FRAME:
        {
            // It's a floating frame!
            if ( svt::EmbeddedObjectRef::TryRunningState( rObjRef.GetObject() ) )
            {
                uno::Reference < beans::XPropertySet > xSet( rObjRef->getComponent(), uno::UNO_QUERY );
                OUString aStr;
                Any aAny2 = xSet->getPropertyValue(u"FrameURL"_ustr);
                aAny2 >>= aStr;

                lcl_addURL( rXMLExport, aStr );

                aAny2 = xSet->getPropertyValue(u"FrameName"_ustr);
                aAny2 >>= aStr;

                if (!aStr.isEmpty())
                    rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FRAME_NAME, aStr );
                eElementName = XML_FLOATING_FRAME;
            }
        }
        break;
    default:
        OSL_ENSURE( false, "unknown object type! Base class should have been called!" );
    }

    {
        SvXMLElementExport aElementExport( rXMLExport, XML_NAMESPACE_DRAW, eElementName,
                                      false, true );
        switch( nType )
        {
        case SV_EMBEDDED_OWN:
            if( rXMLExport.getExportFlags() & SvXMLExportFlags::EMBEDDED )
            {
                Reference < XEmbeddedObjectSupplier > xEOS( rPropSet, UNO_QUERY );
                OSL_ENSURE( xEOS.is(), "no embedded object supplier for own object" );
                Reference < XComponent > xComp = xEOS->getEmbeddedObject();
                rXMLExport.ExportEmbeddedOwnObject( xComp );
            }
            break;
        case SV_EMBEDDED_OUTPLACE:
            if( rXMLExport.getExportFlags() & SvXMLExportFlags::EMBEDDED )
            {
                OUString sURL( gsEmbeddedObjectProtocol + rOLEObj.GetCurrentPersistName() );

                if ( !( rXMLExport.getExportFlags() & SvXMLExportFlags::OASIS ) )
                    sURL += "?oasis=false";

                rXMLExport.AddEmbeddedObjectAsBase64( sURL );
            }
            break;
        default:
            break;
        }
    }
    if( SV_EMBEDDED_OUTPLACE==nType || SV_EMBEDDED_OWN==nType )
    {
        OUString sURL = XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE + rOLEObj.GetCurrentPersistName();
        if( !(rXMLExport.getExportFlags() & SvXMLExportFlags::EMBEDDED) )
        {
            sURL = GetExport().AddEmbeddedObject( sURL );
            lcl_addURL( rXMLExport, sURL, false );
        }

        SvXMLElementExport aElementExport( GetExport(), XML_NAMESPACE_DRAW,
                                  XML_IMAGE, false, true );

        if( rXMLExport.getExportFlags() & SvXMLExportFlags::EMBEDDED )
            GetExport().AddEmbeddedObjectAsBase64( sURL );
    }

    // Lastly the stuff common to Floating Frame
    exportEvents( rPropSet );
    exportTitleAndDescription( rPropSet, rPropSetInfo );  // #i73249#
    exportContour( rPropSet, rPropSetInfo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
