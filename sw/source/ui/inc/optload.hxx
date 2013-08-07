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
#ifndef _OPTLOAD_HXX
#define _OPTLOAD_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <svx/strarray.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/checklbx.hxx>
#include <swlbox.hxx>
#include <caption.hxx>

class SwFldMgr;
class SvTreeListEntry;
class SwWrtShell;

class SwLoadOptPage : public SfxTabPage
{
private:
    RadioButton* m_pAlwaysRB;
    RadioButton* m_pRequestRB;
    RadioButton* m_pNeverRB;

    CheckBox*    m_pAutoUpdateFields;
    CheckBox*    m_pAutoUpdateCharts;

    ListBox*     m_pMetricLB;
    FixedText*   m_pTabFT;
    MetricField* m_pTabMF;
    CheckBox*    m_pUseSquaredPageMode;
    CheckBox*    m_pUseCharUnit;
    Edit*        m_pWordCountED;

    SwWrtShell*  m_pWrtShell;
    sal_uInt16   m_nLastTab;
    sal_Int32    m_nOldLinkMode;

    DECL_LINK(MetricHdl, void *);

public:
    SwLoadOptPage(Window* pParent, const SfxItemSet& rSet);

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

class SwCaptionOptDlg : public SfxNoLayoutSingleTabDialog
{
public:
     SwCaptionOptDlg(Window* pParent, const SfxItemSet& rSet);
    ~SwCaptionOptDlg();
};

class CaptionComboBox : public SwComboBox
{
protected:
    virtual void KeyInput( const KeyEvent& );

public:
    CaptionComboBox( Window* pParent, const ResId& rResId)
        : SwComboBox(pParent, rResId)
    {}
};

class SwCaptionPreview : public Window
{
private:
    String maText;
    Point maDrawPos;
public:
    SwCaptionPreview( Window* pParent );
    SwCaptionPreview( Window* pParent, const ResId& rResId );
    void Init();
    void SetPreviewText( const String& rText );
    virtual void Paint( const Rectangle& rRect );
};

class SwCaptionOptPage : public SfxTabPage
{
private:
    FixedText       aCheckFT;
    SvxCheckListBox aCheckLB;

    FixedText       aFtCaptionOrder;
    ListBox         aLbCaptionOrder;

    SwCaptionPreview    aPreview;

    FixedLine       aSettingsGroupFL;
    FixedText       aCategoryText;
    CaptionComboBox aCategoryBox;
    FixedText       aFormatText;
    ListBox         aFormatBox;
    //#i61007# order of captions
    FixedText       aNumberingSeparatorFT;
    Edit            aNumberingSeparatorED;
    FixedText       aTextText;
    Edit            aTextEdit;
    FixedText       aPosText;
    ListBox         aPosBox;

    FixedLine       aNumCaptFL;
    FixedText       aFtLevel;
    ListBox         aLbLevel;
    FixedText       aFtDelim;
    Edit            aEdDelim;

    FixedLine       aCategoryFL;
    FixedText       aCharStyleFT;
    ListBox         aCharStyleLB;
    CheckBox        aApplyBorderCB;

    OUString        m_sSWTable;
    OUString        m_sSWFrame;
    OUString        m_sSWGraphic;
    OUString        m_sOLE;

    OUString        m_sIllustration;
    OUString        m_sTable;
    OUString        m_sText;
    OUString        m_sDrawing;

    OUString        m_sBegin;
    OUString        m_sEnd;
    OUString        m_sAbove;
    OUString        m_sBelow;

    OUString        m_sNone;

    SwFldMgr        *pMgr;
    bool            bHTMLMode;

    DECL_LINK(SelectHdl, void *);
    DECL_LINK(ModifyHdl, void * = 0);
    DECL_LINK( OrderHdl, ListBox* );
    DECL_LINK(ShowEntryHdl, void *);
    DECL_LINK(SaveEntryHdl, void *);

    void                DelUserData();
    void                SetOptions( const sal_uInt16 nPos,
                                    const SwCapObjType eType,
                                    const SvGlobalName *pOleId = 0);
    void                SaveEntry( SvTreeListEntry* pEntry );
    void                DrawSample();

public:
                        SwCaptionOptPage( Window* pParent,
                                         const SfxItemSet& rSet );
                        ~SwCaptionOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
