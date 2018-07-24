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
#ifndef INCLUDED_SW_SOURCE_UIBASE_LINGU_SDRHHCWRAP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_LINGU_SDRHHCWRAP_HXX

#include <svx/svdoutl.hxx>

class SwView;
class SdrTextObj;
class OutlinerView;

class SdrHHCWrapper : public SdrOutliner
{
    // modified version of SdrSpeller

    SwView*             pView;
    SdrTextObj*         pTextObj;
    std::unique_ptr<OutlinerView> pOutlView;
    sal_Int32           nOptions;
    sal_uInt16          nDocIndex;
    LanguageType        nSourceLang;
    LanguageType        nTargetLang;
    const vcl::Font*    pTargetFont;
    bool                bIsInteractive;

public:
    SdrHHCWrapper( SwView* pVw,
                   LanguageType nSourceLanguage, LanguageType nTargetLanguage,
                   const vcl::Font* pTargetFnt,
                   sal_Int32 nConvOptions, bool bInteractive );

    virtual ~SdrHHCWrapper() override;

    virtual bool ConvertNextDocument() override;
    void    StartTextConversion();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
