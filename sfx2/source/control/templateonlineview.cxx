/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateonlineview.hxx>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/templateonlineviewitem.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

TemplateOnlineView::TemplateOnlineView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : ThumbnailView(pParent,nWinStyle,bDisableTransientChildren)
{
    Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
    Reference< XInteractionHandler >  xGlobalInteractionHandler = Reference< XInteractionHandler >(
        xFactory->createInstance("com.sun.star.task.InteractionHandler" ), UNO_QUERY );

    m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );
}

TemplateOnlineView::~TemplateOnlineView ()
{
}

void TemplateOnlineView::Populate()
{
    uno::Reference < uno::XComponentContext > m_context(comphelper::getProcessComponentContext());

    // Load from user settings
    maUrls = officecfg::Office::Common::Misc::FilePickerPlacesUrls::get(m_context);
    maNames = officecfg::Office::Common::Misc::FilePickerPlacesNames::get(m_context);

    for (sal_Int32 i = 0; i < maUrls.getLength() && i < maNames.getLength(); ++i)
    {
        TemplateOnlineViewItem *pItem = new TemplateOnlineViewItem(*this,this);

        pItem->mnId = i+1;
        pItem->maTitle = maNames[i];

        mItemList.push_back(pItem);
    }

    CalculateItemPositions();

    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
