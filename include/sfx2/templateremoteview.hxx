/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_TEMPLATEREMOTEVIEW_HXX
#define INCLUDED_SFX2_TEMPLATEREMOTEVIEW_HXX

#include <sfx2/templateabstractview.hxx>
#include <sfx2/templateproperties.hxx>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

class TemplateRepository;

class TemplateRemoteView : public TemplateAbstractView
{
public:

    TemplateRemoteView (vcl::Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren);

    virtual void showRootRegion () override;

    virtual void showRegion (ThumbnailViewItem *pItem) override;

    bool loadRepository (TemplateRepository* pRepository, bool bRefresh);

    virtual sal_uInt16 createRegion (const OUString &rName) override;

    virtual bool isNestedRegionAllowed () const override;

    virtual bool isImportAllowed () const override;

private:

    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > m_xCmdEnv;
};

#endif // INCLUDED_SFX2_TEMPLATEREMOTEVIEW_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
