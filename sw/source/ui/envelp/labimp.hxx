/*************************************************************************
 *
 *  $RCSfile: labimp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-09-26 11:55:45 $
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
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
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

#include "label.hxx"
#include "labimg.hxx"

#define GETFLDVAL(rField)         (rField).Denormalize((rField).GetValue(FUNIT_TWIP))
#define SETFLDVAL(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FUNIT_TWIP)

class SwNewDBMgr;

class SwLabRec
{
    long GetLong(const String& rStr, MetricField& rField);

public:
    SwLabRec() {}
    SwLabRec( String& rStr, MetricField& rField );

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

class SwLabPage : public SfxTabPage
{
    SwNewDBMgr*   pNewDBMgr;
    String        sActDBName;
    SwLabItem     aItem;

    FixedText     aWritingText;
    CheckBox      aAddrBox;
    MultiLineEdit aWritingEdit;
    FixedText     aDatabaseFT;
    ListBox       aDatabaseLB;
    FixedText     aTableFT;
    ListBox       aTableLB;
    ImageButton   aInsertBT;
    FixedText     aDBFieldFT;
    ListBox       aDBFieldLB;
//  PushButton    aDatabaseButton;
    GroupBox      aWritingGroup;

    RadioButton   aContButton;
    RadioButton   aSheetButton;
    FixedText     aMakeText;
    ListBox       aMakeBox;
    FixedText     aTypeText;
    ListBox       aTypeBox;
    FixedInfo     aFormatInfo;
    GroupBox      aFormatGroup;

    sal_Bool        m_bLabel;

     SwLabPage(Window* pParent, const SfxItemSet& rSet);
    ~SwLabPage();

    DECL_LINK( AddrHdl, Button * );
    DECL_LINK( DatabaseHdl, ListBox *pListBox );
//    DECL_LINK( DatabaseButtonHdl, Button * );
    DECL_LINK( FieldHdl, Button * );
    DECL_LINK( PageHdl, Button * );
    DECL_LINK( MakeHdl, ListBox * );
    DECL_LINK( TypeHdl, ListBox * );

    void DisplayFormat  ();
    SwLabRec* GetSelectedEntryPos();

public:
    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwLabItem& rItem);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);

    SwLabDlg* GetParent() {return (SwLabDlg*) SfxTabPage::GetParent()->GetParent();}

    void    SetToBusinessCard();

    void InitDatabaseBox();
    inline void SetNewDBMgr(SwNewDBMgr* pDBMgr) { pNewDBMgr = pDBMgr; }
    inline SwNewDBMgr* GetNewDBMgr() const { return pNewDBMgr; }
};

/* -----------------08.07.99 13:48-------------------

 --------------------------------------------------*/
class SwOneExampleFrame;
class SwVisitingCardPage : public SfxTabPage
{
    SvTreeListBox   aAutoTextLB;
    FixedText       aAutoTextGroupFT;
    ListBox         aAutoTextGroupLB;

    GroupBox        aContentGB;

    Window          aExampleWIN;
    GroupBox        aExampleGB;

    String          sVisCardGroup;
    String          sTempURL;

    SwLabItem       aLabItem;

    SwOneExampleFrame*  pExampleFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    _xAutoText;

    DECL_LINK( AutoTextSelectHdl, void* );
    DECL_LINK( FrameControlInitializedHdl, void* );

    void            InitFrameControl();
    void            UpdateFields();

    SwVisitingCardPage(Window* pParent, const SfxItemSet& rSet);
    ~SwVisitingCardPage();

public:
    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};
/* -----------------29.09.99 08:51-------------------

 --------------------------------------------------*/
class SwPrivateDataPage : public SfxTabPage
{
    GroupBox        aDataGB;

    FixedText       aNameFT;
    Edit            aFirstNameED;
    Edit            aNameED;
    Edit            aShortCutED;

    FixedText       aName2FT;
    Edit            aFirstName2ED;
    Edit            aName2ED;
    Edit            aShortCut2ED;

    FixedText       aStreetFT;
    Edit            aStreetED;
    FixedText       aZipCityFT;
    Edit            aZipED;
    Edit            aCityED;
    FixedText       aCountryStateFT;
    Edit            aCountryED;
    Edit            aStateED;
    FixedText       aTitleProfessionFT;
    Edit            aTitleED;
    Edit            aProfessionED;
    FixedText       aPhoneFT;
    Edit            aPhoneED;
    Edit            aMobilePhoneED;
    FixedText       aFaxFT;
    Edit            aFaxED;
    FixedText       aWWWMailFT;
    Edit            aHomePageED;
    Edit            aMailED;

    SwPrivateDataPage(Window* pParent, const SfxItemSet& rSet);
    ~SwPrivateDataPage();

public:
    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};
/* -----------------29.09.99 08:51-------------------

 --------------------------------------------------*/
class SwBusinessDataPage : public SfxTabPage
{
    GroupBox        aDataGB;
    FixedText       aCompanyFT;
    Edit            aCompanyED;
    FixedText       aCompanyExtFT;
    Edit            aCompanyExtED;
    FixedText       aSloganFT;
    Edit            aSloganED;

    FixedText       aStreetFT;
    Edit            aStreetED;
    FixedText       aZipCityFT;
    Edit            aZipED;
    Edit            aCityED;
    FixedText       aCountryStateFT;
    Edit            aCountryED;
    Edit            aStateED;

    FixedText       aPositionFT;
    Edit            aPositionED;

    FixedText       aPhoneFT;
    Edit            aPhoneED;
    Edit            aMobilePhoneED;
    Edit            aFaxED;

    FixedText       aWWWMailFT;
    Edit            aHomePageED;
    Edit            aMailED;

    SwBusinessDataPage(Window* pParent, const SfxItemSet& rSet);
    ~SwBusinessDataPage();

public:
    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

#endif

