/*************************************************************************
 *
 *  $RCSfile: xmltexte.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 16:28:45 $
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


#pragma hdrstop

#ifndef _SOT_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_XEMBEDOBJECTCREATOR_HPP_
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XLINKAGESUPPORT_HPP_
#include <com/sun/star/embed/XLinkageSupport.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XCLASSIFIEDOBJECT_HPP_
#include <com/sun/star/embed/XClassifiedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDOBJECTSUPPLIER_HPP_
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif

#include <svx/svdobj.hxx>
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif

#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLTEXTE_HXX
#include "xmltexte.hxx"
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SW_APPLET_IMPL_HXX
#include <SwAppletImpl.hxx>
#endif

#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

#ifndef _SFX_FRMDESCRHXX
#include <sfx2/frmdescr.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _XMLOFF_XMLBASE64EXPORT_HXX_
#include <xmloff/XMLBase64Export.hxx>
#endif

using namespace ::rtl;
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

const sal_Char sObjectReplacements[] = "ObjectReplacements/";

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
    return  pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetNoTxtNode();
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
         SvXMLAutoStylePoolP& rAutoStylePool ) :
    XMLTextParagraphExport( rExp, rAutoStylePool ),
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
    if( !rURL.getLength() )
        return;

    SwGrfNode *pGrfNd = GetNoTxtNode( rPropSet )->GetGrfNode();
    if( !pGrfNd->IsGrfLink() )
    {
        String aNewURL( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.Package:") );
        aNewURL += String(rURL);
        pGrfNd->SetNewStreamName( aNewURL );

        // #i15411# save-as will swap all graphics in; we need to swap
        // them out again, to prevent excessive memory use
        pGrfNd->SwapOut();
    }
}
/*
static void lcl_addParam ( SvXMLExport &rExport, const SvCommand &rCommand )
{
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, rCommand.GetCommand() );
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, rCommand.GetArgument() );
    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
}*/

static void lcl_addURL ( SvXMLExport &rExport, const String &rURL,
                         sal_Bool bToRel = sal_True )
{
    String sRelURL;

    if( bToRel && (rURL.Len() > 0) )
        sRelURL = INetURLObject::AbsToRel(rURL,
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
        const uno::Reference < embed::XEmbeddedObject >& xObj,
        const XMLPropertyState **pStates,
        const UniReference < XMLPropertySetMapper >& rMapper )
{
    {
        //TODO/LATER: only VisAreaSize is available!
        //const Rectangle& rVisArea = pEmbed->GetVisArea();
        awt::Size aSz = xObj->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
        if( aSz.Width && aSz.Height )
        {
            Any aAny;
            //aAny <<= (sal_Int32)rVisArea.Left();
            aAny <<= 0L;
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_LEFT ), aAny );
            pStates++;

            //aAny <<= (sal_Int32)rVisArea.Top();
            aAny <<= 0L;
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_TOP ), aAny );
            pStates++;

            //aAny <<= (sal_Int32)rVisArea.GetWidth();
            aAny <<= aSz.Width;
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_WIDTH ), aAny );
            pStates++;

            //aAny <<= (sal_Int32)rVisArea.GetHeight();
            aAny <<= aSz.Height;
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_VIS_AREA_HEIGHT ), aAny );
            pStates++;

            //TODO/LATER: no aspect in object and wrong data type
            //aAny <<= (sal_Int32)pEmbed->GetViewAspect();
            aAny <<= (sal_Int32) embed::Aspects::MSOLE_CONTENT;
            *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_OLE_DRAW_ASPECT ), aAny );
            pStates++;

            // TODO: aspect
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
    Any aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameURL") );
    aAny >>= aURL;

    ::rtl::OUString aName;
    aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameName") );
    aAny >>= aName;

    sal_Bool bIsAutoScroll, bIsScrollingMode;
    aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsAutoScroll") );
    aAny >>= bIsAutoScroll;
    if ( !bIsAutoScroll )
    {
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsScrollingMode") );
        aAny >>= bIsScrollingMode;
    }

    sal_Bool bIsBorderSet, bIsAutoBorder;
    aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsAutoBorder") );
    aAny >>= bIsAutoBorder;
    if ( !bIsAutoBorder )
    {
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsBorder") );
        aAny >>= bIsBorderSet;
    }

    sal_Int32 nWidth, nHeight;
    aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameMarginWidth") );
    aAny >>= nWidth;
    aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameMarginHeight") );
    aAny >>= nHeight;

    if( !bIsAutoScroll )
    {
        Any aAny( &bIsScrollingMode, ::getBooleanCppuType() );
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_SCROLLBAR ), aAny );
        pStates++;
    }
    if( !bIsAutoBorder )
    {
        Any aAny( &bIsBorderSet, ::getBooleanCppuType() );
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_DISPLAY_BORDER ), aAny );
        pStates++;
    }
    if( SIZE_NOT_SET != nWidth )
    {
        Any aAny;
        aAny <<= nWidth;
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_HORI ), aAny );
        pStates++;
    }
    if( SIZE_NOT_SET != nHeight )
    {
        Any aAny;
        aAny <<= nHeight;
        *pStates = new XMLPropertyState( rMapper->FindEntryIndex( CTF_FRAME_MARGIN_VERT ), aAny );
        pStates++;
    }
}

void SwXMLTextParagraphExport::_collectTextEmbeddedAutoStyles(
        const Reference < XPropertySet > & rPropSet )
{
    SwOLENode *pOLENd = GetNoTxtNode( rPropSet )->GetOLENode();
    SwOLEObj& rOLEObj = pOLENd->GetOLEObj();
    SfxObjectShell *pPersist = pOLENd->GetDoc()->GetPersist();
    ASSERT( pPersist, "no persist" );
    uno::Reference < embed::XEmbeddedObject > xObj = pPersist->GetEmbeddedObjectContainer().GetEmbeddedObject( rOLEObj.GetCurrentPersistName() );
    if( !xObj.is() )
        return;

    const XMLPropertyState *aStates[7] = { 0, 0, 0, 0, 0, 0, 0 };
    SvGlobalName aClassId( xObj->getClassID() );

    if( aIFrameClassId == aClassId )
    {
        lcl_addFrameProperties( xObj, aStates,
               GetAutoFramePropMapper()->getPropertySetMapper() );
    }
    else if ( !SotExchange::IsInternal( aClassId ) )
    {
        lcl_addOutplaceProperties( xObj, aStates,
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
    SfxObjectShell *pPersist = pOLENd->GetDoc()->GetPersist();
    uno::Reference < embed::XEmbeddedObject > xObj = pPersist->GetEmbeddedObjectContainer().GetEmbeddedObject( rOLEObj.GetCurrentPersistName() );
    if( !xObj.is() )
        return;

    SvGlobalName aClassId( xObj->getClassID() );

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
        lcl_addFrameProperties( xObj, aStates,
            GetAutoFramePropMapper()->getPropertySetMapper() );
        break;
    case SV_EMBEDDED_OUTPLACE:
        lcl_addOutplaceProperties( xObj, aStates,
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

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              XML_FRAME, sal_False, sal_True );

    switch (nType)
    {
    case SV_EMBEDDED_OUTPLACE:
    case SV_EMBEDDED_OWN:
        if( (rExport.getExportFlags() & EXPORT_EMBEDDED) == 0 )
        {
            OUString sURL;

            sal_Bool bIsOwnLink = sal_False;
            if( SV_EMBEDDED_OWN == nType )
            {
                try
                {
                    uno::Reference< embed::XLinkageSupport > xLinkage( xObj, uno::UNO_QUERY );
                    bIsOwnLink = xLinkage.is() && xLinkage->isLink();
                    if ( bIsOwnLink )
                        sURL = xLinkage->getLinkURL();
                }
                catch( uno::Exception )
                {
                    // TODO/LATER: error handling
                    DBG_ERROR( "Link detection or retrieving of the URL of OOo link is failed!\n" );
                }
            }

            if ( !bIsOwnLink )
            {
                sURL = OUString( sEmbeddedObjectProtocol );
                sURL += rOLEObj.GetCurrentPersistName();
                sURL = GetExport().AddEmbeddedObject( sURL );
            }

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
            if( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
            {
                uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
                ::rtl::OUString aStr;
                Any aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("AppletCodeBase") );
                aAny >>= aStr;
                if (aStr.getLength() )
                    lcl_addURL(rExport, aStr);

                aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("AppletName") );
                aAny >>= aStr;
                if (aStr.getLength())
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_APPLET_NAME, aStr );

                aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("AppletCode") );
                aAny >>= aStr;
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CODE, aStr );

                sal_Bool bScript = sal_False;
                aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("AppletIsScript") );
                aAny >>= bScript;
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MAY_SCRIPT, bScript ? XML_TRUE : XML_FALSE );

                uno::Sequence < beans::PropertyValue > aProps;
                aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("AppletCommands") );
                aAny >>= aProps;

                sal_Int32 i = aProps.getLength();
                while ( i > 0 )
                {
                    beans::PropertyValue& aProp = aProps[--i];
                    USHORT nType = SwApplet_Impl::GetOptionType( aProp.Name, TRUE );
                    if ( nType == SWHTML_OPTTYPE_TAG)
                    {
                        ::rtl::OUString aStr;
                        aProp.Value >>= aStr;
                        rExport.AddAttribute( XML_NAMESPACE_DRAW, aProp.Name, aStr);
                    }
                }

                eElementName = XML_APPLET;
            }
        }
        break;
    case SV_EMBEDDED_PLUGIN:
        {
            // It's a plugin!
            if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
            {
                uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
                ::rtl::OUString aStr;
                Any aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("PluginURL") );
                aAny >>= aStr;
                lcl_addURL( rExport, aStr );

                aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("PluginMimeType") );
                aAny >>= aStr;
                if (aStr.getLength())
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, aStr );
                eElementName = XML_PLUGIN;
            }
        }
        break;
    case SV_EMBEDDED_FRAME:
        {
            // It's a floating frame!
            if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
            {
                uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
                ::rtl::OUString aStr;
                Any aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameURL") );
                aAny >>= aStr;

                lcl_addURL( rExport, aStr );

                aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameName") );
                aAny >>= aStr;

                if (aStr.getLength())
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FRAME_NAME, aStr );
                eElementName = XML_FLOATING_FRAME;
            }
        }
        break;
    default:
        ASSERT( !this, "unknown object type! Base class should have been called!" );
    }

    {
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
                sURL += rOLEObj.GetCurrentPersistName();
                GetExport().AddEmbeddedObjectAsBase64( sURL );
            }
            break;
        case SV_EMBEDDED_APPLET:
            {
                if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
                {
                    uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
                    uno::Sequence < beans::PropertyValue > aProps;
                    aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("AppletCommands") );
                    aAny >>= aProps;

                    sal_Int32 i = aProps.getLength();
                    while ( i > 0 )
                    {
                        beans::PropertyValue& aProp = aProps[--i];
                        USHORT nType = SwApplet_Impl::GetOptionType( aProp.Name, TRUE );
                        if (SWHTML_OPTTYPE_PARAM == nType || SWHTML_OPTTYPE_SIZE == nType )
                        {
                            ::rtl::OUString aStr;
                            aProp.Value >>= aStr;
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aProp.Name );
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                            SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
                        }
                    }
                }
            }
            break;
        case SV_EMBEDDED_PLUGIN:
            {
                if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
                {
                    uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
                    uno::Sequence < beans::PropertyValue > aProps;
                    aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("PluginCommands") );
                    aAny >>= aProps;

                    sal_Int32 i = aProps.getLength();
                    while ( i > 0 )
                    {
                        beans::PropertyValue& aProp = aProps[--i];
                        USHORT nType = SwApplet_Impl::GetOptionType( aProp.Name, FALSE );
                        if ( nType == SWHTML_OPTTYPE_TAG)
                        {
                            ::rtl::OUString aStr;
                            aProp.Value >>= aStr;
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aProp.Name );
                            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                            SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
                        }
                    }
                }
            }
            break;
        }
    }
    if( SV_EMBEDDED_OUTPLACE==nType || SV_EMBEDDED_OWN==nType )
    {
        OUString sURL( sGraphicObjectProtocol );
        sURL += rOLEObj.GetCurrentPersistName();
        if( (rExport.getExportFlags() & EXPORT_EMBEDDED) == 0 )
        {
            sURL = GetExport().AddEmbeddedObject( sURL );
            lcl_addURL( rExport, sURL, sal_False );
        }

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                                  XML_IMAGE, sal_False, sal_True );

        if( (rExport.getExportFlags() & EXPORT_EMBEDDED) != 0 )
            GetExport().AddEmbeddedObjectAsBase64( sURL );
    }

    // Lastly the stuff common to each of Applet/Plugin/Floating Frame
    exportEvents( rPropSet );
    exportAlternativeText( rPropSet, rPropSetInfo );
    exportContour( rPropSet, rPropSetInfo );
}
