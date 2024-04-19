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

#include "htmlex.hxx"
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <sal/log.hxx>
#include <rtl/tencinfo.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/xmlencode.hxx>
#include <o3tl/safeint.hxx>
#include <osl/file.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <sfx2/progress.hxx>
#include <utility>
#include <svx/svditer.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outlobj.hxx>
#include <svtools/htmlout.hxx>
#include <editeng/editeng.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdotable.hxx>
#include <tools/urlobj.hxx>
#include <svtools/sfxecode.hxx>
#include <tools/debug.hxx>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <Outliner.hxx>
#include <sdpage.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <sdresid.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::document;

using namespace sdr::table;

namespace {

// Helper class for the simple creation of files local/remote
class EasyFile
{
private:
    std::unique_ptr<SvStream> pOStm;
    bool        bOpen;

public:

    EasyFile();
    ~EasyFile();

    ErrCode   createStream( const OUString& rUrl, SvStream*& rpStr );
    void      createFileName(  const OUString& rUrl, OUString& rFileName );
    void      close();
};

// create area for a circle; we expect pixel coordinates
OUString ColorToHTMLString( Color aColor )
{
    static const char hex[] = "0123456789ABCDEF";
    OUStringBuffer aStr( "#xxxxxx" );
    aStr[1] = hex[(aColor.GetRed() >> 4) & 0xf];
    aStr[2] = hex[aColor.GetRed() & 0xf];
    aStr[3] = hex[(aColor.GetGreen() >> 4) & 0xf];
    aStr[4] = hex[aColor.GetGreen() & 0xf];
    aStr[5] = hex[(aColor.GetBlue() >> 4) & 0xf];
    aStr[6] = hex[aColor.GetBlue() & 0xf];

    return aStr.makeStringAndClear();
}

} //namespace

// Helper class for the embedding of text attributes into the html output
class HtmlState
{
private:
    bool mbColor;
    bool mbWeight;
    bool mbItalic;
    bool mbUnderline;
    bool mbStrike;
    bool mbLink;
    Color maColor;
    Color maDefColor;
    OUString maLink;
    OUString maTarget;

public:
    explicit HtmlState( Color aDefColor );

    OUString SetWeight( bool bWeight );
    OUString SetItalic( bool bItalic );
    OUString SetUnderline( bool bUnderline );
    OUString SetColor( Color aColor );
    OUString SetStrikeout( bool bStrike );
    OUString SetLink( const OUString& aLink, const OUString& aTarget );
    OUString Flush();
};

// close all still open tags
OUString HtmlState::Flush()
{
    OUString aStr = SetWeight(false)
                  + SetItalic(false)
                  + SetUnderline(false)
                  + SetStrikeout(false)
                  + SetColor(maDefColor)
                  + SetLink("","");

    return aStr;
}

// c'tor with default color for the page
HtmlState::HtmlState( Color aDefColor )
  : mbColor(false),
    mbWeight(false),
    mbItalic(false),
    mbUnderline(false),
    mbStrike(false),
    mbLink(false),
    maDefColor(aDefColor)
{
}

// enables/disables bold print
OUString HtmlState::SetWeight( bool bWeight )
{
    OUString aStr;

    if(bWeight && !mbWeight)
        aStr = "<b>";
    else if(!bWeight && mbWeight)
        aStr = "</b>";

    mbWeight = bWeight;
    return aStr;
}

// enables/disables italic

OUString HtmlState::SetItalic( bool bItalic )
{
    OUString aStr;

    if(bItalic && !mbItalic)
        aStr = "<i>";
    else if(!bItalic && mbItalic)
        aStr = "</i>";

    mbItalic = bItalic;
    return aStr;
}

// enables/disables underlines

OUString HtmlState::SetUnderline( bool bUnderline )
{
    OUString aStr;

    if(bUnderline && !mbUnderline)
        aStr = "<u>";
    else if(!bUnderline && mbUnderline)
        aStr = "</u>";

    mbUnderline = bUnderline;
    return aStr;
}

// enables/disables strike through
OUString HtmlState::SetStrikeout( bool bStrike )
{
    OUString aStr;

    if(bStrike && !mbStrike)
        aStr = "<strike>";
    else if(!bStrike && mbStrike)
        aStr = "</strike>";

    mbStrike = bStrike;
    return aStr;
}

// Sets the specified text color
OUString HtmlState::SetColor( Color aColor )
{
    OUString aStr;

    if(mbColor && aColor == maColor)
        return aStr;

    if(mbColor)
    {
        aStr = "</font>";
        mbColor = false;
    }

    if(aColor != maDefColor)
    {
        maColor = aColor;
        aStr += "<font color=\"" + ColorToHTMLString(aColor) + "\">";
        mbColor = true;
    }

    return aStr;
}

// enables/disables a hyperlink
OUString HtmlState::SetLink( const OUString& aLink, const OUString& aTarget )
{
    OUString aStr;

    if(mbLink&&maLink == aLink&&maTarget==aTarget)
        return aStr;

    if(mbLink)
    {
        aStr = "</a>";
        mbLink = false;
    }

    if (!aLink.isEmpty())
    {
        aStr += "<a href=\"" + comphelper::string::encodeForXml(aLink);
        if (!aTarget.isEmpty())
        {
            aStr += "\" target=\"" + comphelper::string::encodeForXml(aTarget);
        }
        aStr += "\">";
        mbLink = true;
        maLink = aLink;
        maTarget = aTarget;
    }

    return aStr;
}

namespace
{

OUString getParagraphStyle( const SdrOutliner* pOutliner, sal_Int32 nPara )
{
    SfxItemSet aParaSet( pOutliner->GetParaAttribs( nPara ) );

    OUString sStyle;

    if( aParaSet.GetItem<SvxFrameDirectionItem>( EE_PARA_WRITINGDIR )->GetValue() == SvxFrameDirection::Horizontal_RL_TB )
    {

        sStyle = "direction: rtl;";
    }
    else
    {
        // This is the default so don't write it out
        // sStyle += "direction: ltr;";
    }
    return sStyle;
}

void lclAppendStyle(OUStringBuffer& aBuffer, std::u16string_view aTag, std::u16string_view aStyle)
{
    if (aStyle.empty())
        aBuffer.append(OUString::Concat("<") + aTag + ">");
    else
        aBuffer.append(OUString::Concat("<") + aTag + " style=\"" + aStyle + "\">");
}

// Depending on the attributes of the specified set and the specified
// HtmlState, it creates the needed html tags in order to get the
// attributes
OUString TextAttribToHTMLString( SfxItemSet const * pSet, HtmlState* pState )
{
    OUStringBuffer aStr;

    if(nullptr == pSet)
        return OUString();

    OUString aLink, aTarget;
    if ( pSet->GetItemState( EE_FEATURE_FIELD ) == SfxItemState::SET )
    {
        const SvxFieldItem* pItem = pSet->GetItem<SvxFieldItem>( EE_FEATURE_FIELD );
        if(pItem)
        {
            const SvxURLField* pURL = dynamic_cast<const SvxURLField*>( pItem->GetField() );
            if(pURL)
            {
                aLink = pURL->GetURL();
                aTarget = pURL->GetTargetFrame();
            }
        }
    }

    bool bTemp;
    OUString aTemp;

    if ( pSet->GetItemState( EE_CHAR_WEIGHT ) == SfxItemState::SET )
    {
        bTemp = pSet->Get( EE_CHAR_WEIGHT ).GetWeight() == WEIGHT_BOLD;
        aTemp = pState->SetWeight( bTemp );
        if( bTemp )
            aStr.insert(0, aTemp);
        else
            aStr.append(aTemp);
    }

    if ( pSet->GetItemState( EE_CHAR_UNDERLINE ) == SfxItemState::SET )
    {
        bTemp = pSet->Get( EE_CHAR_UNDERLINE ).GetLineStyle() != LINESTYLE_NONE;
        aTemp = pState->SetUnderline( bTemp );
        if( bTemp )
            aStr.insert(0, aTemp);
        else
            aStr.append(aTemp);
    }

    if ( pSet->GetItemState( EE_CHAR_STRIKEOUT ) == SfxItemState::SET )
    {
        bTemp = pSet->Get( EE_CHAR_STRIKEOUT ).GetStrikeout() != STRIKEOUT_NONE;
        aTemp = pState->SetStrikeout( bTemp );
        if( bTemp )
            aStr.insert(0, aTemp);
        else
            aStr.append(aTemp);
    }

    if ( pSet->GetItemState( EE_CHAR_ITALIC ) == SfxItemState::SET )
    {
        bTemp = pSet->Get( EE_CHAR_ITALIC ).GetPosture() != ITALIC_NONE;
        aTemp = pState->SetItalic( bTemp );
        if( bTemp )
            aStr.insert(0, aTemp);
        else
            aStr.append(aTemp);
    }

    if (!aLink.isEmpty())
        aStr.insert(0, pState->SetLink(aLink, aTarget));
    else
        aStr.append(pState->SetLink(aLink, aTarget));

    return aStr.makeStringAndClear();
}

// escapes a string for html
OUString StringToHTMLString( std::u16string_view rString )
{
    SvMemoryStream aMemStm;
    HTMLOutFuncs::Out_String( aMemStm, rString );
    return OUString( static_cast<char const *>(aMemStm.GetData()), aMemStm.GetSize(), RTL_TEXTENCODING_UTF8 );
}

// converts a paragraph of the outliner to html
OUString ParagraphToHTMLString( SdrOutliner const * pOutliner, sal_Int32 nPara )
{
    OUStringBuffer aStr;

    if(nullptr == pOutliner)
        return OUString();

    // TODO: MALTE!!!
    EditEngine& rEditEngine = *const_cast<EditEngine*>(&pOutliner->GetEditEngine());
    bool bOldUpdateMode = rEditEngine.SetUpdateLayout(true);

    Paragraph* pPara = pOutliner->GetParagraph(nPara);
    if(nullptr == pPara)
        return OUString();

    HtmlState aState( COL_BLACK );
    std::vector<sal_Int32> aPortionList;
    rEditEngine.GetPortions( nPara, aPortionList );

    sal_Int32 nPos1 = 0;
    for( sal_Int32 nPos2 : aPortionList )
    {
        ESelection aSelection( nPara, nPos1, nPara, nPos2);

        SfxItemSet aSet( rEditEngine.GetAttribs( aSelection ) );

        aStr.append(TextAttribToHTMLString( &aSet, &aState ) +
            StringToHTMLString(rEditEngine.GetText( aSelection )));

        nPos1 = nPos2;
    }
    aStr.append(aState.Flush());
    rEditEngine.SetUpdateLayout(bOldUpdateMode);

    return aStr.makeStringAndClear();
}

void WriteOutlinerParagraph(OUStringBuffer& aStr, SdrOutliner* pOutliner,
                                        OutlinerParaObject const * pOutlinerParagraphObject,
                                        bool bHeadLine)
{
    if (pOutlinerParagraphObject == nullptr)
        return;

    pOutliner->SetText(*pOutlinerParagraphObject);

    sal_Int32 nCount = pOutliner->GetParagraphCount();


    sal_Int16 nCurrentDepth = -1;

    for (sal_Int32 nIndex = 0; nIndex < nCount; nIndex++)
    {
        Paragraph* pParagraph = pOutliner->GetParagraph(nIndex);
        if(pParagraph == nullptr)
            continue;

        const sal_Int16 nDepth = static_cast<sal_uInt16>(pOutliner->GetDepth(nIndex));
        OUString aParaText = ParagraphToHTMLString(pOutliner, nIndex);

        if (aParaText.isEmpty())
            continue;

        if (nDepth < 0)
        {
            OUString aTag = bHeadLine ? OUString("h2") : OUString("p");
            lclAppendStyle(aStr, aTag, getParagraphStyle(pOutliner, nIndex));

            aStr.append(aParaText);
            aStr.append("</" + aTag + ">\r\n");
        }
        else
        {
            while(nCurrentDepth < nDepth)
            {
                aStr.append("<ul>\r\n");
                nCurrentDepth++;
            }
            while(nCurrentDepth > nDepth)
            {
                aStr.append("</ul>\r\n");
                nCurrentDepth--;
            }
            lclAppendStyle(aStr, u"li", getParagraphStyle(pOutliner, nIndex));
            aStr.append(aParaText);
            aStr.append("</li>\r\n");
        }
    }
    while(nCurrentDepth >= 0)
    {
        aStr.append("</ul>\r\n");
        nCurrentDepth--;
    }
    pOutliner->Clear();
}

void WriteTable(OUStringBuffer& aStr, SdrTableObj const * pTableObject, SdrOutliner* pOutliner)
{
    CellPos aStart, aEnd;

    aStart = SdrTableObj::getFirstCell();
    aEnd = pTableObject->getLastCell();

    sal_Int32 nColCount = pTableObject->getColumnCount();
    aStr.append("<table>\r\n");
    for (sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++)
    {
        aStr.append("  <tr>\r\n");
        for (sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++)
        {
            aStr.append("    <td>\r\n");
            sal_Int32 nCellIndex = nRow * nColCount + nCol;
            SdrText* pText = pTableObject->getText(nCellIndex);

            if (pText == nullptr)
                continue;
            WriteOutlinerParagraph(aStr, pOutliner, pText->GetOutlinerParaObject(), false);
            aStr.append("    </td>\r\n");
        }
        aStr.append("  </tr>\r\n");
    }
    aStr.append("</table>\r\n");
}

void WriteObjectGroup(OUStringBuffer& aStr, SdrObjGroup const * pObjectGroup, SdrOutliner* pOutliner,
                                  bool bHeadLine)
{
    SdrObjListIter aGroupIterator(pObjectGroup->GetSubList(), SdrIterMode::DeepNoGroups);
    while (aGroupIterator.IsMore())
    {
        SdrObject* pCurrentObject = aGroupIterator.Next();
        if (pCurrentObject->GetObjIdentifier() == SdrObjKind::Group)
        {
            SdrObjGroup* pCurrentGroupObject = static_cast<SdrObjGroup*>(pCurrentObject);
            WriteObjectGroup(aStr, pCurrentGroupObject, pOutliner, bHeadLine);
        }
        else
        {
            OutlinerParaObject* pOutlinerParagraphObject = pCurrentObject->GetOutlinerParaObject();
            if (pOutlinerParagraphObject != nullptr)
            {
                WriteOutlinerParagraph(aStr, pOutliner, pOutlinerParagraphObject, bHeadLine);
            }
        }
    }
}

// get SdrTextObject with layout text of this page
SdrTextObj* GetLayoutTextObject(SdrPage const * pPage)
{
    SdrTextObj*     pResult      = nullptr;

    for (const rtl::Reference<SdrObject>& pObject : *pPage)
    {
        if (pObject->GetObjInventor() == SdrInventor::Default &&
            pObject->GetObjIdentifier() == SdrObjKind::OutlineText)
        {
            pResult = static_cast<SdrTextObj*>(pObject.get());
            break;
        }
    }
    return pResult;
}


/** creates an outliner text for the title objects of a page
 */
OUString CreateTextForTitle( SdrOutliner* pOutliner, SdPage* pPage )
{
    SdrTextObj* pTO = static_cast<SdrTextObj*>(pPage->GetPresObj(PresObjKind::Title));
    if(!pTO)
        pTO = GetLayoutTextObject(pPage);

    if (pTO && !pTO->IsEmptyPresObj())
    {
        OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
        if(pOPO && pOutliner->GetParagraphCount() != 0)
        {
            pOutliner->Clear();
            pOutliner->SetText(*pOPO);
            return ParagraphToHTMLString(pOutliner, 0);
        }
    }

    return OUString();
}

// creates an outliner text for a page
OUString CreateTextForPage(SdrOutliner* pOutliner, SdPage const * pPage,
                                       bool bHeadLine)
{
    OUStringBuffer aStr;

    for (const rtl::Reference<SdrObject>& pObject : *pPage)
    {
        PresObjKind eKind = pPage->GetPresObjKind(pObject.get());

        switch (eKind)
        {
            case PresObjKind::NONE:
            {
                if (pObject->GetObjIdentifier() == SdrObjKind::Group)
                {
                    SdrObjGroup* pObjectGroup = static_cast<SdrObjGroup*>(pObject.get());
                    WriteObjectGroup(aStr, pObjectGroup, pOutliner, false);
                }
                else if (pObject->GetObjIdentifier() == SdrObjKind::Table)
                {
                    SdrTableObj* pTableObject = static_cast<SdrTableObj*>(pObject.get());
                    WriteTable(aStr, pTableObject, pOutliner);
                }
                else
                {
                    if (pObject->GetOutlinerParaObject())
                    {
                        WriteOutlinerParagraph(aStr, pOutliner, pObject->GetOutlinerParaObject(), false);
                    }
                }
            }
            break;

            case PresObjKind::Table:
            {
                SdrTableObj* pTableObject = static_cast<SdrTableObj*>(pObject.get());
                WriteTable(aStr, pTableObject, pOutliner);
            }
            break;

            case PresObjKind::Text:
            case PresObjKind::Outline:
            {
                SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject.get());
                if (pTextObject->IsEmptyPresObj())
                    continue;
                WriteOutlinerParagraph(aStr, pOutliner, pTextObject->GetOutlinerParaObject(), bHeadLine);
            }
            break;

            default:
                break;
        }
    }
    return aStr.makeStringAndClear();
}

} // namespace

constexpr OUStringLiteral gaHTMLHeader(
            u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\r\n"
            "     \"http://www.w3.org/TR/html4/transitional.dtd\">\r\n"
            "<html>\r\n<head>\r\n" );

// constructor for the html export helper classes
HtmlExport::HtmlExport(
    OUString aPath,
    SdDrawDocument* pExpDoc,
    sd::DrawDocShell* pDocShell )
    :   maPath(std::move( aPath )),
        mpDoc(pExpDoc),
        mpDocSh( pDocShell )
{
    bool bChange = mpDoc->IsChanged();

    Init();

    ExportSingleDocument();

    mpDoc->SetChanged(bChange);
}

HtmlExport::~HtmlExport()
{
}

void HtmlExport::Init()
{
    SdPage* pPage = mpDoc->GetSdPage(0, PageKind::Standard);

    // we come up with a destination...
    INetURLObject aINetURLObj( maPath );
    DBG_ASSERT( aINetURLObj.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    maExportPath = aINetURLObj.GetPartBeforeLastName(); // with trailing '/'
    maIndex = aINetURLObj.GetLastName();

    mnSdPageCount = mpDoc->GetSdPageCount( PageKind::Standard );
    for( sal_uInt16 nPage = 0; nPage < mnSdPageCount; nPage++ )
    {
        pPage = mpDoc->GetSdPage( nPage, PageKind::Standard );

        maPages.push_back( pPage );
    }
    mnSdPageCount = maPages.size();

    maDocFileName = maIndex;
}

void HtmlExport::ExportSingleDocument()
{
    SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();

    mnPagesWritten = 0;
    InitProgress(mnSdPageCount);

    OUStringBuffer aStr(gaHTMLHeader
        + DocumentMetadata()
        + "\r\n"
        "</head>\r\n"
        "<body>\r\n");

    for(sal_uInt16 nSdPage = 0; nSdPage < mnSdPageCount; ++nSdPage)
    {
        SdPage* pPage = maPages[nSdPage];

        // page title
        OUString sTitleText(CreateTextForTitle(pOutliner, pPage));
        OUString sStyle;

        if (nSdPage != 0) // First page - no need for a page break here
            sStyle += "page-break-before:always; ";
        sStyle += getParagraphStyle(pOutliner, 0);

        lclAppendStyle(aStr, u"h1", sStyle);

        aStr.append(sTitleText + "</h1>\r\n");

        // write outline text
        aStr.append(CreateTextForPage( pOutliner, pPage, true));

        if (mpProgress)
            mpProgress->SetState(++mnPagesWritten);

    }

    // close page
    aStr.append("</body>\r\n</html>");

    WriteHtml(maDocFileName, aStr);

    pOutliner->Clear();
    ResetProgress();
}

void HtmlExport::InitProgress( sal_uInt16 nProgrCount )
{
    mpProgress.reset(new SfxProgress( mpDocSh, SdResId(STR_CREATE_PAGES), nProgrCount ));
}

void HtmlExport::ResetProgress()
{
    mpProgress.reset();
}

OUString HtmlExport::DocumentMetadata() const
{
    SvMemoryStream aStream;

    uno::Reference<document::XDocumentProperties> xDocProps;
    if (mpDocSh)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            mpDocSh->GetModel(), uno::UNO_QUERY_THROW);
        xDocProps.set(xDPS->getDocumentProperties());
    }

    SfxFrameHTMLWriter::Out_DocInfo(aStream, maDocFileName, xDocProps,
            "  ");

    const sal_uInt64 nLen = aStream.GetSize();
    OSL_ENSURE(nLen < o3tl::make_unsigned(SAL_MAX_INT32), "Stream can't fit in OString");
    std::string_view aData(static_cast<const char*>(aStream.GetData()), static_cast<sal_Int32>(nLen));

    return OStringToOUString(aData, RTL_TEXTENCODING_UTF8);
}

/** exports the given html data into a non unicode file in the current export path with
    the given filename */
bool HtmlExport::WriteHtml( std::u16string_view rFileName, std::u16string_view rHtmlData )
{
    ErrCode nErr = ERRCODE_NONE;

    EasyFile aFile;
    SvStream* pStr;
    OUString aFull(maExportPath + rFileName);
    nErr = aFile.createStream(aFull , pStr);
    if(nErr == ERRCODE_NONE)
    {
        OString aStr(OUStringToOString(rHtmlData, RTL_TEXTENCODING_UTF8));
        pStr->WriteOString( aStr );
        aFile.close();
    }

    if( nErr != ERRCODE_NONE )
        ErrorHandler::HandleError(nErr);

    return nErr == ERRCODE_NONE;
}

EasyFile::EasyFile() : bOpen(false)
{
}

EasyFile::~EasyFile()
{
    if( bOpen )
        close();
}

ErrCode EasyFile::createStream(  const OUString& rUrl, SvStream* &rpStr )
{
    if(bOpen)
        close();

    OUString aFileName;
    createFileName( rUrl, aFileName );

    ErrCode nErr = ERRCODE_NONE;
    pOStm = ::utl::UcbStreamHelper::CreateStream( aFileName, StreamMode::WRITE | StreamMode::TRUNC );
    if( pOStm )
    {
        bOpen = true;
        nErr = pOStm->GetError();
    }
    else
    {
        nErr = ERRCODE_SFX_CANTCREATECONTENT;
    }

    if( nErr != ERRCODE_NONE )
    {
        bOpen = false;
        pOStm.reset();
    }

    rpStr = pOStm.get();

    return nErr;
}

void EasyFile::createFileName(  const OUString& rURL, OUString& rFileName )
{
    if( bOpen )
        close();

    INetURLObject aURL( rURL );

    if( aURL.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aURLStr;
        osl::FileBase::getFileURLFromSystemPath( rURL, aURLStr );
        aURL = INetURLObject( aURLStr );
    }
    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
    rFileName = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}

void EasyFile::close()
{
    pOStm.reset();
    bOpen = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
