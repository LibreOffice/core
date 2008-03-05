/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numpages.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:42:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_NUMPAGES_HXX
#define _SVX_NUMPAGES_HXX

// include ---------------------------------------------------------------

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SVX_NUMDEF_HXX
#include <svx/numdef.hxx>
#endif
#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

// -----------------------------------------------------------------------
class SvxNumRule;
class SvxBmpNumValueSet;
class SvxNumValueSet;
class ValueSet;
class SvxNumberingPreview;
class SvxBrushItem;

/*-----------------13.02.97 14.02-------------------

--------------------------------------------------*/

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

/*-----------------07.02.97 11.36-------------------

--------------------------------------------------*/
class SvxSingleNumPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    FixedLine               aValuesFL;
    SvxNumValueSet*         pExamplesVS;
    SvxNumSettingsArr_Impl  aNumSettingsArr;
    SvxNumRule*             pActNum;
    SvxNumRule*             pSaveNum;
    USHORT                  nActNumLvl;
    BOOL                    bModified   : 1;
    BOOL                    bHasChild   : 1;
    BOOL                    bPreset     : 1;

    String              sNumCharFmtName;
    USHORT              nNumItemId;

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
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetNumCharFmtName(const String& rName){sNumCharFmtName = rName;}
};

/*-----------------07.02.97 11.36-------------------

--------------------------------------------------*/

class SvxBulletPickTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    FixedLine           aValuesFL;
    SvxNumValueSet*     pExamplesVS;
    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;
    USHORT              nActNumLvl;
    BOOL                bModified   : 1;
    BOOL                bHasChild   : 1;
    BOOL                bPreset     : 1;
    USHORT              nNumItemId;

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
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetCharFmtName(const String& rName){sBulletCharFmtName = rName;}
    virtual void        PageCreated(SfxAllItemSet aSet); // add CHINA001
};

/*-----------------07.02.97 11.36-------------------
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
    USHORT              nActNumLvl;
    USHORT              nNumItemId;
    BOOL                bModified   : 1;
    BOOL                bPreset     : 1;


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
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetCharFmtNames(const String& rCharName, const String& rBulName)
                            {   sNumCharFmtName = rCharName;
                                sBulletCharFmtName = rBulName;}
    virtual void        PageCreated(SfxAllItemSet aSet); // add CHINA001
};

/*-----------------12.02.97 07.42-------------------

--------------------------------------------------*/
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
    USHORT              nActNumLvl;
    USHORT              nNumItemId;
    SfxMapUnit          eCoreUnit;
    BOOL                bModified   : 1;
    BOOL                bHasChild   : 1;
    BOOL                bPreset     : 1;

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
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetNumCharFmtName(const String& rName){sNumCharFmtName = rName;}
};

/*-----------------01.12.97 16:13-------------------

--------------------------------------------------*/
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

    BOOL                bHasChild           : 1;
    BOOL                bLastWidthModified  : 1;
    BOOL                bModified           : 1;
    BOOL                bPreset             : 1;
    BOOL                bAutomaticCharStyles: 1;
    BOOL                bHTMLMode           : 1;
    BOOL                bMenuButtonInitialized : 1;

    List                aGrfNames;
    Font                aActBulletFont;
    String              sBullet;
    String              sStartWith;

    BYTE                nBullet;
    USHORT              nActNumLvl;
    USHORT              nNumItemId;
    SfxMapUnit          eCoreUnit;

    void                InitControls();
    void                SwitchNumberType( BYTE nType, BOOL bBmp = FALSE );
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
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

//???   void                SetWrtShell(SwWrtShell* pSh);
    void                SetCharFmts(const String& rNumName, const String& rBulletName )
                                {
                                    sNumCharFmtName = rNumName;
                                    sBulletCharFmtName = rBulletName;
                                }
    void                SetMetric(FieldUnit eSet);

    ListBox&            GetCharFmtListBox() {return aCharFmtLB;}
    void                SetModified(BOOL bRepaint = TRUE);
    virtual void        PageCreated(SfxAllItemSet aSet); // add CHINA001
};

/*-----------------03.12.97 10:18-------------------

--------------------------------------------------*/
class SvxNumPositionTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    FixedLine           aPositionFL;
    FixedText           aLevelFT;
    MultiListBox        aLevelLB;

    // --> OD 2008-01-11 #newlistlevelattrs#
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
    // <--

    // --> OD 2008-01-10 #newlistlevelattrs#
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
    // <--

    PushButton          aStandardPB;

#if OSL_DEBUG_LEVEL > 1
    FixedText*          pDebugFixedText;
#endif
    SvxNumberingPreview* pPreviewWIN;

    SvxNumRule*         pActNum;
    SvxNumRule*         pSaveNum;

    USHORT              nActNumLvl;
    USHORT              nNumItemId;
    SfxMapUnit          eCoreUnit;

    BOOL                bModified           : 1;
    BOOL                bHasChild           : 1;
    BOOL                bPreset             : 1;
    BOOL                bInInintControl     : 1;  //Modify-Fehler umgehen, soll ab 391 behoben sein
    // --> OD 2008-01-11 #newlistlevelattrs#
    bool                bLabelAlignmentPosAndSpaceModeActive;
    // <--

    void                InitControls();

    DECL_LINK( LevelHdl_Impl, ListBox * );
    DECL_LINK( EditModifyHdl_Impl, Edit*);
    DECL_LINK( DistanceHdl_Impl, MetricField * );
    DECL_LINK( RelativeHdl_Impl, CheckBox * );
    DECL_LINK( StandardHdl_Impl, PushButton * );

    // --> OD 2008-01-11 #newlistlevelattrs#
    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK( LabelFollowedByHdl_Impl, ListBox* );
    DECL_LINK( ListtabPosHdl_Impl, MetricField* );
    DECL_LINK( AlignAtHdl_Impl, MetricField* );
    DECL_LINK( IndentAtHdl_Impl, MetricField* );
    // <--

public:
        SvxNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet);
        ~SvxNumPositionTabPage();

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SetMetric(FieldUnit eSet);
    void                SetModified(BOOL bRepaint = TRUE);
    virtual void        PageCreated(SfxAllItemSet aSet); // add CHINA001
};

#endif




