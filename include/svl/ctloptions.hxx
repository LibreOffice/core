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
#ifndef INCLUDED_SVL_CTLOPTIONS_HXX
#define INCLUDED_SVL_CTLOPTIONS_HXX

#include <svl/svldllapi.h>
#include <unotools/options.hxx>
#include <memory>

class SvtCTLOptions_Impl;

// class SvtCTLOptions --------------------------------------------------------

class SVL_DLLPUBLIC SvtCTLOptions final : public utl::detail::Options
{
private:
    std::shared_ptr<SvtCTLOptions_Impl>    m_pImpl;

public:

    // bDontLoad is for referencing purposes only
    SvtCTLOptions( bool bDontLoad = false );
    virtual ~SvtCTLOptions() override;

    void        SetCTLFontEnabled( bool _bEnabled );
    static bool IsCTLFontEnabled();

    void SetCTLVerticalText(bool bVertical);
    static bool IsCTLVerticalText();

    void        SetCTLSequenceChecking( bool _bEnabled );
    static bool IsCTLSequenceChecking();

    void        SetCTLSequenceCheckingRestricted( bool _bEnable );
    static bool IsCTLSequenceCheckingRestricted();

    void        SetCTLSequenceCheckingTypeAndReplace( bool _bEnable );
    static bool IsCTLSequenceCheckingTypeAndReplace();

    enum CursorMovement
    {
        MOVEMENT_LOGICAL = 0,
        MOVEMENT_VISUAL
    };
    void            SetCTLCursorMovement( CursorMovement _eMovement );
    static CursorMovement GetCTLCursorMovement();

    enum TextNumerals
    {
        NUMERALS_ARABIC = 0,
        NUMERALS_HINDI,
        NUMERALS_SYSTEM,
        NUMERALS_CONTEXT
    };
    void            SetCTLTextNumerals( TextNumerals _eNumerals );
    static TextNumerals GetCTLTextNumerals();

    enum EOption
    {
        E_CTLFONT,
        E_CTLSEQUENCECHECKING,
        E_CTLCURSORMOVEMENT,
        E_CTLTEXTNUMERALS,
        E_CTLSEQUENCECHECKINGRESTRICTED,
        E_CTLSEQUENCECHECKINGTYPEANDREPLACE,
        E_CTLVERTICALTEXT
    };
    bool IsReadOnly(EOption eOption) const;
};

#endif // INCLUDED_SVL_CTLOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
