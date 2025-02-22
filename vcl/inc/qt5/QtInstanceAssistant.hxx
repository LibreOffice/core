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
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
