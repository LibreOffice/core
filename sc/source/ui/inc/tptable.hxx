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
#if 1

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>

//===================================================================

/** A vcl/NumericField that additionally supports empty text.
    @descr  Value 0 is set as empty text, and empty text is returned as 0. */
class EmptyNumericField : public NumericField
{
public:
    inline explicit     EmptyNumericField( Window* pParent, WinBits nWinStyle ) :
                            NumericField( pParent, nWinStyle ) {}
    inline explicit     EmptyNumericField( Window* pParent, const ResId& rResId ) :
                            NumericField( pParent, rResId ) {}

    virtual void        Modify();
    virtual void        SetValue( sal_Int64 nValue );
    virtual sal_Int64   GetValue() const;
};

//===================================================================

class ScTablePage : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*           pParent,
                                          const SfxItemSet& rCoreSet );
    static  sal_uInt16*     GetRanges       ();
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );
    virtual void        DataChanged     ( const DataChangedEvent& rDCEvt );

private:
                    ScTablePage( Window* pParent, const SfxItemSet& rCoreSet );
    virtual         ~ScTablePage();

    void            ShowImage();

private:
    FixedLine       aFlPageDir;
    RadioButton     aBtnTopDown;
    RadioButton     aBtnLeftRight;
    FixedImage      aBmpPageDir;
    Image           aImgLeftRight;
    Image           aImgTopDown;
    CheckBox        aBtnPageNo;
    NumericField    aEdPageNo;

    FixedLine       aFlPrint;
    CheckBox        aBtnHeaders;
    CheckBox        aBtnGrid;
    CheckBox        aBtnNotes;
    CheckBox        aBtnObjects;
    CheckBox        aBtnCharts;
    CheckBox        aBtnDrawings;
    CheckBox        aBtnFormulas;
    CheckBox        aBtnNullVals;

    FixedLine           aFlScale;
    FixedText           aFtScaleMode;
    ListBox             aLbScaleMode;
    FixedText           aFtScaleAll;
    MetricField         aEdScaleAll;
    FixedText           aFtScalePageWidth;
    EmptyNumericField   aEdScalePageWidth;
    FixedText           aFtScalePageHeight;
    EmptyNumericField   aEdScalePageHeight;
    FixedText           aFtScalePageNum;
    NumericField        aEdScalePageNum;

private:
    //------------------------------------
    // Handler:
    DECL_LINK(PageDirHdl, void *);
    DECL_LINK( PageNoHdl,       CheckBox* );
    DECL_LINK(ScaleHdl, void *);
};

#endif // SC_TPTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
