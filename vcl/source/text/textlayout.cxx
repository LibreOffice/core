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

#include <osl/file.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

#include <textlayout.hxx>
#include <textlineinfo.hxx>

static bool lcl_IsCharIn(sal_Unicode c, const char* pStr)
{
    while ( *pStr )
    {
        if ( *pStr == c )
            return true;
        pStr++;
    }

    return false;
}

ImplMultiTextLineInfo::ImplMultiTextLineInfo()
{
}

ImplMultiTextLineInfo::~ImplMultiTextLineInfo()
{
}

void ImplMultiTextLineInfo::AddLine( const ImplTextLineInfo& rLine )
{
    mvLines.push_back(rLine);
}

void ImplMultiTextLineInfo::Clear()
{
    mvLines.clear();
}

namespace vcl
{
    TextLayoutCommon::~TextLayoutCommon() COVERITY_NOEXCEPT_FALSE
    {}

    OUString TextLayoutCommon::GetCenterEllipsisString(std::u16string_view rOrigStr, sal_Int32 nIndex, tools::Long nMaxWidth)
    {
        OUStringBuffer aTmpStr(rOrigStr);

        // speed it up by removing all but 1.33x as many as the break pos.
        sal_Int32 nEraseChars = std::max<sal_Int32>(4, rOrigStr.size() - (nIndex*4)/3);
        while(nEraseChars < static_cast<sal_Int32>(rOrigStr.size()) && GetTextWidth(aTmpStr.toString(), 0, aTmpStr.getLength()) > nMaxWidth)
        {
            aTmpStr = rOrigStr;
            sal_Int32 i = (aTmpStr.getLength() - nEraseChars)/2;
            aTmpStr.remove(i, nEraseChars++);
            aTmpStr.insert(i, "...");
        }

        return aTmpStr.makeStringAndClear();
    }

    OUString TextLayoutCommon::GetEndEllipsisString(OUString const& rOrigStr, sal_Int32 nIndex, tools::Long nMaxWidth, bool bClipText)
    {
        OUString aStr = rOrigStr;
        aStr = aStr.copy(0, nIndex);

        if (nIndex > 1)
        {
            aStr += "...";
            while (!aStr.isEmpty() && (GetTextWidth(aStr, 0, aStr.getLength()) > nMaxWidth))
            {
                if ((nIndex > 1) || (nIndex == aStr.getLength()))
                    nIndex--;

                aStr = aStr.replaceAt(nIndex, 1, u"");
            }
        }

        if (aStr.isEmpty() && bClipText)
            aStr += OUStringChar(rOrigStr[0]);

        return aStr;
    }

    namespace
    {
        OUString lcl_GetPathEllipsisString(OUString const& rOrigStr, sal_Int32 nIndex)
        {
            OUString aPath(rOrigStr);
            OUString aAbbreviatedPath;
            osl_abbreviateSystemPath(aPath.pData, &aAbbreviatedPath.pData, nIndex, nullptr);
            return aAbbreviatedPath;
        }
    }

    OUString TextLayoutCommon::GetNewsEllipsisString(OUString const& rOrigStr, tools::Long nMaxWidth, DrawTextFlags nStyle)
    {
        OUString aStr = rOrigStr;
        static char const pSepChars[] = ".";

        // Determine last section
        sal_Int32 nLastContent = aStr.getLength();
        while (nLastContent)
        {
            nLastContent--;

            if (lcl_IsCharIn(aStr[nLastContent], pSepChars))
                break;
        }

        while (nLastContent && lcl_IsCharIn(aStr[nLastContent-1], pSepChars))
        {
            nLastContent--;
        }

        OUString aLastStr = aStr.copy(nLastContent);
        OUString aTempLastStr1 = "..." + aLastStr;

        if (GetTextWidth(aTempLastStr1, 0, aTempLastStr1.getLength()) > nMaxWidth)
            return GetEllipsisString(aStr, nMaxWidth, DrawTextFlags::EndEllipsis);

        sal_Int32 nFirstContent = 0;
        while (nFirstContent < nLastContent)
        {
            nFirstContent++;
            if (lcl_IsCharIn(aStr[nFirstContent], pSepChars))
                break;
        }

        while ((nFirstContent < nLastContent) && lcl_IsCharIn(aStr[nFirstContent], pSepChars))
        {
            nFirstContent++;
        }

        if (nFirstContent >= nLastContent)
            return GetEllipsisString(aStr, nMaxWidth, nStyle | DrawTextFlags::EndEllipsis);

        if (nFirstContent > 4)
            nFirstContent = 4;

        OUString aFirstStr = OUString::Concat(aStr.subView(0, nFirstContent)) + "...";
        OUString aTempStr = aFirstStr + aLastStr;

        if (GetTextWidth(aTempStr, 0, aTempStr.getLength() ) > nMaxWidth)
            return GetEllipsisString(aStr, nMaxWidth, nStyle | DrawTextFlags::EndEllipsis);

        do
        {
            aStr = aTempStr;

            if (nLastContent > aStr.getLength())
                nLastContent = aStr.getLength();

            while (nFirstContent < nLastContent)
            {
                nLastContent--;
                if (lcl_IsCharIn(aStr[nLastContent], pSepChars))
                    break;

            }

            while ((nFirstContent < nLastContent) && lcl_IsCharIn(aStr[nLastContent-1], pSepChars))
            {
                nLastContent--;
            }

            if (nFirstContent < nLastContent)
            {
                std::u16string_view aTempLastStr = aStr.subView(nLastContent);
                aTempStr = aFirstStr + aTempLastStr;

                if (GetTextWidth(aTempStr, 0, aTempStr.getLength()) > nMaxWidth)
                    break;
            }
        }
        while (nFirstContent < nLastContent);

        return aStr;
    }

    OUString TextLayoutCommon::GetEllipsisString(OUString const& rOrigStr, tools::Long nMaxWidth, DrawTextFlags nStyle)
    {
        OUString aStr = rOrigStr;
        sal_Int32 nIndex = GetTextBreak( aStr, nMaxWidth, 0, aStr.getLength() );

        if (nIndex == -1)
            return aStr;

        if ((nStyle & DrawTextFlags::CenterEllipsis) == DrawTextFlags::CenterEllipsis)
            aStr = GetCenterEllipsisString(rOrigStr, nIndex, nMaxWidth);
        else if (nStyle & DrawTextFlags::EndEllipsis)
            aStr = GetEndEllipsisString(rOrigStr, nIndex, nMaxWidth, (nStyle & DrawTextFlags::Clip) == DrawTextFlags::Clip);
        else if (nStyle & DrawTextFlags::PathEllipsis)
            aStr = lcl_GetPathEllipsisString(rOrigStr, nIndex);
        else if ( nStyle & DrawTextFlags::NewsEllipsis )
            aStr = GetNewsEllipsisString(rOrigStr, nMaxWidth, nStyle);

        return aStr;
    }

    std::tuple<sal_Int32, sal_Int32> TextLayoutCommon::BreakLine(const tools::Long nWidth, OUString const& rStr,
                        css::uno::Reference< css::linguistic2::XHyphenator > const& xHyph,
                        css::uno::Reference<css::i18n::XBreakIterator>& xBI,
                        const bool bHyphenate, const tools::Long nOrigLineWidth,
                        const sal_Int32 nPos, const sal_Int32 nLen)
    {
        if (!xBI.is())
            xBI = vcl::unohelper::CreateBreakIterator();

        if (!xBI.is())
            return BreakLineSimple(nWidth, rStr, nPos, nLen, nOrigLineWidth);

        const css::lang::Locale& rDefLocale(Application::GetSettings().GetUILanguageTag().getLocale());

        sal_Int32 nSoftBreak = GetTextBreak(rStr, nWidth, nPos, nLen - nPos);
        if (nSoftBreak == -1)
            nSoftBreak = nPos;

        SAL_WARN_IF( nSoftBreak >= nLen, "vcl", "Break?!" );

        css::i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, css::uno::Sequence <css::beans::PropertyValue>(), 1 );
        css::i18n::LineBreakUserOptions aUserOptions;
        css::i18n::LineBreakResults aLBR = xBI->getLineBreak( rStr, nSoftBreak, rDefLocale, nPos, aHyphOptions, aUserOptions );

        sal_Int32 nBreakPos = aLBR.breakIndex;

        if (nBreakPos <= nPos)
            nBreakPos = nSoftBreak;

        if (!bHyphenate || !xHyph.is())
            return { nBreakPos, GetTextWidth(rStr, nPos, nBreakPos - nPos) };

        // Whether hyphen or not: Put the word after the hyphen through
        // the word boundary.

        // We run into a problem if the doc is so narrow, that a word
        // is broken into more than two lines ...

        css::i18n::Boundary aBoundary = xBI->getWordBoundary( rStr, nBreakPos, rDefLocale, css::i18n::WordType::DICTIONARY_WORD, true );
        sal_Int32 nWordStart = nPos;
        sal_Int32 nWordEnd = aBoundary.endPos;
        SAL_WARN_IF(nWordEnd <= nWordStart, "vcl", "Start >= End?");

        sal_Int32 nWordLen = nWordEnd - nWordStart;
        if ((nWordEnd < nSoftBreak) || (nWordLen <= 3))
            return { nBreakPos, GetTextWidth(rStr, nPos, nBreakPos - nPos) };

        OUString aWord = rStr.copy( nWordStart, nWordLen );
        sal_Int32 nMinTrail = nWordEnd-nSoftBreak+1;  //+1: Before the "broken off" char
        css::uno::Reference< css::linguistic2::XHyphenatedWord > xHyphWord;
        if (xHyph.is())
            xHyphWord = xHyph->hyphenate( aWord, rDefLocale, aWord.getLength() - nMinTrail, css::uno::Sequence< css::beans::PropertyValue >() );

        if (!xHyphWord.is())
            return { nBreakPos, GetTextWidth(rStr, nPos, nBreakPos - nPos) };

        bool bAlternate = xHyphWord->isAlternativeSpelling();
        sal_Int32 _nWordLen = 1 + xHyphWord->getHyphenPos();

        if ((_nWordLen < 2 ) || ( (nWordStart + _nWordLen) < 2))
            return { nBreakPos, GetTextWidth(rStr, nPos, nBreakPos - nPos) };

        if (bAlternate)
        {
            nBreakPos = nWordStart + _nWordLen;
            return { nBreakPos, GetTextWidth(rStr, nPos, nBreakPos - nPos) };
        }

        OUString aAlt( xHyphWord->getHyphenatedWord() );

        // We can have two cases:
        // 1) "packen" turns into "pak-ken"
        // 2) "Schiffahrt" turns into "Schiff-fahrt"

        // In case 1 we need to replace a char
        // In case 2 we add a char

        // Correct recognition is made harder by words such as
        // "Schiffahrtsbrennesseln", as the Hyphenator splits all
        // positions of the word and comes up with "Schifffahrtsbrennnesseln"
        // Thus, we cannot infer the aWord from the AlternativeWord's
        // index.
        // TODO: The whole junk will be made easier by a function in
        // the Hyphenator, as soon as AMA adds it.
        sal_Int32 nAltStart = _nWordLen - 1;
        sal_Int32 nTxtStart = nAltStart - (aAlt.getLength() - aWord.getLength());
        sal_Int32 nTxtEnd = nTxtStart;
        sal_Int32 nAltEnd = nAltStart;

        // The area between nStart and nEnd is the difference
        // between AlternativeString and OriginalString
        while( nTxtEnd < aWord.getLength() && nAltEnd < aAlt.getLength() &&
               aWord[nTxtEnd] != aAlt[nAltEnd] )
        {
            ++nTxtEnd;
            ++nAltEnd;
        }

        // If a char was added, we notice it now:
        if( nAltEnd > nTxtEnd && nAltStart == nAltEnd &&
            aWord[ nTxtEnd ] == aAlt[nAltEnd] )
        {
            ++nAltEnd;
            ++nTxtStart;
            ++nTxtEnd;
        }

        SAL_INFO_IF( ( nAltEnd - nAltStart ) != 1, "vcl", "Alternate: Wrong assumption!" );

        sal_Unicode cAlternateReplChar = 0;
        if ( nTxtEnd > nTxtStart )
            cAlternateReplChar = aAlt[ nAltStart ];

        nBreakPos = nWordStart + nTxtStart;
        if ( cAlternateReplChar )
            nBreakPos++;

        return { nBreakPos, GetTextWidth(rStr, nPos, nBreakPos - nPos) };
    }

    std::tuple<sal_Int32, sal_Int32> TextLayoutCommon::BreakLineSimple(const tools::Long nWidth, OUString const& rStr,
                                                 const sal_Int32 nPos, const sal_Int32 nLen, const tools::Long nOrigLineWidth)
    {
        sal_Int32 nBreakPos = nLen;
        tools::Long nLineWidth = nOrigLineWidth;
        sal_Int32 nSpacePos = rStr.getLength();
        tools::Long nW = 0;

        do
        {
            nSpacePos = rStr.lastIndexOf( ' ', nSpacePos );
            if( nSpacePos != -1 )
            {
                if( nSpacePos > nPos )
                    nSpacePos--;
                nW = GetTextWidth( rStr, nPos, nSpacePos-nPos );
            }
        } while( nW > nWidth );

        if( nSpacePos != -1 )
        {
            nBreakPos = nSpacePos;
            nLineWidth = GetTextWidth( rStr, nPos, nBreakPos-nPos );
            if( nBreakPos < rStr.getLength()-1 )
                nBreakPos++;
        }

        return { nBreakPos, nLineWidth };
    }

    namespace
    {
        bool lcl_ShouldBreakWord(const sal_Int32 nLineWidth, const sal_Int32 nWidth, const DrawTextFlags nStyle)
        {
            return ((nLineWidth > nWidth) && (nStyle & DrawTextFlags::WordBreak));
        }

        sal_Int32 lcl_GetEndOfLine(std::u16string_view rStr, const sal_Int32 nPos, const sal_Int32 nLen)
        {
            sal_Int32 nBreakPos = nPos;

            while ((nBreakPos < nLen) && (rStr[nBreakPos] != '\r') && (rStr[nBreakPos] != '\n'))
            {
                nBreakPos++;
            }

            return nBreakPos;
        }
    }

    tools::Long TextLayoutCommon::GetTextLines(tools::Rectangle const& rRect, const tools::Long nTextHeight,
                                               ImplMultiTextLineInfo& rLineInfo,
                                               tools::Long nWidth, OUString const& rStr,
                                               DrawTextFlags nStyle)
    {
        SAL_WARN_IF( nWidth <= 0, "vcl", "ImplGetTextLines: nWidth <= 0!" );

        if ( nWidth <= 0 )
            nWidth = 1;

        rLineInfo.Clear();
        if (rStr.isEmpty())
            return 0;

        tools::Long nMaxLineWidth  = 0;
        const bool bHyphenate = (nStyle & DrawTextFlags::WordBreakHyphenation) == DrawTextFlags::WordBreakHyphenation;
        css::uno::Reference< css::linguistic2::XHyphenator > xHyph;
        if (bHyphenate)
        {
            // get service provider
            css::uno::Reference<css::uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
            css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLinguMgr = css::linguistic2::LinguServiceManager::create(xContext);
            xHyph = xLinguMgr->getHyphenator();
        }

        css::uno::Reference<css::i18n::XBreakIterator> xBI;
        sal_Int32 nPos = 0;
        sal_Int32 nLen = rStr.getLength();
        sal_Int32 nCurrentTextY = 0;

        while ( nPos < nLen )
        {
            sal_Int32 nBreakPos = lcl_GetEndOfLine(rStr, nPos, nLen);
            tools::Long nLineWidth = GetTextWidth(rStr, nPos, nBreakPos-nPos);


            if (lcl_ShouldBreakWord(nLineWidth, nWidth, nStyle))
                std::tie(nBreakPos, nLineWidth) = BreakLine(nWidth, rStr, xHyph, xBI, bHyphenate, nLineWidth, nPos, nBreakPos);

            if ( nLineWidth > nMaxLineWidth )
                nMaxLineWidth = nLineWidth;

            rLineInfo.AddLine( ImplTextLineInfo( nLineWidth, nPos, nBreakPos-nPos ) );

            if ( nBreakPos == nPos )
                nBreakPos++;

            nPos = nBreakPos;

            if ( nPos < nLen && ( ( rStr[ nPos ] == '\r' ) || ( rStr[ nPos ] == '\n' ) ) )
            {
                nPos++;
                // CR/LF?
                if ( ( nPos < nLen ) && ( rStr[ nPos ] == '\n' ) && ( rStr[ nPos-1 ] == '\r' ) )
                    nPos++;
            }
            nCurrentTextY += nTextHeight;

            const bool bClipping = (nStyle & DrawTextFlags::Clip) && !(nStyle & DrawTextFlags::EndEllipsis);

            if (bClipping && nCurrentTextY > rRect.GetHeight())
                break;
        }

#ifdef DBG_UTIL
        for ( sal_Int32 nL = 0; nL < rLineInfo.Count(); nL++ )
        {
            ImplTextLineInfo& rLine = rLineInfo.GetLine( nL );
            OUString aLine = rStr.copy( rLine.GetIndex(), rLine.GetLen() );
            SAL_WARN_IF( aLine.indexOf( '\r' ) != -1, "vcl", "ImplGetTextLines - Found CR!" );
            SAL_WARN_IF( aLine.indexOf( '\n' ) != -1, "vcl", "ImplGetTextLines - Found LF!" );
        }
#endif

        return nMaxLineWidth;
    }

    DefaultTextLayout::~DefaultTextLayout()
    {
    }

    tools::Long DefaultTextLayout::GetTextWidth( const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        return m_rTargetDevice.GetTextWidth( _rText, _nStartIndex, _nLength );
    }

    void DefaultTextLayout::DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex,
        sal_Int32 _nLength, std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText )
    {
        m_rTargetDevice.DrawText( _rStartPoint, _rText, _nStartIndex, _nLength, _pVector, _pDisplayText );
    }

    tools::Long DefaultTextLayout::GetTextArray( const OUString& _rText, KernArray* _pDXArray,
        sal_Int32 _nStartIndex, sal_Int32 _nLength, bool bCaret ) const
    {
        return basegfx::fround<tools::Long>(m_rTargetDevice.GetTextArray( _rText, _pDXArray, _nStartIndex, _nLength, bCaret ));
    }

    sal_Int32 DefaultTextLayout::GetTextBreak( const OUString& _rText, tools::Long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        return m_rTargetDevice.GetTextBreak( _rText, _nMaxTextWidth, _nStartIndex, _nLength );
    }

    bool DefaultTextLayout::DecomposeTextRectAction() const
    {
        return false;
    }

    class ReferenceDeviceTextLayout : public TextLayoutCommon
    {
    public:
        ReferenceDeviceTextLayout( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice );
        virtual ~ReferenceDeviceTextLayout();

        // TextLayoutCommon
        virtual tools::Long        GetTextWidth( const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen ) const override;
        virtual void        DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength, std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText ) override;
        virtual tools::Long GetTextArray( const OUString& _rText, KernArray* _pDXAry, sal_Int32 _nStartIndex, sal_Int32 _nLength, bool bCaret = false ) const override;
        virtual sal_Int32   GetTextBreak(const OUString& _rText, tools::Long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength) const override;
        virtual bool        DecomposeTextRectAction() const override;

    public:
        // equivalents to the respective OutputDevice methods, which take the reference device into account
        tools::Rectangle   DrawText( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText, const Size* i_pDeviceSize );
        tools::Rectangle   GetTextRect( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, Size* o_pDeviceSize );

    private:

        OutputDevice&   m_rTargetDevice;
        OutputDevice&   m_rReferenceDevice;
        const bool      m_bRTLEnabled;

        tools::Rectangle       m_aCompleteTextRect;
    };

    ReferenceDeviceTextLayout::ReferenceDeviceTextLayout( const Control& _rControl, OutputDevice& _rTargetDevice,
        OutputDevice& _rReferenceDevice )
        :m_rTargetDevice( _rTargetDevice )
        ,m_rReferenceDevice( _rReferenceDevice )
        ,m_bRTLEnabled( _rControl.IsRTLEnabled() )
    {
        Font const aUnzoomedPointFont( _rControl.GetUnzoomedControlPointFont() );
        const Fraction& aZoom( _rControl.GetZoom() );
        m_rTargetDevice.Push( PushFlags::MAPMODE | PushFlags::FONT | PushFlags::TEXTLAYOUTMODE );

        MapMode aTargetMapMode( m_rTargetDevice.GetMapMode() );
        SAL_WARN_IF(aTargetMapMode.GetOrigin() != Point(), "vcl", "uhm, the code below won't work here ...");

        // normally, controls simulate "zoom" by "zooming" the font. This is responsible for (part of) the discrepancies
        // between text in Writer and text in controls in Writer, though both have the same font.
        // So, if we have a zoom set at the control, then we do not scale the font, but instead modify the map mode
        // to accommodate for the zoom.
        aTargetMapMode.SetScaleX( aZoom );    // TODO: shouldn't this be "current_scale * zoom"?
        aTargetMapMode.SetScaleY( aZoom );

        // also, use a higher-resolution map unit than "pixels", which should save us some rounding errors when
        // translating coordinates between the reference device and the target device.
        SAL_WARN_IF(aTargetMapMode.GetMapUnit() != MapUnit::MapPixel, "vcl", "this class is not expected to work with such target devices!");
            // we *could* adjust all the code in this class to handle this case, but at the moment, it's not necessary
        const MapUnit eTargetMapUnit = m_rReferenceDevice.GetMapMode().GetMapUnit();
        aTargetMapMode.SetMapUnit( eTargetMapUnit );
        SAL_WARN_IF(aTargetMapMode.GetMapUnit() == MapUnit::MapPixel, "vcl", "a reference device which has map mode PIXEL?!");

        m_rTargetDevice.SetMapMode( aTargetMapMode );

        // now that the Zoom is part of the map mode, reset the target device's font to the "unzoomed" version
        Font aDrawFont( aUnzoomedPointFont );
        aDrawFont.SetFontSize( OutputDevice::LogicToLogic(aDrawFont.GetFontSize(), MapMode(MapUnit::MapPoint), MapMode(eTargetMapUnit)) );
        _rTargetDevice.SetFont( aDrawFont );

        // transfer font to the reference device
        m_rReferenceDevice.Push( PushFlags::FONT | PushFlags::TEXTLAYOUTMODE );
        Font aRefFont( aUnzoomedPointFont );
        aRefFont.SetFontSize( OutputDevice::LogicToLogic(
            aRefFont.GetFontSize(), MapMode(MapUnit::MapPoint), m_rReferenceDevice.GetMapMode()) );
        m_rReferenceDevice.SetFont( aRefFont );
    }

    ReferenceDeviceTextLayout::~ReferenceDeviceTextLayout()
    {
        m_rReferenceDevice.Pop();
        m_rTargetDevice.Pop();
    }

    namespace
    {
        bool lcl_normalizeLength( std::u16string_view _rText, const sal_Int32 _nStartIndex, sal_Int32& _io_nLength )
        {
            sal_Int32 nTextLength = _rText.size();
            if ( _nStartIndex > nTextLength )
                return false;
            if ( _nStartIndex + _io_nLength > nTextLength )
                _io_nLength = nTextLength - _nStartIndex;
            return true;
        }
    }

    tools::Long ReferenceDeviceTextLayout::GetTextArray( const OUString& _rText, KernArray* _pDXAry, sal_Int32 _nStartIndex, sal_Int32 _nLength, bool bCaret ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        // retrieve the character widths from the reference device
        tools::Long nTextWidth = basegfx::fround<tools::Long>(m_rReferenceDevice.GetTextArray( _rText, _pDXAry, _nStartIndex, _nLength, bCaret ));
#if OSL_DEBUG_LEVEL > 1
        if ( _pDXAry )
        {
            OStringBuffer aTrace;
            aTrace.append( "ReferenceDeviceTextLayout::GetTextArray( " );
            aTrace.append( OUStringToOString( _rText, RTL_TEXTENCODING_UTF8 ) );
            aTrace.append( " ): " );
            aTrace.append( nTextWidth );
            aTrace.append( " = ( " );
            for ( sal_Int32 i=0; i<_nLength; )
            {
                aTrace.append( _pDXAry->get(i) );
                if ( ++i < _nLength )
                    aTrace.append( ", " );
            }
            aTrace.append( ")" );
            SAL_INFO( "vcl", aTrace.makeStringAndClear() );
        }
#endif
        return nTextWidth;
    }

    tools::Long ReferenceDeviceTextLayout::GetTextWidth( const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        return GetTextArray( _rText, nullptr, _nStartIndex, _nLength );
    }

    void ReferenceDeviceTextLayout::DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength, std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText )
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return;

        if ( _pVector && _pDisplayText )
        {
            std::vector< tools::Rectangle > aGlyphBounds;
            m_rReferenceDevice.GetGlyphBoundRects( _rStartPoint, _rText, _nStartIndex, _nLength, aGlyphBounds );
            _pVector->insert( _pVector->end(), aGlyphBounds.begin(), aGlyphBounds.end() );
            *_pDisplayText += _rText.subView( _nStartIndex, _nLength );
            return;
        }

        KernArray aCharWidths;
        tools::Long nTextWidth = GetTextArray( _rText, &aCharWidths, _nStartIndex, _nLength );
        m_rTargetDevice.DrawTextArray( _rStartPoint, _rText, aCharWidths, {}, _nStartIndex, _nLength );

        m_aCompleteTextRect.Union( tools::Rectangle( _rStartPoint, Size( nTextWidth, m_rTargetDevice.GetTextHeight() ) ) );
    }

    sal_Int32 ReferenceDeviceTextLayout::GetTextBreak( const OUString& _rText, tools::Long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const
    {
        if ( !lcl_normalizeLength( _rText, _nStartIndex, _nLength ) )
            return 0;

        return m_rReferenceDevice.GetTextBreak( _rText, _nMaxTextWidth, _nStartIndex, _nLength );
    }

    bool ReferenceDeviceTextLayout::DecomposeTextRectAction() const
    {
        return true;
    }

    tools::Rectangle ReferenceDeviceTextLayout::DrawText( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle,
                                                   std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText, const Size* i_pDeviceSize )
    {
        if ( _rText.isEmpty() )
            return tools::Rectangle();

        // determine text layout mode from the RTL-ness of the control whose text we render
        text::ComplexTextLayoutFlags nTextLayoutMode = m_bRTLEnabled ? text::ComplexTextLayoutFlags::BiDiRtl : text::ComplexTextLayoutFlags::Default;
        m_rReferenceDevice.SetLayoutMode( nTextLayoutMode );
        m_rTargetDevice.SetLayoutMode( nTextLayoutMode | text::ComplexTextLayoutFlags::TextOriginLeft );

        // ComplexTextLayoutFlags::TextOriginLeft is because when we do actually draw the text (in DrawText( Point, ... )), then
        // our caller gives us the left border of the draw position, regardless of script type, text layout,
        // and the like in our ctor, we set the map mode of the target device from pixel to twip, but our caller doesn't know this,
        // but passed pixel coordinates. So, adjust the rect.
        tools::Rectangle aRect( m_rTargetDevice.PixelToLogic( _rRect ) );
        if (i_pDeviceSize)
        {
            //if i_pDeviceSize is passed in here, it was the original pre logic-to-pixel size of _rRect
            SAL_WARN_IF(std::abs(_rRect.GetSize().Width() - m_rTargetDevice.LogicToPixel(*i_pDeviceSize).Width()) > 1, "vcl", "DeviceSize width was expected to match Pixel width");
            SAL_WARN_IF(std::abs(_rRect.GetSize().Height() - m_rTargetDevice.LogicToPixel(*i_pDeviceSize).Height()) > 1, "vcl", "DeviceSize height was expected to match Pixel height");
            aRect.SetSize(*i_pDeviceSize);
        }

        m_aCompleteTextRect.SetEmpty();
        m_rTargetDevice.DrawText( aRect, _rText, _nStyle, _pVector, _pDisplayText, this );
        tools::Rectangle aTextRect = m_aCompleteTextRect;

        if ( aTextRect.IsEmpty() && !aRect.IsEmpty() )
        {
            // this happens for instance if we're in a PaintToDevice call, where only a MetaFile is recorded,
            // but no actual painting happens, so our "DrawText( Point, ... )" is never called
            // In this case, calculate the rect from what OutputDevice::GetTextRect would give us. This has
            // the disadvantage of less accuracy, compared with the approach to calculate the rect from the
            // single "DrawText( Point, ... )" calls, since more intermediate arithmetic will translate
            // from ref- to target-units.
            aTextRect = m_rTargetDevice.GetTextRect( aRect, _rText, _nStyle, nullptr, this );
        }

        // similar to above, the text rect now contains TWIPs (or whatever unit the ref device has), but the caller
        // expects pixel coordinates
        aTextRect = m_rTargetDevice.LogicToPixel( aTextRect );

        // convert the metric vector
        if ( _pVector )
        {
            for ( auto& rCharRect : *_pVector )
            {
                rCharRect = m_rTargetDevice.LogicToPixel( rCharRect );
            }
        }

        return aTextRect;
    }

    tools::Rectangle ReferenceDeviceTextLayout::GetTextRect( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, Size* o_pDeviceSize )
    {
        if ( _rText.isEmpty() )
            return tools::Rectangle();

        // determine text layout mode from the RTL-ness of the control whose text we render
        text::ComplexTextLayoutFlags nTextLayoutMode = m_bRTLEnabled ? text::ComplexTextLayoutFlags::BiDiRtl : text::ComplexTextLayoutFlags::Default;
        m_rReferenceDevice.SetLayoutMode( nTextLayoutMode );
        m_rTargetDevice.SetLayoutMode( nTextLayoutMode | text::ComplexTextLayoutFlags::TextOriginLeft );

        // ComplexTextLayoutFlags::TextOriginLeft is because when we do actually draw the text (in DrawText( Point, ... )), then
        // our caller gives us the left border of the draw position, regardless of script type, text layout,
        // and the like in our ctor, we set the map mode of the target device from pixel to twip, but our caller doesn't know this,
        // but passed pixel coordinates. So, adjust the rect.
        tools::Rectangle aRect( m_rTargetDevice.PixelToLogic( _rRect ) );

        tools::Rectangle aTextRect = m_rTargetDevice.GetTextRect( aRect, _rText, _nStyle, nullptr, this );

        //if o_pDeviceSize is available, stash the pre logic-to-pixel size in it
        if (o_pDeviceSize)
        {
            *o_pDeviceSize = aTextRect.GetSize();
        }

        // similar to above, the text rect now contains TWIPs (or whatever unit the ref device has), but the caller
        // expects pixel coordinates
        aTextRect = m_rTargetDevice.LogicToPixel( aTextRect );

        return aTextRect;
    }

    ControlTextRenderer::ControlTextRenderer( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice )
        :m_pImpl( new ReferenceDeviceTextLayout( _rControl, _rTargetDevice, _rReferenceDevice ) )
    {
    }

    ControlTextRenderer::~ControlTextRenderer()
    {
    }

    tools::Rectangle ControlTextRenderer::DrawText( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle,
        std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText, const Size* i_pDeviceSize )
    {
        return m_pImpl->DrawText( _rRect, _rText, _nStyle, _pVector, _pDisplayText, i_pDeviceSize );
    }

    tools::Rectangle ControlTextRenderer::GetTextRect( const tools::Rectangle& _rRect, const OUString& _rText, DrawTextFlags _nStyle, Size* o_pDeviceSize = nullptr )
    {
        return m_pImpl->GetTextRect( _rRect, _rText, _nStyle, o_pDeviceSize );
    }

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
