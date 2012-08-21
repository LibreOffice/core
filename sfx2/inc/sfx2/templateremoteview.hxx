/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_TEMPLATEREMOTEVIEW_HXX__
#define __SFX2_TEMPLATEREMOTEVIEW_HXX__

#include <sfx2/templateabstractview.hxx>
#include <sfx2/templateproperties.hxx>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

class TemplateRemoteViewItem;

class TemplateRemoteView : public TemplateAbstractView
{
public:

    TemplateRemoteView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren);

    virtual ~TemplateRemoteView ();

    // Load repositories from user settings.
    virtual void Populate ();

    virtual void reload ();

    virtual void filterTemplatesByApp (const FILTER_APPLICATION &eApp);

    virtual void showOverlay (bool bVisible);

    void setOverlayChangeNameHdl (const Link &rLink);

    bool loadRepository (const sal_uInt16 nRepositoryId, bool bRefresh);

    const std::vector<TemplateRemoteViewItem*>& getRepositories () const { return maRepositories; }

    bool insertRepository (const OUString &rName, const OUString &rURL);

    bool deleteRepository (const sal_uInt16 nRepositoryId);

    void syncRepositories () const;

private:

    DECL_LINK(ChangeNameHdl, TemplateView*);

private:

    bool mbIsSynced;
    Link maChangeNameHdl;
    std::vector<TemplateRemoteViewItem*> maRepositories;
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > m_xCmdEnv;
};

#endif // __SFX2_TEMPLATEREMOTEVIEW_HXX__


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
