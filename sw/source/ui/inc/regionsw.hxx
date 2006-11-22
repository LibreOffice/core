/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: regionsw.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:27:00 $
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
#ifndef _REGIONSW_HXX
#define _REGIONSW_HXX
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif

#ifndef _CONDEDIT_HXX
#include <condedit.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _NUMBERINGTYPELISTBOX_HXX
#include <numberingtypelistbox.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include "svx/frmdiritem.hxx"
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SVX_PARAPREV_HXX
#include <svx/paraprev.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
class SwWrtShell;
class EditRegionDlg;

namespace sfx2
{
    class DocumentInserter;
    class FileDialogHelper;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SectRepr
{
    SwSection               aSection;
    SwFmtCol                aCol;
    SvxBrushItem            aBrush;
    SwFmtFtnAtTxtEnd        aFtnNtAtEnd;
    SwFmtEndAtTxtEnd        aEndNtAtEnd;
    SwFmtNoBalancedColumns  aBalance;
    SvxFrameDirectionItem   aFrmDirItem;
    SvxLRSpaceItem          aLRSpaceItem;
    USHORT                  nArrPos;
    USHORT                  nColumn;
    BOOL                    bContent    : 1; //zeigt an, ob evtl. Textinhalt im Bereich ist
    BOOL                    bSelected   : 1; //fuer Multiselektion erst markieren, dann mit der TreeListBox arbeiten!
    ::com::sun::star::uno::Sequence <sal_Int8 >     aTempPasswd;
public:
    SectRepr(USHORT nPos, SwSection& rSect);
    int     operator ==(SectRepr& rSectRef) const
            {return nArrPos==rSectRef.GetArrPos();}

    int     operator <(SectRepr& rSectRef) const
            {return nArrPos<rSectRef.GetArrPos();}

    SwSection&          GetSection()        { return aSection; }
    SwFmtCol&           GetCol()            { return aCol; }
    SvxBrushItem&       GetBackground()     { return aBrush; }
    SwFmtFtnAtTxtEnd&   GetFtnNtAtEnd()     { return aFtnNtAtEnd; }
    SwFmtEndAtTxtEnd&   GetEndNtAtEnd()     { return aEndNtAtEnd; }
    SwFmtNoBalancedColumns& GetBalance()        { return aBalance; }
    SvxFrameDirectionItem&  GetFrmDir()         { return aFrmDirItem; }
    SvxLRSpaceItem&         GetLRSpace()        { return aLRSpaceItem; }

    USHORT              GetArrPos() const {return nArrPos;}
    const String&       GetCondition() const {return aSection.GetCondition();}
    const String&       GetName() const { return aSection.GetName(); }
    String              GetFile() const;
    String              GetSubRegion() const;
    void                SetFile( const String& rFile );
    void                SetFilter( const String& rFilter );
    void                SetSubRegion( const String& rSubRegion );

    void                SetFilePasswd( const String& rPasswd )
                        { aSection.SetLinkFilePassWd( rPasswd ); }
    void                SetCondition( const String& rString )
                        {aSection.SetCondition( rString);}
    int                 IsCondHidden()const
                        {return aSection.IsCondHidden();}
    int                 IsHidden()const
                        {return aSection.IsHidden();}
    int                 IsProtect()const
                        {return aSection.IsProtect();}
    // --> FME 2004-06-22 #114856# edit in readonly sections
    int                 IsEditInReadonly()const
                        {return aSection.IsEditInReadonly();}
    void                SetEditInReadonly(int bFlag = TRUE)
                        {aSection.SetEditInReadonly(bFlag);}
    // <--
    void                SetHidden(int bFlag = TRUE)
                        {aSection.SetHidden(bFlag);}
    void                SetCondHidden(int bFlag = TRUE)
                        {aSection.SetCondHidden(bFlag);}
    void                SetProtect(int bFlag = TRUE)
                        {aSection.SetProtect(bFlag);}
    BOOL                IsContent(){return bContent;}
    void                SetContent(BOOL bValue){bContent = bValue;}
    void                SetSectionType(SectionType eSectionType) {aSection.SetType(eSectionType);}
    SectionType         GetSectionType(){return aSection.GetType();}

    void                SetSelected(){bSelected = TRUE;}
    BOOL                IsSelected() const {return bSelected;}


    const ::com::sun::star::uno::Sequence <sal_Int8 >& GetPasswd() const {return aSection.GetPasswd();}
    ::com::sun::star::uno::Sequence <sal_Int8 >&    GetTempPasswd() {return aTempPasswd;}
    void                                            SetTempPasswd(const ::com::sun::star::uno::Sequence <sal_Int8 >& aPasswd)    {aTempPasswd = aPasswd;}
};

/*************************************************************************
    Dialog "Bereiche bearbeiten"
*************************************************************************/

typedef SectRepr* SectReprPtr;
SV_DECL_PTRARR_SORT( SectReprArr, SectReprPtr, 0, 4 )

class SwEditRegionDlg : public SfxModalDialog
{
    FixedLine       aNameFL;
    Edit            aCurName;
    SvTreeListBox   aTree;

    FixedLine       aLinkFL;
    TriStateBox     aFileCB;
#ifdef DDE_AVAILABLE
    CheckBox        aDDECB;
#endif
    FixedText       aFileNameFT;
#ifdef DDE_AVAILABLE
    FixedText       aDDECommandFT;
#endif
    Edit            aFileNameED;
    PushButton      aFilePB;
    ComboBox        aSubRegionED;
    FixedText       aSubRegionFT;

    FixedLine       aProtectFL;
    TriStateBox     aProtectCB;
    CheckBox        aPasswdCB;
    PushButton      aPasswdPB;

    FixedLine       aHideFL;
    TriStateBox     aHideCB;
    FixedText       aConditionFT;
    ConditionEdit   aConditionED;

    // --> FME 2004-06-22 #114856# edit in readonly sections
    FixedLine       aPropertiesFL;
    TriStateBox     aEditInReadonlyCB;
    // <--

    OKButton        aOK;
    CancelButton    aCancel;
    PushButton      aOptionsPB;
    PushButton      aDismiss;
    HelpButton      aHelp;
    ImageList       aImageIL;
    ImageList       aImageILH;

    SwWrtShell&             rSh;
    SectReprArr             aSectReprArr;
    SvLBoxEntry*            pAktEntry;
    const SwSection*        pCurrSect;
    sfx2::DocumentInserter* pDocInserter;
    Window*                 pOldDefDlgParent;

    BOOL            bDontCheckPasswd :1;
    BOOL            bWeb            :1;


    Image  BuildBitmap(BOOL bProtect,BOOL bHidden,BOOL bHighContrast);

    void    RecurseList( const SwSectionFmt* pFmt, SvLBoxEntry* pEntry);
    USHORT  FindArrPos(const SwSectionFmt* pFmt);

    DECL_LINK( GetFirstEntryHdl, SvTreeListBox * );
    DECL_LINK( DeselectHdl, SvTreeListBox * );

    DECL_LINK( OkHdl, CheckBox * );
    DECL_LINK( NameEditHdl, Edit * );
    DECL_LINK( ConditionEditHdl, Edit * );

    DECL_LINK( ChangePasswdHdl, Button * );
    DECL_LINK( ChangeProtectHdl, TriStateBox * );
    DECL_LINK( ChangeHideHdl, TriStateBox * );
    // --> FME 2004-06-22 #114856# edit in readonly sections
    DECL_LINK( ChangeEditInReadonlyHdl, TriStateBox * );
    // <--
    DECL_LINK( ChangeDismissHdl, CheckBox * );
    DECL_LINK( UseFileHdl, CheckBox* );
    DECL_LINK( FileSearchHdl, PushButton* );
    DECL_LINK( OptionsHdl, PushButton* );
    DECL_LINK( FileNameHdl, Edit* );
#ifdef DDE_AVAILABLE
    DECL_LINK( DDEHdl, CheckBox* );
#endif
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper* );

    BOOL CheckPasswd(CheckBox* pBox = 0);

public:
    SwEditRegionDlg( Window* pParent, SwWrtShell& rWrtSh );
    virtual ~SwEditRegionDlg();

    void    SelectSection(const String& rSectionName);

};
/*************************************************************************
    Dialog "Bereich einfuegen"
*************************************************************************/

/* -----------------21.05.99 11:05-------------------
 *
 * --------------------------------------------------*/
class SwInsertSectionTabPage : public SfxTabPage
{
    FixedLine       aNameFL;
    ComboBox        aCurName;

    FixedLine       aLinkFL;
    CheckBox        aFileCB;
#ifdef DDE_AVAILABLE
    CheckBox        aDDECB;
    FixedText       aDDECommandFT;
#endif
    FixedText       aFileNameFT;
    Edit            aFileNameED;
    PushButton      aFilePB;
    FixedText       aSubRegionFT;
    ComboBox        aSubRegionED;

    FixedLine       aProtectFL;
    CheckBox        aProtectCB;
    CheckBox        aPasswdCB;
    PushButton      aPasswdPB;

    FixedLine       aHideFL;
    CheckBox        aHideCB;
    FixedText       aConditionFT;
    ConditionEdit   aConditionED;

    // --> FME 2004-06-22 #114856# edit in readonly sections
    FixedLine       aPropertiesFL;
    CheckBox        aEditInReadonlyCB;
    // <--

    String          sSection;
    String          sFileName;
    String          sFilterName;
    String          sFilePasswd;

//  SwFmtCol*       pCols;
    ::com::sun::star::uno::Sequence <sal_Int8 > aNewPasswd;
    SwWrtShell*     pWrtSh;
    sfx2::DocumentInserter* pDocInserter;
    Window*                 pOldDefDlgParent;

    DECL_LINK( ChangeHideHdl, CheckBox * );
    // --> FME 2004-06-22 #114856# edit in readonly sections
    DECL_LINK( ChangeEditInReadonlyHdl, CheckBox * );
    // <--
    DECL_LINK( ChangeProtectHdl, CheckBox * );
    DECL_LINK( ChangePasswdHdl, Button * );
    DECL_LINK( NameEditHdl, Edit * );
    DECL_LINK( UseFileHdl, CheckBox* );
    DECL_LINK( FileSearchHdl, PushButton* );
#ifdef DDE_AVAILABLE
    DECL_LINK( DDEHdl, CheckBox* );
#endif
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper* );

    void            FillList(  const SwSectionFmt* pFmt = 0 );

public:
    SwInsertSectionTabPage(Window *pParent, const SfxItemSet &rAttrSet);
    virtual ~SwInsertSectionTabPage();

    void    SetWrtShell(SwWrtShell& rSh);

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
};

class SwSectionFtnEndTabPage : public SfxTabPage
{
    FixedLine       aFtnFL;
    CheckBox        aFtnNtAtTextEndCB;

    CheckBox        aFtnNtNumCB;
    FixedText       aFtnOffsetLbl;
    NumericField    aFtnOffsetFld;

    CheckBox        aFtnNtNumFmtCB;
    FixedText       aFtnPrefixFT;
    Edit            aFtnPrefixED;
    SwNumberingTypeListBox   aFtnNumViewBox;
    FixedText       aFtnSuffixFT;
    Edit            aFtnSuffixED;

    FixedLine       aEndFL;
    CheckBox        aEndNtAtTextEndCB;

    CheckBox        aEndNtNumCB;
    FixedText       aEndOffsetLbl;
    NumericField    aEndOffsetFld;

    CheckBox        aEndNtNumFmtCB;
    FixedText       aEndPrefixFT;
    Edit            aEndPrefixED;
    SwNumberingTypeListBox   aEndNumViewBox;
    FixedText       aEndSuffixFT;
    Edit            aEndSuffixED;


    DECL_LINK( FootEndHdl, CheckBox * );
    void ResetState( BOOL bFtn, const SwFmtFtnEndAtTxtEnd& );

public:
    SwSectionFtnEndTabPage( Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionFtnEndTabPage();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
};
/* -----------------13.06.2003 09:51-----------------

 --------------------------------------------------*/
class SwSectionIndentTabPage : public SfxTabPage
{
    FixedLine           aIndentFL;
    FixedText           aBeforeFT;
    MetricField         aBeforeMF;
    FixedText           aAfterFT;
    MetricField         aAfterMF;

    SvxParaPrevWindow   aPreviewWin;

    DECL_LINK(IndentModifyHdl, MetricField*);
public:
    SwSectionIndentTabPage( Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionIndentTabPage();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void    SetWrtShell(SwWrtShell& rSh);
};

/* -----------------21.05.99 13:07-------------------
 *
 * --------------------------------------------------*/
class SwInsertSectionTabDialog : public SfxTabDialog
{
    SwWrtShell&     rWrtSh;
    SwSection*      pToInsertSection;
protected:
    virtual void    PageCreated( USHORT nId, SfxTabPage &rPage );
    virtual short   Ok();
public:
    SwInsertSectionTabDialog(Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwInsertSectionTabDialog();

    void        SetSection(const SwSection& rSect);
    SwSection*  GetSection() { return pToInsertSection;}
};

/* -----------------21.05.99 13:07-------------------
 *
 * --------------------------------------------------*/
class SwSectionPropertyTabDialog : public SfxTabDialog
{
    SwWrtShell& rWrtSh;
protected:
    virtual void    PageCreated( USHORT nId, SfxTabPage &rPage );
public:
    SwSectionPropertyTabDialog(Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwSectionPropertyTabDialog();
};

#endif


