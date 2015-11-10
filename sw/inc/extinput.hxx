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
#ifndef INCLUDED_SW_INC_EXTINPUT_HXX
#define INCLUDED_SW_INC_EXTINPUT_HXX

#include <pam.hxx>
#include <i18nlangtag/lang.h>
#include <vector>

class CommandExtTextInputData;

class SwExtTextInput : public SwPaM
{
    std::vector<sal_uInt16> aAttrs;
    OUString sOverwriteText;
    bool bInsText : 1;
    bool bIsOverwriteCursor : 1;
    LanguageType eInputLanguage;
public:
    SwExtTextInput( const SwPaM& rPam, Ring* pRing = nullptr );
    virtual ~SwExtTextInput();

    void SetInputData( const CommandExtTextInputData& rData );
    const std::vector<sal_uInt16>& GetAttrs() const { return aAttrs; }
    void SetInsText( bool bFlag )       { bInsText = bFlag; }
    bool IsOverwriteCursor() const      { return bIsOverwriteCursor; }
    void SetOverwriteCursor( bool bFlag );
    void SetLanguage(LanguageType eSet) { eInputLanguage = eSet;}
};

#endif // INCLUDED_SW_INC_EXTINPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
