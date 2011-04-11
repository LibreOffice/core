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
#ifndef _SWUI_IDXMRK_HXX
#define _SWUI_IDXMRK_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <sfx2/basedlgs.hxx>

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <svx/stddlg.hxx>

#include <vcl/field.hxx>

#include <vcl/group.hxx>

#include <vcl/button.hxx>

#include <vcl/button.hxx>
#include <sfx2/childwin.hxx>
#include "toxe.hxx"
#include <svtools/stdctrl.hxx>
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>

class SwWrtShell;
class SwTOXMgr;
class SwTOXMark;
/*--------------------------------------------------------------------
     Beschreibung:  Markierung fuer Verzeichniseintrag einfuegen
 --------------------------------------------------------------------*/
class SwIndexMarkFloatDlg;
class SwIndexMarkModalDlg;

class SwIndexMarkDlg : public Window
{
    friend class SwIndexMarkFloatDlg;
    friend class SwIndexMarkModalDlg;
    FixedLine       aIndexFL;
    FixedText       aTypeFT;
    ListBox         aTypeDCB;
    ImageButton     aNewBT;

    FixedText       aEntryFT;
    Edit            aEntryED;
    FixedText       aPhoneticFT0;
    Edit            aPhoneticED0;

    FixedText       aKeyFT;
    ComboBox        aKeyDCB;
    FixedText       aPhoneticFT1;
    Edit            aPhoneticED1;

    FixedText       aKey2FT;
    ComboBox        aKey2DCB;
    FixedText       aPhoneticFT2;
    Edit            aPhoneticED2;

    FixedText       aLevelFT;
    NumericField    aLevelED;
     CheckBox       aMainEntryCB;
     CheckBox       aApplyToAllCB;
     CheckBox       aSearchCaseSensitiveCB;
     CheckBox       aSearchCaseWordOnlyCB;


    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;
    PushButton      aDelBT;
    //PushButton        aNewBT;

    ImageButton     aPrevSameBT;
    ImageButton     aNextSameBT;
    ImageButton     aPrevBT;
    ImageButton     aNextBT;

    String          aOrgStr;
    sal_Int32       nOptionsId;
    sal_Bool            bDel;
    sal_Bool            bNewMark;
    sal_Bool            bSelected;

    sal_Bool            bPhoneticED0_ChangedByUser;
    sal_Bool            bPhoneticED1_ChangedByUser;
    sal_Bool            bPhoneticED2_ChangedByUser;
    LanguageType    nLangForPhoneticReading; //Language of current text used for phonetic reading proposal
    sal_Bool            bIsPhoneticReadingEnabled; //this value states wether phopentic reading is enabled in principle dependend of global cjk settings and language of current entry
    com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedIndexEntrySupplier >
                    xExtendedIndexEntrySupplier;

    SwTOXMgr*       pTOXMgr;
    SwWrtShell*     pSh;

    void            Apply();
    void            InitControls();
    void            InsertMark();
    void            UpdateMark();

    DECL_LINK( InsertHdl, Button * );
    DECL_LINK( CloseHdl, Button * );
    DECL_LINK( DelHdl, Button * );
    DECL_LINK( NextHdl, Button * );
    DECL_LINK( NextSameHdl, Button * );
    DECL_LINK( PrevHdl, Button * );
    DECL_LINK( PrevSameHdl, Button * );
    DECL_LINK( ModifyHdl, ListBox* pBox = 0 );
    DECL_LINK( KeyDCBModifyHdl, ComboBox * );
    DECL_LINK( NewUserIdxHdl, Button*);
    DECL_LINK( SearchTypeHdl, CheckBox*);
    DECL_LINK( PhoneticEDModifyHdl, Edit * );

    //this method updates the values from 'nLangForPhoneticReading' and 'bIsPhoneticReadingEnabled'
    //it needs to be called ones if this dialog is opened to create a new entry (in InitControls),
    //or otherwise it has to be called for each changed TOXMark (in UpdateDialog)
    void            UpdateLanguageDependenciesForPhoneticReading();
    String          GetDefaultPhoneticReading( const String& rText );

    void            UpdateKeyBoxes();

    void            UpdateDialog();
    void            InsertUpdate();

    virtual void    Activate();

public:

    SwIndexMarkDlg( Window *pParent,
                       sal_Bool bNewDlg,
                    const ResId& rResId,
                    sal_Int32 _nOptionsId,
                    SwWrtShell& rWrtShell );


    ~SwIndexMarkDlg();

    void    ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark* pCurTOXMark = 0);
    sal_Bool    IsTOXType(const String& rName)
                {return LISTBOX_ENTRY_NOTFOUND != aTypeDCB.GetEntryPos(rName);}
};

class SwIndexMarkFloatDlg : public SfxModelessDialog
{
    SwIndexMarkDlg      aDlg;
    virtual void    Activate();
    public:
        SwIndexMarkFloatDlg(    SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                SfxChildWinInfo* pInfo,
                                   sal_Bool bNew=sal_True);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwIndexMarkModalDlg : public SvxStandardDialog
{
    SwIndexMarkDlg      aDlg;
public:
    SwIndexMarkModalDlg(Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark);

    virtual void        Apply();
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkModalDlg;

class SwAuthMarkDlg : public Window
{
    static sal_Bool     bIsFromComponent;

    friend class SwAuthMarkModalDlg;
    friend class SwAuthMarkFloatDlg;

    RadioButton     aFromComponentRB;
    RadioButton     aFromDocContentRB;
    FixedText       aAuthorFT;
    FixedInfo       aAuthorFI;
    FixedText       aTitleFT;
    FixedInfo       aTitleFI;
    FixedText       aEntryFT;
    Edit            aEntryED;
    ListBox         aEntryLB;

    FixedLine       aEntryFL;

    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;
    PushButton      aCreateEntryPB;
    PushButton      aEditEntryPB;

    String          sChangeST;
    sal_Bool            bNewEntry;
    sal_Bool            bBibAccessInitialized;

    SwWrtShell*     pSh;

    String          m_sColumnTitles[AUTH_FIELD_END];
    String          m_sFields[AUTH_FIELD_END];

    String          m_sCreatedEntry[AUTH_FIELD_END];

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xBibAccess;

    DECL_LINK(InsertHdl, PushButton*);
    DECL_LINK(CloseHdl, PushButton*);
    DECL_LINK(CreateEntryHdl, PushButton*);
    DECL_LINK(CompEntryHdl, ListBox*);
    DECL_LINK(ChangeSourceHdl, RadioButton*);
    DECL_LINK(IsEntryAllowedHdl, Edit*);
    DECL_LINK(EditModifyHdl, Edit*);

    void InitControls();
    virtual void    Activate();
public:

    SwAuthMarkDlg( Window *pParent,
                    const ResId& rResId,
                       sal_Bool bNew=sal_True);
    ~SwAuthMarkDlg();

    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkFloatDlg : public SfxModelessDialog
{
    SwAuthMarkDlg       aDlg;
    virtual void    Activate();
    public:
        SwAuthMarkFloatDlg(     SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                SfxChildWinInfo* pInfo,
                                   sal_Bool bNew=sal_True);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkModalDlg : public SvxStandardDialog
{
    SwAuthMarkDlg       aDlg;
public:
    SwAuthMarkModalDlg(Window *pParent, SwWrtShell& rSh);

    virtual void        Apply();
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

#endif // _SWUI_IDXMRK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
