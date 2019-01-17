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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORFTN_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORFTN_HXX

#include "porfld.hxx"

class SwTextFrame;
class SwTextFootnote;

class SwFootnotePortion : public SwFieldPortion
{
    SwTextFootnote * const pFootnote;
    sal_uInt16 nOrigHeight;
    // #i98418#
    bool mbPreferredScriptTypeSet;
    SwFontScript mnPreferredScriptType;
public:
    SwFootnotePortion( const OUString &rExpand, SwTextFootnote *pFootnote,
                  sal_uInt16 nOrig = USHRT_MAX );
    sal_uInt16& Orig() { return nOrigHeight; }

    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // #i98418#
    void SetPreferredScriptType( SwFontScript nPreferredScriptType );

    const SwTextFootnote* GetTextFootnote() const { return pFootnote; };
};

class SwFootnoteNumPortion : public SwNumberPortion
{
public:
    SwFootnoteNumPortion( const OUString &rExpand, std::unique_ptr<SwFont> pFntL )
         : SwNumberPortion( rExpand, std::move(pFntL), true, false, 0, false )
         { SetWhichPor( PortionType::FootnoteNum ); }
};

class SwQuoVadisPortion : public SwFieldPortion
{
    OUString   aErgo;
public:
    SwQuoVadisPortion( const OUString &rExp, const OUString& rStr );
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;

    void SetNumber( const OUString& rStr ) { aErgo = rStr; }
    const OUString& GetQuoText() const { return m_aExpand; }
    const OUString &GetContText() const { return aErgo; }

    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwErgoSumPortion : public SwFieldPortion
{
public:
    SwErgoSumPortion( const OUString &rExp, const OUString& rStr );
    virtual TextFrameIndex GetCursorOfst(sal_uInt16 nOfst) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
