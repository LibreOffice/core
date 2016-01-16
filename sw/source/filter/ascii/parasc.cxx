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

#include <memory>
#include <tools/stream.hxx>
#include <hintids.hxx>
#include <rtl/tencinfo.h>
#include <sfx2/printer.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <shellio.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <swtypes.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <frmatr.hxx>
#include <fltini.hxx>
#include <pagedesc.hxx>
#include <breakit.hxx>
#include <swerror.h>
#include <statstr.hrc>
#include <mdiexp.hxx>
#include <poolfmt.hxx>

#include <vcl/metric.hxx>

#define ASC_BUFFLEN 4096

class SwASCIIParser
{
    SwDoc* pDoc;
    SwPaM* pPam;
    SvStream& rInput;
    sal_Char* pArr;
    const SwAsciiOptions& rOpt;
    SfxItemSet* pItemSet;
    long nFileSize;
    SvtScriptType nScript;
    bool bNewDoc;

    sal_uLong ReadChars();
    void InsertText( const OUString& rStr );

public:
    SwASCIIParser( SwDoc* pD, const SwPaM& rCursor, SvStream& rIn,
                            bool bReadNewDoc, const SwAsciiOptions& rOpts );
    ~SwASCIIParser();

    sal_uLong CallParser();
};

// Call for the general reader interface
sal_uLong AsciiReader::Read( SwDoc &rDoc, const OUString&, SwPaM &rPam, const OUString & )
{
    if( !pStrm )
    {
        OSL_ENSURE( false, "ASCII read without a stream" );
        return ERR_SWG_READ_ERROR;
    }

    SwASCIIParser* pParser = new SwASCIIParser( &rDoc, rPam, *pStrm,
                                        !bInsertMode, aOpt.GetASCIIOpts() );
    sal_uLong nRet = pParser->CallParser();

    delete pParser;
    // after Read reset the options
    aOpt.ResetASCIIOpts();
    return nRet;
}

SwASCIIParser::SwASCIIParser(SwDoc* pD, const SwPaM& rCursor, SvStream& rIn,
    bool bReadNewDoc, const SwAsciiOptions& rOpts)
    : pDoc(pD), rInput(rIn), rOpt(rOpts), nFileSize(0), nScript(SvtScriptType::NONE)
    , bNewDoc(bReadNewDoc)
{
    pPam = new SwPaM( *rCursor.GetPoint() );
    pArr = new sal_Char [ ASC_BUFFLEN + 2 ];

    pItemSet = new SfxItemSet( pDoc->GetAttrPool(),
                RES_CHRATR_FONT,        RES_CHRATR_LANGUAGE,
                RES_CHRATR_CJK_FONT,    RES_CHRATR_CJK_LANGUAGE,
                RES_CHRATR_CTL_FONT,    RES_CHRATR_CTL_LANGUAGE,
                0 );

    // set defaults from the options
    if( rOpt.GetLanguage() )
    {
        SvxLanguageItem aLang( (LanguageType)rOpt.GetLanguage(),
                                 RES_CHRATR_LANGUAGE );
        pItemSet->Put( aLang );
        pItemSet->Put( aLang, RES_CHRATR_CJK_LANGUAGE );
        pItemSet->Put( aLang, RES_CHRATR_CTL_LANGUAGE );
    }
    if( !rOpt.GetFontName().isEmpty() )
    {
        vcl::Font aTextFont( rOpt.GetFontName(), Size( 0, 10 ) );
        if( pDoc->getIDocumentDeviceAccess().getPrinter( false ) )
            aTextFont = pDoc->getIDocumentDeviceAccess().getPrinter( false )->GetFontMetric( aTextFont );
        SvxFontItem aFont( aTextFont.GetFamily(), aTextFont.GetFamilyName(),
                           OUString(), aTextFont.GetPitch(), aTextFont.GetCharSet(), RES_CHRATR_FONT );
        pItemSet->Put( aFont );
        pItemSet->Put( aFont, RES_CHRATR_CJK_FONT );
        pItemSet->Put( aFont, RES_CHRATR_CTL_FONT );
    }
}

SwASCIIParser::~SwASCIIParser()
{
    delete pPam;
    delete [] pArr;
    delete pItemSet;
}

// Calling the parser
sal_uLong SwASCIIParser::CallParser()
{
    rInput.Seek(STREAM_SEEK_TO_END);
    rInput.ResetError();

    nFileSize = rInput.Tell();
    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();

    ::StartProgress( STR_STATSTR_W4WREAD, 0, nFileSize, pDoc->GetDocShell() );

    SwPaM* pInsPam = nullptr;
    sal_Int32 nSttContent = 0;
    if (!bNewDoc)
    {
        const SwNodeIndex& rTmp = pPam->GetPoint()->nNode;
        pInsPam = new SwPaM( rTmp, rTmp, 0, -1 );
        nSttContent = pPam->GetPoint()->nContent.GetIndex();
    }

    SwTextFormatColl *pColl = nullptr;

    if (bNewDoc)
    {
        pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HTML_PRE, false);
        if (!pColl)
            pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD,false);
        if (pColl)
            pDoc->SetTextFormatColl(*pPam, pColl);
    }

    sal_uLong nError = ReadChars();

    if( pItemSet )
    {
        // set only the attribute, for scanned scripts.
        if( !( SvtScriptType::LATIN & nScript ))
        {
            pItemSet->ClearItem( RES_CHRATR_FONT );
            pItemSet->ClearItem( RES_CHRATR_LANGUAGE );
        }
        if( !( SvtScriptType::ASIAN & nScript ))
        {
            pItemSet->ClearItem( RES_CHRATR_CJK_FONT );
            pItemSet->ClearItem( RES_CHRATR_CJK_LANGUAGE );
        }
        if( !( SvtScriptType::COMPLEX & nScript ))
        {
            pItemSet->ClearItem( RES_CHRATR_CTL_FONT );
            pItemSet->ClearItem( RES_CHRATR_CTL_LANGUAGE );
        }
        if( pItemSet->Count() )
        {
            if( bNewDoc )
            {
                if (pColl)
                {
                    // Using the pool defaults for the font causes significant
                    // trouble for the HTML filter, because it is not able
                    // to export the pool defaults (or to be more precise:
                    // the HTML filter is not able to detect whether a pool
                    // default has changed or not. Even a comparison with the
                    // HTML template does not work, because the defaults are
                    // not copied when a new doc is created. The result of
                    // comparing pool defaults therefore would be that the
                    // defaults are exported always if the have changed for
                    // text documents in general. That's not sensible, as well
                    // as it is not sensible to export them always.
                    sal_uInt16 aWhichIds[4] =
                    {
                        RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                        RES_CHRATR_CTL_FONT, 0
                    };
                    sal_uInt16 *pWhichIds = aWhichIds;
                    while (*pWhichIds)
                    {
                        const SfxPoolItem *pItem;
                        if (SfxItemState::SET == pItemSet->GetItemState(*pWhichIds,
                            false, &pItem))
                        {
                            pColl->SetFormatAttr( *pItem );
                            pItemSet->ClearItem( *pWhichIds );
                        }
                        ++pWhichIds;
                    }
                }
                if (pItemSet->Count())
                    pDoc->SetDefault(*pItemSet);
            }
            else if( pInsPam )
            {
                // then set over the insert range the defined attributes
                *pInsPam->GetMark() = *pPam->GetPoint();
                ++pInsPam->GetPoint()->nNode;
                pInsPam->GetPoint()->nContent.Assign(
                                    pInsPam->GetContentNode(), nSttContent );

                // !!!!!
                OSL_ENSURE( false, "Have to change - hard attr. to para. style" );
                pDoc->getIDocumentContentOperations().InsertItemSet( *pInsPam, *pItemSet );
            }
        }
        delete pItemSet, pItemSet = nullptr;
    }

    delete pInsPam;

    ::EndProgress( pDoc->GetDocShell() );
    return nError;
}

sal_uLong SwASCIIParser::ReadChars()
{
    sal_Unicode *pStt = nullptr, *pEnd = nullptr, *pLastStt = nullptr;
    long nReadCnt = 0, nLineLen = 0;
    sal_Unicode cLastCR = 0;
    bool bSwapUnicode = false;

    const SwAsciiOptions *pUseMe=&rOpt;
    SwAsciiOptions aEmpty;
    if (nFileSize >= 2 &&
        aEmpty.GetFontName() == rOpt.GetFontName() &&
        aEmpty.GetCharSet() == rOpt.GetCharSet() &&
        aEmpty.GetLanguage() == rOpt.GetLanguage() &&
        aEmpty.GetParaFlags() == rOpt.GetParaFlags())
    {
        sal_uLong nLen, nOrig;
        nOrig = nLen = rInput.Read(pArr, ASC_BUFFLEN);
        rtl_TextEncoding eCharSet;
        bool bRet = SwIoSystem::IsDetectableText(pArr, nLen, &eCharSet, &bSwapUnicode);
        OSL_ENSURE(bRet, "Autodetect of text import without nag dialog must have failed");
        if (bRet && eCharSet != RTL_TEXTENCODING_DONTKNOW)
        {
            aEmpty.SetCharSet(eCharSet);
            rInput.SeekRel(-(long(nLen)));
        }
        else
            rInput.SeekRel(-(long(nOrig)));
        pUseMe=&aEmpty;
    }

    rtl_TextToUnicodeConverter hConverter=nullptr;
    rtl_TextToUnicodeContext hContext=nullptr;
    rtl_TextEncoding currentCharSet = pUseMe->GetCharSet();
    if (RTL_TEXTENCODING_UCS2 != currentCharSet)
    {
        if( currentCharSet == RTL_TEXTENCODING_DONTKNOW )
                currentCharSet = RTL_TEXTENCODING_ASCII_US;
        hConverter = rtl_createTextToUnicodeConverter( currentCharSet );
        OSL_ENSURE( hConverter, "no string convert available" );
        if (!hConverter)
            return ERROR_SW_READ_BASE;
        bSwapUnicode = false;
        hContext = rtl_createTextToUnicodeContext( hConverter );
    }
    else if (pUseMe != &aEmpty)  //Already successfully figured out type
    {
        rInput.StartReadingUnicodeText( currentCharSet );
        bSwapUnicode = rInput.IsEndianSwap();
    }

    std::unique_ptr<sal_Unicode[]> aWork;
    sal_uLong nArrOffset = 0;

    do {
        if( pStt >= pEnd )
        {
            if( pLastStt != pStt )
                InsertText( OUString( pLastStt ));

            // Read a new block
            sal_uLong lGCount;
            if( SVSTREAM_OK != rInput.GetError() || 0 == (lGCount =
                        rInput.Read( pArr + nArrOffset,
                                     ASC_BUFFLEN - nArrOffset )))
                break;      // break from the while loop

            /*
            If there was some unconverted bytes on the last cycle then they
            were put at the beginning of the array, so total bytes available
            to convert this cycle includes them. If we found 0 following bytes
            then we ignore the previous partial character.
            */
            lGCount+=nArrOffset;

            if( hConverter )
            {
                sal_uInt32 nInfo;
                sal_Size nNewLen = lGCount, nCntBytes;
                aWork.reset(new sal_Unicode[nNewLen + 1]); // add 1 for '\0'
                sal_Unicode* pBuf = aWork.get();
                pBuf[nNewLen] = 0;                         // ensure '\0'

                nNewLen = rtl_convertTextToUnicode( hConverter, hContext,
                                pArr, lGCount, pBuf, nNewLen,
                                (
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE
                                ),
                                &nInfo,
                                &nCntBytes );
                if( 0 != ( nArrOffset = lGCount - nCntBytes ) )
                    memmove( pArr, pArr + nCntBytes, nArrOffset );

                pStt = pLastStt = aWork.get();
                pEnd = pStt + nNewLen;
            }
            else
            {
                pStt = pLastStt = reinterpret_cast<sal_Unicode*>(pArr);
                pEnd = reinterpret_cast<sal_Unicode*>(pArr + lGCount);

                if( bSwapUnicode )
                {
                    sal_Char* pF = pArr, *pN = pArr + 1;
                    for( sal_uLong n = 0; n < lGCount; n += 2, pF += 2, pN += 2 )
                    {
                        sal_Char c = *pF;
                        *pF = *pN;
                        *pN = c;
                    }
                }
            }

            *pEnd = 0;
            nReadCnt += lGCount;

            ::SetProgressState( nReadCnt, pDoc->GetDocShell() );

            if( cLastCR )
            {
                if( 0x0a == *pStt && 0x0d == cLastCR )
                    pLastStt = ++pStt;
                cLastCR = 0;
                nLineLen = 0;
                // We skip the last one at the end
                if( !rInput.IsEof() || !(pEnd == pStt ||
                    ( !*pEnd && pEnd == pStt+1 ) ) )
                    pDoc->getIDocumentContentOperations().SplitNode( *pPam->GetPoint(), false );
            }
        }

        bool bIns = true, bSplitNode = false;
        switch( *pStt )
        {

        case 0x0a:  if( LINEEND_LF == pUseMe->GetParaFlags() )
                    {
                        bIns = false;
                        *pStt = 0;
                        ++pStt;

                        // We skip the last one at the end
                        if( !rInput.IsEof() || pEnd != pStt )
                            bSplitNode = true;
                    }
                    break;

        case 0x0d:  if( LINEEND_LF != pUseMe->GetParaFlags() )
                    {
                        bIns = false;
                        *pStt = 0;
                        ++pStt;

                        bool bChkSplit = false;
                        if( LINEEND_CRLF == pUseMe->GetParaFlags() )
                        {
                            if( pStt == pEnd )
                                cLastCR = 0x0d;
                            else if( 0x0a == *pStt )
                            {
                                ++pStt;
                                bChkSplit = true;
                            }
                        }
                        else
                            bChkSplit = true;

                        // We skip the last one at the end
                        if( bChkSplit && ( !rInput.IsEof() || pEnd != pStt ))
                            bSplitNode = true;
                    }
                    break;

        case 0x0c:
                    {
                        // Insert a hard page break
                        *pStt++ = 0;
                        if( nLineLen )
                        {
                            InsertText( OUString( pLastStt ));
                        }
                        pDoc->getIDocumentContentOperations().SplitNode( *pPam->GetPoint(), false );
                        pDoc->getIDocumentContentOperations().InsertPoolItem(
                            *pPam, SvxFormatBreakItem( SVX_BREAK_PAGE_BEFORE, RES_BREAK ) );
                        pLastStt = pStt;
                        nLineLen = 0;
                        bIns = false;
                    }
                    break;

        case 0x1a:
                    if( nReadCnt == nFileSize && pStt+1 == pEnd )
                        *pStt = 0;
                    else
                        *pStt = '#';        // Replacement visualisation
                    break;

        case '\t':  break;

        default:
            if( ' ' > *pStt )
            // Found control char, replace with '#'
                *pStt = '#';
            break;
        }

        if( bIns )
        {
            if( ( nLineLen >= MAX_ASCII_PARA - 100 ) &&
                ( ( *pStt == ' ' ) || ( nLineLen >= MAX_ASCII_PARA - 1 ) ) )
            {
                sal_Unicode c = *pStt;
                *pStt = 0;
                InsertText( OUString( pLastStt ));
                pDoc->getIDocumentContentOperations().SplitNode( *pPam->GetPoint(), false );
                pLastStt = pStt;
                nLineLen = 0;
                *pStt = c;
            }
            ++pStt;
            ++nLineLen;
        }
        else if( bSplitNode )
        {
            // We found a CR/LF, thus save the text
            InsertText( OUString( pLastStt ));
            pDoc->getIDocumentContentOperations().SplitNode( *pPam->GetPoint(), false );
            pLastStt = pStt;
            nLineLen = 0;
        }
    } while(true);

    if( hConverter )
    {
        rtl_destroyTextToUnicodeContext( hConverter, hContext );
        rtl_destroyTextToUnicodeConverter( hConverter );
    }
    return 0;
}

void SwASCIIParser::InsertText( const OUString& rStr )
{
    pDoc->getIDocumentContentOperations().InsertString( *pPam, rStr );
    pDoc->UpdateRsid( *pPam, rStr.getLength() );
    pDoc->UpdateParRsid( pPam->GetPoint()->nNode.GetNode().GetTextNode() );

    if( pItemSet && g_pBreakIt && nScript != ( SvtScriptType::LATIN |
                                             SvtScriptType::ASIAN |
                                             SvtScriptType::COMPLEX ) )
        nScript |= g_pBreakIt->GetAllScriptsOfText( rStr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
