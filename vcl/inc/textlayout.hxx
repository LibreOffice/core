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

#pragma once

#include <tools/long.hxx>
#include <vcl/outdev.hxx>

#include <memory>
#include <vector>

#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>

class Control;

namespace vcl
{
    class VCL_DLLPUBLIC TextLayoutCommon
    {
    public:
        virtual tools::Long        GetTextWidth( const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const = 0;
        virtual void        DrawText( const Point& _rStartPoint, const OUString& _rText, sal_Int32 _nStartIndex, sal_Int32 _nLength,
                                std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText ) = 0;
        virtual tools::Long GetTextArray( const OUString& _rText, KernArray* _pDXArray, sal_Int32 _nStartIndex, sal_Int32 _nLength, bool bCaret = false ) const = 0;
        virtual sal_Int32   GetTextBreak( const OUString& _rText, tools::Long _nMaxTextWidth, sal_Int32 _nStartIndex, sal_Int32 _nLength ) const = 0;
        virtual bool        DecomposeTextRectAction() const = 0;

        SAL_DLLPRIVATE OUString GetEllipsisString(OUString const& rOrigStr, tools::Long nMaxWidth, DrawTextFlags nStyle);

        std::tuple<sal_Int32, sal_Int32> BreakLine(const tools::Long nWidth, OUString const& rStr,
                        css::uno::Reference< css::linguistic2::XHyphenator > const& xHyph,
                        css::uno::Reference<css::i18n::XBreakIterator>& xBI,
                        const bool bHyphenate, const tools::Long nOrigLineWidth,
                        const sal_Int32 nPos, const sal_Int32 nLen);

        SAL_DLLPRIVATE std::tuple<sal_Int32, sal_Int32> BreakLineSimple(const tools::Long nWidth, OUString const& rStr,
                                  const sal_Int32 nPos, sal_Int32 nBreakPos, const tools::Long nOrigLineWidth);

        SAL_DLLPRIVATE tools::Long GetTextLines(tools::Rectangle const& rRect, const tools::Long nTextHeight,
                                 ImplMultiTextLineInfo& rLineInfo,
                                 tools::Long nWidth, OUString const& rStr,
                                 DrawTextFlags nStyle);

    protected:
        SAL_DLLPRIVATE ~TextLayoutCommon() COVERITY_NOEXCEPT_FALSE;

    private:
        SAL_DLLPRIVATE OUString GetCenterEllipsisString(std::u16string_view rOrigStr, sal_Int32 nIndex, tools::Long nMaxWidth);
        SAL_DLLPRIVATE OUString GetEndEllipsisString(OUString const& rOrigStr, sal_Int32 nIndex, tools::Long nMaxWidth, bool bClipText);
        SAL_DLLPRIVATE OUString GetNewsEllipsisString(OUString const& rOrigStr, tools::Long nMaxWidth, DrawTextFlags nStyle);
    };

    /** is an implementation of TextLayoutCommon which simply delegates its calls to the respective
        methods of an OutputDevice instance, without any inbetween magic.
    */
    class VCL_DLLPUBLIC DefaultTextLayout final : public TextLayoutCommon
    {
    public:
        DefaultTextLayout( OutputDevice& _rTargetDevice )
            : m_rTargetDevice( _rTargetDevice )
        {
        }
        virtual ~DefaultTextLayout();

        // TextLayoutCommon overridables
        SAL_DLLPRIVATE virtual tools::Long GetTextWidth( const OUString& _rText,
                                          sal_Int32 _nStartIndex,
                                          sal_Int32 _nLength ) const override;

        virtual void        DrawText( const Point& _rStartPoint,
                                      const OUString& _rText,
                                      sal_Int32 _nStartIndex,
                                      sal_Int32 _nLength,
                                      std::vector< tools::Rectangle >* _pVector,
                                      OUString* _pDisplayText ) override;

        SAL_DLLPRIVATE virtual tools::Long GetTextArray( const OUString& _rText,
                                          KernArray* _pDXArray,
                                          sal_Int32 _nStartIndex,
                                          sal_Int32 _nLength,
                                          bool bCaret = false ) const override;

        SAL_DLLPRIVATE virtual sal_Int32 GetTextBreak( const OUString& _rText,
                                          tools::Long _nMaxTextWidth,
                                          sal_Int32 _nStartIndex,
                                          sal_Int32 _nLength ) const override;

        SAL_DLLPRIVATE virtual bool DecomposeTextRectAction() const override;

    private:
        OutputDevice&   m_rTargetDevice;
    };

    class ReferenceDeviceTextLayout;
    /** a class which allows rendering text of a Control onto a device, by taking into account the metrics of
        a reference device.
    */
    class ControlTextRenderer final
    {
    public:
        ControlTextRenderer( const Control& _rControl, OutputDevice& _rTargetDevice, OutputDevice& _rReferenceDevice );
        ~ControlTextRenderer();

        tools::Rectangle DrawText( const tools::Rectangle& _rRect,
                              const OUString& _rText, DrawTextFlags _nStyle,
                              std::vector< tools::Rectangle >* _pVector, OUString* _pDisplayText, const Size* i_pDeviceSize );

        tools::Rectangle   GetTextRect( const tools::Rectangle& _rRect,
                                 const OUString& _rText, DrawTextFlags _nStyle, Size* o_pDeviceSize );

    private:
        ControlTextRenderer( const ControlTextRenderer& ) = delete;
        ControlTextRenderer& operator=( const ControlTextRenderer& ) = delete;

    private:
        ::std::unique_ptr< ReferenceDeviceTextLayout >   m_pImpl;
    };

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
