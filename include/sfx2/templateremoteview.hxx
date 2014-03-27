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

    TemplateRemoteView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren);

    virtual ~TemplateRemoteView ();

    virtual void showRootRegion () SAL_OVERRIDE;

    virtual void showRegion (ThumbnailViewItem *pItem) SAL_OVERRIDE;

    bool loadRepository (TemplateRepository* pRepository, bool bRefresh);

    virtual sal_uInt16 createRegion (const OUString &rName) SAL_OVERRIDE;

    virtual bool isNestedRegionAllowed () const SAL_OVERRIDE;

    virtual bool isImportAllowed () const SAL_OVERRIDE;

private:

    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > m_xCmdEnv;
};

#endif // INCLUDED_SFX2_TEMPLATEREMOTEVIEW_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
