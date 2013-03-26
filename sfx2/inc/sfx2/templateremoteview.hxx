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

class TemplateRepository;

class TemplateRemoteView : public TemplateAbstractView
{
public:

    TemplateRemoteView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren);

    virtual ~TemplateRemoteView ();

    virtual void showRootRegion ();

    virtual void showRegion (ThumbnailViewItem *pItem);

    bool loadRepository (TemplateRepository* pRepository, bool bRefresh);

    virtual sal_uInt16 createRegion (const OUString &rName);

private:

    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > m_xCmdEnv;
};

#endif // __SFX2_TEMPLATEREMOTEVIEW_HXX__


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
