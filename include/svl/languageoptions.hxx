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
#ifndef INCLUDED_SVL_LANGUAGEOPTIONS_HXX
#define INCLUDED_SVL_LANGUAGEOPTIONS_HXX

#include <svl/svldllapi.h>
#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>
#include <unotools/options.hxx>
#include <i18nlangtag/lang.h>
#include <memory>

// class SvtLanguageOptions ----------------------------------------------------

// these defines can be ORed
// note these values DO NOT match the values in css::i18n::ScriptType
enum class SvtScriptType
{
    NONE         = 0x00,
    LATIN        = 0x01,
    ASIAN        = 0x02,
    COMPLEX      = 0x04,
    UNKNOWN      = 0x08 // (only used in SC) if type has not been determined yet
};
namespace o3tl
{
    template<> struct typed_flags<SvtScriptType> : is_typed_flags<SvtScriptType, 0x0f> {};
}

namespace SvtLanguageOptions
{
    // CJK options
//    SVL_DLLPUBLIC bool    IsCJKFontEnabled() { m_pCJKOptions->IsCJKFontEnabled(); }
//    SVL_DLLPUBLIC bool    IsVerticalTextEnabled() m_pCJKOptions->IsVerticalTextEnabled();
//    SVL_DLLPUBLIC bool    IsAsianTypographyEnabled() m_pCJKOptions->IsAsianTypographyEnabled();
//    SVL_DLLPUBLIC bool    IsJapaneseFindEnabled() m_pCJKOptions->IsJapaneseFindEnabled();
//    SVL_DLLPUBLIC void    SetAll( bool _bSet ) m_pCJKOptions->SetAll( _bSet );
//    SVL_DLLPUBLIC bool    IsAnyEnabled() m_pCJKOptions->IsAnyEnabled();

    // CTL options
    //SVL_DLLPUBLIC void    SetCTLFontEnabled( bool _bEnabled ) m_pCTLOptions->SetCTLFontEnabled( _bEnabled );
//    SVL_DLLPUBLIC bool    IsCTLFontEnabled() m_pCTLOptions->IsCTLFontEnabled();

//    SVL_DLLPUBLIC void    SetCTLSequenceChecking( bool _bEnabled ) m_pCTLOptions->SetCTLSequenceChecking( _bEnabled );

    //SVL_DLLPUBLIC void    SetCTLSequenceCheckingRestricted( bool _bEnable ) m_pCTLOptions->SetCTLSequenceCheckingRestricted( _bEnable );

//    SVL_DLLPUBLIC void    SetCTLSequenceCheckingTypeAndReplace( bool _bEnable ) m_pCTLOptions->SetCTLSequenceCheckingTypeAndReplace( _bEnable );

    // returns for a language the scripttype
    SVL_DLLPUBLIC SvtScriptType GetScriptTypeOfLanguage( LanguageType nLang );

    // convert from css::i18n::ScriptType constants to SvtScriptType
    SVL_DLLPUBLIC SvtScriptType FromI18NToSvtScriptType( sal_Int16 nI18NType );

    SVL_DLLPUBLIC sal_Int16 FromSvtScriptTypeToI18N( SvtScriptType nI18NType );

    SVL_DLLPUBLIC sal_Int16 GetI18NScriptTypeOfLanguage( LanguageType nLang );

};

/** #i42730# Gives access to the Windows 16bit system locale
 */
namespace SvtSystemLanguageOptions
{
    bool            isCJKKeyboardLayoutInstalled();
};

#endif // INCLUDED_SVL_LANGUAGEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
