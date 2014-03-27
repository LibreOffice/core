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

#ifndef SC_TPTABLE_HXX
#define SC_TPTABLE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>

class ScTablePage : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*           pParent,
                                          const SfxItemSet& rCoreSet );
    static  sal_uInt16* GetRanges       ();
    virtual bool        FillItemSet     ( SfxItemSet& rCoreSet ) SAL_OVERRIDE;
    virtual void        Reset           ( const SfxItemSet& rCoreSet ) SAL_OVERRIDE;
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL ) SAL_OVERRIDE;
    virtual void        DataChanged     ( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

private:
                    ScTablePage( Window* pParent, const SfxItemSet& rCoreSet );
    virtual         ~ScTablePage();

    void            ShowImage();

private:
    RadioButton*     m_pBtnTopDown;
    RadioButton*     m_pBtnLeftRight;
    FixedImage*      m_pBmpPageDir;
    CheckBox*        m_pBtnPageNo;
    NumericField*    m_pEdPageNo;

    CheckBox*        m_pBtnHeaders;
    CheckBox*        m_pBtnGrid;
    CheckBox*        m_pBtnNotes;
    CheckBox*        m_pBtnObjects;
    CheckBox*        m_pBtnCharts;
    CheckBox*        m_pBtnDrawings;
    CheckBox*        m_pBtnFormulas;
    CheckBox*        m_pBtnNullVals;

    ListBox*             m_pLbScaleMode;
    VclHBox*             m_pBxScaleAll;
    MetricField*         m_pEdScaleAll;
    VclGrid*             m_pGrHeightWidth;
    NumericField*        m_pEdScalePageWidth;
    NumericField*        m_pEdScalePageHeight;
    VclHBox*             m_pBxScalePageNum;
    NumericField*        m_pEdScalePageNum;

private:

    // Handler:
    DECL_LINK(PageDirHdl, void *);
    DECL_LINK( PageNoHdl,       CheckBox* );
    DECL_LINK(ScaleHdl, void *);
};

#endif // SC_TPTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
