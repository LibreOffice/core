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

#include <svl/style.hxx>
#include "scdllapi.h"

class ScStyleSheet;
class ScDocument;

class SC_DLLPUBLIC ScStyleSheetPool final : public SfxStyleSheetPool
{
public:
                        ScStyleSheetPool( const SfxItemPool& rPool,
                                          ScDocument*   pDocument );

    void                SetDocument( ScDocument* pDocument );
    ScDocument*         GetDocument() const { return pDoc; }

    virtual void        Remove( SfxStyleSheetBase* pStyle ) override;

    void                SetActualStyleSheet ( SfxStyleSheetBase* pActStyleSheet )
                                { pActualStyleSheet = pActStyleSheet; }

    SfxStyleSheetBase*  GetActualStyleSheet ()
                                { return pActualStyleSheet; }

    void                CreateStandardStyles();
    void                CopyStdStylesFrom( ScStyleSheetPool* pSrcPool );

    void                CopyStyleFrom( ScStyleSheetPool* pSrcPool,
                                       const OUString& rName, SfxStyleFamily eFamily );

    bool                HasStandardStyles() const { return bHasStandardStyles; }

    ScStyleSheet*       FindCaseIns( const OUString& rName, SfxStyleFamily eFam );

    virtual SfxStyleSheetBase& Make( const OUString&, SfxStyleFamily eFam,
                                     SfxStyleSearchBits nMask = SfxStyleSearchBits::All) override;

    void setAllParaStandard();

private:
    virtual             ~ScStyleSheetPool() override;

    using SfxStyleSheetPool::Create;    // calcwarnings: Create(const SfxStyleSheet&) - ever used?

    virtual rtl::Reference<SfxStyleSheetBase> Create( const OUString&  rName,
                                       SfxStyleFamily   eFamily,
                                       SfxStyleSearchBits     nMask) override;
    virtual rtl::Reference<SfxStyleSheetBase> Create( const SfxStyleSheetBase& rStyle ) override;

    SfxStyleSheetBase*  pActualStyleSheet;
    ScDocument*         pDoc;
    bool                bHasStandardStyles;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
