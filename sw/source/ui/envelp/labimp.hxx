/*************************************************************************
 *
 *  $RCSfile: labimp.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:24:24 $
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
#ifndef _LABIMP_HXX
#define _LABIMP_HXX


#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XWORDCURSOR_HPP_
#include <com/sun/star/text/XWordCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XPARAGRAPHCURSOR_HPP_
#include <com/sun/star/text/XParagraphCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XPAGECURSOR_HPP_
#include <com/sun/star/text/XPageCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XSENTENCECURSOR_HPP_
#include <com/sun/star/text/XSentenceCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFILEDIALOG_HPP_
#include <com/sun/star/awt/XFileDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLISTBOX_HPP_
#include <com/sun/star/awt/XListBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPROGRESSMONITOR_HPP_
#include <com/sun/star/awt/XProgressMonitor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSCROLLBAR_HPP_
#include <com/sun/star/awt/XScrollBar.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLCONTAINERPEER_HPP_
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMESSAGEBOX_HPP_
#include <com/sun/star/awt/XMessageBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTEDITFIELD_HPP_
#include <com/sun/star/awt/XTextEditField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_STYLE_HPP_
#include <com/sun/star/awt/Style.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTIMEFIELD_HPP_
#include <com/sun/star/awt/XTimeField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSPINFIELD_HPP_
#include <com/sun/star/awt/XSpinField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XUNOCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XNUMERICFIELD_HPP_
#include <com/sun/star/awt/XNumericField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBUTTON_HPP_
#include <com/sun/star/awt/XButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTAREA_HPP_
#include <com/sun/star/awt/XTextArea.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XIMAGEBUTTON_HPP_
#include <com/sun/star/awt/XImageButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFIXEDTEXT_HPP_
#include <com/sun/star/awt/XFixedText.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XRADIOBUTTON_HPP_
#include <com/sun/star/awt/XRadioButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCURRENCYFIELD_HPP_
#include <com/sun/star/awt/XCurrencyField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPATTERNFIELD_HPP_
#include <com/sun/star/awt/XPatternField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLER_HPP_
#include <com/sun/star/awt/XTabController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLCONTAINER_HPP_
#include <com/sun/star/awt/XVclContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDATEFIELD_HPP_
#include <com/sun/star/awt/XDateField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCOMBOBOX_HPP_
#include <com/sun/star/awt/XComboBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCHECKBOX_HPP_
#include <com/sun/star/awt/XCheckBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MESSAGEBOXCOMMAND_HPP_
#include <com/sun/star/awt/MessageBoxCommand.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPROGRESSBAR_HPP_
#include <com/sun/star/awt/XProgressBar.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _LABEL_HXX
#include <label.hxx>
#endif
#ifndef _LABIMG_HXX
#include <labimg.hxx>
#endif

#define GETFLDVAL(rField)         (rField).Denormalize((rField).GetValue(FUNIT_TWIP))
#define SETFLDVAL(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FUNIT_TWIP)

class SwNewDBMgr;

class SwLabRec
{
public:
    SwLabRec() {}

    void SetFromItem( const SwLabItem& rItem );
    void FillItem( SwLabItem& rItem ) const;

    String          aMake;
    String          aType;
    long            lHDist;
    long            lVDist;
    long            lWidth;
    long            lHeight;
    long            lLeft;
    long            lUpper;
    sal_Int32       nCols;
    sal_Int32       nRows;
    sal_Bool        bCont;
};

/* --------------------------------------------------

 --------------------------------------------------*/
SV_DECL_PTRARR_DEL( SwLabRecs, SwLabRec*, 110, 10 );

//CHINA001 class SwLabPage : public SfxTabPage
//CHINA001 {
//CHINA001 SwNewDBMgr*   pNewDBMgr;
//CHINA001 String        sActDBName;
//CHINA001 SwLabItem      aItem;
//CHINA001
//CHINA001 FixedText      aWritingText;
//CHINA001 CheckBox   aAddrBox;
//CHINA001 MultiLineEdit aWritingEdit;
//CHINA001 FixedText      aDatabaseFT;
//CHINA001 ListBox    aDatabaseLB;
//CHINA001 FixedText      aTableFT;
//CHINA001 ListBox    aTableLB;
//CHINA001 ImageButton   aInsertBT;
//CHINA001 FixedText      aDBFieldFT;
//CHINA001 ListBox    aDBFieldLB;
//CHINA001 //   PushButton    aDatabaseButton;
//CHINA001 FixedLine     aWritingFL;
//CHINA001
//CHINA001 RadioButton   aContButton;
//CHINA001 RadioButton   aSheetButton;
//CHINA001 FixedText      aMakeText;
//CHINA001 ListBox    aMakeBox;
//CHINA001 FixedText      aTypeText;
//CHINA001 ListBox    aTypeBox;
//CHINA001 ListBox       aHiddenSortTypeBox;
//CHINA001 FixedInfo     aFormatInfo;
//CHINA001 FixedLine     aFormatFL;
//CHINA001
//CHINA001 sal_Bool     m_bLabel;
//CHINA001
//CHINA001 SwLabPage(Window* pParent, const SfxItemSet& rSet);
//CHINA001 ~SwLabPage();
//CHINA001
//CHINA001 DECL_LINK( AddrHdl, Button * );
//CHINA001 DECL_LINK( DatabaseHdl, ListBox *pListBox );
//CHINA001 //    DECL_LINK( DatabaseButtonHdl, Button * );
//CHINA001 DECL_LINK( FieldHdl, Button * );
//CHINA001 DECL_LINK( PageHdl, Button * );
//CHINA001 DECL_LINK( MakeHdl, ListBox * );
//CHINA001 DECL_LINK( TypeHdl, ListBox * );
//CHINA001
//CHINA001 void DisplayFormat   ();
//CHINA001 SwLabRec* GetSelectedEntryPos();
//CHINA001
//CHINA001 public:
//CHINA001 static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);
//CHINA001
//CHINA001 virtual void ActivatePage(const SfxItemSet& rSet);
//CHINA001 virtual int  DeactivatePage(SfxItemSet* pSet = 0);
//CHINA001 void FillItem(SwLabItem& rItem);
//CHINA001 virtual sal_Bool FillItemSet(SfxItemSet& rSet);
//CHINA001 virtual void Reset(const SfxItemSet& rSet);
//CHINA001
//CHINA001 SwLabDlg* GetParent() {return (SwLabDlg*) SfxTabPage::GetParent()->GetParent();}
//CHINA001
//CHINA001 void     SetToBusinessCard();
//CHINA001
//CHINA001 void InitDatabaseBox();
//CHINA001 inline void SetNewDBMgr(SwNewDBMgr* pDBMgr) { pNewDBMgr = pDBMgr; }
//CHINA001 inline SwNewDBMgr* GetNewDBMgr() const { return pNewDBMgr; }
//CHINA001 };
//CHINA001
//CHINA001 /* -----------------08.07.99 13:48-------------------
//CHINA001
//CHINA001 --------------------------------------------------*/
//CHINA001 class SwOneExampleFrame;
//CHINA001 class SwVisitingCardPage : public SfxTabPage
//CHINA001 {
//CHINA001 SvTreeListBox    aAutoTextLB;
//CHINA001 FixedText        aAutoTextGroupFT;
//CHINA001 ListBox          aAutoTextGroupLB;
//CHINA001
//CHINA001 FixedLine       aContentFL;
//CHINA001
//CHINA001 Window           aExampleWIN;
//CHINA001
//CHINA001 String           sVisCardGroup;
//CHINA001 String           sTempURL;
//CHINA001
//CHINA001 SwLabItem        aLabItem;
//CHINA001
//CHINA001 SwOneExampleFrame*   pExampleFrame;
//CHINA001 ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     _xAutoText;
//CHINA001
//CHINA001
//CHINA001 DECL_LINK( AutoTextSelectHdl, void* );
//CHINA001 DECL_LINK( FrameControlInitializedHdl, void* );
//CHINA001
//CHINA001 void         InitFrameControl();
//CHINA001 void         UpdateFields();
//CHINA001
//CHINA001 void         ClearUserData();
//CHINA001 void         SetUserData( sal_uInt32 nCnt,
//CHINA001 const rtl::OUString* pNames,
//CHINA001 const rtl::OUString* pValues );
//CHINA001
//CHINA001 SwVisitingCardPage(Window* pParent, const SfxItemSet& rSet);
//CHINA001 ~SwVisitingCardPage();
//CHINA001
//CHINA001 public:
//CHINA001 static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);
//CHINA001
//CHINA001 virtual void ActivatePage(const SfxItemSet& rSet);
//CHINA001 virtual int  DeactivatePage(SfxItemSet* pSet = 0);
//CHINA001 virtual sal_Bool FillItemSet(SfxItemSet& rSet);
//CHINA001 virtual void Reset(const SfxItemSet& rSet);
//CHINA001 };
//CHINA001 /* -----------------29.09.99 08:51-------------------
//CHINA001
//CHINA001 --------------------------------------------------*/
//CHINA001 class SwPrivateDataPage : public SfxTabPage
//CHINA001 {
//CHINA001 FixedLine       aDataFL;
//CHINA001
//CHINA001 FixedText        aNameFT;
//CHINA001 Edit         aFirstNameED;
//CHINA001 Edit         aNameED;
//CHINA001 Edit         aShortCutED;
//CHINA001
//CHINA001 FixedText        aName2FT;
//CHINA001 Edit         aFirstName2ED;
//CHINA001 Edit         aName2ED;
//CHINA001 Edit         aShortCut2ED;
//CHINA001
//CHINA001 FixedText        aStreetFT;
//CHINA001 Edit         aStreetED;
//CHINA001 FixedText        aZipCityFT;
//CHINA001 Edit         aZipED;
//CHINA001 Edit         aCityED;
//CHINA001 FixedText        aCountryStateFT;
//CHINA001 Edit         aCountryED;
//CHINA001 Edit         aStateED;
//CHINA001 FixedText        aTitleProfessionFT;
//CHINA001 Edit         aTitleED;
//CHINA001 Edit         aProfessionED;
//CHINA001 FixedText        aPhoneFT;
//CHINA001 Edit         aPhoneED;
//CHINA001 Edit             aMobilePhoneED;
//CHINA001 FixedText        aFaxFT;
//CHINA001 Edit         aFaxED;
//CHINA001 FixedText        aWWWMailFT;
//CHINA001 Edit         aHomePageED;
//CHINA001 Edit         aMailED;
//CHINA001
//CHINA001 SwPrivateDataPage(Window* pParent, const SfxItemSet& rSet);
//CHINA001 ~SwPrivateDataPage();
//CHINA001
//CHINA001 public:
//CHINA001 static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);
//CHINA001
//CHINA001 virtual void ActivatePage(const SfxItemSet& rSet);
//CHINA001 virtual int  DeactivatePage(SfxItemSet* pSet = 0);
//CHINA001 virtual sal_Bool FillItemSet(SfxItemSet& rSet);
//CHINA001 virtual void Reset(const SfxItemSet& rSet);
//CHINA001 };
//CHINA001 /* -----------------29.09.99 08:51-------------------
//CHINA001
//CHINA001 --------------------------------------------------*/
//CHINA001 class SwBusinessDataPage : public SfxTabPage
//CHINA001 {
//CHINA001 FixedLine       aDataFL;
//CHINA001 FixedText        aCompanyFT;
//CHINA001 Edit         aCompanyED;
//CHINA001 FixedText        aCompanyExtFT;
//CHINA001 Edit         aCompanyExtED;
//CHINA001 FixedText        aSloganFT;
//CHINA001 Edit         aSloganED;
//CHINA001
//CHINA001 FixedText        aStreetFT;
//CHINA001 Edit         aStreetED;
//CHINA001 FixedText        aZipCityFT;
//CHINA001 Edit         aZipED;
//CHINA001 Edit         aCityED;
//CHINA001 FixedText        aCountryStateFT;
//CHINA001 Edit         aCountryED;
//CHINA001 Edit         aStateED;
//CHINA001
//CHINA001 FixedText        aPositionFT;
//CHINA001 Edit         aPositionED;
//CHINA001
//CHINA001 FixedText        aPhoneFT;
//CHINA001 Edit         aPhoneED;
//CHINA001 Edit             aMobilePhoneED;
//CHINA001 FixedText        aFaxFT;
//CHINA001 Edit         aFaxED;
//CHINA001
//CHINA001 FixedText        aWWWMailFT;
//CHINA001 Edit         aHomePageED;
//CHINA001 Edit         aMailED;
//CHINA001
//CHINA001 SwBusinessDataPage(Window* pParent, const SfxItemSet& rSet);
//CHINA001 ~SwBusinessDataPage();
//CHINA001
//CHINA001 public:
//CHINA001 static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);
//CHINA001
//CHINA001 virtual void ActivatePage(const SfxItemSet& rSet);
//CHINA001 virtual int  DeactivatePage(SfxItemSet* pSet = 0);
//CHINA001 virtual sal_Bool FillItemSet(SfxItemSet& rSet);
//CHINA001 virtual void Reset(const SfxItemSet& rSet);
//CHINA001 };

#endif

