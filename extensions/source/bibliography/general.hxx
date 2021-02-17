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

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>

#include <vcl/InterimItemWindow.hxx>
#include <cppuhelper/implbase1.hxx>
#include "bibshortcuthandler.hxx"


class BibDataManager;
#define TYPE_COUNT 22
#define FIELD_COUNT 31

/**
  * We need to split off the listener because both it and the vcl::Window baseclass are ref-counted
  */
class BibGeneralPage;
class BibGeneralPageFocusListener : public cppu::WeakAggImplHelper1 < css::awt::XFocusListener >
{
private:
    VclPtr<BibGeneralPage> mpBibGeneralPage;
public:
    explicit BibGeneralPageFocusListener(BibGeneralPage *pBibGeneralPage);
    virtual void SAL_CALL       focusGained( const css::awt::FocusEvent& e ) override;
    virtual void SAL_CALL       focusLost( const css::awt::FocusEvent& e ) override;
    virtual void SAL_CALL       disposing( const css::lang::EventObject& Source ) override;

};

class BibGeneralPage : public InterimItemWindow
                     , public BibShortCutHandler
{
//TODO    VclPtr<VclGrid>            pGrid;
//TODO    VclPtr<VclScrolledWindow>  pScrolledWindow;

    std::unique_ptr<weld::Label> xIdentifierFT;
    std::unique_ptr<weld::Entry> xIdentifierED;

    std::unique_ptr<weld::Label> xAuthTypeFT;
    std::unique_ptr<weld::ComboBox> xAuthTypeLB;
    std::unique_ptr<weld::Label> xYearFT;
    std::unique_ptr<weld::Entry> xYearED;

    std::unique_ptr<weld::Label> xAuthorFT;
    std::unique_ptr<weld::Entry> xAuthorED;
    std::unique_ptr<weld::Label> xTitleFT;
    std::unique_ptr<weld::Entry> xTitleED;

    std::unique_ptr<weld::Label> xPublisherFT;
    std::unique_ptr<weld::Entry> xPublisherED;
    std::unique_ptr<weld::Label> xAddressFT;
    std::unique_ptr<weld::Entry> xAddressED;
    std::unique_ptr<weld::Label> xISBNFT;
    std::unique_ptr<weld::Entry> xISBNED;

    std::unique_ptr<weld::Label> xChapterFT;
    std::unique_ptr<weld::Label> xPagesFT;

    std::unique_ptr<weld::Label> xEditorFT;
    std::unique_ptr<weld::Label> xEditionFT;

    std::unique_ptr<weld::Label> xBooktitleFT;
    std::unique_ptr<weld::Label> xVolumeFT;
    std::unique_ptr<weld::Label> xHowpublishedFT;

    std::unique_ptr<weld::Label> xOrganizationsFT;
    std::unique_ptr<weld::Label> xInstitutionFT;
    std::unique_ptr<weld::Label> xSchoolFT;

    std::unique_ptr<weld::Label> xReportTypeFT;
    std::unique_ptr<weld::Label> xMonthFT;

    std::unique_ptr<weld::Label> xJournalFT;
    std::unique_ptr<weld::Label> xNumberFT;
    std::unique_ptr<weld::Label> xSeriesFT;

    std::unique_ptr<weld::Label> xAnnoteFT;
    std::unique_ptr<weld::Label> xNoteFT;
    std::unique_ptr<weld::Label> xURLFT;

    std::unique_ptr<weld::Label> xCustom1FT;
    std::unique_ptr<weld::Label> xCustom2FT;
    std::unique_ptr<weld::Label> xCustom3FT;
    std::unique_ptr<weld::Label> xCustom4FT;
    std::unique_ptr<weld::Label> xCustom5FT;

#if 0
    weld::Label* aFixedTexts[FIELD_COUNT];
    sal_Int16           nFT2CtrlMap[ FIELD_COUNT ];

    css::uno::Reference< css::awt::XWindow >
                        aControls[ FIELD_COUNT ];
#endif

    OUString            sTableErrorString;

    OUString            sTypeColumnName;

    css::uno::Reference< css::awt::XControlContainer >
                        xCtrlContnr;

    css::uno::Reference< css::form::XBoundComponent >
                        xLBModel;

    css::uno::Reference< css::sdbc::XRowSetListener >
                        xPosListener;

    rtl::Reference<BibGeneralPageFocusListener> mxBibGeneralPageFocusListener;

    BibDataManager*     pDatMan;

    bool                        AddXControl(const OUString& rName, weld::Entry& rEntry);
    bool                        AddXControl(const OUString& rName, weld::ComboBox& rList);

    template<class Target> void AddControlWithError(const OUString& rColumnName, const OUString& rColumnUIName,
        Target& rWidget, OUString& rErrorString, const OString& rHelpId);

public:
                                BibGeneralPage(vcl::Window* pParent, BibDataManager* pDatMan);
    virtual                     ~BibGeneralPage() override;
    virtual void                dispose() override;

    inline const OUString&      GetErrorString() const;

    inline const css::uno::Reference< css::form::XBoundComponent >&
                                GetTypeListBoxModel() const;
    inline const css::uno::Reference< css::awt::XControlContainer >&
                                GetControlContainer() const;

    inline BibDataManager*      GetDataManager();

    void                        CommitActiveControl();

    void                        RemoveListeners();

    virtual void                GetFocus() override;

    /// @throws css::uno::RuntimeException
    void focusGained(const css::awt::FocusEvent& rEvent);
    /// @throws css::uno::RuntimeException
    void focusLost();

};


inline const OUString& BibGeneralPage::GetErrorString() const
{
    return sTableErrorString;
}

inline const css::uno::Reference< css::form::XBoundComponent >&
    BibGeneralPage::GetTypeListBoxModel() const
{
    return xLBModel;
}

inline const css::uno::Reference< css::awt::XControlContainer >&
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
