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
#ifndef _SVX_NUMPAGES_HXX
#define _SVX_NUMPAGES_HXX

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <editeng/numdef.hxx>
#include <svtools/ctrlbox.hxx>

// -----------------------------------------------------------------------
class SvxNumRule;
class SvxBmpNumValueSet;
class SvxNumValueSet;
class SvxBrushItem;

class SvxNumberingPreview : public Window
{
    const SvxNumRule*   pActNum;
    Font                aStdFont;
    long                nPageWidth;
    const String*       pOutlineNames;
    sal_Bool                bPosition;
    sal_uInt16              nActLevel;

    protected:
        virtual void        Paint( const Rectangle& rRect );

    public:
        SvxNumberingPreview(Window* pParent, WinBits nWinBits = WB_BORDER);

        void    SetNumRule(const SvxNumRule* pNum)
                    {pActNum = pNum; Invalidate();};
        void    SetPageWidth(long nPgWidth)
                                {nPageWidth = nPgWidth;}
        void    SetOutlineNames(const String* pNames)
                        {pOutlineNames = pNames;}
        void    SetPositionMode()
                        { bPosition = sal_True;}
        void    SetLevel(sal_uInt16 nSet) {nActLevel = nSet;}

};

//------------------------------------------------

struct SvxNumSettings_Impl
{
    short           nNumberType;
    short           nParentNumbering;
    OUString   sPrefix;
    OUString   sSuffix;
    OUString   sBulletChar;
    OUString   sBulletFont;
    SvxNumSettings_Impl() :
        nNumberType(0),
        nParentNumbering(0)
        {}
};

typedef boost::ptr_vector<SvxNumSettings_Impl> SvxNumSettingsArr_Impl;


//------------------------------------------------
class SvxSingleNumPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    SvxNumValueSet*         m_pExamplesVS;
    SvxNumSettingsArr_Impl  aNumSettingsArr;
    SvxNumRule*             pActNum;
    SvxNumRule*             pSaveNum;
    sal_uInt16                  nActNumLvl;
    sal_Bool                    bModified   : 1;
    sal_Bool                    bPreset     : 1;

    String              sNumCharFmtName;
    sal_uInt16              nNumItemId;

    protected:
        DECL_LINK(NumSelectHdl_Impl, void *);
        DECL_LINK(DoubleClickHdl_Impl, void *);

    public:
        SvxSingleNumPickTabPage(Window* pParent,
                               const SfxItemSet& rSet);
        ~SvxSingleNumPickTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetNumCharFmtName(const String& rName){sNumCharFmtName = rName;}
};


//------------------------------------------------

class SvxBulletPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    SvxNumValueSet*     m_pExamplesVS;
    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_Bool                bModified   : 1;
    sal_Bool                bPreset     : 1;
    sal_uInt16              nNumItemId;

    String              sBulletCharFmtName;
    protected:
        DECL_LINK(NumSelectHdl_Impl, void *);
        DECL_LINK(DoubleClickHdl_Impl, void *);
    public:
        SvxBulletPickTabPage(Window* pParent,
                               const SfxItemSet& rSet);
        ~SvxBulletPickTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetCharFmtName(const String& rName){sBulletCharFmtName = rName;}
    virtual void        PageCreated(SfxAllItemSet aSet);
};

#define NUM_VALUSET_COUNT 16

/// TabPage for complete numeration
class SvxNumPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    SvxNumValueSet*     m_pExamplesVS;
    String              sNumCharFmtName;
    String              sBulletCharFmtName;

    SvxNumSettingsArr_Impl  aNumSettingsArrays[NUM_VALUSET_COUNT];  // is initialized with the five formats

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    sal_Bool                bModified   : 1;
    sal_Bool                bPreset     : 1;


    protected:
        DECL_LINK(NumSelectHdl_Impl, void *);
        DECL_LINK(DoubleClickHdl_Impl, void *);

    public:
        SvxNumPickTabPage(Window* pParent,
                               const SfxItemSet& rSet);
        ~SvxNumPickTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetCharFmtNames(const String& rCharName, const String& rBulName)
                            {   sNumCharFmtName = rCharName;
                                sBulletCharFmtName = rBulName;}
    virtual void        PageCreated(SfxAllItemSet aSet);
};


//------------------------------------------------
class SvxBitmapPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    FixedText*          m_pErrorText;
    SvxBmpNumValueSet*  m_pExamplesVS;

    std::vector<String> aGrfNames;
    String              sNumCharFmtName;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;
    sal_Bool                bModified   : 1;
    sal_Bool                bPreset     : 1;

    protected:
        DECL_LINK(NumSelectHdl_Impl, void *);
        DECL_LINK(DoubleClickHdl_Impl, void *);
        DECL_LINK(LinkBmpHdl_Impl, void *);

    public:
        SvxBitmapPickTabPage(Window* pParent,
                               const SfxItemSet& rSet);
        ~SvxBitmapPickTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetNumCharFmtName(const String& rName){sNumCharFmtName = rName;}
};

//------------------------------------------------
class SvxNumOptionsTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    ListBox*        m_pLevelLB;

    ListBox*        m_pFmtLB;

    FixedText*      m_pSeparatorFT;
    FixedText*      m_pPrefixFT;
    Edit*           m_pPrefixED;
    FixedText*      m_pSuffixFT;
    Edit*           m_pSuffixED;
    FixedText*      m_pCharFmtFT;
    ListBox*        m_pCharFmtLB;
    FixedText*      m_pBulColorFT;
    ColorListBox*   m_pBulColLB;
    FixedText*      m_pBulRelSizeFT;
    MetricField*    m_pBulRelSizeMF;
    FixedText*      m_pAllLevelFT;
    NumericField*   m_pAllLevelNF;
    FixedText*      m_pStartFT;
    NumericField*   m_pStartED;
    FixedText*      m_pBulletFT;
    PushButton*     m_pBulletPB;
    FixedText*      m_pAlignFT;
    ListBox*        m_pAlignLB;
    FixedText*      m_pBitmapFT;
    MenuButton*     m_pBitmapMB;
    sal_uInt16      m_nGalleryId;
    FixedText*      m_pWidthFT;
    MetricField*    m_pWidthMF;
    FixedText*      m_pHeightFT;
    MetricField*    m_pHeightMF;
    CheckBox*       m_pRatioCB;
    FixedText*      m_pOrientFT;
    ListBox*        m_pOrientLB;

    VclContainer*   m_pAllLevelsFrame;
    CheckBox*       m_pSameLevelCB;

    SvxNumberingPreview* m_pPreviewWIN;

    OUString        m_sNumCharFmtName;
    OUString        m_sBulletCharFmtName;

    Timer           aInvalidateTimer;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;

    Size                aInitSize[SVX_MAX_NUM];

    sal_Bool                bLastWidthModified  : 1;
    sal_Bool                bModified           : 1;
    sal_Bool                bPreset             : 1;
    sal_Bool                bAutomaticCharStyles: 1;
    sal_Bool                bHTMLMode           : 1;
    sal_Bool                bMenuButtonInitialized : 1;

    std::vector<String> aGrfNames;
    Font                aActBulletFont;

    sal_uInt8               nBullet;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;

    void                InitControls();
    /** To switch between the numbering type
        0 - Number;
        1 - Bullet;
        2 - Bitmap; */
    void                SwitchNumberType( sal_uInt8 nType, sal_Bool bBmp = sal_False );
    void                CheckForStartValue_Impl(sal_uInt16 nNumberingType);

        DECL_LINK( NumberTypeSelectHdl_Impl, ListBox * );
        DECL_LINK( LevelHdl_Impl, ListBox * );
        DECL_LINK(PopupActivateHdl_Impl, void *);
        DECL_LINK( GraphicHdl_Impl, MenuButton * );
        DECL_LINK(BulletHdl_Impl, void *);
        DECL_LINK( SizeHdl_Impl, MetricField * );
        DECL_LINK( RatioHdl_Impl, CheckBox * );
        DECL_LINK(CharFmtHdl_Impl, void *);
        DECL_LINK( EditModifyHdl_Impl, Edit * );
        DECL_LINK( AllLevelHdl_Impl, NumericField * );
        DECL_LINK( OrientHdl_Impl, ListBox * );
        DECL_LINK( SameLevelHdl_Impl, CheckBox * );
        DECL_LINK( BulColorHdl_Impl, ColorListBox* );
        DECL_LINK( BulRelSizeHdl_Impl, MetricField *);
        DECL_LINK(PreviewInvalidateHdl_Impl, void *);

        DECL_STATIC_LINK( SvxNumOptionsTabPage, GraphicArrivedHdl_Impl, SvxBrushItem* );

    public:
        SvxNumOptionsTabPage(Window* pParent,
                               const SfxItemSet& rSet);
        ~SvxNumOptionsTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetCharFmts(const OUString& rNumName, const OUString& rBulletName)
                        {
                            m_sNumCharFmtName = rNumName;
                            m_sBulletCharFmtName = rBulletName;
                        }
    void                SetMetric(FieldUnit eSet);

    ListBox&            GetCharFmtListBox() {return *m_pCharFmtLB;}
    void                SetModified(sal_Bool bRepaint = sal_True);
    virtual void        PageCreated(SfxAllItemSet aSet);

    /** Get the numberings provided by the i18n framework (CTL, Asian, ...) and
        add them to the listbox. Extended numbering schemes present in the
        resource and already in the listbox but not offered by the i18n
        framework per configuration are removed.

        @param nDoNotRemove
            A value that shall not be removed, i.e. the ugly 0x88
            (SVX_NUM_BITMAP|0x80)
            Pass ::std::numeric_limits<sal_uInt16>::max() if there is no such
            restriction.
     */
    static void         GetI18nNumbering( ListBox& rFmtLB, sal_uInt16 nDoNotRemove );
};

//------------------------------------------------
class SvxNumPositionTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    ListBox*            m_pLevelLB;

    // former set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
    FixedText*          m_pDistBorderFT;
    MetricField*        m_pDistBorderMF;
    CheckBox*           m_pRelativeCB;
    FixedText*          m_pIndentFT;
    MetricField*        m_pIndentMF;
    FixedText*          m_pDistNumFT;
    MetricField*        m_pDistNumMF;
    FixedText*          m_pAlignFT;
    ListBox*            m_pAlignLB;

    // new set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_ALIGNMENT
    FixedText*          m_pLabelFollowedByFT;
    ListBox*            m_pLabelFollowedByLB;
    FixedText*          m_pListtabFT;
    MetricField*        m_pListtabMF;
    FixedText*          m_pAlign2FT;
    ListBox*            m_pAlign2LB;
    FixedText*          m_pAlignedAtFT;
    MetricField*        m_pAlignedAtMF;
    FixedText*          m_pIndentAtFT;
    MetricField*        m_pIndentAtMF;

    PushButton*         m_pStandardPB;

    SvxNumberingPreview* m_pPreviewWIN;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;

    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;

    sal_Bool                bModified           : 1;
    sal_Bool                bPreset             : 1;
    sal_Bool                bInInintControl     : 1;  // workaround for Modify-error, is said to be correctet from 391 on
    bool                bLabelAlignmentPosAndSpaceModeActive;

    void                InitControls();

    DECL_LINK( LevelHdl_Impl, ListBox * );
    DECL_LINK(EditModifyHdl_Impl, void *);
    DECL_LINK( DistanceHdl_Impl, MetricField * );
    DECL_LINK( RelativeHdl_Impl, CheckBox * );
    DECL_LINK(StandardHdl_Impl, void *);

    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK(LabelFollowedByHdl_Impl, void *);
    DECL_LINK( ListtabPosHdl_Impl, MetricField* );
    DECL_LINK( AlignAtHdl_Impl, MetricField* );
    DECL_LINK( IndentAtHdl_Impl, MetricField* );

public:
        SvxNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet);
        ~SvxNumPositionTabPage();

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SetMetric(FieldUnit eSet);
    void                SetModified(sal_Bool bRepaint = sal_True);
    virtual void        PageCreated(SfxAllItemSet aSet);
};

#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
