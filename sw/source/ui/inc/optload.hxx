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
    FixedLine   aUpdateFL;
    FixedText   aLinkFT;
    RadioButton aAlwaysRB;
    RadioButton aRequestRB;
    RadioButton aNeverRB;

    FixedText   aFieldFT;
    CheckBox    aAutoUpdateFields;
    CheckBox    aAutoUpdateCharts;

    FixedLine   aSettingsFL;
    FixedText   aMetricFT;
    ListBox     aMetricLB;
    FixedText   aTabFT;
    MetricField aTabMF;
    CheckBox    aUseSquaredPageMode;
    CheckBox    aUseCharUnit;
    FixedLine   aWordCountFL;
    FixedText   aWordCountFT;
    Edit        aWordCountED;

    SwWrtShell* pWrtShell;
    sal_Bool    bHTMLMode;
    sal_uInt16      nLastTab;
    sal_Int32   nOldLinkMode;

    DECL_LINK(MetricHdl, void *);

public:
    SwLoadOptPage( Window* pParent, const SfxItemSet& rSet );
    ~SwLoadOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

class SwCaptionOptDlg : public SfxSingleTabDialog
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
    String          maText;
    Point           maDrawPos;
public:
                    SwCaptionPreview( Window* pParent, const ResId& rResId );
    void            SetPreviewText( const String& rText );
    virtual void    Paint( const Rectangle& rRect );
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

    String          sSWTable;
    String          sSWFrame;
    String          sSWGraphic;
    String          sOLE;

    String          sIllustration;
    String          sTable;
    String          sText;
    String          sDrawing;

    String          sBegin;
    String          sEnd;
    String          sAbove;
    String          sBelow;

    String          sNone;

    SwFldMgr        *pMgr;
    sal_Bool            bHTMLMode;

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
