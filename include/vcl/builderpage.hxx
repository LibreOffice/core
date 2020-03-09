/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_BUILDERPAGE_HXX
#define INCLUDED_VCL_BUILDERPAGE_HXX

#include <vcl/weld.hxx>

class VCL_DLLPUBLIC BuilderPage
{
public:
    BuilderPage(weld::Widget* pParent, weld::DialogController* pController,
                const OUString& rUIXMLDescription, const OString& rID, bool bIsMobile = false);
    virtual ~BuilderPage() COVERITY_NOEXCEPT_FALSE;

    /* The title of the page, in an Assistant the dialog may append this page title to the
       dialog title.

       While in a Dialog hosting a single Page it may use the title as
       the dialog title.
    */
    void SetPageTitle(const OUString& rPageTitle) { m_aPageTitle = rPageTitle; }
    const OUString& GetPageTitle() const { return m_aPageTitle; }

    // In a Notebook or Assistant the controller typically calls Activate on entering
    // this page, and Deactivate on leaving
    virtual void Activate();
    virtual void Deactivate();

    OString GetHelpId() const { return m_xContainer->get_help_id(); }

protected:
    weld::DialogController* m_pDialogController;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;

private:
    OUString m_aPageTitle;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
