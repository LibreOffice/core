/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swuilabimp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:15:51 $
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
#ifndef _SWUILABIMP_HXX
#define _SWUILABIMP_HXX
#include "labimp.hxx"

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
    FixedLine     aWritingFL;

    RadioButton   aContButton;
    RadioButton   aSheetButton;
    FixedText     aMakeText;
    ListBox       aMakeBox;
    FixedText     aTypeText;
    ListBox       aTypeBox;
    ListBox       aHiddenSortTypeBox;
    FixedInfo     aFormatInfo;
    FixedLine     aFormatFL;

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

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    using Window::GetParent;

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

    FixedLine       aContentFL;

    Window          aExampleWIN;

    String          sVisCardGroup;
    String          sTempURL;

    SwLabItem       aLabItem;

    SwOneExampleFrame*  pExampleFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    _xAutoText;


    DECL_LINK( AutoTextSelectHdl, void* );
    DECL_LINK( FrameControlInitializedHdl, void* );

    void            InitFrameControl();
    void            UpdateFields();

    void            ClearUserData();

    using SfxTabPage::SetUserData;
    void            SetUserData( sal_uInt32 nCnt,
                                    const rtl::OUString* pNames,
                                    const rtl::OUString* pValues );

    SwVisitingCardPage(Window* pParent, const SfxItemSet& rSet);
    ~SwVisitingCardPage();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

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
    FixedLine       aDataFL;

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

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

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
    FixedLine       aDataFL;
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
    FixedText       aFaxFT;
    Edit            aFaxED;

    FixedText       aWWWMailFT;
    Edit            aHomePageED;
    Edit            aMailED;

    SwBusinessDataPage(Window* pParent, const SfxItemSet& rSet);
    ~SwBusinessDataPage();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

#endif

