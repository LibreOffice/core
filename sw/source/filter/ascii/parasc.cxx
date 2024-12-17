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
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/sfxsids.hrc>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <svl/languageoptions.hxx>
#include <shellio.hxx>
#include <doc.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pam.hxx>
#include <breakit.hxx>
#include <swerror.h>
#include <strings.hrc>
#include <mdiexp.hxx>
#include <poolfmt.hxx>
#include <iodetect.hxx>

#include <vcl/metric.hxx>
#include <osl/diagnose.h>

#define ASC_BUFFLEN 4096

namespace {

class SwASCIIParser
{
    SwDoc& m_rDoc;
    std::optional<SwPaM> m_oPam;
    SvStream& m_rInput;
    std::unique_ptr<char[]> m_pArr;
    const SwAsciiOptions& m_rOpt;
    SwAsciiOptions m_usedAsciiOptions;
    std::optional<SfxItemSet> m_oItemSet;
    tools::Long m_nFileSize;
    SvtScriptType m_nScript;
    bool m_bNewDoc;

    ErrCode ReadChars();
    void InsertText( const OUString& rStr );

    SwASCIIParser(const SwASCIIParser&) = delete;
    SwASCIIParser& operator=(const SwASCIIParser&) = delete;

public:
    SwASCIIParser( SwDoc& rD, const SwPaM& rCursor, SvStream& rIn,
                            bool bReadNewDoc, const SwAsciiOptions& rOpts );

    ErrCode CallParser();
    const SwAsciiOptions& GetUsedAsciiOptions() const { return m_usedAsciiOptions; }
};

}

// Call for the general reader interface
ErrCodeMsg AsciiReader::Read( SwDoc& rDoc, const OUString&, SwPaM &rPam, const OUString & )
{
    if( !m_pStream )
    {
        OSL_ENSURE( false, "ASCII read without a stream" );
        return ERR_SWG_READ_ERROR;
    }

    ErrCode nRet;
    {
        SwASCIIParser aParser( rDoc, rPam, *m_pStream,
                                !m_bInsertMode, m_aOption.GetASCIIOpts() );
        nRet = aParser.CallParser();

        OUString optionsString;
        aParser.GetUsedAsciiOptions().WriteUserData(optionsString);

        if(m_pMedium != nullptr)
            m_pMedium->GetItemSet().Put(SfxStringItem(SID_FILE_FILTEROPTIONS, optionsString));
    }
    // after Read reset the options
    m_aOption.ResetASCIIOpts();
    return nRet;
}

SwASCIIParser::SwASCIIParser(SwDoc& rD, const SwPaM& rCursor, SvStream& rIn, bool bReadNewDoc,
                             const SwAsciiOptions& rOpts)
    : m_rDoc(rD)
    , m_rInput(rIn)
    , m_rOpt(rOpts)
    , m_usedAsciiOptions(rOpts)
    , m_nFileSize(0)
    , m_nScript(SvtScriptType::NONE)
    , m_bNewDoc(bReadNewDoc)
{
    m_oPam.emplace(*rCursor.GetPoint());
    m_pArr.reset(new char[ASC_BUFFLEN + 2]);

    m_oItemSet.emplace(
        m_rDoc.GetAttrPool(),
        svl::Items<RES_CHRATR_FONT, RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_FONT,
                   RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_LANGUAGE>);

    // set defaults from the options
    if (m_rOpt.GetLanguage())
    {
        SvxLanguageItem aLang(m_rOpt.GetLanguage(), RES_CHRATR_LANGUAGE);
        m_oItemSet->Put(aLang);
        aLang.SetWhich(RES_CHRATR_CJK_LANGUAGE);
        m_oItemSet->Put(aLang);
        aLang.SetWhich(RES_CHRATR_CTL_LANGUAGE);
        m_oItemSet->Put(aLang);
    }
    if (m_rOpt.GetFontName().isEmpty())
        return;

    vcl::Font aTextFont(m_rOpt.GetFontName(), Size(0, 10));
    if (m_rDoc.getIDocumentDeviceAccess().getPrinter(false))
        aTextFont = m_rDoc.getIDocumentDeviceAccess().getPrinter(false)->GetFontMetric(aTextFont);
    SvxFontItem aFont( aTextFont.GetFamilyType(), aTextFont.GetFamilyName(),
                       OUString(), aTextFont.GetPitch(), aTextFont.GetCharSet(), RES_CHRATR_FONT );
    m_oItemSet->Put(aFont);
    aFont.SetWhich(RES_CHRATR_CJK_FONT);
    m_oItemSet->Put(aFont);
    aFont.SetWhich(RES_CHRATR_CTL_FONT);
    m_oItemSet->Put(aFont);
}

// Calling the parser
ErrCode SwASCIIParser::CallParser()
{
    m_rInput.ResetError();
    m_nFileSize = m_rInput.TellEnd();
    m_rInput.Seek(STREAM_SEEK_TO_BEGIN);
    m_rInput.ResetError();

    ::StartProgress(STR_STATSTR_W4WREAD, 0, m_nFileSize, m_rDoc.GetDocShell());

    std::optional<SwPaM> pInsPam;
    sal_Int32 nSttContent = 0;
    if (!m_bNewDoc)
    {
        const SwNode& rTmp = m_oPam->GetPoint()->GetNode();
        pInsPam.emplace( rTmp, rTmp, SwNodeOffset(0), SwNodeOffset(-1) );
        nSttContent = m_oPam->GetPoint()->GetContentIndex();
    }

    SwTextFormatColl *pColl = nullptr;

    if (m_bNewDoc)
    {
        pColl = m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_HTML_PRE,
                                                                         false);
        if (!pColl)
            pColl = m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD,
                                                                             false);
        if (pColl)
            m_rDoc.SetTextFormatColl(*m_oPam, pColl);
    }

    ErrCode nError = ReadChars();

    if (m_oItemSet)
    {
        // set only the attribute, for scanned scripts.
        if (!(SvtScriptType::LATIN & m_nScript))
        {
            m_oItemSet->ClearItem(RES_CHRATR_FONT);
            m_oItemSet->ClearItem(RES_CHRATR_LANGUAGE);
        }
        if (!(SvtScriptType::ASIAN & m_nScript))
        {
            m_oItemSet->ClearItem(RES_CHRATR_CJK_FONT);
            m_oItemSet->ClearItem(RES_CHRATR_CJK_LANGUAGE);
        }
        if (!(SvtScriptType::COMPLEX & m_nScript))
        {
            m_oItemSet->ClearItem(RES_CHRATR_CTL_FONT);
            m_oItemSet->ClearItem(RES_CHRATR_CTL_LANGUAGE);
        }
        if (m_oItemSet->Count())
        {
            if (m_bNewDoc)
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
                        if (SfxItemState::SET
                            == m_oItemSet->GetItemState(*pWhichIds, false, &pItem))
                        {
                            pColl->SetFormatAttr( *pItem );
                            m_oItemSet->ClearItem(*pWhichIds);
                        }
                        ++pWhichIds;
                    }
                }
                if (m_oItemSet->Count())
                    m_rDoc.SetDefault(*m_oItemSet);
            }
            else if( pInsPam )
            {
                // then set over the insert range the defined attributes
                *pInsPam->GetMark() = *m_oPam->GetPoint();
                pInsPam->GetPoint()->Assign(pInsPam->GetPoint()->GetNode(), SwNodeOffset(1),
                                    nSttContent );

                // !!!!!
                OSL_ENSURE( false, "Have to change - hard attr. to para. style" );
                m_rDoc.getIDocumentContentOperations().InsertItemSet(*pInsPam, *m_oItemSet);
            }
        }
        m_oItemSet.reset();
    }

    pInsPam.reset();

    ::EndProgress(m_rDoc.GetDocShell());
    return nError;
}

ErrCode SwASCIIParser::ReadChars()
{
    sal_Unicode *pStart = nullptr, *pEnd = nullptr, *pLastStt = nullptr;
    tools::Long nReadCnt = 0, nLineLen = 0;
    sal_Unicode cLastCR = 0;
    bool bSwapUnicode = false;

    const SwAsciiOptions* pUseMe = &m_rOpt;
    SwAsciiOptions aEmpty;
    if (m_nFileSize >= 2 && aEmpty.GetFontName() == m_rOpt.GetFontName()
        && aEmpty.GetCharSet() == m_rOpt.GetCharSet()
        && aEmpty.GetLanguage() == m_rOpt.GetLanguage()
        && aEmpty.GetParaFlags() == m_rOpt.GetParaFlags())
    {
        sal_Size nLen, nOrig;
        nOrig = nLen = m_rInput.ReadBytes(m_pArr.get(), ASC_BUFFLEN);
        rtl_TextEncoding eCharSet;
        LineEnd eLineEnd;
        bool bHasBom;
        const bool bRet
            = SwIoSystem::IsDetectableText(m_pArr.get(), nLen, &eCharSet,
                                            &bSwapUnicode, &eLineEnd, &bHasBom);
        if (!bRet)
            return ERRCODE_IO_BROKENPACKAGE;

        OSL_ENSURE(bRet, "Autodetect of text import without nag dialog must have failed");
        if (bRet && eCharSet != RTL_TEXTENCODING_DONTKNOW)
        {
            aEmpty.SetCharSet(eCharSet);
            aEmpty.SetParaFlags(eLineEnd);
            aEmpty.SetIncludeBOM(bHasBom);
            m_rInput.SeekRel(-(tools::Long(nLen)));
        }
        else
            m_rInput.SeekRel(-(tools::Long(nOrig)));
        pUseMe=&aEmpty;
    }
    m_usedAsciiOptions = *pUseMe;

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
            return ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 0);
        bSwapUnicode = false;
        hContext = rtl_createTextToUnicodeContext( hConverter );
    }
    else if (pUseMe != &aEmpty)  //Already successfully figured out type
    {
        m_rInput.StartReadingUnicodeText(currentCharSet);
        bSwapUnicode = m_rInput.IsEndianSwap();
    }

    std::unique_ptr<sal_Unicode[]> aWork;
    sal_Size nArrOffset = 0;

    do {
        if( pStart >= pEnd )
        {
            if( pLastStt != pStart )
                InsertText( OUString( pLastStt ));

            // Read a new block
            sal_Size lGCount;
            if (ERRCODE_NONE != m_rInput.GetError()
                || 0
                       == (lGCount = m_rInput.ReadBytes(m_pArr.get() + nArrOffset,
                                                        ASC_BUFFLEN - nArrOffset)))
                break;      // break from the while loop

            /*
            If there was some unconverted bytes on the last cycle then they
            were put at the beginning of the array, so total bytes available
            to convert this cycle includes them. If we found 0 following bytes
            then we ignore the previous partial character.
            */
            lGCount += nArrOffset;

            if( hConverter )
            {
                sal_uInt32 nInfo;
                sal_Size nNewLen = lGCount, nCntBytes;
                aWork.reset(new sal_Unicode[nNewLen + 1]); // add 1 for '\0'
                sal_Unicode* pBuf = aWork.get();
                pBuf[nNewLen] = 0;                         // ensure '\0'

                nNewLen = rtl_convertTextToUnicode(hConverter, hContext, m_pArr.get(), lGCount,
                                                   pBuf, nNewLen,
                                                   (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT
                                                    | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT
                                                    | RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT
                                                    | RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE),
                                                   &nInfo, &nCntBytes);
                nArrOffset = lGCount - nCntBytes;
                if( 0 != nArrOffset )
                    memmove(m_pArr.get(), m_pArr.get() + nCntBytes, nArrOffset);

                pStart = pLastStt = aWork.get();
                pEnd = pStart + nNewLen;
            }
            else
            {
                pStart = pLastStt = reinterpret_cast<sal_Unicode*>(m_pArr.get());
                auto nChars = lGCount / 2;
                pEnd = pStart + nChars;

                if( bSwapUnicode )
                {
                    char *pF = m_pArr.get(), *pN = m_pArr.get() + 1;
                    for (sal_Size n = 0; n < nChars; ++n, pF += 2, pN += 2)
                    {
                        char c = *pF;
                        *pF = *pN;
                        *pN = c;
                    }
                }
            }

            *pEnd = 0;
            nReadCnt += lGCount;

            ::SetProgressState(nReadCnt, m_rDoc.GetDocShell());

            if( cLastCR )
            {
                if( 0x0a == *pStart && 0x0d == cLastCR )
                    pLastStt = ++pStart;
                cLastCR = 0;
                nLineLen = 0;
                // We skip the last one at the end
                if (!m_rInput.eof() || !(pEnd == pStart || (!*pEnd && pEnd == pStart + 1)))
                    m_rDoc.getIDocumentContentOperations().SplitNode(*m_oPam->GetPoint(), false);
            }
        }

        bool bIns = true, bSplitNode = false;
        switch( *pStart )
        {

        case 0x0a:  if( LINEEND_LF == pUseMe->GetParaFlags() )
                    {
                        bIns = false;
                        *pStart = 0;
                        ++pStart;

                        // We skip the last one at the end
                        if (!m_rInput.eof() || pEnd != pStart)
                            bSplitNode = true;
                    }
                    break;

        case 0x0d:  if( LINEEND_LF != pUseMe->GetParaFlags() )
                    {
                        bIns = false;
                        *pStart = 0;
                        ++pStart;

                        bool bChkSplit = true;
                        if( LINEEND_CRLF == pUseMe->GetParaFlags() )
                        {
                            if( pStart == pEnd )
                            {
                                cLastCR = 0x0d;
                                bChkSplit = false;
                            }
                            else if( 0x0a == *pStart )
                                ++pStart;
                        }

                        // We skip the last one at the end
                        if (bChkSplit && (!m_rInput.eof() || pEnd != pStart))
                            bSplitNode = true;
                    }
                    break;

        case 0x0c:
                    {
                        // Insert a hard page break
                        *pStart++ = 0;
                        if( nLineLen )
                        {
                            InsertText( OUString( pLastStt ));
                        }
                        m_rDoc.getIDocumentContentOperations().SplitNode(*m_oPam->GetPoint(),
                                                                         false);
                        m_rDoc.getIDocumentContentOperations().InsertPoolItem(
                            *m_oPam, SvxFormatBreakItem(SvxBreak::PageBefore, RES_BREAK));
                        pLastStt = pStart;
                        nLineLen = 0;
                        bIns = false;
                    }
                    break;

        case 0x1a:
            if (nReadCnt == m_nFileSize && pStart + 1 == pEnd)
                *pStart = 0;
            else
                *pStart = '#'; // Replacement visualisation
            break;

        case '\t':  break;

        default:
            if( ' ' > *pStart )
            // Found control char, replace with '#'
                *pStart = '#';
            break;
        }

        if( bIns )
        {
            if( ( nLineLen >= MAX_ASCII_PARA - 100 ) &&
                ( ( *pStart == ' ' ) || ( nLineLen >= MAX_ASCII_PARA - 1 ) ) )
            {
                sal_Unicode c = *pStart;
                *pStart = 0;
                InsertText( OUString( pLastStt ));
                m_rDoc.getIDocumentContentOperations().SplitNode(*m_oPam->GetPoint(), false);
                pLastStt = pStart;
                nLineLen = 0;
                *pStart = c;
            }
            ++pStart;
            ++nLineLen;
        }
        else if( bSplitNode )
        {
            // We found a CR/LF, thus save the text
            InsertText( OUString( pLastStt ));
            if (m_bNewDoc)
                m_rDoc.getIDocumentContentOperations().AppendTextNode(*m_oPam->GetPoint());
            else
                m_rDoc.getIDocumentContentOperations().SplitNode(*m_oPam->GetPoint(), false);
            pLastStt = pStart;
            nLineLen = 0;
        }
    } while(true);

    if( hConverter )
    {
        rtl_destroyTextToUnicodeContext( hConverter, hContext );
        rtl_destroyTextToUnicodeConverter( hConverter );
    }
    return ERRCODE_NONE;
}

void SwASCIIParser::InsertText( const OUString& rStr )
{
    m_rDoc.getIDocumentContentOperations().InsertString(*m_oPam, rStr);

    if (m_oItemSet && g_pBreakIt
        && m_nScript != (SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX))
        m_nScript |= g_pBreakIt->GetAllScriptsOfText(rStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
