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
#ifndef _SVX_NUMPAGES_HXX
#define _SVX_NUMPAGES_HXX

// include ---------------------------------------------------------------

#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <editeng/numdef.hxx>
#include <svtools/ctrlbox.hxx>
#include <svl/svarray.hxx>

// -----------------------------------------------------------------------
class SvxNumRule;
class SvxBmpNumValueSet;
class SvxNumValueSet;
class ValueSet;
class SvxNumberingPreview;
class SvxBrushItem;


//------------------------------------------------

struct SvxNumSettings_Impl
{
    short           nNumberType;
    short           nParentNumbering;
    rtl::OUString   sPrefix;
    rtl::OUString   sSuffix;
    rtl::OUString   sBulletChar;
    rtl::OUString   sBulletFont;
    SvxNumSettings_Impl() :
        nNumberType(0),
        nParentNumbering(0)
        {}
};

typedef SvxNumSettings_Impl* SvxNumSettings_ImplPtr;
SV_DECL_PTRARR_DEL(SvxNumSettingsArr_Impl,SvxNumSettings_ImplPtr,8,4)


//------------------------------------------------
class SvxSingleNumPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    FixedLine               aValuesFL;
    SvxNumValueSet*         pExamplesVS;
    SvxNumSettingsArr_Impl  aNumSettingsArr;
    SvxNumRule*             pActNum;
    SvxNumRule*             pSaveNum;
    sal_uInt16                  nActNumLvl;
    sal_Bool                    bModified   : 1;
    sal_Bool                    bPreset     : 1;

    String              sNumCharFmtName;
    sal_uInt16              nNumItemId;

    protected:
        DECL_LINK( NumSelectHdl_Impl, ValueSet*);
        DECL_LINK( DoubleClickHdl_Impl, ValueSet* );

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

    FixedLine           aValuesFL;
    SvxNumValueSet*     pExamplesVS;
    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_Bool                bModified   : 1;
    sal_Bool                bPreset     : 1;
    sal_uInt16              nNumItemId;

    String              sBulletCharFmtName;
    protected:
        DECL_LINK(NumSelectHdl_Impl, ValueSet*);
        DECL_LINK( DoubleClickHdl_Impl, ValueSet* );
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

/*--------------------------------------------------
    TabPage fuer vollstaendig Numerierung
--------------------------------------------------*/
#define NUMTYPE_MEMBER 4   // Anzahl der Elemente des FormatStrings je Ebene
#define NUM_VALUSET_COUNT 16
class SvxNumPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    FixedLine           aValuesFL;
    SvxNumValueSet*     pExamplesVS;
    String              sNumCharFmtName;
    String              sBulletCharFmtName;

    SvxNumSettingsArr_Impl  aNumSettingsArrays[NUM_VALUSET_COUNT];  // wird mit den fuenf Formaten initialisiert

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    sal_Bool                bModified   : 1;
    sal_Bool                bPreset     : 1;


    protected:
        DECL_LINK(NumSelectHdl_Impl, ValueSet*);
        DECL_LINK( DoubleClickHdl_Impl, ValueSet* );

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

    FixedLine           aValuesFL;
    SvxBmpNumValueSet*  pExamplesVS;
    FixedText           aErrorText;
    CheckBox            aLinkedCB;

    List                aGrfNames;
    String              sNumCharFmtName;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;
    sal_Bool                bModified   : 1;
    sal_Bool                bPreset     : 1;

    protected:
        DECL_LINK( NumSelectHdl_Impl, ValueSet*);
        DECL_LINK( DoubleClickHdl_Impl, ValueSet* );
        DECL_LINK( LinkBmpHdl_Impl, CheckBox* );

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

    FixedLine       aFormatFL;
    FixedText       aLevelFT;
    MultiListBox    aLevelLB;

    FixedText       aFmtFT;
    ListBox         aFmtLB;
    FixedText       aPrefixFT;
    Edit            aPrefixED;
    FixedText       aSuffixFT;
    Edit            aSuffixED;
    FixedText       aCharFmtFT;
    ListBox         aCharFmtLB;
    FixedText       aBulColorFT;
    ColorListBox    aBulColLB;
    FixedText       aBulRelSizeFT;
    MetricField     aBulRelSizeMF;
    FixedText       aAllLevelFT;
    NumericField    aAllLevelNF;
    FixedText       aStartFT;
    NumericField    aStartED;
    PushButton      aBulletPB;
    FixedText       aAlignFT;
    ListBox         aAlignLB;
    FixedText       aBitmapFT;
    MenuButton      aBitmapMB;
    FixedText       aSizeFT;
    MetricField     aWidthMF;
    FixedText       aMultFT;
    MetricField     aHeightMF;
    CheckBox        aRatioCB;
    FixedText       aOrientFT;
    ListBox         aOrientLB;

    FixedLine       aSameLevelFL;
    CheckBox        aSameLevelCB;

    String          sNumCharFmtName;
    String          sBulletCharFmtName;

    Timer           aInvalidateTimer;

    SvxNumberingPreview*    pPreviewWIN;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;

    Size                aInitSize[SVX_MAX_NUM];

    sal_Bool                bLastWidthModified  : 1;
    sal_Bool                bModified           : 1;
    sal_Bool                bPreset             : 1;
    sal_Bool                bAutomaticCharStyles: 1;
    sal_Bool                bHTMLMode           : 1;
    sal_Bool                bMenuButtonInitialized : 1;

    List                aGrfNames;
    Font                aActBulletFont;
    String              sBullet;
    String              sStartWith;

    sal_uInt8               nBullet;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;

    void                InitControls();
    void                SwitchNumberType( sal_uInt8 nType, sal_Bool bBmp = sal_False );
    void                CheckForStartValue_Impl(sal_uInt16 nNumberingType);

        DECL_LINK( NumberTypeSelectHdl_Impl, ListBox * );
        DECL_LINK( LevelHdl_Impl, ListBox * );
        DECL_LINK( PopupActivateHdl_Impl, Menu * );
        DECL_LINK( GraphicHdl_Impl, MenuButton * );
        DECL_LINK( BulletHdl_Impl, Button * );
        DECL_LINK( SizeHdl_Impl, MetricField * );
        DECL_LINK( RatioHdl_Impl, CheckBox * );
        DECL_LINK( CharFmtHdl_Impl, ListBox * );
        DECL_LINK( EditModifyHdl_Impl, Edit * );
        DECL_LINK( AllLevelHdl_Impl, NumericField * );
        DECL_LINK( OrientHdl_Impl, ListBox * );
        DECL_LINK( SameLevelHdl_Impl, CheckBox * );
        DECL_LINK( BulColorHdl_Impl, ColorListBox* );
        DECL_LINK( BulRelSizeHdl_Impl, MetricField *);
        DECL_LINK( PreviewInvalidateHdl_Impl, Timer* );

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

    void                SetCharFmts(const String& rNumName, const String& rBulletName )
                                {
                                    sNumCharFmtName = rNumName;
                                    sBulletCharFmtName = rBulletName;
                                }
    void                SetMetric(FieldUnit eSet);

    ListBox&            GetCharFmtListBox() {return aCharFmtLB;}
    void                SetModified(sal_Bool bRepaint = sal_True);
    virtual void        PageCreated(SfxAllItemSet aSet);
};

//------------------------------------------------
class SvxNumPositionTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    FixedLine           aPositionFL;
    FixedText           aLevelFT;
    MultiListBox        aLevelLB;

    // former set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
    FixedText           aDistBorderFT;
    MetricField         aDistBorderMF;
    CheckBox            aRelativeCB;
    FixedText           aIndentFT;
    MetricField         aIndentMF;
    FixedText           aDistNumFT;
    MetricField         aDistNumMF;
    FixedText           aAlignFT;
    ListBox             aAlignLB;

    // new set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_ALIGNMENT
    FixedText           aLabelFollowedByFT;
    ListBox             aLabelFollowedByLB;
    FixedText           aListtabFT;
    MetricField         aListtabMF;
    FixedText           aAlign2FT;
    ListBox             aAlign2LB;
    FixedText           aAlignedAtFT;
    MetricField         aAlignedAtMF;
    FixedText           aIndentAtFT;
    MetricField         aIndentAtMF;

    PushButton          aStandardPB;

#if OSL_DEBUG_LEVEL > 1
    FixedText*          pDebugFixedText;
#endif
    SvxNumberingPreview* pPreviewWIN;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;

    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    SfxMapUnit          eCoreUnit;

    sal_Bool                bModified           : 1;
    sal_Bool                bPreset             : 1;
    sal_Bool                bInInintControl     : 1;  //Modify-Fehler umgehen, soll ab 391 behoben sein
    bool                bLabelAlignmentPosAndSpaceModeActive;

    void                InitControls();

    DECL_LINK( LevelHdl_Impl, ListBox * );
    DECL_LINK( EditModifyHdl_Impl, Edit*);
    DECL_LINK( DistanceHdl_Impl, MetricField * );
    DECL_LINK( RelativeHdl_Impl, CheckBox * );
    DECL_LINK( StandardHdl_Impl, PushButton * );

    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK( LabelFollowedByHdl_Impl, ListBox* );
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
