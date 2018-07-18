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

#include <config_java.h>

#include <hintids.hxx>
#include <rtl/strbuf.hxx>
#include <svl/urihelper.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/frmhtml.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <vcl/wrkwin.hxx>
#include <sot/storage.hxx>
#include <svx/xoutbmp.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>
#include <SwAppletImpl.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <frmfmt.hxx>

#include <svl/ownlist.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <swerror.h>
#include <ndole.hxx>
#include <swtable.hxx>
#include <docsh.hxx>
#include "swhtml.hxx"
#include "wrthtml.hxx"
#include "htmlfly.hxx"
#include "swcss1.hxx"
#include "htmlreqifreader.hxx"
#include <unoframe.hxx>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/embed/XEmbedPersist2.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/classids.hxx>
#include <rtl/uri.hxx>
#include <comphelper/storagehelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/propertysequence.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <comphelper/fileurl.hxx>

using namespace com::sun::star;

#define HTML_DFLT_EMBED_WIDTH ((MM50*5)/2)
#define HTML_DFLT_EMBED_HEIGHT ((MM50*5)/2)

#define HTML_DFLT_APPLET_WIDTH ((MM50*5)/2)
#define HTML_DFLT_APPLET_HEIGHT ((MM50*5)/2)


const HtmlFrmOpts HTML_FRMOPTS_EMBED_ALL      =
    HtmlFrmOpts::Alt |
    HtmlFrmOpts::Size |
    HtmlFrmOpts::Name;
const HtmlFrmOpts HTML_FRMOPTS_EMBED_CNTNR    =
    HTML_FRMOPTS_EMBED_ALL |
    HtmlFrmOpts::AbsSize;
const HtmlFrmOpts HTML_FRMOPTS_EMBED          =
    HTML_FRMOPTS_EMBED_ALL |
    HtmlFrmOpts::Align |
    HtmlFrmOpts::Space |
    HtmlFrmOpts::BrClear |
    HtmlFrmOpts::Name;
const HtmlFrmOpts HTML_FRMOPTS_HIDDEN_EMBED   =
    HtmlFrmOpts::Alt |
    HtmlFrmOpts::Name;

const HtmlFrmOpts HTML_FRMOPTS_APPLET_ALL     =
    HtmlFrmOpts::Alt |
    HtmlFrmOpts::Size;
const HtmlFrmOpts HTML_FRMOPTS_APPLET_CNTNR   =
    HTML_FRMOPTS_APPLET_ALL |
    HtmlFrmOpts::AbsSize;
const HtmlFrmOpts HTML_FRMOPTS_APPLET         =
    HTML_FRMOPTS_APPLET_ALL |
    HtmlFrmOpts::Align |
    HtmlFrmOpts::Space |
    HtmlFrmOpts::BrClear;

const HtmlFrmOpts HTML_FRMOPTS_IFRAME_ALL     =
    HtmlFrmOpts::Alt |
    HtmlFrmOpts::Size;
const HtmlFrmOpts HTML_FRMOPTS_IFRAME_CNTNR   =
    HTML_FRMOPTS_IFRAME_ALL |
    HtmlFrmOpts::AbsSize;
const HtmlFrmOpts HTML_FRMOPTS_IFRAME         =
    HTML_FRMOPTS_IFRAME_ALL |
    HtmlFrmOpts::Align |
    HtmlFrmOpts::Space |
    HtmlFrmOpts::Border |
    HtmlFrmOpts::BrClear;

const HtmlFrmOpts HTML_FRMOPTS_OLE_CSS1       =
    HtmlFrmOpts::SAlign |
    HtmlFrmOpts::SSpace;

void SwHTMLParser::SetFixSize( const Size& rPixSize,
                               const Size& rTwipDfltSize,
                               bool bPrcWidth, bool bPrcHeight,
                               SvxCSS1PropertyInfo const & rCSS1PropInfo,
                               SfxItemSet& rFlyItemSet )
{
    // convert absolute size values into Twip
    sal_uInt8 nPrcWidth = 0, nPrcHeight = 0;
    Size aTwipSz( bPrcWidth || USHRT_MAX==rPixSize.Width() ? 0 : rPixSize.Width(),
                  bPrcHeight || USHRT_MAX==rPixSize.Height() ? 0 : rPixSize.Height() );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz =
            Application::GetDefaultDevice()->PixelToLogic( aTwipSz,
                                                MapMode(MapUnit::MapTwip) );
    }

    // process width
    if( SVX_CSS1_LTYPE_PERCENTAGE == rCSS1PropInfo.m_eWidthType )
    {
        nPrcWidth = static_cast<sal_uInt8>(rCSS1PropInfo.m_nWidth);
        aTwipSz.setWidth( rTwipDfltSize.Width() );
    }
    else if( SVX_CSS1_LTYPE_TWIP== rCSS1PropInfo.m_eWidthType )
    {
        aTwipSz.setWidth( rCSS1PropInfo.m_nWidth );
    }
    else if( bPrcWidth && rPixSize.Width() )
    {
        nPrcWidth = static_cast<sal_uInt8>(rPixSize.Width());
        if( nPrcWidth > 100 )
            nPrcWidth = 100;

        aTwipSz.setWidth( rTwipDfltSize.Width() );
    }
    else if( USHRT_MAX==rPixSize.Width() )
    {
        aTwipSz.setWidth( rTwipDfltSize.Width() );
    }
    if( aTwipSz.Width() < MINFLY )
    {
        aTwipSz.setWidth( MINFLY );
    }

    // process height
    if( SVX_CSS1_LTYPE_PERCENTAGE == rCSS1PropInfo.m_eHeightType )
    {
        nPrcHeight = static_cast<sal_uInt8>(rCSS1PropInfo.m_nHeight);
        aTwipSz.setHeight( rTwipDfltSize.Height() );
    }
    else if( SVX_CSS1_LTYPE_TWIP== rCSS1PropInfo.m_eHeightType )
    {
        aTwipSz.setHeight( rCSS1PropInfo.m_nHeight );
    }
    else if( bPrcHeight && rPixSize.Height() )
    {
        nPrcHeight = static_cast<sal_uInt8>(rPixSize.Height());
        if( nPrcHeight > 100 )
            nPrcHeight = 100;

        aTwipSz.setHeight( rTwipDfltSize.Height() );
    }
    else if( USHRT_MAX==rPixSize.Height() )
    {
        aTwipSz.setHeight( rTwipDfltSize.Height() );
    }
    if( aTwipSz.Height() < MINFLY )
    {
        aTwipSz.setHeight( MINFLY );
    }

    // set size
    SwFormatFrameSize aFrameSize( ATT_FIX_SIZE, aTwipSz.Width(), aTwipSz.Height() );
    aFrameSize.SetWidthPercent( nPrcWidth );
    aFrameSize.SetHeightPercent( nPrcHeight );
    rFlyItemSet.Put( aFrameSize );
}

void SwHTMLParser::SetSpace( const Size& rPixSpace,
                             SfxItemSet& rCSS1ItemSet,
                             SvxCSS1PropertyInfo& rCSS1PropInfo,
                             SfxItemSet& rFlyItemSet )
{
    sal_Int32 nLeftSpace = 0, nRightSpace = 0;
    sal_uInt16 nUpperSpace = 0, nLowerSpace = 0;
    if( (rPixSpace.Width() || rPixSpace.Height()) && Application::GetDefaultDevice() )
    {
        Size aTwipSpc( rPixSpace.Width(), rPixSpace.Height() );
        aTwipSpc =
            Application::GetDefaultDevice()->PixelToLogic( aTwipSpc,
                                                MapMode(MapUnit::MapTwip) );
        nLeftSpace = nRightSpace = aTwipSpc.Width();
        nUpperSpace = nLowerSpace = static_cast<sal_uInt16>(aTwipSpc.Height());
    }

    // set left/right margin
    const SfxPoolItem *pItem;
    if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_LR_SPACE, true, &pItem ) )
    {
        // if applicable remove the first line indent
        const SvxLRSpaceItem *pLRItem = static_cast<const SvxLRSpaceItem *>(pItem);
        SvxLRSpaceItem aLRItem( *pLRItem );
        aLRItem.SetTextFirstLineOfst( 0 );
        if( rCSS1PropInfo.m_bLeftMargin )
        {
            nLeftSpace = aLRItem.GetLeft();
            rCSS1PropInfo.m_bLeftMargin = false;
        }
        if( rCSS1PropInfo.m_bRightMargin )
        {
            nRightSpace = aLRItem.GetRight();
            rCSS1PropInfo.m_bRightMargin = false;
        }
        rCSS1ItemSet.ClearItem( RES_LR_SPACE );
    }
    if( nLeftSpace > 0 || nRightSpace > 0 )
    {
        SvxLRSpaceItem aLRItem( RES_LR_SPACE );
        aLRItem.SetLeft( std::max<sal_Int32>(nLeftSpace, 0) );
        aLRItem.SetRight( std::max<sal_Int32>(nRightSpace, 0) );
        rFlyItemSet.Put( aLRItem );
        if( nLeftSpace )
        {
            const SwFormatHoriOrient& rHoriOri =
                rFlyItemSet.Get( RES_HORI_ORIENT );
            if( text::HoriOrientation::NONE == rHoriOri.GetHoriOrient() )
            {
                SwFormatHoriOrient aHoriOri( rHoriOri );
                aHoriOri.SetPos( aHoriOri.GetPos() + nLeftSpace );
                rFlyItemSet.Put( aHoriOri );
            }
        }
    }

    // set top/bottom margin
    if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_UL_SPACE, true, &pItem ) )
    {
        // if applicable remove the first line indent
        const SvxULSpaceItem *pULItem = static_cast<const SvxULSpaceItem *>(pItem);
        if( rCSS1PropInfo.m_bTopMargin )
        {
            nUpperSpace = pULItem->GetUpper();
            rCSS1PropInfo.m_bTopMargin = false;
        }
        if( rCSS1PropInfo.m_bBottomMargin )
        {
            nLowerSpace = pULItem->GetLower();
            rCSS1PropInfo.m_bBottomMargin = false;
        }
        rCSS1ItemSet.ClearItem( RES_UL_SPACE );
    }
    if( nUpperSpace || nLowerSpace )
    {
        SvxULSpaceItem aULItem( RES_UL_SPACE );
        aULItem.SetUpper( nUpperSpace );
        aULItem.SetLower( nLowerSpace );
        rFlyItemSet.Put( aULItem );
        if( nUpperSpace )
        {
            const SwFormatVertOrient& rVertOri =
                rFlyItemSet.Get( RES_VERT_ORIENT );
            if( text::VertOrientation::NONE == rVertOri.GetVertOrient() )
            {
                SwFormatVertOrient aVertOri( rVertOri );
                aVertOri.SetPos( aVertOri.GetPos() + nUpperSpace );
                rFlyItemSet.Put( aVertOri );
            }
        }
    }
}

OUString SwHTMLParser::StripQueryFromPath(const OUString& rBase, const OUString& rPath)
{
    if (!comphelper::isFileUrl(rBase))
        return rPath;

    sal_Int32 nIndex = rPath.indexOf('?');

    if (nIndex != -1)
        return rPath.copy(0, nIndex);

    return rPath;
}

bool SwHTMLParser::InsertEmbed()
{
    OUString aURL, aType, aName, aAlt, aId, aStyle, aClass;
    OUString aData;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( USHRT_MAX, USHRT_MAX );
    bool bPrcWidth = false, bPrcHeight = false, bHidden = false;
    sal_Int16 eVertOri = text::VertOrientation::NONE;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    SvCommandList aCmdLst;
    const HTMLOptions& rHTMLOptions = GetOptions();

    // The options are read forwards, because the plug-ins expect them in this
    // order. Still only the first value of an option may be regarded.
    for (const auto & rOption : rHTMLOptions)
    {
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::ID:
            aId = rOption.GetString();
            break;
        case HtmlOptionId::STYLE:
            aStyle = rOption.GetString();
            break;
        case HtmlOptionId::CLASS:
            aClass = rOption.GetString();
            break;
        case HtmlOptionId::NAME:
            aName = rOption.GetString();
            break;
        case HtmlOptionId::SRC:
            if( aURL.isEmpty() )
                aURL = rOption.GetString();
            break;
        case HtmlOptionId::ALT:
            aAlt = rOption.GetString();
            break;
        case HtmlOptionId::TYPE:
            if( aType.isEmpty() )
                aType = rOption.GetString();
            break;
        case HtmlOptionId::ALIGN:
            if( eVertOri==text::VertOrientation::NONE && eHoriOri==text::HoriOrientation::NONE )
            {
                eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
                eHoriOri = rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            }
            break;
        case HtmlOptionId::WIDTH:
            if( USHRT_MAX==aSize.Width() )
            {
                bPrcWidth = (rOption.GetString().indexOf('%') != -1);
                aSize.setWidth( static_cast<long>(rOption.GetNumber()) );
            }
            break;
        case HtmlOptionId::HEIGHT:
            if( USHRT_MAX==aSize.Height() )
            {
                bPrcHeight = (rOption.GetString().indexOf('%') != -1);
                aSize.setHeight( static_cast<long>(rOption.GetNumber()) );
            }
            break;
        case HtmlOptionId::HSPACE:
            if( USHRT_MAX==aSpace.Width() )
                aSpace.setWidth( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::VSPACE:
            if( USHRT_MAX==aSpace.Height() )
                aSpace.setHeight( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::DATA:
            if (m_bXHTML && aURL.isEmpty())
                aData = rOption.GetString();
            break;
        case HtmlOptionId::UNKNOWN:
            if (rOption.GetTokenString().equalsIgnoreAsciiCase(
                        OOO_STRING_SW_HTML_O_Hidden))
            {
                bHidden = !rOption.GetString().equalsIgnoreAsciiCase(
                                "FALSE");
            }
            break;
        default: break;
        }

        // All parameters are passed to the plug-in.
        aCmdLst.Append( rOption.GetTokenString(), rOption.GetString() );
    }

    if (aType == "image/png" && m_aEmbeds.empty())
        // Toplevel <object> for PNG -> that's an image, not an OLE object.
        return false;

    SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    // Convert the default values (except height/width, which is done by SetFrameSize())
    if( eVertOri==text::VertOrientation::NONE && eHoriOri==text::HoriOrientation::NONE )
        eVertOri = text::VertOrientation::TOP;
    if( USHRT_MAX==aSpace.Width() )
        aSpace.setWidth( 0 );
    if( USHRT_MAX==aSpace.Height() )
        aSpace.setHeight( 0 );
    if( bHidden )
    {
        // Size (0,0) will be changed to (MINFLY, MINFLY) in SetFrameSize()
        aSize.setWidth( 0 ); aSize.setHeight( 0 );
        aSpace.setWidth( 0 ); aSpace.setHeight( 0 );
        bPrcWidth = bPrcHeight = false;
    }

    // prepare the URL
    INetURLObject aURLObj;
    bool bHasURL = !aURL.isEmpty() &&
                   aURLObj.SetURL(
                       URIHelper::SmartRel2Abs(
                           INetURLObject(m_sBaseURL), aURL,
                           URIHelper::GetMaybeFileHdl()) );
    bool bHasData = !aData.isEmpty();

    try
    {
        // Strip query and everything after that for file:// URLs, browsers do
        // the same.
        aURLObj.SetURL(rtl::Uri::convertRelToAbs(
            m_sBaseURL, SwHTMLParser::StripQueryFromPath(m_sBaseURL, aData)));
    }
    catch (const rtl::MalformedUriException& /*rException*/)
    {
        bHasData = false;
    }

    // do not insert plugin if it has neither URL nor type
    bool bHasType = !aType.isEmpty();
    if( !bHasURL && !bHasType && !bHasData )
        return true;

    if (!m_aEmbeds.empty())
    {
        // Nested XHTML <object> element: points to replacement graphic.
        SwOLENode* pOLENode = m_aEmbeds.top();
        svt::EmbeddedObjectRef& rObj = pOLENode->GetOLEObj().GetObject();
        Graphic aGraphic;
        if (GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURLObj) != ERRCODE_NONE)
            return true;

        rObj.SetGraphic(aGraphic, aType);

        // Set the size of the OLE frame to the size of the graphic.
        OutputDevice* pDevice = Application::GetDefaultDevice();
        if (aSize.getHeight() == USHRT_MAX || aSize.getWidth() == USHRT_MAX)
        {
            Size aPixelSize = aGraphic.GetSizePixel(pDevice);
            if (aSize.getWidth() == USHRT_MAX)
                aSize.setWidth(aPixelSize.getWidth());
            if (aSize.getHeight() == USHRT_MAX)
                aSize.setHeight(aPixelSize.getHeight());
        }

        SwFrameFormat* pFormat = pOLENode->GetFlyFormat();
        if (!pFormat)
            return true;

        SwAttrSet aAttrSet(pFormat->GetAttrSet());
        aAttrSet.ClearItem(RES_CNTNT);
        Size aTwipSize(pDevice->PixelToLogic(aSize, MapMode(MapUnit::MapTwip)));
        SwFormatFrameSize aFrameSize(ATT_FIX_SIZE, aTwipSize.Width(), aTwipSize.Height());
        aAttrSet.Put(aFrameSize);
        pOLENode->GetDoc()->SetFlyFrameAttr(*pFormat, aAttrSet);
        return true;
    }

    // create the plug-in
    comphelper::EmbeddedObjectContainer aCnt;
    OUString aObjName;
    uno::Reference < embed::XEmbeddedObject > xObj;
    if (!bHasData)
    {
        xObj = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_PLUGIN_CLASSID ).GetByteSequence(), aObjName );
        if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        {
            uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
            if ( xSet.is() )
            {
                if( bHasURL )
                    xSet->setPropertyValue("PluginURL", uno::makeAny( aURL ) );
                if( bHasType )
                    xSet->setPropertyValue("PluginMimeType", uno::makeAny( aType ) );

                uno::Sequence < beans::PropertyValue > aProps;
                aCmdLst.FillSequence( aProps );
                xSet->setPropertyValue("PluginCommands", uno::makeAny( aProps ) );

            }
        }
    }
    else if (SwDocShell* pDocSh = m_xDoc->GetDocShell())
    {
        // Has non-empty data attribute in XHTML: map that to an OLE object.
        uno::Reference<embed::XStorage> xStorage = pDocSh->GetStorage();
        aCnt.SwitchPersistence(xStorage);
        aObjName = aCnt.CreateUniqueObjectName();
        {
            SvFileStream aFileStream(aURLObj.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                                     StreamMode::READ);
            uno::Reference<io::XInputStream> xInStream;
            SvMemoryStream aMemoryStream;
            if (aType == "text/rtf")
            {
                OString aMagic("{\\object");
                OString aHeader(read_uInt8s_ToOString(aFileStream, aMagic.getLength()));
                aFileStream.Seek(0);
                if (aHeader == aMagic)
                {
                    // OLE2 wrapped in RTF: either own format or real OLE2 embedding.
                    bool bOwnFormat = false;
                    if (SwReqIfReader::ExtractOleFromRtf(aFileStream, aMemoryStream, bOwnFormat))
                    {
                        xInStream.set(new utl::OStreamWrapper(aMemoryStream));
                    }

                    if (bOwnFormat)
                    {
                        uno::Sequence<beans::PropertyValue> aMedium = comphelper::InitPropertySequence(
                            { { "InputStream", uno::makeAny(xInStream) },
                              { "URL", uno::makeAny(OUString("private:stream")) },
                              { "DocumentBaseURL", uno::makeAny(m_sBaseURL) } });
                        xObj = aCnt.InsertEmbeddedObject(aMedium, aName, &m_sBaseURL);
                    }
                    else
                    {
                        // The type is now an OLE2 container, not the original XHTML type.
                        aType = "application/vnd.sun.star.oleobject";
                    }
                }
            }

            if (!xInStream.is())
                // Non-RTF case.
                xInStream.set(new utl::OStreamWrapper(aFileStream));

            if (!xObj.is())
            {
                uno::Reference<io::XStream> xOutStream
                    = xStorage->openStreamElement(aObjName, embed::ElementModes::READWRITE);
                if (aFileStream.IsOpen())
                    comphelper::OStorageHelper::CopyInputToOutput(xInStream,
                                                                  xOutStream->getOutputStream());

                if (!aType.isEmpty())
                {
                    // Set media type of the native data.
                    uno::Reference<beans::XPropertySet> xOutStreamProps(xOutStream, uno::UNO_QUERY);
                    if (xOutStreamProps.is())
                        xOutStreamProps->setPropertyValue("MediaType", uno::makeAny(aType));
                }
            }
            xObj = aCnt.GetEmbeddedObject(aObjName);
        }
    }

    SfxItemSet aFrameSet( m_xDoc->GetAttrPool(),
                        svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
    if( !IsNewDoc() )
        Reader::ResetFrameFormatAttrs( aFrameSet );

    // set the anchor
    if( !bHidden )
    {
        SetAnchorAndAdjustment( eVertOri, eHoriOri, aPropInfo, aFrameSet );
    }
    else
    {
        SwFormatAnchor aAnchor( RndStdIds::FLY_AT_PARA );
        aAnchor.SetAnchor( m_pPam->GetPoint() );
        aFrameSet.Put( aAnchor );
        aFrameSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::LEFT, text::RelOrientation::FRAME) );
        aFrameSet.Put( SwFormatSurround( css::text::WrapTextMode_THROUGH ) );
        aFrameSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::PRINT_AREA ) );
    }

    // and the size of the frame
    Size aDfltSz( HTML_DFLT_EMBED_WIDTH, HTML_DFLT_EMBED_HEIGHT );
    SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight, aPropInfo, aFrameSet );
    SetSpace( aSpace, aItemSet, aPropInfo, aFrameSet );

    // and insert into the document
    SwFrameFormat* pFlyFormat =
        m_xDoc->getIDocumentContentOperations().InsertEmbObject(*m_pPam,
                ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                &aFrameSet);

    // set name at FrameFormat
    if( !aName.isEmpty() )
        pFlyFormat->SetName( aName );

    // set the alternative text
    SwNoTextNode *pNoTextNd =
        m_xDoc->GetNodes()[ pFlyFormat->GetContent().GetContentIdx()
                          ->GetIndex()+1 ]->GetNoTextNode();
    pNoTextNd->SetTitle( aAlt );

    // if applicable create frames and register auto-bound frames
    if( !bHidden )
    {
        // HIDDEN plug-ins should stay paragraph-bound. Since RegisterFlyFrame()
        // will change paragraph-bound frames with wrap-through into a
        // character-bound frame, here we must create the frames by hand.
        RegisterFlyFrame( pFlyFormat );
    }

    if (!bHasData)
        return true;

    SwOLENode* pOLENode = pNoTextNd->GetOLENode();
    if (!pOLENode)
        return true;

    m_aEmbeds.push(pOLENode);

    return true;
}

#if HAVE_FEATURE_JAVA
void SwHTMLParser::NewObject()
{
    OUString aClassID;
    OUString aStandBy, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( 0, 0 );
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;

    bool bPrcWidth = false, bPrcHeight = false,
             bDeclare = false;
    // create a new Command list
    m_pAppletImpl.reset(new SwApplet_Impl( m_xDoc->GetAttrPool() ));

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::ID:
            aId = rOption.GetString();
            break;
        case HtmlOptionId::STYLE:
            aStyle = rOption.GetString();
            break;
        case HtmlOptionId::CLASS:
            aClass = rOption.GetString();
            break;
        case HtmlOptionId::DECLARE:
            bDeclare = true;
            break;
        case HtmlOptionId::CLASSID:
            aClassID = rOption.GetString();
            break;
        case HtmlOptionId::CODEBASE:
            break;
        case HtmlOptionId::DATA:
            break;
        case HtmlOptionId::TYPE:
            break;
        case HtmlOptionId::CODETYPE:
            break;
        case HtmlOptionId::ARCHIVE:
        case HtmlOptionId::UNKNOWN:
            break;
        case HtmlOptionId::STANDBY:
            aStandBy = rOption.GetString();
            break;
        case HtmlOptionId::WIDTH:
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            aSize.setWidth( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::HEIGHT:
            bPrcHeight = (rOption.GetString().indexOf('%') != -1);
            aSize.setHeight( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::ALIGN:
            eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri = rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HtmlOptionId::USEMAP:
            break;
        case HtmlOptionId::NAME:
            break;
        case HtmlOptionId::HSPACE:
            aSpace.setWidth( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::VSPACE:
            aSpace.setHeight( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::BORDER:
            break;

        case HtmlOptionId::SDONCLICK:
        case HtmlOptionId::ONCLICK:
        case HtmlOptionId::SDONMOUSEOVER:
        case HtmlOptionId::ONMOUSEOVER:
        case HtmlOptionId::SDONMOUSEOUT:
        case HtmlOptionId::ONMOUSEOUT:
            break;
        default: break;
        }
        // All parameters are passed to the applet.
        m_pAppletImpl->AppendParam( rOption.GetTokenString(),
                                  rOption.GetString() );

    }

    // Objects that are declared only are not evaluated. Moreover, only
    // Java applets are supported.
    bool bIsApplet = false;

    if( !bDeclare && aClassID.getLength() == 42 &&
        aClassID.startsWith("clsid:") )
    {
        aClassID = aClassID.copy(6);
        SvGlobalName aCID;
        if( aCID.MakeId( aClassID ) )
        {
            SvGlobalName aJavaCID( 0x8AD9C840UL, 0x044EU, 0x11D1U, 0xB3U, 0xE9U,
                                   0x00U, 0x80U, 0x5FU, 0x49U, 0x9DU, 0x93U );

            bIsApplet = aJavaCID == aCID;
        }
    }

    if( !bIsApplet )
    {
        m_pAppletImpl.reset();
        return;
    }

    m_pAppletImpl->SetAltText( aStandBy );

    SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    SfxItemSet& rFrameSet = m_pAppletImpl->GetItemSet();
    if( !IsNewDoc() )
        Reader::ResetFrameFormatAttrs( rFrameSet );

    // set the anchor and the adjustment
    SetAnchorAndAdjustment( eVertOri, eHoriOri, aPropInfo, rFrameSet );

    // and still the size of the frame
    Size aDfltSz( HTML_DFLT_APPLET_WIDTH, HTML_DFLT_APPLET_HEIGHT );
    SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight, aPropInfo, rFrameSet );
    SetSpace( aSpace, aItemSet, aPropInfo, rFrameSet );
}
#endif

void SwHTMLParser::EndObject()
{
#if HAVE_FEATURE_JAVA
    if( !m_pAppletImpl )
        return;
    if( m_pAppletImpl->CreateApplet( m_sBaseURL ) )
    {
        m_pAppletImpl->FinishApplet();

        // and insert into the document
        SwFrameFormat* pFlyFormat =
            m_xDoc->getIDocumentContentOperations().InsertEmbObject(*m_pPam,
                    ::svt::EmbeddedObjectRef( m_pAppletImpl->GetApplet(), embed::Aspects::MSOLE_CONTENT ),
                    &m_pAppletImpl->GetItemSet() );

        // set the alternative name
        SwNoTextNode *pNoTextNd =
            m_xDoc->GetNodes()[ pFlyFormat->GetContent().GetContentIdx()
                              ->GetIndex()+1 ]->GetNoTextNode();
        pNoTextNd->SetTitle( m_pAppletImpl->GetAltText() );

        // if applicable create frames and register auto-bound frames
        RegisterFlyFrame( pFlyFormat );

        m_pAppletImpl.reset();
    }
#else
    (void) this;                // Silence loplugin:staticmethods
#endif
}

#if HAVE_FEATURE_JAVA
void SwHTMLParser::InsertApplet()
{
    OUString aCodeBase, aCode, aName, aAlt, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( 0, 0 );
    bool bPrcWidth = false, bPrcHeight = false, bMayScript = false;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;

    // create a new Command list
    m_pAppletImpl.reset(new SwApplet_Impl( m_xDoc->GetAttrPool() ));

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::ID:
            aId = rOption.GetString();
            break;
        case HtmlOptionId::STYLE:
            aStyle = rOption.GetString();
            break;
        case HtmlOptionId::CLASS:
            aClass = rOption.GetString();
            break;
        case HtmlOptionId::CODEBASE:
            aCodeBase = rOption.GetString();
            break;
        case HtmlOptionId::CODE:
            aCode = rOption.GetString();
            break;
        case HtmlOptionId::NAME:
            aName = rOption.GetString();
            break;
        case HtmlOptionId::ALT:
            aAlt = rOption.GetString();
            break;
        case HtmlOptionId::ALIGN:
            eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri = rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HtmlOptionId::WIDTH:
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            aSize.setWidth( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::HEIGHT:
            bPrcHeight = (rOption.GetString().indexOf('%') != -1);
            aSize.setHeight( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::HSPACE:
            aSpace.setWidth( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::VSPACE:
            aSpace.setHeight( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::MAYSCRIPT:
            bMayScript = true;
            break;
        default: break;
        }

        // All parameters are passed to the applet.
        m_pAppletImpl->AppendParam( rOption.GetTokenString(),
                                  rOption.GetString() );
    }

    if( aCode.isEmpty() )
    {
        m_pAppletImpl.reset();
        return;
    }

    if ( !aCodeBase.isEmpty() )
        aCodeBase = INetURLObject::GetAbsURL( m_sBaseURL, aCodeBase );
    m_pAppletImpl->CreateApplet( aCode, aName, bMayScript, aCodeBase, m_sBaseURL );//, aAlt );
    m_pAppletImpl->SetAltText( aAlt );

    SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    SfxItemSet& rFrameSet = m_pAppletImpl->GetItemSet();
    if( !IsNewDoc() )
        Reader::ResetFrameFormatAttrs( rFrameSet );

    // set the anchor and the adjustment
    SetAnchorAndAdjustment( eVertOri, eHoriOri, aPropInfo, rFrameSet );

    // and still the size or the frame
    Size aDfltSz( HTML_DFLT_APPLET_WIDTH, HTML_DFLT_APPLET_HEIGHT );
    SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight, aPropInfo, rFrameSet );
    SetSpace( aSpace, aItemSet, aPropInfo, rFrameSet );
}
#endif

void SwHTMLParser::EndApplet()
{
#if HAVE_FEATURE_JAVA
    if( !m_pAppletImpl )
        return;

    m_pAppletImpl->FinishApplet();

    // and insert into the document
    SwFrameFormat* pFlyFormat =
        m_xDoc->getIDocumentContentOperations().InsertEmbObject(*m_pPam,
                    ::svt::EmbeddedObjectRef( m_pAppletImpl->GetApplet(), embed::Aspects::MSOLE_CONTENT ),
                    &m_pAppletImpl->GetItemSet());

    // set the alternative name
    SwNoTextNode *pNoTextNd =
        m_xDoc->GetNodes()[ pFlyFormat->GetContent().GetContentIdx()
                          ->GetIndex()+1 ]->GetNoTextNode();
    pNoTextNd->SetTitle( m_pAppletImpl->GetAltText() );

    // if applicable create frames and register auto-bound frames
    RegisterFlyFrame( pFlyFormat );

    m_pAppletImpl.reset();
#else
    (void) this;
#endif
}

void SwHTMLParser::InsertParam()
{
#if HAVE_FEATURE_JAVA
    if( !m_pAppletImpl )
        return;

    OUString aName, aValue;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::NAME:
            aName = rOption.GetString();
            break;
        case HtmlOptionId::VALUE:
            aValue = rOption.GetString();
            break;
        default: break;
        }
    }

    if( aName.isEmpty() )
        return;

    m_pAppletImpl->AppendParam( aName, aValue );
#else
    (void) this;
#endif
}

void SwHTMLParser::InsertFloatingFrame()
{
    OUString aAlt, aId, aStyle, aClass;
    Size aSize( USHRT_MAX, USHRT_MAX );
    Size aSpace( 0, 0 );
    bool bPrcWidth = false, bPrcHeight = false;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;

    const HTMLOptions& rHTMLOptions = GetOptions();

    // First fetch the options of the Writer-Frame-Format
    for (const auto & rOption : rHTMLOptions)
    {
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::ID:
            aId = rOption.GetString();
            break;
        case HtmlOptionId::STYLE:
            aStyle = rOption.GetString();
            break;
        case HtmlOptionId::CLASS:
            aClass = rOption.GetString();
            break;
        case HtmlOptionId::ALT:
            aAlt = rOption.GetString();
            break;
        case HtmlOptionId::ALIGN:
            eVertOri = rOption.GetEnum( aHTMLImgVAlignTable, eVertOri );
            eHoriOri = rOption.GetEnum( aHTMLImgHAlignTable, eHoriOri );
            break;
        case HtmlOptionId::WIDTH:
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            aSize.setWidth( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::HEIGHT:
            bPrcHeight = (rOption.GetString().indexOf('%') != -1);
            aSize.setHeight( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::HSPACE:
            aSpace.setWidth( static_cast<long>(rOption.GetNumber()) );
            break;
        case HtmlOptionId::VSPACE:
            aSpace.setHeight( static_cast<long>(rOption.GetNumber()) );
            break;
        default: break;
        }
    }

    // and now the ones for the SfxFrame
    SfxFrameDescriptor aFrameDesc;

    SfxFrameHTMLParser::ParseFrameOptions( &aFrameDesc, rHTMLOptions, m_sBaseURL );

    // create a floating frame
    comphelper::EmbeddedObjectContainer aCnt;
    OUString aObjName;
    uno::Reference < embed::XEmbeddedObject > xObj = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_IFRAME_CLASSID ).GetByteSequence(), aObjName );

    try
    {
        // TODO/MBA: testing
        if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        {
            uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
            if ( xSet.is() )
            {
                OUString aName = aFrameDesc.GetName();
                ScrollingMode eScroll = aFrameDesc.GetScrollingMode();
                bool bHasBorder = aFrameDesc.HasFrameBorder();
                Size aMargin = aFrameDesc.GetMargin();

                xSet->setPropertyValue("FrameURL", uno::makeAny( aFrameDesc.GetURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) );
                xSet->setPropertyValue("FrameName", uno::makeAny( aName ) );

                if ( eScroll == ScrollingMode::Auto )
                    xSet->setPropertyValue("FrameIsAutoScroll",
                        uno::makeAny( true ) );
                else
                    xSet->setPropertyValue("FrameIsScrollingMode",
                        uno::makeAny( eScroll == ScrollingMode::Yes ) );

                xSet->setPropertyValue("FrameIsBorder",
                        uno::makeAny( bHasBorder ) );

                xSet->setPropertyValue("FrameMarginWidth",
                    uno::makeAny( sal_Int32( aMargin.Width() ) ) );

                xSet->setPropertyValue("FrameMarginHeight",
                    uno::makeAny( sal_Int32( aMargin.Height() ) ) );
            }
        }
    }
    catch ( uno::Exception& )
    {
    }

    SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass ) )
        ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo );

    // fetch the ItemSet
    SfxItemSet aFrameSet( m_xDoc->GetAttrPool(),
                        svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
    if( !IsNewDoc() )
        Reader::ResetFrameFormatAttrs( aFrameSet );

    // set the anchor and the adjustment
    SetAnchorAndAdjustment( eVertOri, eHoriOri, aPropInfo, aFrameSet );

    // and still the size of the frame
    Size aDfltSz( HTML_DFLT_APPLET_WIDTH, HTML_DFLT_APPLET_HEIGHT );
    SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight, aPropInfo, aFrameSet );
    SetSpace( aSpace, aItemSet, aPropInfo, aFrameSet );

    // and insert into the document
    SwFrameFormat* pFlyFormat =
        m_xDoc->getIDocumentContentOperations().InsertEmbObject(*m_pPam,
            ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
            &aFrameSet);

    // set the alternative name
    SwNoTextNode *pNoTextNd =
        m_xDoc->GetNodes()[ pFlyFormat->GetContent().GetContentIdx()
                          ->GetIndex()+1 ]->GetNoTextNode();
    pNoTextNd->SetTitle( aAlt );

    // if applicable create frames and register auto-bound frames
    RegisterFlyFrame( pFlyFormat );

    m_bInFloatingFrame = true;
}

sal_uInt16 SwHTMLWriter::GuessOLENodeFrameType( const SwNode& rNode )
{
    SwOLEObj& rObj = const_cast<SwOLENode*>(rNode.GetOLENode())->GetOLEObj();

    SwHTMLFrameType eType = HTML_FRMTYPE_OLE;

    uno::Reference < embed::XClassifiedObject > xClass ( rObj.GetOleRef(), uno::UNO_QUERY );
    SvGlobalName aClass( xClass->getClassID() );
    if( aClass == SvGlobalName( SO3_PLUGIN_CLASSID ) )
    {
        eType = HTML_FRMTYPE_PLUGIN;
    }
    else if( aClass == SvGlobalName( SO3_IFRAME_CLASSID ) )
    {
        eType = HTML_FRMTYPE_IFRAME;
    }
#if HAVE_FEATURE_JAVA
    else if( aClass == SvGlobalName( SO3_APPLET_CLASSID ) )
    {
        eType = HTML_FRMTYPE_APPLET;
    }
#endif

    return static_cast< sal_uInt16 >(eType);
}

Writer& OutHTML_FrameFormatOLENode( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                               bool bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex()+1;
    SwOLENode *pOLENd = rHTMLWrt.m_pDoc->GetNodes()[ nStt ]->GetOLENode();

    OSL_ENSURE( pOLENd, "OLE-Node expected" );
    if( !pOLENd )
        return rWrt;

    SwOLEObj &rObj = pOLENd->GetOLEObj();

    uno::Reference < embed::XEmbeddedObject > xObj( rObj.GetOleRef() );
    if ( !svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        return rWrt;

    uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
    bool bHiddenEmbed = false;

    if( !xSet.is() )
    {
        OSL_FAIL("Unknown Object" );
        return rWrt;
    }

    HtmlFrmOpts nFrameOpts;

    // if possible output a line break before the "object"
    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine( true );

    if( !rFrameFormat.GetName().isEmpty() )
        rHTMLWrt.OutImplicitMark( rFrameFormat.GetName(),
                                  "ole" );
    uno::Any aAny;
    SvGlobalName aGlobName( xObj->getClassID() );
    OStringBuffer sOut;
    sOut.append('<');
    if( aGlobName == SvGlobalName( SO3_PLUGIN_CLASSID ) )
    {
        // first the plug-in specifics
        sOut.append(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_embed);

        OUString aStr;
        OUString aURL;
        aAny = xSet->getPropertyValue("PluginURL");
        if( (aAny >>= aStr) && !aStr.isEmpty() )
        {
            aURL = URIHelper::simpleNormalizedMakeRelative( rWrt.GetBaseURL(),
                      aStr);
        }

        if( !aURL.isEmpty() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_src)
                .append("=\"");
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rWrt.Strm(), aURL, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            sOut.append('\"');
        }

        OUString aType;
        aAny = xSet->getPropertyValue("PluginMimeType");
        if( (aAny >>= aType) && !aType.isEmpty() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_type)
                .append("=\"");
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rWrt.Strm(), aType, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            sOut.append('\"');
        }

        if ((RndStdIds::FLY_AT_PARA == rFrameFormat.GetAnchor().GetAnchorId()) &&
            css::text::WrapTextMode_THROUGH == rFrameFormat.GetSurround().GetSurround() )
        {
            // A HIDDEN plug-in
            sOut.append(' ').append(OOO_STRING_SW_HTML_O_Hidden);
            nFrameOpts = HTML_FRMOPTS_HIDDEN_EMBED;
            bHiddenEmbed = true;
        }
        else
        {
            nFrameOpts = bInCntnr ? HTML_FRMOPTS_EMBED_CNTNR
                                : HTML_FRMOPTS_EMBED;
        }
    }
    else if( aGlobName == SvGlobalName( SO3_APPLET_CLASSID ) )
    {
        // or the applet specifics

        sOut.append(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_applet);

        // CODEBASE
        OUString aCd;
        aAny = xSet->getPropertyValue("AppletCodeBase");
        if( (aAny >>= aCd) && !aCd.isEmpty() )
        {
            OUString sCodeBase( URIHelper::simpleNormalizedMakeRelative(rWrt.GetBaseURL(), aCd) );
            if( !sCodeBase.isEmpty() )
            {
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_codebase)
                    .append("=\"");
                rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
                HTMLOutFuncs::Out_String( rWrt.Strm(), sCodeBase, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
                sOut.append('\"');
            }
        }

        // CODE
        OUString aClass;
        aAny = xSet->getPropertyValue("AppletCode");
        aAny >>= aClass;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_code)
            .append("=\"");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( rWrt.Strm(), aClass, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        sOut.append('\"');

        // NAME
        OUString aAppletName;
        aAny = xSet->getPropertyValue("AppletName");
        aAny >>= aAppletName;
        if( !aAppletName.isEmpty() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_name)
                .append("=\"");
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rWrt.Strm(), aAppletName, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            sOut.append('\"');
        }

        bool bScript = false;
        aAny = xSet->getPropertyValue("AppletIsScript");
        aAny >>= bScript;
        if( bScript )
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_mayscript);

        nFrameOpts = bInCntnr ? HTML_FRMOPTS_APPLET_CNTNR
                            : HTML_FRMOPTS_APPLET;
    }
    else
    {
        // or the Floating-Frame specifics

        sOut.append(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_iframe);
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

        SfxFrameHTMLWriter::Out_FrameDescriptor( rWrt.Strm(), rWrt.GetBaseURL(),
                                        xSet,
                                        rHTMLWrt.m_eDestEnc,
                                        &rHTMLWrt.m_aNonConvertableCharacters );

        nFrameOpts = bInCntnr ? HTML_FRMOPTS_IFRAME_CNTNR
                            : HTML_FRMOPTS_IFRAME;
    }

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // ALT, WIDTH, HEIGHT, HSPACE, VSPACE, ALIGN
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bHiddenEmbed )
        nFrameOpts |= HTML_FRMOPTS_OLE_CSS1;
    OString aEndTags = rHTMLWrt.OutFrameFormatOptions( rFrameFormat, pOLENd->GetTitle(), nFrameOpts );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_FLY ) && !bHiddenEmbed )
        rHTMLWrt.OutCSS1_FrameFormatOptions( rFrameFormat, nFrameOpts );

    if( aGlobName == SvGlobalName( SO3_APPLET_CLASSID ) )
    {
        // output the parameters of applets as separate tags
        // and write a </APPLET>

        uno::Sequence < beans::PropertyValue > aProps;
        aAny = xSet->getPropertyValue("AppletCommands");
        aAny >>= aProps;

        SvCommandList aCommands;
        aCommands.FillFromSequence( aProps );
        std::vector<sal_uLong> aParams;
        size_t i = aCommands.size();
        while( i > 0 )
        {
            const SvCommand& rCommand = aCommands[ --i ];
            const OUString& rName = rCommand.GetCommand();
            SwHtmlOptType nType = SwApplet_Impl::GetOptionType( rName, true );
            if( SwHtmlOptType::TAG == nType )
            {
                const OUString& rValue = rCommand.GetArgument();
                rWrt.Strm().WriteChar( ' ' );
                HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
                rWrt.Strm().WriteCharPtr( "=\"" );
                HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters ).WriteChar( '\"' );
            }
            else if( SwHtmlOptType::PARAM == nType )
            {
                aParams.push_back( i );
            }
        }

        rHTMLWrt.Strm().WriteChar( '>' );

        rHTMLWrt.IncIndentLevel(); // indent the applet content

        size_t ii = aParams.size();
        while( ii > 0  )
        {
            const SvCommand& rCommand = aCommands[ aParams[--ii] ];
            const OUString& rName = rCommand.GetCommand();
            const OUString& rValue = rCommand.GetArgument();
            rHTMLWrt.OutNewLine();
            OStringBuffer sBuf;
            sBuf.append('<').append(rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_param)
                .append(' ').append(OOO_STRING_SVTOOLS_HTML_O_name)
                .append("=\"");
            rWrt.Strm().WriteCharPtr( sBuf.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            sBuf.append("\" ").append(OOO_STRING_SVTOOLS_HTML_O_value)
                .append("=\"");
            rWrt.Strm().WriteCharPtr( sBuf.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters ).WriteCharPtr( "\">" );
        }

        rHTMLWrt.DecIndentLevel(); // indent the applet content
        if( aCommands.size() )
            rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_applet, false );
    }
    else if( aGlobName == SvGlobalName( SO3_PLUGIN_CLASSID ) )
    {
        // write plug-ins parameters as options

        uno::Sequence < beans::PropertyValue > aProps;
        aAny = xSet->getPropertyValue("PluginCommands");
        aAny >>= aProps;

        SvCommandList aCommands;
        aCommands.FillFromSequence( aProps );
        for( size_t i = 0; i < aCommands.size(); i++ )
        {
            const SvCommand& rCommand = aCommands[ i ];
            const OUString& rName = rCommand.GetCommand();

            if( SwApplet_Impl::GetOptionType( rName, false ) == SwHtmlOptType::TAG )
            {
                const OUString& rValue = rCommand.GetArgument();
                rWrt.Strm().WriteChar( ' ' );
                HTMLOutFuncs::Out_String( rWrt.Strm(), rName, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
                rWrt.Strm().WriteCharPtr( "=\"" );
                HTMLOutFuncs::Out_String( rWrt.Strm(), rValue, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters ).WriteChar( '\"' );
            }
        }
        rHTMLWrt.Strm().WriteChar( '>' );
    }
    else
    {
        // and for Floating-Frames just output another </IFRAME>

        rHTMLWrt.Strm().WriteChar( '>' );
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_iframe, false );
    }

    if( !aEndTags.isEmpty() )
        rWrt.Strm().WriteCharPtr( aEndTags.getStr() );

    return rWrt;
}

Writer& OutHTML_FrameFormatOLENodeGrf( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                                  bool bInCntnr )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    const SwFormatContent& rFlyContent = rFrameFormat.GetContent();
    sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex()+1;
    SwOLENode *pOLENd = rHTMLWrt.m_pDoc->GetNodes()[ nStt ]->GetOLENode();

    OSL_ENSURE( pOLENd, "OLE-Node expected" );
    if( !pOLENd )
        return rWrt;

    if (rHTMLWrt.mbSkipImages)
    {
        // If we skip images, embedded objects would be completely lost.
        // Instead, try to use the HTML export of the embedded object.
        uno::Reference<text::XTextContent> xTextContent = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(*rHTMLWrt.m_pDoc, const_cast<SwFrameFormat*>(&rFrameFormat));
        uno::Reference<document::XEmbeddedObjectSupplier2> xEmbeddedObjectSupplier(xTextContent, uno::UNO_QUERY);
        uno::Reference<frame::XStorable> xStorable(xEmbeddedObjectSupplier->getEmbeddedObject(), uno::UNO_QUERY);
        SAL_WARN_IF(!xStorable.is(), "sw.html", "OutHTML_FrameFormatOLENodeGrf: no embedded object");

        // Figure out what is the filter name of the embedded object.
        uno::Reference<lang::XServiceInfo> xServiceInfo(xStorable, uno::UNO_QUERY);
        OUString aFilter;
        if (xServiceInfo.is())
        {
            if (xServiceInfo->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
                aFilter = "HTML (StarCalc)";
            else if (xServiceInfo->supportsService("com.sun.star.text.TextDocument"))
                aFilter = "HTML (StarWriter)";
        }

        if (xStorable.is() && !aFilter.isEmpty())
        {
            try
            {
                // FIXME: exception for the simplest test document, too
                SvMemoryStream aStream;
                uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
                utl::MediaDescriptor aMediaDescriptor;
                aMediaDescriptor["FilterName"] <<= aFilter;
                aMediaDescriptor["FilterOptions"] <<= OUString("SkipHeaderFooter");
                aMediaDescriptor["OutputStream"] <<= xOutputStream;
                xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
                SAL_WARN_IF(aStream.GetSize()>=static_cast<sal_uInt64>(SAL_MAX_INT32), "sw.html", "Stream can't fit in OString");
                OString aData(static_cast<const char*>(aStream.GetData()), static_cast<sal_Int32>(aStream.GetSize()));
                // Wrap output in a <span> tag to avoid 'HTML parser error: Unexpected end tag: p'
                HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_span);
                rWrt.Strm().WriteCharPtr(aData.getStr());
                HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_span, false);
            }
            catch ( uno::Exception& )
            {
            }
        }

        return rWrt;
    }

    Graphic aGraphic( *pOLENd->GetGraphic() );

    SwDocShell* pDocSh = rHTMLWrt.m_pDoc->GetDocShell();
    bool bObjectOpened = false;
    if (rHTMLWrt.mbXHTML && pDocSh)
    {
        // Map native data to an outer <object> element.

        // Calculate the file name, which is meant to be the same as the
        // replacement image, just with a .ole extension.
        OUString aFileName;
        if (rHTMLWrt.GetOrigFileName())
            aFileName = *rHTMLWrt.GetOrigFileName();
        INetURLObject aURL(aFileName);
        OUString aName(aURL.getBase());
        aName += "_";
        aName += aURL.getExtension();
        aName += "_";
        aName += OUString::number(aGraphic.GetChecksum(), 16);
        aURL.setBase(aName);
        aURL.setExtension("ole");
        aFileName = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);

        // Write the data.
        SwOLEObj& rOLEObj = pOLENd->GetOLEObj();
        uno::Reference<embed::XEmbeddedObject> xEmbeddedObject = rOLEObj.GetOleRef();
        OUString aFileType;
        SvFileStream aOutStream(aFileName, StreamMode::WRITE);
        uno::Reference<io::XActiveDataStreamer> xStreamProvider;
        uno::Reference<embed::XEmbedPersist2> xOwnEmbedded;
        if (xEmbeddedObject.is())
        {
            xStreamProvider.set(xEmbeddedObject, uno::UNO_QUERY);
            xOwnEmbedded.set(xEmbeddedObject, uno::UNO_QUERY);
        }
        if (xStreamProvider.is())
        {
            // Real OLE2 case: OleEmbeddedObject.
            uno::Reference<io::XInputStream> xStream(xStreamProvider->getStream(), uno::UNO_QUERY);
            if (xStream.is())
            {
                std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xStream));
                if (SwReqIfReader::WrapOleInRtf(*pStream, aOutStream))
                {
                    // Data always wrapped in RTF.
                    aFileType = "text/rtf";
                }
            }
        }
        else if (xOwnEmbedded.is())
        {
            // Our own embedded object: OCommonEmbeddedObject.
            SvxMSExportOLEObjects aOLEExp(0);
            // Trigger the load of the OLE object if needed, otherwise we can't
            // export it.
            pOLENd->GetTwipSize();
            SvMemoryStream aMemory;
            tools::SvRef<SotStorage> pStorage = new SotStorage(aMemory);
            aOLEExp.ExportOLEObject(rOLEObj.GetObject(), *pStorage);
            pStorage->Commit();
            aMemory.Seek(0);
            if (SwReqIfReader::WrapOleInRtf(aMemory, aOutStream))
            {
                // Data always wrapped in RTF.
                aFileType = "text/rtf";
            }
        }
        else
        {
            // Otherwise the native data is just a grab-bag: ODummyEmbeddedObject.
            OUString aStreamName = rOLEObj.GetCurrentPersistName();
            uno::Reference<embed::XStorage> xStorage = pDocSh->GetStorage();
            uno::Reference<io::XStream> xInStream
                = xStorage->openStreamElement(aStreamName, embed::ElementModes::READ);
            uno::Reference<io::XStream> xOutStream(new utl::OStreamWrapper(aOutStream));
            comphelper::OStorageHelper::CopyInputToOutput(xInStream->getInputStream(),
                                                          xOutStream->getOutputStream());
            uno::Reference<beans::XPropertySet> xOutStreamProps(xInStream, uno::UNO_QUERY);
            if (xOutStreamProps.is())
                xOutStreamProps->getPropertyValue("MediaType") >>= aFileType;
        }
        aFileName = URIHelper::simpleNormalizedMakeRelative(rWrt.GetBaseURL(), aFileName);

        // Refer to this data.
        if (rHTMLWrt.m_bLFPossible)
            rHTMLWrt.OutNewLine();
        rWrt.Strm().WriteOString("<" + rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_object);
        rWrt.Strm().WriteOString(" data=\"" + aFileName.toUtf8() + "\"");
        if (!aFileType.isEmpty())
            rWrt.Strm().WriteOString(" type=\"" + aFileType.toUtf8() + "\"");
        rWrt.Strm().WriteOString(">");
        bObjectOpened = true;
        rHTMLWrt.m_bLFPossible = true;
    }

    OUString aGraphicURL;
    OUString aMimeType;
    if(!rHTMLWrt.mbEmbedImages)
    {
        const OUString* pTempFileName = rHTMLWrt.GetOrigFileName();
        if(pTempFileName)
            aGraphicURL = *pTempFileName;

        OUString aFilterName("JPG");
        XOutFlags nFlags = XOutFlags::UseGifIfPossible | XOutFlags::UseNativeIfPossible;

        if (bObjectOpened)
        {
            aFilterName = "PNG";
            nFlags = XOutFlags::NONE;
            aMimeType = "image/png";
        }

        ErrCode nErr = XOutBitmap::WriteGraphic( aGraphic, aGraphicURL,
                                    aFilterName,
                                    nFlags );
        if( nErr )              // error, don't write anything
        {
            rHTMLWrt.m_nWarn = WARN_SWG_POOR_LOAD;
            return rWrt;
        }
        aGraphicURL = URIHelper::SmartRel2Abs(
            INetURLObject(rWrt.GetBaseURL()), aGraphicURL,
            URIHelper::GetMaybeFileHdl() );

    }
    HtmlFrmOpts nFlags = bInCntnr ? HtmlFrmOpts::GenImgAllMask
        : HtmlFrmOpts::GenImgMask;
    if (bObjectOpened)
        nFlags |= HtmlFrmOpts::Replacement;
    OutHTML_Image( rWrt, rFrameFormat, aGraphicURL, aGraphic,
            pOLENd->GetTitle(), pOLENd->GetTwipSize(),
            nFlags, "ole", nullptr, aMimeType );

    if (bObjectOpened)
        // Close native data.
        rWrt.Strm().WriteOString("</" + rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_object
                                 ">");

    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
