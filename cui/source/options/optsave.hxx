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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTSAVE_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTSAVE_HXX

#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
  namespace beans {
    struct PropertyValue;
}}}}

// class SvxSaveTabPage --------------------------------------------------

struct SvxSaveTabPage_Impl;

class SvxSaveTabPage : public SfxTabPage
{
private:
    VclPtr<CheckBox>               aLoadUserSettingsCB;
    VclPtr<CheckBox>               aLoadDocPrinterCB;

    VclPtr<CheckBox>               aDocInfoCB;
    VclPtr<CheckBox>               aBackupCB;
    VclPtr<CheckBox>               aAutoSaveCB;
    VclPtr<NumericField>           aAutoSaveEdit;
    VclPtr<FixedText>              aMinuteFT;
    VclPtr<CheckBox>               aUserAutoSaveCB;
    VclPtr<CheckBox>               aRelativeFsysCB;
    VclPtr<CheckBox>               aRelativeInetCB;

    VclPtr<ListBox>                aODFVersionLB;
    VclPtr<CheckBox>               aWarnAlienFormatCB;
    VclPtr<ListBox>                aDocTypeLB;
    VclPtr<FixedText>              aSaveAsFT;
    VclPtr<ListBox>                aSaveAsLB;
    VclPtr<FixedImage>             aODFWarningFI;
    VclPtr<FixedText>              aODFWarningFT;

    SvxSaveTabPage_Impl*    pImpl;

    DECL_LINK_TYPED( AutoClickHdl_Impl, Button*, void );
    DECL_LINK_TYPED( FilterHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED(ODFVersionHdl_Impl, ListBox&, void );

    void    DetectHiddenControls();

public:
    SvxSaveTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxSaveTabPage();
    virtual void        dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTSAVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
