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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <sot/clsids.hxx>

#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/txtprmap.hxx>

#include <bf_sfx2/frmdescr.hxx>


#include <horiornt.hxx>

#include <doc.hxx>

#include <errhdl.hxx>

#include <ndole.hxx>

#include <cppuhelper/implbase4.hxx>

#include <unostyle.hxx>
#include <unoframe.hxx>
#include <ndgrf.hxx>

#include "xmlexp.hxx"
#include "xmltexte.hxx"




#include <SwAppletImpl.hxx>

#define _SVSTDARR_ULONGS
#include <bf_svtools/svstdarr.hxx>

#include <SwStyleNameMapper.hxx>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::binfilter::xmloff::token;

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
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for embedded" );
    SwXFrame *pFrame =
                (SwXFrame *)xCrsrTunnel->getSomething(
                                    SwXFrame::getUnoTunnelId() );
    ASSERT( pFrame, "SwXFrame missing" );
    SwFrmFmt *pFrmFmt = pFrame->GetFrmFmt();
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    const SwNodeIndex *pNdIdx = rCntnt.GetCntntIdx();
    return	pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetNoTxtNode();
}

void SwXMLTextParagraphExport::exportStyleContent(
        const Reference< XStyle > & rStyle )
{

    const SwXStyle* pStyle = 0;
    Reference<XUnoTunnel> xStyleTunnel( rStyle, UNO_QUERY);
    if( xStyleTunnel.is() )
    {
        pStyle = (SwXStyle*)xStyleTunnel->getSomething(
                                            SwXStyle::getUnoTunnelId() );
    }
    if( pStyle && SFX_STYLE_FAMILY_PARA == pStyle->GetFamily() )
    {
        const SwDoc *pDoc = pStyle->GetDoc();
        const SwTxtFmtColl *pColl =
            pDoc->FindTxtFmtCollByName( pStyle->GetStyleName() );
        ASSERT( pColl, "There is the text collection?" );
        if( pColl && RES_CONDTXTFMTCOLL == pColl->Which() )
        {
            const SwFmtCollConditions& rConditions =
                ((const SwConditionTxtFmtColl *)pColl)->GetCondColls();
            for( sal_uInt16 i=0; i < rConditions.Count(); i++ )
            {
                const SwCollCondition& rCond = *rConditions[i];

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

                DBG_ASSERT( eFunc != XML_TOKEN_INVALID,
                            "SwXMLExport::ExportFmt: unknown condition" );
                if( eFunc != XML_TOKEN_INVALID )
                {
                    sBuffer.append( GetXMLToken(eFunc) );
                    sBuffer.append( (sal_Unicode)'(' );
                    sBuffer.append( (sal_Unicode)')' );
                    if( sVal.getLength() )
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
                                    GET_POOLID_TXTCOLL,
                                    sal_True);
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
         SvXMLAutoStylePoolP& rAutoStylePool ) :
    XMLTextParagraphExport( rExp, rAutoStylePool ),
    sTextTable( RTL_CONSTASCII_USTRINGPARAM( "TextTable" ) ),
    sEmbeddedObjectProtocol( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ) ),
    aAppletClassId( SO3_APPLET_CLASSID ), //STRIP003 
    aPluginClassId( SO3_PLUGIN_CLASSID ), //STRIP003 
    aIFrameClassId( BF_SO3_IFRAME_CLASSID ),
    aOutplaceClassId( SO3_OUT_CLASSID ) //STRIP003 
{
}

SwXMLTextParagraphExport::~SwXMLTextParagraphExport()
{
}

void SwXMLTextParagraphExport::setTextEmbeddedGraphicURL(
    const Reference < XPropertySet >& rPropSet,
    OUString& rURL) const
{
    if( !rURL.getLength() )
        return;

    SwGrfNode *pGrfNd = GetNoTxtNode( rPropSet )->GetGrfNode();
    if( !pGrfNd->IsGrfLink() )
    {
        String aNewURL( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.Package:") );
        aNewURL += String(rURL.copy( 1 ) );
        pGrfNd->SetNewStreamName( aNewURL );
    }
}

static void lcl_addParam ( SvXMLExport &rExport, const SvCommand &rCommand )
{
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, rCommand.GetCommand() );
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, rCommand.GetArgument() );
    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
}
static void lcl_addURL ( SvXMLExport &rExport, const String &rURL,
                         sal_Bool bToRel = sal_True )
{
    String sRelURL;

    if( bToRel && (rURL.Len() > 0) )
        sRelURL = ::binfilter::StaticBaseUrl::AbsToRel(rURL,
                      INetURLObject::WAS_ENCODED,
                      INetURLObject::DECODE_UNAMBIGUOUS);
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

void lcl_addOutplaceProperties(
        const SvInfoObject *pInfo,
        const XMLPropertyState **pStates,
        const UniReference < XMLPropertySetMapper >& rMapper )
{
    SvEmbeddedInfoObject * pEmbed = PTR_CAST(SvEmbeddedInfoObject, pInfo );
    if( pEmbed )
    {
        const Rectangle& rVisArea = pEmbed->GetVisArea();
        if( !rVisArea.IsEmpty() )
        {
            Any aAny;

            aAny <<= (sal_Int32)rVisArea.Left();
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_LEFT ), aAny );
            pStates++;

            aAny <<= (sal_Int32)rVisArea.Top();
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_TOP ), aAny );
            pStates++;

            aAny <<= (sal_Int32)rVisArea.GetWidth();
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_WIDTH ), aAny );
            pStates++;

            aAny <<= (sal_Int32)rVisArea.GetHeight();
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_HEIGHT ), aAny );
            pStates++;

            aAny <<= (sal_Int32)pEmbed->GetViewAspect();
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_DRAW_ASPECT ), aAny );
            pStates++;

            // TODO: aspect
        }
    }
}

void lcl_addFrameProperties(
        const SfxFrameDescriptor *pDescriptor,
        const XMLPropertyState **pStates,
        const UniReference < XMLPropertySetMapper >& rMapper )
{
    if( ScrollingAuto != pDescriptor->GetScrollingMode() )
    {
        sal_Bool bValue = ScrollingYes == pDescriptor->GetScrollingMode();
        Any aAny( &bValue, ::getBooleanCppuType() );
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_SCROLLBAR ), aAny );
        pStates++;
    }
    if( pDescriptor->IsFrameBorderSet() )
    {
        sal_Bool bValue = pDescriptor->IsFrameBorderOn();
        Any aAny( &bValue, ::getBooleanCppuType() );
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_BORDER ), aAny );
        pStates++;
    }
    const Size& rMargins = pDescriptor->GetMargin();
    if( SIZE_NOT_SET != rMargins.Width() )
    {
        Any aAny;
        aAny <<= (sal_Int32)rMargins.Width();
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_HORI ), aAny );
        pStates++;
    }
    if( SIZE_NOT_SET != rMargins.Height() )
    {
        Any aAny;
        aAny <<= (sal_Int32)rMargins.Height();
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_VERT ), aAny );
        pStates++;
    }
}

void SwXMLTextParagraphExport::_collectTextEmbeddedAutoStyles(
        const Reference < XPropertySet > & rPropSet )
{
    SwOLENode *pOLENd = GetNoTxtNode( rPropSet )->GetOLENode();
    SwOLEObj& rOLEObj = pOLENd->GetOLEObj();
    SvPersist *pPersist = pOLENd->GetDoc()->GetPersist();
    ASSERT( pPersist, "no persist" );
    const SvInfoObject *pInfo = pPersist->Find( rOLEObj.GetName() );
    DBG_ASSERT( pInfo, "no info object for OLE object found" );

    if( !pInfo )
        return;

    const XMLPropertyState *aStates[7] = { 0, 0, 0, 0, 0, 0, 0 };
    SvGlobalName aClassId( pInfo->GetClassName() );

    if( aOutplaceClassId == aClassId )
    {
        lcl_addOutplaceProperties( pInfo, aStates,
               GetAutoFramePropMapper()->getPropertySetMapper() );
    }
    else if( aIFrameClassId == aClassId )
    {
        SfxFrameObjectRef xFrame( rOLEObj.GetOleRef() );
        ASSERT( xFrame.Is(), "wrong class id for frame" );

        lcl_addFrameProperties( xFrame->GetFrameDescriptor(), aStates,
               GetAutoFramePropMapper()->getPropertySetMapper() );
    }

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
    SvPersist *pPersist = pOLENd->GetDoc()->GetPersist();
    const SvInfoObject *pInfo = pPersist->Find( rOLEObj.GetName() );
    DBG_ASSERT( pInfo, "no info object for OLE object found" );

    if( !pInfo )
        return;

    SvGlobalName aClassId( pInfo->GetClassName() );

    SvEmbeddedObjectTypes nType = SV_EMBEDDED_OWN;
    SvPlugInObjectRef xPlugin;
    SvAppletObjectRef xApplet;
    SfxFrameObjectRef xFrame;
    if( aPluginClassId == aClassId )
    {
        xPlugin = SvPlugInObjectRef( rOLEObj.GetOleRef() );
        ASSERT( xPlugin.Is(), "wrong class id for plugin" );
        nType = SV_EMBEDDED_PLUGIN;
    }
    else if( aAppletClassId == aClassId )
    {
        xApplet = SvAppletObjectRef( rOLEObj.GetOleRef() );
        ASSERT( xApplet.Is(), "wrong class id for applet" );
        nType = SV_EMBEDDED_APPLET;
    }
    else if( aIFrameClassId == aClassId )
    {
        xFrame = SfxFrameObjectRef( rOLEObj.GetOleRef() );
        ASSERT( xFrame.Is(), "wrong class id for frame" );
        nType = SV_EMBEDDED_FRAME;
    }
    else if( aOutplaceClassId == aClassId )
    {
        nType = SV_EMBEDDED_OUTPLACE;
    }

    SvULongs aParams;
    enum XMLTokenEnum eElementName;
    SvXMLExport &rExport = GetExport();

    // First the stuff common to each of Applet/Plugin/Floating Frame
    OUString sStyle;
    Any aAny;
    if( rPropSetInfo->hasPropertyByName( sFrameStyleName ) )
    {
        aAny = rPropSet->getPropertyValue( sFrameStyleName );
        aAny >>= sStyle;
    }

    const XMLPropertyState *aStates[7] = { 0, 0, 0, 0, 0, 0, 0 };
    switch( nType )
    {
    case SV_EMBEDDED_FRAME:
        lcl_addFrameProperties( xFrame->GetFrameDescriptor(), aStates,
            GetAutoFramePropMapper()->getPropertySetMapper() );
        break;
    case SV_EMBEDDED_OUTPLACE:
        lcl_addOutplaceProperties( pInfo, aStates,
            GetAutoFramePropMapper()->getPropertySetMapper() );
        break;
    }

    OUString sAutoStyle( sStyle );
    sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_FRAME, rPropSet, sStyle,
                       aStates );
    const XMLPropertyState **pStates = aStates;
    while( *pStates )
    {
        delete *pStates;
        pStates++;
    }

    if( sAutoStyle.getLength() )
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE_NAME, sAutoStyle );
    addTextFrameAttributes( rPropSet, sal_False );

    switch (nType)
    {
    case SV_EMBEDDED_OUTPLACE:
    case SV_EMBEDDED_OWN:
        if( (rExport.getExportFlags() & EXPORT_EMBEDDED) == 0 )
        {
            OUString sURL( sEmbeddedObjectProtocol );
            sURL += pInfo->GetStorageName();
            sURL = GetExport().AddEmbeddedObject( sURL );
            lcl_addURL( rExport, sURL, sal_False );
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

            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES,
            sRange );
        }
        eElementName = SV_EMBEDDED_OUTPLACE==nType ? XML_OBJECT_OLE
                                                   : XML_OBJECT;
        break;
    case SV_EMBEDDED_APPLET:
        {
            // It's an applet!
            const XubString & rURL = xApplet->GetCodeBase();
            if (rURL.Len() )
                lcl_addURL(rExport, rURL);

            const String &rName = xApplet->GetName();
            if (rName.Len())
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_APPLET_NAME,
                                      rName );

            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CODE,
                                  xApplet->GetClass() );

            const SvCommandList& rCommands = xApplet->GetCommandList();

            ULONG i = rCommands.Count();
            while ( i > 0 )
            {
                const SvCommand& rCommand = rCommands [ --i ];
                const String &rName = rCommand.GetCommand();
                USHORT nType = SwApplet_Impl::GetOptionType( rName, TRUE );
                if ( nType == SWHTML_OPTTYPE_TAG)
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, rName, rCommand.GetArgument());
                else if (SWHTML_OPTTYPE_PARAM == nType ||
                            SWHTML_OPTTYPE_SIZE == nType )
                    aParams.Insert( i, aParams.Count() );
            }

            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MAY_SCRIPT,
                        xApplet->IsMayScript() ? XML_TRUE : XML_FALSE );
            eElementName = XML_APPLET;
        }
        break;
    case SV_EMBEDDED_PLUGIN:
        {
            // It's a plugin!
            lcl_addURL( rExport, xPlugin->GetURL()->GetMainURL( INetURLObject::NO_DECODE ) );
            const String &rType = xPlugin->GetMimeType();
            if (rType.Len())
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, rType );
            eElementName = XML_PLUGIN;
        }
        break;
    case SV_EMBEDDED_FRAME:
        {
            // It's a floating frame!
            const SfxFrameDescriptor *pDescriptor = xFrame->GetFrameDescriptor();

            lcl_addURL( rExport, pDescriptor->GetURL().GetMainURL( INetURLObject::NO_DECODE ) );

            const String &rName = pDescriptor->GetName();
            if (rName.Len())
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FRAME_NAME, rName );
            eElementName = XML_FLOATING_FRAME;
        }
        break;
    default:
        ASSERT( !this, "unknown object type! Base class should have been called!" );
    }

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, eElementName,
                                  sal_False, sal_True );
    switch( nType )
    {
    case SV_EMBEDDED_OWN:
        if( (rExport.getExportFlags() & EXPORT_EMBEDDED) != 0 )
        {
            Reference < XEmbeddedObjectSupplier > xEOS( rPropSet, UNO_QUERY );
            ASSERT( xEOS.is(), "no embedded object supplier for own object" );
            Reference < XComponent > xComp = xEOS->getEmbeddedObject();
            rExport.ExportEmbeddedOwnObject( xComp );
        }
        break;
    case SV_EMBEDDED_OUTPLACE:
        if( (rExport.getExportFlags() & EXPORT_EMBEDDED) != 0 )
        {
            OUString sURL( sEmbeddedObjectProtocol );
            sURL += rOLEObj.GetName();
            GetExport().AddEmbeddedObjectAsBase64( sURL );
        }
        break;
    case SV_EMBEDDED_APPLET:
        {
            const SvCommandList& rCommands = xApplet->GetCommandList();
            USHORT ii = aParams.Count();
            while ( ii > 0 )
            {
                const SvCommand& rCommand = rCommands [ aParams [ --ii] ];
                lcl_addParam (rExport, rCommand );
            }
        }
        break;
    case SV_EMBEDDED_PLUGIN:
        {
            const SvCommandList& rCommands = xPlugin->GetCommandList();
            ULONG nCommands = rCommands.Count();
            for ( ULONG i = 0; i < nCommands; i++)
            {
                const SvCommand& rCommand = rCommands [ i ];
                const String& rName = rCommand.GetCommand();
                if (SwApplet_Impl::GetOptionType( rName, FALSE ) == SWHTML_OPTTYPE_TAG )
                    lcl_addParam (rExport, rCommand );
            }
        }
        break;
    }

    // Lastly the stuff common to each of Applet/Plugin/Floating Frame
    exportEvents( rPropSet );
    exportAlternativeText( rPropSet, rPropSetInfo );
    exportContour( rPropSet, rPropSetInfo );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
