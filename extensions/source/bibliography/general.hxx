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

#pragma once

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

class ChangeListener;

class BibGeneralPage : public InterimItemWindow
                     , public BibShortCutHandler
{
    std::unique_ptr<weld::ScrolledWindow> xScrolledWindow;
    std::unique_ptr<weld::Widget> xGrid;

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
    std::unique_ptr<weld::Entry> xChapterED;
    std::unique_ptr<weld::Label> xPagesFT;
    std::unique_ptr<weld::Entry> xPagesED;

    std::unique_ptr<weld::Label> xEditorFT;
    std::unique_ptr<weld::Entry> xEditorED;
    std::unique_ptr<weld::Label> xEditionFT;
    std::unique_ptr<weld::Entry> xEditionED;

    std::unique_ptr<weld::Label> xBooktitleFT;
    std::unique_ptr<weld::Entry> xBooktitleED;
    std::unique_ptr<weld::Label> xVolumeFT;
    std::unique_ptr<weld::Entry> xVolumeED;
    std::unique_ptr<weld::Label> xHowpublishedFT;
    std::unique_ptr<weld::Entry> xHowpublishedED;

    std::unique_ptr<weld::Label> xOrganizationsFT;
    std::unique_ptr<weld::Entry> xOrganizationsED;
    std::unique_ptr<weld::Label> xInstitutionFT;
    std::unique_ptr<weld::Entry> xInstitutionED;
    std::unique_ptr<weld::Label> xSchoolFT;
    std::unique_ptr<weld::Entry> xSchoolED;

    std::unique_ptr<weld::Label> xReportTypeFT;
    std::unique_ptr<weld::Entry> xReportTypeED;
    std::unique_ptr<weld::Label> xMonthFT;
    std::unique_ptr<weld::Entry> xMonthED;

    std::unique_ptr<weld::Label> xJournalFT;
    std::unique_ptr<weld::Entry> xJournalED;
    std::unique_ptr<weld::Label> xNumberFT;
    std::unique_ptr<weld::Entry> xNumberED;
    std::unique_ptr<weld::Label> xSeriesFT;
    std::unique_ptr<weld::Entry> xSeriesED;

    std::unique_ptr<weld::Label> xAnnoteFT;
    std::unique_ptr<weld::Entry> xAnnoteED;
    std::unique_ptr<weld::Label> xNoteFT;
    std::unique_ptr<weld::Entry> xNoteED;
    std::unique_ptr<weld::Label> xURLFT;
    std::unique_ptr<weld::Entry> xURLED;

    std::unique_ptr<weld::Label> xCustom1FT;
    std::unique_ptr<weld::Entry> xCustom1ED;
    std::unique_ptr<weld::Label> xCustom2FT;
    std::unique_ptr<weld::Entry> xCustom2ED;
    std::unique_ptr<weld::Label> xCustom3FT;
    std::unique_ptr<weld::Entry> xCustom3ED;
    std::unique_ptr<weld::Label> xCustom4FT;
    std::unique_ptr<weld::Entry> xCustom4ED;
    std::unique_ptr<weld::Label> xCustom5FT;
    std::unique_ptr<weld::Entry> xCustom5ED;

    OUString            sTableErrorString;

    std::vector<rtl::Reference<ChangeListener>> maChangeListeners;

    BibDataManager*     pDatMan;

    bool                        AddXControl(const OUString& rName, weld::Entry& rEntry);
    bool                        AddXControl(const OUString& rName, weld::ComboBox& rList);

    template<class Target> void AddControlWithError(const OUString& rColumnName, const OUString& rColumnUIName,
        Target& rWidget, OUString& rErrorString, const OString& rHelpId);

    void SaveChanges();

    DECL_LINK(GainFocusHdl, weld::Widget&, void);

    DECL_LINK(FirstElementKeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(LastElementKeyInputHdl, const KeyEvent&, bool);

public:
                                BibGeneralPage(vcl::Window* pParent, BibDataManager* pDatMan);
    virtual                     ~BibGeneralPage() override;
    virtual void                dispose() override;

    const OUString&      GetErrorString() const
    {
        return sTableErrorString;
    }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
