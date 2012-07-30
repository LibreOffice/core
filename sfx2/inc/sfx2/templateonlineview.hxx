/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_TEMPLATEONLINEVIEW_HXX__
#define __SFX2_TEMPLATEONLINEVIEW_HXX__

#include <sfx2/templateabstractview.hxx>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

class TemplateOnlineView : public TemplateAbstractView
{
public:

    TemplateOnlineView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren);

    virtual ~TemplateOnlineView ();

    // Load repositories from user settings.
    virtual void Populate ();

    virtual void filterTemplatesByApp (const FILTER_APPLICATION &eApp);

    virtual void showOverlay (bool bVisible);

    void setItemDimensions (long ItemWidth, long ThumbnailHeight, long DisplayHeight, int itemPadding);

private:

    virtual void Resize ();

    virtual void OnItemDblClicked (ThumbnailViewItem *pItem);

private:

    com::sun::star::uno::Sequence< rtl::OUString > maUrls;
    com::sun::star::uno::Sequence< rtl::OUString > maNames;
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > m_xCmdEnv;
};

#endif // __SFX2_TEMPLATEONLINEVIEW_HXX__


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
