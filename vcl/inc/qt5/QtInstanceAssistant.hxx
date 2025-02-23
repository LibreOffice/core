/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceDialog.hxx"

#include <vcl/weldutils.hxx>

#include <QtWidgets/QWizard>

/*
 * weld::Assistant implementation using a QWizard widget.
 *
 * The weld::Assistant API uses two ways to refer to specific pages:
 * 1) an int page index
 * 2) an OUString identifier
 *
 * Qt's QWizard widget used by this class uses an int id to refer to pages.
 * This concept of an id does not match the index concept that weld::Assistant uses.
 * (The id generally does not match an index in the range between 0 and the number of pages - 1.)
 *
 * Instead, 1) is mapped to a custom QObject property and for 2), the QObject::objectName
 * of the QWizardPage objects is used.
 */
class QtInstanceAssistant : public QtInstanceDialog, public virtual weld::Assistant
{
    Q_OBJECT

    QWizard* m_pWizard;

    std::vector<std::unique_ptr<QtInstanceContainer>> m_aPages;

public:
    QtInstanceAssistant(QWizard* pWizard);

    virtual int get_current_page() const override;
    virtual int get_n_pages() const override;
    virtual OUString get_page_ident(int nPage) const override;
    virtual OUString get_current_page_ident() const override;
    virtual void set_current_page(int nPage) override;
    virtual void set_current_page(const OUString& rIdent) override;
    virtual void set_page_index(const OUString& rIdent, int nIndex) override;
    virtual void set_page_title(const OUString& rIdent, const OUString& rTitle) override;
    virtual OUString get_page_title(const OUString& rIdent) const override;
    virtual void set_page_sensitive(const OUString& rIdent, bool bSensitive) override;
    virtual weld::Container* append_page(const OUString& rIdent) override;

    virtual void set_page_side_help_id(const OUString& rHelpId) override;

    virtual void set_page_side_image(const OUString& rImage) override;

private:
    QWizardPage* page(const OUString& rIdent) const;
    QWizardPage* page(int nPageIndex) const;

    // get/set the page index as used in the weld::Assistant API
    static int pageIndex(QWizardPage& rPage);
    static void setPageIndex(QWizardPage& rPage, int nIndex);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
