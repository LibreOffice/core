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

#ifndef _BIB_GENERAL_HXX
#define _BIB_GENERAL_HXX

#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <svtools/stdctrl.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/combobox.hxx>
#include <vcl/scrbar.hxx>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include "bibshortcuthandler.hxx"


class BibDataManager;
#define TYPE_COUNT 22
#define FIELD_COUNT 31

typedef cppu::WeakAggImplHelper1 < ::com::sun::star::awt::XFocusListener > BibGeneralPageBaseClass;

class BibGeneralPage: public BibGeneralPageBaseClass, public BibTabPage
{
    Window              aControlParentWin;
    FixedText           aIdentifierFT;
    FixedText           aAuthTypeFT;
    FixedText           aYearFT;

    FixedText           aAuthorFT;
    FixedText           aTitleFT;

    FixedText           aPublisherFT;
    FixedText           aAddressFT;
    FixedText           aISBNFT;

    FixedText           aChapterFT;
    FixedText           aPagesFT;
    FixedLine           aFirstFL;

    FixedText           aEditorFT;
    FixedText           aEditionFT;

    FixedText           aBooktitleFT;
    FixedText           aVolumeFT;
    FixedText           aHowpublishedFT;

    FixedText           aOrganizationsFT;
    FixedText           aInstitutionFT;
    FixedText           aSchoolFT;

    FixedText           aReportTypeFT;
    FixedText           aMonthFT;
    FixedLine           aSecondFL;

    FixedText           aJournalFT;
    FixedText           aNumberFT;
    FixedText           aSeriesFT;

    FixedText           aAnnoteFT;
    FixedText           aNoteFT;
    FixedText           aURLFT;
    FixedLine           aThirdFL;

    FixedText           aCustom1FT;
    FixedText           aCustom2FT;
    FixedText           aCustom3FT;
    FixedText           aCustom4FT;
    FixedText           aCustom5FT;

    ScrollBar           aHoriScroll;
    ScrollBar           aVertScroll;

    FixedText*          aFixedTexts[ FIELD_COUNT ];
    sal_Int16           nFT2CtrlMap[ FIELD_COUNT ];

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                        aControls[ FIELD_COUNT ];

    Size                aStdSize;
    Point               aBasePos;

    String              aBibTypeArr[ TYPE_COUNT ];
    String              sErrorPrefix;
    String              sTableErrorString;

    String              sTypeColumnName;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                        xCtrlContnr;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        xMgr;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >
                        xCurrentBoundComponent;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >
                        xLBModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >
                        xPosListener;


    BibDataManager*     pDatMan;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                                AddXControl( const String& rName, Point aPos, Size aSize, const rtl::OString& sHelpId,
                                            sal_Int16& rIndex );

    void                        AddControlWithError( const rtl::OUString& rColumnName, const Point& rPos,
                                            const Size& rSize, String& rErrorString, String aColumnUIName,
                                            const rtl::OString& sHelpId, sal_uInt16 nIndexInFTArray );

    void                        AdjustScrollbars();

    DECL_LINK( ScrollHdl, ScrollBar* );

protected:
    virtual void                Resize();
    void                        InitFixedTexts( void );     // create mnemonics and set text an all fixed texts

public:
                                BibGeneralPage(Window* pParent, BibDataManager* pDatMan);
    virtual                     ~BibGeneralPage();

    inline const String&        GetErrorString() const;

    inline const ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >&
                                GetTypeListBoxModel() const;
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >&
                                GetControlContainer() const;

    inline BibDataManager*      GetDataManager();

    void                        CommitActiveControl();

    virtual void SAL_CALL       disposing( const ::com::sun::star::lang::EventObject& Source ) throw( com::sun::star::uno::RuntimeException );

    void SAL_CALL               focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw( com::sun::star::uno::RuntimeException );
    void SAL_CALL               focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw( com::sun::star::uno::RuntimeException );

    void                        RemoveListeners();

    virtual void                GetFocus();

    virtual sal_Bool                HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled
};


inline const String& BibGeneralPage::GetErrorString() const
{
    return sTableErrorString;
}

inline const ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >&
    BibGeneralPage::GetTypeListBoxModel() const
{
    return xLBModel;
}

inline const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >&
    BibGeneralPage::GetControlContainer() const
{
    return xCtrlContnr;
}

inline BibDataManager* BibGeneralPage::GetDataManager()
{
    return pDatMan;
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
