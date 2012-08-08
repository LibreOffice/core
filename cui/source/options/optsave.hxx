/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_OPTSAVE_HXX
#define _SVX_OPTSAVE_HXX

#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <com/sun/star/uno/Reference.h>
#include <readonlyimage.hxx>

namespace com { namespace sun { namespace star {
  namespace beans {
    struct PropertyValue;
}}}}

// define ----------------------------------------------------------------

#define SfxSaveTabPage SvxSaveTabPage

// class SvxSaveTabPage --------------------------------------------------

struct SvxSaveTabPage_Impl;

class SvxSaveTabPage : public SfxTabPage
{
private:
    FixedLine               aLoadFL;
    CheckBox                aLoadUserSettingsCB;
    CheckBox                aLoadDocPrinterCB;

    FixedLine               aSaveFL;
    CheckBox                aDocInfoCB;
    ReadOnlyImage           aBackupFI;
    CheckBox                aBackupCB;
    CheckBox                aAutoSaveCB;
    NumericField            aAutoSaveEdit;
    FixedText               aMinuteFT;
    CheckBox                aRelativeFsysCB;
    CheckBox                aRelativeInetCB;

    FixedLine               aDefaultFormatFL;
    FixedText               aODFVersionFT;
    ListBox                 aODFVersionLB;
    CheckBox                aSizeOptimizationCB;
    CheckBox                aWarnAlienFormatCB;
    FixedText               aDocTypeFT;
    ListBox                 aDocTypeLB;
    FixedText               aSaveAsFT;
    ReadOnlyImage           aSaveAsFI;
    ListBox                 aSaveAsLB;
    FixedImage              aODFWarningFI;
    FixedText               aODFWarningFT;

    SvxSaveTabPage_Impl*    pImpl;

#ifdef _SVX_OPTSAVE_CXX
    DECL_LINK( AutoClickHdl_Impl, CheckBox * );
    DECL_LINK( FilterHdl_Impl, ListBox * );
    DECL_LINK(ODFVersionHdl_Impl, void *);

    void    DetectHiddenControls();
#endif

public:
    SvxSaveTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxSaveTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif  // #ifndef _SVX_OPTSAVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
