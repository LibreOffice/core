/*************************************************************************
 *
 *  $RCSfile: regionsw.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
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

class SwWrtShell;
class EditRegionDlg;
class DropEvent;

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
    USHORT                  nArrPos;
    USHORT                  nColumn;
    BOOL                    bContent    : 1; //zeigt an, ob evtl. Textinhalt im Bereich ist
    BOOL                    bSelected   : 1; //fuer Multiselektion erst markieren, dann mit der TreeListBox arbeiten!
    BOOL                    bIsCondition: 1; //

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

    BOOL                IsConditionValid() const {return bIsCondition;}
    void                SetConditionValid(BOOL bSet) { bIsCondition = bSet;}

};

/*************************************************************************
    Dialog "Bereiche bearbeiten"
*************************************************************************/

typedef SectRepr* SectReprPtr;
SV_DECL_PTRARR_SORT( SectReprArr, SectReprPtr, 0, 4 )

class SwEditRegionDlg : public SfxModalDialog
{
    SvTreeListBox   aTree;
    CheckBox        aPasswdCB;
    FixedText       aNameFT;
    Edit            aCurName;
    TriStateBox     aProtectCB;
    TriStateBox     aHideCB;
    TriStateBox     aCondCB;
    ConditionEdit   aConditionED;

    TriStateBox     aFileCB;
#ifdef DDE_AVAILABLE
    CheckBox        aDDECB;
#endif
    FixedText       aFileNameFT;
#ifdef DDE_AVAILABLE
    FixedText       aDDECommandFT;
#endif
    Edit            aFileNameED;
    ComboBox        aSubRegionED;
    FixedText       aSubRegionFT;
    OKButton        aOK;
    CancelButton    aCancel;
    PushButton      aOptionsPB;
    PushButton      aDismiss;
    HelpButton      aHelp;
    PushButton      aFilePB;
    GroupBox        aGroupBoxName;
    GroupBox        aGroupBoxOptions;
    Bitmap          aProtHideBM;
    Bitmap          aProtNoHideBM;
    Bitmap          aNoProtHideBM;
    Bitmap          aNoProtNoHideBM;
    Bitmap          aExpNode;
    Bitmap          aCollNode;

    Bitmap          aBmpArr[4];
    String          aNewPasswd;
    SwWrtShell&     rSh;
    SectReprArr     aSectReprArr;
    SvLBoxEntry*    pAktEntry;
    const SwSection*pCurrSect;

    BOOL            bIsPasswd       :1;
    BOOL            bIsPasswdSet    :1;
    BOOL            bWeb            :1;


    Bitmap&         BuildBitmap(BOOL bProtect,BOOL bHidden)
                    { return aBmpArr[bProtect+(bHidden<<1)]; }

public:
    SwEditRegionDlg( Window* pParent, SwWrtShell& rWrtSh );
    virtual ~SwEditRegionDlg();

    void    RecurseList( const SwSectionFmt* pFmt, SvLBoxEntry* pEntry);
    USHORT  FindArrPos(const SwSectionFmt* pFmt);

    void    SetPassword(const String& rPassWd);

    DECL_LINK( GetFirstEntryHdl, SvTreeListBox * );
    DECL_LINK( DeselectHdl, SvTreeListBox * );

    DECL_LINK( OkHdl, CheckBox * );
    DECL_LINK( NameEditHdl, Edit * );
    DECL_LINK( ConditionEditHdl, Edit * );

    DECL_LINK( ChangePasswdHdl, CheckBox * );
    DECL_LINK( ChangeProtectHdl, TriStateBox * );
    DECL_LINK( ChangeHideHdl, TriStateBox * );
    DECL_LINK( ChangeCondHdl, TriStateBox * );
    DECL_LINK( ChangeDismissHdl, CheckBox * );
    DECL_LINK( UseFileHdl, CheckBox* );
    DECL_LINK( FileSearchHdl, PushButton* );
    DECL_LINK( OptionsHdl, PushButton* );
    DECL_LINK( FileNameHdl, Edit* );
#ifdef DDE_AVAILABLE
    DECL_LINK( DDEHdl, CheckBox* );
#endif
};
/*************************************************************************
    Dialog "Bereich einfuegen"
*************************************************************************/

/* -----------------21.05.99 11:05-------------------
 *
 * --------------------------------------------------*/
class SwInsertSectionTabPage : public SfxTabPage
{
    ComboBox        aCurName;
    CheckBox        aProtectCB;
    CheckBox        aHideCB;
    CheckBox        aCondCB;
    ConditionEdit   aConditionED;
    CheckBox        aFileCB;
#ifdef DDE_AVAILABLE
    CheckBox        aDDECB;
    FixedText       aDDECommandFT;
#endif
    FixedText       aFileNameFT;
    Edit            aFileNameED;
    FixedText       aSubRegionFT;
    ComboBox        aSubRegionED;
    PushButton      aFilePB;
    GroupBox        aGroupBoxName;
    GroupBox        aGroupBoxOptions;
    String          sSection;
    String          sFileName;
    String          sFilterName;
    String          sFilePasswd;

//  SwFmtCol*       pCols;
    const String*   pPasswdSect;
//  SfxRequest*     pRequest;
    SwWrtShell*     pWrtSh;

    DECL_LINK( ChangeHideHdl, CheckBox * );
    DECL_LINK( ChangeProtectHdl, CheckBox * );
    DECL_LINK( ChangeCondHdl, CheckBox * );
    DECL_LINK( NameEditHdl, Edit * );
    DECL_LINK( UseFileHdl, CheckBox* );
    DECL_LINK( FileSearchHdl, PushButton* );
#ifdef DDE_AVAILABLE
    DECL_LINK( DDEHdl, CheckBox* );
#endif

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
    GroupBox        aGroupBoxFtn;
    CheckBox        aFtnNtAtTextEndCB;

    CheckBox        aFtnNtNumCB;
    FixedText       aFtnOffsetLbl;
    NumericField    aFtnOffsetFld;

    CheckBox        aFtnNtNumFmtCB;
    FixedText       aFtnPrefixFT;
    Edit            aFtnPrefixED;
    ListBox         aFtnNumViewBox;
    FixedText       aFtnSuffixFT;
    Edit            aFtnSuffixED;

    GroupBox        aGroupBoxEnd;
    CheckBox        aEndNtAtTextEndCB;

    CheckBox        aEndNtNumCB;
    FixedText       aEndOffsetLbl;
    NumericField    aEndOffsetFld;

    CheckBox        aEndNtNumFmtCB;
    FixedText       aEndPrefixFT;
    Edit            aEndPrefixED;
    ListBox         aEndNumViewBox;
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
protected:
    virtual void    PageCreated( USHORT nId, SfxTabPage &rPage );
public:
    SwSectionPropertyTabDialog(Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwSectionPropertyTabDialog();
};

#endif


