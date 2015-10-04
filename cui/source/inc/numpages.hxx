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
#ifndef INCLUDED_CUI_SOURCE_INC_NUMPAGES_HXX
#define INCLUDED_CUI_SOURCE_INC_NUMPAGES_HXX

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
#include <vcl/dialog.hxx>

class SvxNumRule;
class SvxBmpNumValueSet;
class SvxNumValueSet;
class SvxBrushItem;
class ValueSet;

class SvxNumberingPreview : public vcl::Window
{
    const SvxNumRule*   pActNum;
    vcl::Font           aStdFont;
    long                nPageWidth;
    const OUString*     pOutlineNames;
    bool                bPosition;
    sal_uInt16              nActLevel;

    protected:
        virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) SAL_OVERRIDE;

    public:
        SvxNumberingPreview(vcl::Window* pParent, WinBits nWinBits = WB_BORDER);

        void    SetNumRule(const SvxNumRule* pNum)
                    {pActNum = pNum; Invalidate();};
        void    SetPositionMode()
                        { bPosition = true;}
        void    SetLevel(sal_uInt16 nSet) {nActLevel = nSet;}

};

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



class SvxSingleNumPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    VclPtr<SvxNumValueSet>         m_pExamplesVS;
    SvxNumSettingsArr_Impl  aNumSettingsArr;
    SvxNumRule*             pActNum;
    SvxNumRule*             pSaveNum;
    sal_uInt16                  nActNumLvl;
    bool                    bModified   : 1;
    bool                    bPreset     : 1;

    OUString              sNumCharFmtName;
    sal_uInt16              nNumItemId;

protected:
        DECL_LINK_TYPED(NumSelectHdl_Impl, ValueSet*, void);
        DECL_LINK_TYPED(DoubleClickHdl_Impl, ValueSet*, void);

public:
        SvxSingleNumPickTabPage(vcl::Window* pParent,
                               const SfxItemSet& rSet);
    virtual ~SvxSingleNumPickTabPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual sfxpg       DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
};




class SvxBulletPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    VclPtr<SvxNumValueSet>     m_pExamplesVS;
    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16          nActNumLvl;
    bool                bModified   : 1;
    bool                bPreset     : 1;
    sal_uInt16          nNumItemId;

    OUString            sBulletCharFormatName;
protected:
        DECL_LINK_TYPED(NumSelectHdl_Impl, ValueSet*, void);
        DECL_LINK_TYPED(DoubleClickHdl_Impl, ValueSet*, void);
public:
        SvxBulletPickTabPage(vcl::Window* pParent,
                               const SfxItemSet& rSet);
    virtual ~SvxBulletPickTabPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual sfxpg       DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    void                SetCharFormatName(const OUString& rName){sBulletCharFormatName = rName;}
    virtual void        PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;
};

#define NUM_VALUSET_COUNT 16

/// TabPage for complete numeration
class SvxNumPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    VclPtr<SvxNumValueSet>     m_pExamplesVS;
    OUString            sNumCharFmtName;
    OUString            sBulletCharFormatName;

    SvxNumSettingsArr_Impl  aNumSettingsArrays[NUM_VALUSET_COUNT];  // is initialized with the five formats

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    bool                bModified   : 1;
    bool                bPreset     : 1;


protected:
        DECL_LINK_TYPED(NumSelectHdl_Impl, ValueSet*, void);
        DECL_LINK_TYPED(DoubleClickHdl_Impl, ValueSet*, void);

public:
    SvxNumPickTabPage(vcl::Window* pParent,
                               const SfxItemSet& rSet);
    virtual ~SvxNumPickTabPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual sfxpg       DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    void                SetCharFormatNames(const OUString& rCharName, const OUString& rBulName)
                            {   sNumCharFmtName = rCharName;
                                sBulletCharFormatName = rBulName;}
    virtual void        PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;
};



class SvxBitmapPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    VclPtr<FixedText>          m_pErrorText;
    VclPtr<SvxBmpNumValueSet>  m_pExamplesVS;
    VclPtr<Button>             m_pBtBrowseFile;

    std::vector<OUString> aGrfNames;
    OUString            sNumCharFmtName;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;
    bool                bModified   : 1;
    bool                bPreset     : 1;

protected:
        DECL_LINK_TYPED(NumSelectHdl_Impl, ValueSet*, void);
        DECL_LINK_TYPED(DoubleClickHdl_Impl, ValueSet*, void);
        DECL_LINK_TYPED(ClickAddBrowseHdl_Impl, Button*, void );


public:
        SvxBitmapPickTabPage(vcl::Window* pParent,
                               const SfxItemSet& rSet);
        virtual ~SvxBitmapPickTabPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual sfxpg       DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
};


class SvxNumOptionsTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    VclPtr<ListBox>        m_pLevelLB;

    VclPtr<ListBox>        m_pFmtLB;

    VclPtr<FixedText>      m_pSeparatorFT;
    VclPtr<FixedText>      m_pPrefixFT;
    VclPtr<Edit>           m_pPrefixED;
    VclPtr<FixedText>      m_pSuffixFT;
    VclPtr<Edit>           m_pSuffixED;
    VclPtr<FixedText>      m_pCharFmtFT;
    VclPtr<ListBox>        m_pCharFmtLB;
    VclPtr<FixedText>      m_pBulColorFT;
    VclPtr<ColorListBox>   m_pBulColLB;
    VclPtr<FixedText>      m_pBulRelSizeFT;
    VclPtr<MetricField>    m_pBulRelSizeMF;
    VclPtr<FixedText>      m_pAllLevelFT;
    VclPtr<NumericField>   m_pAllLevelNF;
    VclPtr<FixedText>      m_pStartFT;
    VclPtr<NumericField>   m_pStartED;
    VclPtr<FixedText>      m_pBulletFT;
    VclPtr<PushButton>     m_pBulletPB;
    VclPtr<FixedText>      m_pAlignFT;
    VclPtr<ListBox>        m_pAlignLB;
    VclPtr<FixedText>      m_pBitmapFT;
    VclPtr<MenuButton>     m_pBitmapMB;
    sal_uInt16      m_nGalleryId;
    VclPtr<FixedText>      m_pWidthFT;
    VclPtr<MetricField>    m_pWidthMF;
    VclPtr<FixedText>      m_pHeightFT;
    VclPtr<MetricField>    m_pHeightMF;
    VclPtr<CheckBox>       m_pRatioCB;
    VclPtr<FixedText>      m_pOrientFT;
    VclPtr<ListBox>        m_pOrientLB;

    VclPtr<VclContainer>   m_pAllLevelsFrame;
    VclPtr<CheckBox>       m_pSameLevelCB;

    VclPtr<SvxNumberingPreview> m_pPreviewWIN;

    OUString        m_sNumCharFmtName;
    OUString        m_sBulletCharFormatName;

    Timer           aInvalidateTimer;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;

    Size                aInitSize[SVX_MAX_NUM];

    bool                bLastWidthModified  : 1;
    bool                bModified           : 1;
    bool                bPreset             : 1;
    bool                bAutomaticCharStyles: 1;
    bool                bHTMLMode           : 1;
    bool                bMenuButtonInitialized : 1;

    std::vector<OUString> aGrfNames;
    vcl::Font             aActBulletFont;

    sal_uInt8               nBullet;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;

    void                InitControls();
    /** To switch between the numbering type
        0 - Number;
        1 - Bullet;
        2 - Bitmap; */
    void                SwitchNumberType( sal_uInt8 nType, bool bBmp = false );
    void                CheckForStartValue_Impl(sal_uInt16 nNumberingType);

        DECL_LINK_TYPED( NumberTypeSelectHdl_Impl, ListBox&, void );
        DECL_LINK_TYPED( LevelHdl_Impl, ListBox&, void );
        DECL_LINK_TYPED( PopupActivateHdl_Impl, Menu *, bool);
        DECL_LINK_TYPED( GraphicHdl_Impl, MenuButton *, void );
        DECL_LINK_TYPED( BulletHdl_Impl, Button*, void);
        DECL_LINK( SizeHdl_Impl, MetricField * );
        DECL_LINK_TYPED( RatioHdl_Impl, Button*, void );
        DECL_LINK_TYPED( CharFmtHdl_Impl, ListBox&, void );
        DECL_LINK( EditModifyHdl_Impl, Edit* );
        DECL_LINK_TYPED( EditListBoxHdl_Impl, ListBox&, void );
        DECL_LINK( AllLevelHdl_Impl, NumericField * );
        DECL_LINK_TYPED( OrientHdl_Impl, ListBox&, void );
        DECL_LINK_TYPED( SameLevelHdl_Impl, Button*, void );
        DECL_LINK_TYPED( BulColorHdl_Impl, ListBox&, void );
        DECL_LINK( BulRelSizeHdl_Impl, MetricField *);
        DECL_LINK_TYPED( PreviewInvalidateHdl_Impl, Timer *, void);

public:
        SvxNumOptionsTabPage(vcl::Window* pParent,
                               const SfxItemSet& rSet);
        virtual ~SvxNumOptionsTabPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual sfxpg       DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    void                SetCharFmts(const OUString& rNumName, const OUString& rBulletName)
                        {
                            m_sNumCharFmtName = rNumName;
                            m_sBulletCharFormatName = rBulletName;
                        }
    void                SetMetric(FieldUnit eSet);

    ListBox&            GetCharFmtListBox() {return *m_pCharFmtLB;}
    void                SetModified(bool bRepaint = true);
    virtual void        PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;

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


class SvxNumPositionTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    VclPtr<ListBox>            m_pLevelLB;

    // former set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
    VclPtr<FixedText>          m_pDistBorderFT;
    VclPtr<MetricField>        m_pDistBorderMF;
    VclPtr<CheckBox>           m_pRelativeCB;
    VclPtr<FixedText>          m_pIndentFT;
    VclPtr<MetricField>        m_pIndentMF;
    VclPtr<FixedText>          m_pDistNumFT;
    VclPtr<MetricField>        m_pDistNumMF;
    VclPtr<FixedText>          m_pAlignFT;
    VclPtr<ListBox>            m_pAlignLB;

    // new set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_ALIGNMENT
    VclPtr<FixedText>          m_pLabelFollowedByFT;
    VclPtr<ListBox>            m_pLabelFollowedByLB;
    VclPtr<FixedText>          m_pListtabFT;
    VclPtr<MetricField>        m_pListtabMF;
    VclPtr<FixedText>          m_pAlign2FT;
    VclPtr<ListBox>            m_pAlign2LB;
    VclPtr<FixedText>          m_pAlignedAtFT;
    VclPtr<MetricField>        m_pAlignedAtMF;
    VclPtr<FixedText>          m_pIndentAtFT;
    VclPtr<MetricField>        m_pIndentAtMF;

    VclPtr<PushButton>         m_pStandardPB;

    VclPtr<SvxNumberingPreview> m_pPreviewWIN;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;

    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;

    bool                bModified           : 1;
    bool                bPreset             : 1;
    bool                bInInintControl     : 1;  // workaround for Modify-error, is said to be corrected from 391 on
    bool                bLabelAlignmentPosAndSpaceModeActive;

    void                InitControls();

    DECL_LINK_TYPED( LevelHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( EditModifyHdl_Impl, ListBox&, void);
    DECL_LINK( DistanceHdl_Impl, MetricField * );
    DECL_LINK_TYPED( DistanceFocusHdl_Impl, Control&, void );
    DECL_LINK_TYPED( RelativeHdl_Impl, Button*, void );
    DECL_LINK_TYPED( StandardHdl_Impl, Button*, void);

    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK_TYPED(LabelFollowedByHdl_Impl, ListBox&, void);
    DECL_LINK( ListtabPosHdl_Impl, MetricField* );
    DECL_LINK( AlignAtHdl_Impl, MetricField* );
    DECL_LINK( IndentAtHdl_Impl, MetricField* );

public:
        SvxNumPositionTabPage(vcl::Window* pParent,
                               const SfxItemSet& rSet);
        virtual ~SvxNumPositionTabPage();
    virtual void dispose() SAL_OVERRIDE;

    virtual void        ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual sfxpg       DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);

    void                SetMetric(FieldUnit eSet);
    void                SetModified(bool bRepaint = true);
    virtual void        PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
