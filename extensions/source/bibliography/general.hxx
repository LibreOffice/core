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

#ifndef INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_GENERAL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_GENERAL_HXX

#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <svtools/stdctrl.hxx>

#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/combobox.hxx>
#include <vcl/scrbar.hxx>
#include <cppuhelper/implbase1.hxx>
#include "bibshortcuthandler.hxx"


class BibDataManager;
#define TYPE_COUNT 22
#define FIELD_COUNT 31

/**
  * We need to split off the listener because both it and the vcl::Window baseclass are ref-counted
  */
class BibGeneralPage;
class BibGeneralPageFocusListener : public cppu::WeakAggImplHelper1 < ::com::sun::star::awt::XFocusListener >
{
private:
    VclPtr<BibGeneralPage> mpBibGeneralPage;
public:
    explicit BibGeneralPageFocusListener(BibGeneralPage *pBibGeneralPage);
    virtual void SAL_CALL       focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL       focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL       disposing( const ::com::sun::star::lang::EventObject& Source ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

};

class BibGeneralPage: public BibTabPage
{
    VclPtr<VclGrid>            pGrid;
    VclPtr<VclScrolledWindow>  pScrolledWindow;

    VclPtr<FixedText>          pIdentifierFT;
    VclPtr<FixedText>          pAuthTypeFT;
    VclPtr<FixedText>          pYearFT;

    VclPtr<FixedText>          pAuthorFT;
    VclPtr<FixedText>          pTitleFT;

    VclPtr<FixedText>          pPublisherFT;
    VclPtr<FixedText>          pAddressFT;
    VclPtr<FixedText>          pISBNFT;

    VclPtr<FixedText>          pChapterFT;
    VclPtr<FixedText>          pPagesFT;

    VclPtr<FixedText>          pEditorFT;
    VclPtr<FixedText>          pEditionFT;

    VclPtr<FixedText>          pBooktitleFT;
    VclPtr<FixedText>          pVolumeFT;
    VclPtr<FixedText>          pHowpublishedFT;

    VclPtr<FixedText>          pOrganizationsFT;
    VclPtr<FixedText>          pInstitutionFT;
    VclPtr<FixedText>          pSchoolFT;

    VclPtr<FixedText>          pReportTypeFT;
    VclPtr<FixedText>          pMonthFT;

    VclPtr<FixedText>          pJournalFT;
    VclPtr<FixedText>          pNumberFT;
    VclPtr<FixedText>          pSeriesFT;

    VclPtr<FixedText>          pAnnoteFT;
    VclPtr<FixedText>          pNoteFT;
    VclPtr<FixedText>          pURLFT;

    VclPtr<FixedText>          pCustom1FT;
    VclPtr<FixedText>          pCustom2FT;
    VclPtr<FixedText>          pCustom3FT;
    VclPtr<FixedText>          pCustom4FT;
    VclPtr<FixedText>          pCustom5FT;

    VclPtr<FixedText>          aFixedTexts[ FIELD_COUNT ];
    sal_Int16           nFT2CtrlMap[ FIELD_COUNT ];

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                        aControls[ FIELD_COUNT ];

    OUString            sErrorPrefix;
    OUString            sTableErrorString;

    OUString            sTypeColumnName;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                        xCtrlContnr;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >
                        xCurrentBoundComponent;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >
                        xLBModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >
                        xPosListener;

    rtl::Reference<BibGeneralPageFocusListener> mxBibGeneralPageFocusListener;

    BibDataManager*     pDatMan;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                                AddXControl( const OUString& rName, FixedText& rLabel, const OString& sHelpId,
                                            sal_Int16& rIndex, std::vector<vcl::Window*>& rChildren );

    void                        AddControlWithError( const OUString& rColumnName, FixedText& rLabel,
                                            OUString& rErrorString,
                                            const OString& sHelpId, sal_uInt16 nIndexInFTArray, std::vector<vcl::Window*>& rChildren );

protected:
    void                        InitFixedTexts();     // create mnemonics and set text an all fixed texts

public:
                                BibGeneralPage(vcl::Window* pParent, BibDataManager* pDatMan);
    virtual                     ~BibGeneralPage();
    virtual void                dispose() SAL_OVERRIDE;

    inline const OUString&      GetErrorString() const;

    inline const ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >&
                                GetTypeListBoxModel() const;
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >&
                                GetControlContainer() const;

    inline BibDataManager*      GetDataManager();

    void                        CommitActiveControl();

    void                        RemoveListeners();

    virtual void                GetFocus() SAL_OVERRIDE;

    virtual bool                HandleShortCutKey( const KeyEvent& rKeyEvent ) SAL_OVERRIDE; // returns true, if key was handled

    inline rtl::Reference<BibGeneralPageFocusListener> GetFocusListener() { return mxBibGeneralPageFocusListener; }

    void focusGained(const css::awt::FocusEvent& rEvent) throw( css::uno::RuntimeException, std::exception );
    void focusLost(const css::awt::FocusEvent& rEvent) throw( css::uno::RuntimeException, std::exception );

};


inline const OUString& BibGeneralPage::GetErrorString() const
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
