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
#ifndef EXTINPUT_HXX
#define EXTINPUT_HXX

#include <pam.hxx>
#include <i18nlangtag/lang.h>
#include <vector>

class CommandExtTextInputData;

class SwExtTextInput : public SwPaM
{
    std::vector<sal_uInt16> aAttrs;
    OUString sOverwriteText;
    sal_Bool bInsText : 1;
    sal_Bool bIsOverwriteCursor : 1;
    LanguageType eInputLanguage;
public:
    SwExtTextInput( const SwPaM& rPam, Ring* pRing = 0 );
    virtual ~SwExtTextInput();

    void SetInputData( const CommandExtTextInputData& rData );
    const std::vector<sal_uInt16>& GetAttrs() const { return aAttrs; }
    void SetInsText( sal_Bool bFlag )       { bInsText = bFlag; }
    sal_Bool IsOverwriteCursor() const      { return bIsOverwriteCursor; }
    void SetOverwriteCursor( sal_Bool bFlag );
    void SetLanguage(LanguageType eSet) { eInputLanguage = eSet;}
};

#endif  //EXTINPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
