/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <svx/svxdllapi.h>
#include <tools/link.hxx>
#include <vcl/weld.hxx>

namespace svx
{
/**
 * Interface for the data of a check, which must be extended by the
 * check itself, to show the data in the GenericCheckDialog.
 */
class CheckData
{
public:
    virtual ~CheckData() {}

    /// check entry text
    virtual OUString getText() = 0;

    /// true, if we can show the "Mark" button
    virtual bool canMarkObject() = 0;
    /// executed, when the "Mark" button is hit
    virtual void markObject() = 0;

    /// true, if we can show the "Properties" button
    virtual bool hasProperties() = 0;
    /// executed, when the "Properties" button is hit
    virtual void runProperties() = 0;
};

/**
 * Check data collection contains all the checks in one data structure,
 * and also serves to define additional attributes that are used by the
 * GenericCheckDialog (the title of the check dialog).
 */
class CheckDataCollection
{
protected:
    std::vector<std::unique_ptr<CheckData>> m_aCollection;

public:
    virtual ~CheckDataCollection() {}

    std::vector<std::unique_ptr<CheckData>>& getCollection() { return m_aCollection; }

    virtual OUString getTitle() = 0;
};

/**
 * A UI piece to show check result text and other widgets, which are
 * relevant for the check (various buttons to act on the check result).
 */
class GenericCheckEntry final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Button> m_xMarkButton;
    std::unique_ptr<weld::Button> m_xPropertiesButton;

    std::unique_ptr<CheckData>& m_pCheckData;

public:
    GenericCheckEntry(weld::Container* pParent, std::unique_ptr<CheckData>& rCheckData);

    weld::Widget* get_widget() const { return m_xContainer.get(); }

    DECL_LINK(MarkButtonClicked, weld::Button&, void);
    DECL_LINK(PropertiesButtonClicked, weld::Button&, void);
};

/**
 * This is a generic dialog, which is used to display results of a
 * document checks, like for example image size check.
 */
class SVX_DLLPUBLIC GenericCheckDialog final : public weld::GenericDialogController
{
private:
    std::vector<std::unique_ptr<GenericCheckEntry>> m_aCheckEntries;
    CheckDataCollection& m_rCheckDataCollection;

    std::unique_ptr<weld::Box> m_xCheckBox;

public:
    GenericCheckDialog(weld::Window* pParent, CheckDataCollection& rCheckDataCollection);
    virtual ~GenericCheckDialog() override;
    virtual short run() override;
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
