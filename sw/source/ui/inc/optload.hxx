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
class SvLBoxEntry;
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

    SwWrtShell* pWrtShell;
    sal_Bool    bHTMLMode;
    sal_uInt16      nLastTab;
    sal_Int32   nOldLinkMode;

    DECL_LINK(CaptionHdl, PushButton*);
    DECL_LINK(MetricHdl, ListBox*);

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
    sal_uInt16          eType;
    sal_Bool            bHTMLMode;

    DECL_LINK( SelectHdl, ListBox *pLB = 0 );
    DECL_LINK( ModifyHdl, Edit *pEdt = 0 );
    DECL_LINK( OrderHdl, ListBox* );
    DECL_LINK( ShowEntryHdl, SvxCheckListBox *pLB = 0 );
    DECL_LINK( SaveEntryHdl, SvxCheckListBox *pLB = 0 );

    void                DelUserData();
    void                SetOptions( const sal_uInt16 nPos,
                                    const SwCapObjType eType,
                                    const SvGlobalName *pOleId = 0);
    void                SaveEntry( SvLBoxEntry* pEntry );
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
