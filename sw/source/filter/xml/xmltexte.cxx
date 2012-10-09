/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sot/clsids.hxx>
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

#include "svl/urihelper.hxx"
#include <sfx2/frmdescr.hxx>
#include <SwStyleNameMapper.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;
using namespace ::xmloff::token;
using rtl::OUString;
using rtl::OUStringBuffer;

enum SvEmbeddedObjectTypes
{
    SV_EMBEDDED_OWN,
    SV_EMBEDDED_OUTPLACE,
    SV_EMBEDDED_APPLET,
    SV_EMBEDDED_PLUGIN,
    SV_EMBEDDED_FRAME
};

// ---------------------------------------------------------------------

SwNoTxtNode *SwXMLTextParagraphExport::GetNoTxtNode(
    const Reference < XPropertySet >& rPropSet ) const
{
    Reference<XUnoTunnel> xCrsrTunnel( rPropSet, UNO_QUERY );
    OSL_ENSURE( xCrsrTunnel.is(), "missing XUnoTunnel for embedded" );
    SwXFrame *pFrame = reinterpret_cast< SwXFrame * >(
                sal::static_int_cast< sal_IntPtr >( xCrsrTunnel->getSomething( SwXFrame::getUnoTunnelId() )));
    OSL_ENSURE( pFrame, "SwXFrame missing" );
    SwFrmFmt *pFrmFmt = pFrame->GetFrmFmt();
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    const SwNodeIndex *pNdIdx = rCntnt.GetCntntIdx();
    return  pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetNoTxtNode();
}

void SwXMLTextParagraphExport::exportStyleContent(
        const Reference< XStyle > & rStyle )
{

    const SwXStyle* pStyle = 0;
    Reference<XUnoTunnel> xStyleTunnel( rStyle, UNO_QUERY);
    if( xStyleTunnel.is() )
    {
        pStyle = reinterpret_cast< SwXStyle * >(
                sal::static_int_cast< sal_IntPtr >( xStyleTunnel->getSomething( SwXStyle::getUnoTunnelId() )));
    }
    if( pStyle && SFX_STYLE_FAMILY_PARA == pStyle->GetFamily() )
    {
        const SwDoc *pDoc = pStyle->GetDoc();
        const SwTxtFmtColl *pColl =
            pDoc->FindTxtFmtCollByName( pStyle->GetStyleName() );
        OSL_ENSURE( pColl, "There is the text collection?" );
        if( pColl && RES_CONDTXTFMTCOLL == pColl->Which() )
        {
            const SwFmtCollConditions& rConditions =
                ((const SwConditionTxtFmtColl *)pColl)->GetCondColls();
            for( sal_uInt16 i=0; i < rConditions.size(); i++ )
            {
                const SwCollCondition& rCond = rConditions[i];

                enum XMLTokenEnum eFunc = XML_TOKEN_INVALID;
                OUStringBuffer sBuffer( 20 );
                switch( rCond.GetCondition() )
                {
                case PARA_IN_LIST:
                    eFunc = XML_LIST_LEVEL;
                    sBuffer.append( (sal_Int32)(rCond.GetSubCondition()+1) );
                    break;
                case PARA_IN_OUTLINE:
                    eFunc = XML_OUTLINE_LEVEL;
                    sBuffer.append( (sal_Int32)(rCond.GetSubCondition()+1) );
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
                OUString sVal( sBuffer.makeStringAndClear() );

                OSL_ENSURE( eFunc != XML_TOKEN_INVALID,
                            "SwXMLExport::ExportFmt: unknown condition" );
                if( eFunc != XML_TOKEN_INVALID )
                {
                    sBuffer.append( GetXMLToken(eFunc) );
                    sBuffer.append( (sal_Unicode)'(' );
                    sBuffer.append( (sal_Unicode)')' );
                    if( !sVal.isEmpty() )
                    {
                        sBuffer.append( (sal_Unicode)'=' );
                        sBuffer.append( sVal );
                    }

                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                XML_CONDITION,
                                sBuffer.makeStringAndClear() );
                    String aString;
                    SwStyleNameMapper::FillProgName(
                                    rCond.GetTxtFmtColl()->GetName(),
                                    aString,
                                    nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL,
                                    true);
                    aString = GetExport().EncodeStyleName( aString );
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                XML_APPLY_STYLE_NAME, aString );
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                              XML_MAP, sal_True, sal_True );
                }
            }
        }
    }
}

SwXMLTextParagraphExport::SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& _rAutoStylePool ) :
    XMLTextParagraphExport( rExp, _rAutoStylePool ),
    sTextTable( RTL_CONSTASCII_USTRINGPARAM( "TextTable" ) ),
    sEmbeddedObjectProtocol( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ) ),
    sGraphicObjectProtocol( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) ),
    aAppletClassId( SO3_APPLET_CLASSID ),
    aPluginClassId( SO3_PLUGIN_CLASSID ),
    aIFrameClassId( SO3_IFRAME_CLASSID ),
    aOutplaceClassId( SO3_OUT_CLASSID )
{
}

SwXMLTextParagraphExport::~SwXMLTextParagraphExport()
{
}

void SwXMLTextParagraphExport::setTextEmbeddedGraphicURL(
    const Reference < XPropertySet >& rPropSet,
    OUString& rURL) const
{
    if( rURL.isEmpty() )
        return;

    SwGrfNode *pGrfNd = GetNoTxtNode( rPropSet )->GetGrfNode();
    if( !pGrfNd->IsGrfLink() )
    {
        String aNewURL( RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.Package:") );
        aNewURL += String(rURL);

// This is nonsensical.
//        pGrfNd->SetNewStreamName( aNewURL );

        // #i15411# save-as will swap all graphics in; we need to swap
        // them out again, to prevent excessive memory use
        pGrfNd->SwapOut();
    }
}

static void lcl_addURL ( SvXMLExport &rExport, const String &rURL,
                         sal_Bool bToRel = sal_True )
{
    String sRelURL;

    if( bToRel && (rURL.Len() > 0) )
    sRelURL = URIHelper::simpleNormalizedMakeRelative(rExport.GetOrigFileName(),
              rURL);
    else
        sRelURL = rURL;

    if (sRelURL.Len())
    {
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, sRelURL );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
    }
}

void lcl_addAspect(
        const svt::EmbeddedObjectRef& rObj,
        const XMLPropertyState **pStates,
        const UniReference < XMLPropertySetMapper >& rMapper )
{
    {
        sal_Int64 nAspect = rObj.GetViewAspect();

        if ( nAspect )
        {
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_DRAW_ASPECT ), uno::makeAny( nAspect ) );
            pStates++;
        }
    }
}

void lcl_addOutplaceProperties(
        const svt::EmbeddedObjectRef& rObj,
        const XMLPropertyState **pStates,
        const UniReference < XMLPropertySetMapper >& rMapper )
{
    {
        MapMode aMode( MAP_100TH_MM ); // the API expects this map mode for the embedded objects
        Size aSize = rObj.GetSize( &aMode ); // get the size in the requested map mode

        if( aSize.Width() && aSize.Height() )
        {
            Any aAny;
            aAny <<= 0L;
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_LEFT ), aAny );
            pStates++;

            aAny <<= 0L;
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_TOP ), aAny );
            pStates++;

            aAny <<= (sal_Int32)aSize.Width();
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_WIDTH ), aAny );
            pStates++;

            aAny <<= (sal_Int32)aSize.Height();
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_HEIGHT ), aAny );
            pStates++;
        }
    }
}

void lcl_addFrameProperties(
        const uno::Reference < embed::XEmbeddedObject >& xObj,
        const XMLPropertyState **pStates,
        const UniReference < XMLPropertySetMapper >& rMapper )
{
    if ( !::svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        return;

    uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
    if ( !xSet.is() )
        return;

    ::rtl::OUString aURL;
    Any aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameURL")) );
    aAny >>= aURL;

    ::rtl::OUString aName;
    aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameName")));
    aAny >>= aName;

    sal_Bool bIsAutoScroll = sal_False, bIsScrollingMode;
    aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsAutoScroll")));
    aAny >>= bIsAutoScroll;
    if ( !bIsAutoScroll )
    {
        aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsScrollingMode")) );
        aAny >>= bIsScrollingMode;
    }

    sal_Bool bIsBorderSet, bIsAutoBorder = sal_False;
    aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsAutoBorder")) );
    aAny >>= bIsAutoBorder;
    if ( !bIsAutoBorder )
    {
        aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameIsBorder")) );
        aAny >>= bIsBorderSet;
    }

    sal_Int32 nWidth, nHeight;
    aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameMarginWidth")) );
    aAny >>= nWidth;
    aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameMarginHeight")) );
    aAny >>= nHeight;

    if( !bIsAutoScroll )
    {
        Any aAny2( &bIsScrollingMode, ::getBooleanCppuType() );
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_SCROLLBAR ), aAny2 );
        pStates++;
    }
    if( !bIsAutoBorder )
    {
        Any aAny2( &bIsBorderSet, ::getBooleanCppuType() );
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_BORDER ), aAny2 );
        pStates++;
    }
    if( SIZE_NOT_SET != nWidth )
    {
        Any aAny2;
        aAny <<= nWidth;
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_HORI ), aAny2 );
        pStates++;
    }
    if( SIZE_NOT_SET != nHeight )
    {
        Any aAny2;
        aAny <<= nHeight;
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_VERT ), aAny2 );
        pStates++;
    }
}

void SwXMLTextParagraphExport::_collectTextEmbeddedAutoStyles(
        const Reference < XPropertySet > & rPropSet )
{
    SwOLENode *pOLENd = GetNoTxtNode( rPropSet )->GetOLENode();
    svt::EmbeddedObjectRef& rObjRef = pOLENd->GetOLEObj().GetObject();
    if( !rObjRef.is() )
        return;

    const XMLPropertyState *aStates[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
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
    SwOLENode *pOLENd = GetNoTxtNode( rPropSet )->GetOLENode();
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

    const XMLPropertyState *aStates[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
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

    OUString sAutoStyle( sStyle );
    sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_FRAME, rPropSet, sStyle,
                       aStates );
    const XMLPropertyState **pStates = aStates;
    while( *pStates )
    {
        delete *pStates;
        pStates++;
    }

    if( !sAutoStyle.isEmpty() )
        rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE_NAME, sAutoStyle );
    addTextFrameAttributes( rPropSet, sal_False );

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              XML_FRAME, sal_False, sal_True );

    switch (nType)
    {
    case SV_EMBEDDED_OUTPLACE:
    case SV_EMBEDDED_OWN:
        if( (rXMLExport.getExportFlags() & EXPORT_EMBEDDED) == 0 )
        {
            OUString sURL;

            sal_Bool bIsOwnLink = sal_False;
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
                sURL = OUString( sEmbeddedObjectProtocol );
                sURL += rOLEObj.GetCurrentPersistName();
            }

            sURL = GetExport().AddEmbeddedObject( sURL );
            lcl_addURL( rXMLExport, sURL, sal_False );
        }
        if( SV_EMBEDDED_OWN == nType && pOLENd->GetChartTblName().Len() )
        {
            OUString sRange( pOLENd->GetChartTblName() );
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
                        if( !aBuffer.getLength() )
                        {
                            aBuffer.append( (sal_Unicode)'\'' );
                            aBuffer.append( sRange.copy( 0, i ) );
                        }
                        if( '\'' == c || '\\' == c )
                            aBuffer.append( (sal_Unicode)'\\' );
                        // no break!
                    default:
                        if( aBuffer.getLength() )
                            aBuffer.append( c );
                }
            }
            if( aBuffer.getLength() )
            {
                aBuffer.append( (sal_Unicode)'\'' );
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
                ::rtl::OUString aStr;
                Any aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletCodeBase")) );
                aAny2 >>= aStr;
                if (!aStr.isEmpty() )
                    lcl_addURL(rXMLExport, aStr);

                aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletName")) );
                aAny2 >>= aStr;
                if (!aStr.isEmpty())
                    rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_APPLET_NAME, aStr );

                aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletCode")) );
                aAny2 >>= aStr;
                rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CODE, aStr );

                sal_Bool bScript = sal_False;
                aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletIsScript")) );
                aAny2 >>= bScript;
                rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MAY_SCRIPT, bScript ? XML_TRUE : XML_FALSE );

                uno::Sequence < beans::PropertyValue > aProps;
                aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletCommands")) );
                aAny2 >>= aProps;

                sal_Int32 i = aProps.getLength();
                while ( i > 0 )
                {
                    beans::PropertyValue& aProp = aProps[--i];
                    sal_uInt16 nType2 = SwApplet_Impl::GetOptionType( aProp.Name, sal_True );
                    if ( nType2 == SWHTML_OPTTYPE_TAG)
                    {
                        ::rtl::OUString aStr2;
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
                ::rtl::OUString aStr;
                Any aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PluginURL")) );
                aAny2 >>= aStr;
                lcl_addURL( rXMLExport, aStr );

                aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PluginMimeType")) );
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
                ::rtl::OUString aStr;
                Any aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameURL")) );
                aAny2 >>= aStr;

                lcl_addURL( rXMLExport, aStr );

                aAny2 = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameName")) );
                aAny2 >>= aStr;

                if (!aStr.isEmpty())
                    rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FRAME_NAME, aStr );
                eElementName = XML_FLOATING_FRAME;
            }
        }
        break;
    default:
        OSL_ENSURE( !this, "unknown object type! Base class should have been called!" );
    }

    {
        SvXMLElementExport aElementExport( rXMLExport, XML_NAMESPACE_DRAW, eElementName,
                                      sal_False, sal_True );
        switch( nType )
        {
        case SV_EMBEDDED_OWN:
            if( (rXMLExport.getExportFlags() & EXPORT_EMBEDDED) != 0 )
            {
                Reference < XEmbeddedObjectSupplier > xEOS( rPropSet, UNO_QUERY );
                OSL_ENSURE( xEOS.is(), "no embedded object supplier for own object" );
                Reference < XComponent > xComp = xEOS->getEmbeddedObject();
                rXMLExport.ExportEmbeddedOwnObject( xComp );
            }
            break;
        case SV_EMBEDDED_OUTPLACE:
            if( (rXMLExport.getExportFlags() & EXPORT_EMBEDDED) != 0 )
            {
                OUString sURL( sEmbeddedObjectProtocol );
                sURL += rOLEObj.GetCurrentPersistName();

                if ( ( rXMLExport.getExportFlags() & EXPORT_OASIS ) == 0 )
                    sURL += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "?oasis=false" ) );

                rXMLExport.AddEmbeddedObjectAsBase64( sURL );
            }
            break;
        case SV_EMBEDDED_APPLET:
            {
                if ( svt::EmbeddedObjectRef::TryRunningState( rObjRef.GetObject() ) )
                {
                    uno::Reference < beans::XPropertySet > xSet( rObjRef->getComponent(), uno::UNO_QUERY );
                    uno::Sequence < beans::PropertyValue > aProps;
                    aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletCommands")) );
                    aAny >>= aProps;

                    sal_Int32 i = aProps.getLength();
                    while ( i > 0 )
                    {
                        beans::PropertyValue& aProp = aProps[--i];
                        sal_uInt16 nType2 = SwApplet_Impl::GetOptionType( aProp.Name, sal_True );
                        if (SWHTML_OPTTYPE_PARAM == nType2 || SWHTML_OPTTYPE_SIZE == nType2 )
                        {
                            ::rtl::OUString aStr;
                            aProp.Value >>= aStr;
                            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aProp.Name );
                            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                            SvXMLElementExport aElementExport2( rXMLExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
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
                    aAny = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PluginCommands")) );
                    aAny >>= aProps;

                    sal_Int32 i = aProps.getLength();
                    while ( i > 0 )
                    {
                        beans::PropertyValue& aProp = aProps[--i];
                        sal_uInt16 nType2 = SwApplet_Impl::GetOptionType( aProp.Name, sal_False );
                        if ( nType2 == SWHTML_OPTTYPE_TAG)
                        {
                            ::rtl::OUString aStr;
                            aProp.Value >>= aStr;
                            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aProp.Name );
                            rXMLExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                            SvXMLElementExport aElementExport2( rXMLExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
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
        OUString sURL( sGraphicObjectProtocol );
        sURL += rOLEObj.GetCurrentPersistName();
        if( (rXMLExport.getExportFlags() & EXPORT_EMBEDDED) == 0 )
        {
            sURL = GetExport().AddEmbeddedObject( sURL );
            lcl_addURL( rXMLExport, sURL, sal_False );
        }

        SvXMLElementExport aElementExport( GetExport(), XML_NAMESPACE_DRAW,
                                  XML_IMAGE, sal_False, sal_True );

        if( (rXMLExport.getExportFlags() & EXPORT_EMBEDDED) != 0 )
            GetExport().AddEmbeddedObjectAsBase64( sURL );
    }

    // Lastly the stuff common to each of Applet/Plugin/Floating Frame
    exportEvents( rPropSet );
    exportTitleAndDescription( rPropSet, rPropSetInfo );  // #i73249#
    exportContour( rPropSet, rPropSetInfo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
