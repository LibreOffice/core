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

#include <comphelper/classids.hxx>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <xmloff/families.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/maptype.hxx>

#include <svx/svdobj.hxx>
#include <doc.hxx>
#include <ndole.hxx>
#include <fmtcntnt.hxx>
#include <unostyle.hxx>
#include <unoframe.hxx>
#include <ndgrf.hxx>
#include "xmlexp.hxx"
#include "xmltexte.hxx"
#include <SwAppletImpl.hxx>

#include <svl/urihelper.hxx>
#include <sfx2/frmdescr.hxx>
#include <SwStyleNameMapper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;
using namespace ::xmloff::token;

enum SvEmbeddedObjectTypes
{
    SV_EMBEDDED_OWN,
    SV_EMBEDDED_OUTPLACE,
    SV_EMBEDDED_APPLET,
    SV_EMBEDDED_PLUGIN,
    SV_EMBEDDED_FRAME
};

SwNoTextNode *SwXMLTextParagraphExport::GetNoTextNode(
    const Reference < XPropertySet >& rPropSet )
{
    Reference<XUnoTunnel> xCursorTunnel( rPropSet, UNO_QUERY );
    assert(xCursorTunnel.is() && "missing XUnoTunnel for embedded");
    SwXFrame *pFrame = reinterpret_cast< SwXFrame * >(
                sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( SwXFrame::getUnoTunnelId() )));
    assert(pFrame && "SwXFrame missing");
    SwFrameFormat *pFrameFormat = pFrame->GetFrameFormat();
    const SwFormatContent& rContent = pFrameFormat->GetContent();
    const SwNodeIndex *pNdIdx = rContent.GetContentIdx();
    return  pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetNoTextNode();
}

void SwXMLTextParagraphExport::exportStyleContent(
        const Reference< XStyle > & rStyle )
{

    const SwXStyle* pStyle = nullptr;
    Reference<XUnoTunnel> xStyleTunnel( rStyle, UNO_QUERY);
    if( xStyleTunnel.is() )
    {
        pStyle = reinterpret_cast< SwXStyle * >(
                sal::static_int_cast< sal_IntPtr >( xStyleTunnel->getSomething( SwXStyle::getUnoTunnelId() )));
    }
    if( pStyle && SFX_STYLE_FAMILY_PARA == pStyle->GetFamily() )
    {
        const SwDoc *pDoc = pStyle->GetDoc();
        const SwTextFormatColl *pColl =
            pDoc->FindTextFormatCollByName( pStyle->GetStyleName() );
        OSL_ENSURE( pColl, "There is the text collection?" );
        if( pColl && RES_CONDTXTFMTCOLL == pColl->Which() )
        {
            const SwFormatCollConditions& rConditions =
                static_cast<const SwConditionTextFormatColl *>(pColl)->GetCondColls();
            for( size_t i=0; i < rConditions.size(); ++i )
            {
                const SwCollCondition& rCond = *rConditions[i];

                enum XMLTokenEnum eFunc = XML_TOKEN_INVALID;
                OUString sVal;
                switch( rCond.GetCondition() )
                {
                case PARA_IN_LIST:
                    eFunc = XML_LIST_LEVEL;
                    sVal = OUString::number(rCond.GetSubCondition()+1);
                    break;
                case PARA_IN_OUTLINE:
                    eFunc = XML_OUTLINE_LEVEL;
                    sVal = OUString::number(rCond.GetSubCondition()+1);
                    break;
                case PARA_IN_FRAME:
                    eFunc = XML_TEXT_BOX;
                    break;
                case PARA_IN_TABLEHEAD:
                    eFunc = XML_TABLE_HEADER;
                    break;
                case PARA_IN_TABLEBODY:
                    eFunc = XML_TABLE;
                    break;
                case PARA_IN_SECTION:
                    eFunc = XML_SECTION;
                    break;
                case PARA_IN_FOOTENOTE:
                    eFunc = XML_FOOTNOTE;
                    break;
                case PARA_IN_FOOTER:
                    eFunc = XML_FOOTER;
                    break;
                case PARA_IN_HEADER:
                    eFunc = XML_HEADER;
                    break;
                case PARA_IN_ENDNOTE:
                    eFunc = XML_ENDNOTE;
                    break;
                }
                OSL_ENSURE( eFunc != XML_TOKEN_INVALID,
                            "SwXMLExport::ExportFormat: unknown condition" );
                if( eFunc != XML_TOKEN_INVALID )
                {
                    OUString sCond = GetXMLToken(eFunc) + "()";
                    if( !sVal.isEmpty() )
                    {
                        sCond += "=" + sVal;
                    }

                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                XML_CONDITION, sCond );
                    OUString aString;
                    SwStyleNameMapper::FillProgName(
                                    rCond.GetTextFormatColl()->GetName(),
                                    aString,
                                    nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL,
                                    true);
                    aString = GetExport().EncodeStyleName( aString );
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                XML_APPLY_STYLE_NAME, aString );
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                              XML_MAP, true, true );
                }
            }
        }
    }
}

SwXMLTextParagraphExport::SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& _rAutoStylePool ) :
    XMLTextParagraphExport( rExp, _rAutoStylePool ),
    sEmbeddedObjectProtocol( "vnd.sun.star.EmbeddedObject:" ),
    sGraphicObjectProtocol( "vnd.sun.star.GraphicObject:" ),
    aAppletClassId( SO3_APPLET_CLASSID ),
    aPluginClassId( SO3_PLUGIN_CLASSID ),
    aIFrameClassId( SO3_IFRAME_CLASSID )
{
}

SwXMLTextParagraphExport::~SwXMLTextParagraphExport()
{
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
        const XMLPropertyState **pStates,
        const rtl::Reference < XMLPropertySetMapper >& rMapper )
{
    {
        sal_Int64 nAspect = rObj.GetViewAspect();

        if ( nAspect )
        {
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_DRAW_ASPECT ), uno::makeAny( nAspect ) );
        }
    }
}

static void lcl_addOutplaceProperties(
        const svt::EmbeddedObjectRef& rObj,
        const XMLPropertyState **pStates,
        const rtl::Reference < XMLPropertySetMapper >& rMapper )
{
    {
        MapMode aMode( MAP_100TH_MM ); // the API expects this map mode for the embedded objects
        Size aSize = rObj.GetSize( &aMode ); // get the size in the requested map mode

        if( aSize.Width() && aSize.Height() )
        {
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_LEFT ), Any(0L) );
            pStates++;

            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_TOP ), Any(0L) );
            pStates++;

            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_WIDTH ), Any((sal_Int32)aSize.Width()) );
            pStates++;

            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_HEIGHT ), Any((sal_Int32)aSize.Height()) );
        }
    }
}

static void lcl_addFrameProperties(
        const uno::Reference < embed::XEmbeddedObject >& xObj,
        const XMLPropertyState **pStates,
        const rtl::Reference < XMLPropertySetMapper >& rMapper )
{
    if ( !::svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        return;

    uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
    if ( !xSet.is() )
        return;

    OUString aURL;
    Any aAny = xSet->getPropertyValue("FrameURL");
    aAny >>= aURL;

    OUString aName;
    aAny = xSet->getPropertyValue("FrameName");
    aAny >>= aName;

    bool bIsAutoScroll = false, bIsScrollingMode = false;
    aAny = xSet->getPropertyValue("FrameIsAutoScroll");
    aAny >>= bIsAutoScroll;
    if ( !bIsAutoScroll )
    {
        aAny = xSet->getPropertyValue("FrameIsScrollingMode");
        aAny >>= bIsScrollingMode;
    }

    bool bIsBorderSet = false, bIsAutoBorder = false;
    aAny = xSet->getPropertyValue("FrameIsAutoBorder");
    aAny >>= bIsAutoBorder;
    if ( !bIsAutoBorder )
    {
        aAny = xSet->getPropertyValue("FrameIsBorder");
        aAny >>= bIsBorderSet;
    }

    sal_Int32 nWidth, nHeight;
    aAny = xSet->getPropertyValue("FrameMarginWidth");
    aAny >>= nWidth;
    aAny = xSet->getPropertyValue("FrameMarginHeight");
    aAny >>= nHeight;

    if( !bIsAutoScroll )
    {
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_SCROLLBAR ), makeAny(bIsScrollingMode) );
        pStates++;
    }
    if( !bIsAutoBorder )
    {
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_BORDER ), makeAny(bIsBorderSet) );
        pStates++;
    }
    if( SIZE_NOT_SET != nWidth )
    {
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_HORI ), Any(nWidth) );
        pStates++;
    }
    if( SIZE_NOT_SET != nHeight )
    {
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_VERT ), Any(nHeight) );
    }
}

void SwXMLTextParagraphExport::_collectTextEmbeddedAutoStyles(
        const Reference < XPropertySet > & rPropSet )
{
    SwOLENode *pOLENd = GetNoTextNode( rPropSet )->GetOLENode();
    svt::EmbeddedObjectRef& rObjRef = pOLENd->GetOLEObj().GetObject();
    if( !rObjRef.is() )
        return;

    const XMLPropertyState *aStates[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    SvGlobalName aClassId( rObjRef->getClassID() );

    if( aIFrameClassId == aClassId )
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

    Add( XML_STYLE_FAMILY_TEXT_FRAME, rPropSet, aStates );

    const XMLPropertyState **pStates = aStates;
    while( *pStates )
    {
        delete *pStates;
        pStates++;
    }
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
    if( aPluginClassId == aClassId )
    {
        nType = SV_EMBEDDED_PLUGIN;
    }
    else if( aAppletClassId == aClassId )
    {
        nType = SV_EMBEDDED_APPLET;
    }
    else if( aIFrameClassId == aClassId )
    {
        nType = SV_EMBEDDED_FRAME;
    }
    else if ( !SotExchange::IsInternal( aClassId ) )
    {
        nType = SV_EMBEDDED_OUTPLACE;
    }

    enum XMLTokenEnum eElementName = XML__UNKNOWN_;
    SvXMLExport &rXMLExport = GetExport();

    // First the stuff common to each of Applet/Plugin/Floating Frame
    OUString sStyle;
    Any aAny;
    if( rPropSetInfo->hasPropertyByName( sFrameStyleName ) )
    {
        aAny = rPropSet->getPropertyValue( sFrameStyleName );
        aAny >>= sStyle;
    }

    const XMLPropertyState *aStates[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
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

    const OUString sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_FRAME,
                                      rPropSet, sStyle, aStates );
    const XMLPropertyState **pStates = aStates;
    while( *pStates )
    {
        delete *pStates;
        pStates++;
    }

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
                catch(const uno::Exception&)
                {
                    // TODO/LATER: error handling
                    OSL_FAIL( "Link detection or retrieving of the URL of OOo link is failed!\n" );
                }
            }

            if ( !bIsOwnLink )
            {
                sURL = sEmbeddedObjectProtocol + rOLEObj.GetCurrentPersistName();
            }

            sURL = GetExport().AddEmbeddedObject( sURL );
            lcl_addURL( rXMLExport, sURL, false );
        }
        if( SV_EMBEDDED_OWN == nType && !pOLENd->GetChartTableName().isEmpty() )
        {
            OUString sRange( pOLENd->GetChartTableName() );
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
                            aBuffer.append( '\'' );
                            aBuffer.append( sRange.copy( 0, i ) );
                        }
                        if( '\'' == c || '\\' == c )
                            aBuffer.append( '\\' );
                        SAL_FALLTHROUGH;
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
    case SV_EMBEDDED_APPLET:
        {
            // It's an applet!
            if( svt::EmbeddedObjectRef::TryRunningState( rObjRef.GetObject() ) )
            {
                uno::Reference < beans::XPropertySet > xSet( rObjRef->getComponent(), uno::UNO_QUERY );
                OUString aStr;
                Any aAny2 = xSet->getPropertyValue("AppletCodeBase");
                aAny2 >>= aStr;
                if (!aStr.isEmpty() )
                    lcl_addURL(rXMLExport, aStr);

                aAny2 = xSet->getPropertyValue("AppletName");
                aAny2 >>= aStr;
                if (!aStr.isEmpty())
                    rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_APPLET_NAME, aStr );

                aAny2 = xSet->getPropertyValue("AppletCode");
                aAny2 >>= aStr;
                rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CODE, aStr );

                bool bScript = false;
                aAny2 = xSet->getPropertyValue("AppletIsScript");
                aAny2 >>= bScript;
                rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MAY_SCRIPT, bScript ? XML_TRUE : XML_FALSE );

                uno::Sequence < beans::PropertyValue > aProps;
                aAny2 = xSet->getPropertyValue("AppletCommands");
                aAny2 >>= aProps;

                sal_Int32 i = aProps.getLength();
                while ( i > 0 )
                {
                    beans::PropertyValue& aProp = aProps[--i];
                    const SwHtmlOptType nType2 = SwApplet_Impl::GetOptionType( aProp.Name, true );
                    if ( nType2 == SwHtmlOptType::TAG)
                    {
                        OUString aStr2;
                        aProp.Value >>= aStr2;
                        rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, aProp.Name, aStr2);
                    }
                }

                eElementName = XML_APPLET;
            }
        }
        break;
    case SV_EMBEDDED_PLUGIN:
        {
            // It's a plugin!
            if ( svt::EmbeddedObjectRef::TryRunningState( rObjRef.GetObject() ) )
            {
                uno::Reference < beans::XPropertySet > xSet( rObjRef->getComponent(), uno::UNO_QUERY );
                OUString aStr;
                Any aAny2 = xSet->getPropertyValue("PluginURL");
                aAny2 >>= aStr;
                lcl_addURL( rXMLExport, aStr );

                aAny2 = xSet->getPropertyValue("PluginMimeType");
                aAny2 >>= aStr;
                if (!aStr.isEmpty())
                    rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, aStr );
                eElementName = XML_PLUGIN;
            }
        }
        break;
    case SV_EMBEDDED_FRAME:
        {
            // It's a floating frame!
            if ( svt::EmbeddedObjectRef::TryRunningState( rObjRef.GetObject() ) )
            {
                uno::Reference < beans::XPropertySet > xSet( rObjRef->getComponent(), uno::UNO_QUERY );
                OUString aStr;
                Any aAny2 = xSet->getPropertyValue("FrameURL");
                aAny2 >>= aStr;

                lcl_addURL( rXMLExport, aStr );

                aAny2 = xSet->getPropertyValue("FrameName");
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
                OUString sURL( sEmbeddedObjectProtocol + rOLEObj.GetCurrentPersistName() );

                if ( !( rXMLExport.getExportFlags() & SvXMLExportFlags::OASIS ) )
                    sURL += "?oasis=false";

                rXMLExport.AddEmbeddedObjectAsBase64( sURL );
            }
            break;
        case SV_EMBEDDED_APPLET:
            {
                if ( svt::EmbeddedObjectRef::TryRunningState( rObjRef.GetObject() ) )
                {
                    uno::Reference < beans::XPropertySet > xSet( rObjRef->getComponent(), uno::UNO_QUERY );
                    uno::Sequence < beans::PropertyValue > aProps;
                    aAny = xSet->getPropertyValue("AppletCommands");
                    aAny >>= aProps;

                    sal_Int32 i = aProps.getLength();
                    while ( i > 0 )
                    {
                        beans::PropertyValue& aProp = aProps[--i];
                        const SwHtmlOptType nType2 = SwApplet_Impl::GetOptionType( aProp.Name, true );
                        if (SwHtmlOptType::PARAM == nType2 || SwHtmlOptType::SIZE == nType2 )
                        {
                            OUString aStr;
                            aProp.Value >>= aStr;
                            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aProp.Name );
                            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                            SvXMLElementExport aElementExport2( rXMLExport, XML_NAMESPACE_DRAW, XML_PARAM, false, true );
                        }
                    }
                }
            }
            break;
        case SV_EMBEDDED_PLUGIN:
            {
                if ( svt::EmbeddedObjectRef::TryRunningState( rObjRef.GetObject() ) )
                {
                    uno::Reference < beans::XPropertySet > xSet( rObjRef->getComponent(), uno::UNO_QUERY );
                    uno::Sequence < beans::PropertyValue > aProps;
                    aAny = xSet->getPropertyValue("PluginCommands");
                    aAny >>= aProps;

                    sal_Int32 i = aProps.getLength();
                    while ( i > 0 )
                    {
                        beans::PropertyValue& aProp = aProps[--i];
                        const SwHtmlOptType nType2 = SwApplet_Impl::GetOptionType( aProp.Name, false );
                        if ( nType2 == SwHtmlOptType::TAG)
                        {
                            OUString aStr;
                            aProp.Value >>= aStr;
                            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aProp.Name );
                            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                            SvXMLElementExport aElementExport2( rXMLExport, XML_NAMESPACE_DRAW, XML_PARAM, false, true );
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    if( SV_EMBEDDED_OUTPLACE==nType || SV_EMBEDDED_OWN==nType )
    {
        OUString sURL( sGraphicObjectProtocol + rOLEObj.GetCurrentPersistName() );
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

    // Lastly the stuff common to each of Applet/Plugin/Floating Frame
    exportEvents( rPropSet );
    exportTitleAndDescription( rPropSet, rPropSetInfo );  // #i73249#
    exportContour( rPropSet, rPropSetInfo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
