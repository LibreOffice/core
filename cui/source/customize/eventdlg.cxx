/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svtools/svmedit.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/documentinfo.hxx>
#include <unotools/configmgr.hxx>
#include <rtl/ustring.hxx>

#include "eventdlg.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <unotools/eventcfg.hxx>
#include <vcl/treelistentry.hxx>

#include <headertablistbox.hxx>
#include "macropg_impl.hxx"

#include <bitmaps.hlst>
#include <cfgutil.hxx>
#include <cfg.hxx>

using namespace ::com::sun::star;


SvxEventConfigPage::SvxEventConfigPage(TabPageParent pParent, const SfxItemSet& rSet,
    SvxEventConfigPage::EarlyInit)
    : SvxMacroTabPage_(pParent, "cui/ui/eventsconfigpage.ui", "EventsConfigPage", rSet)
    , m_xSaveInListBox(m_xBuilder->weld_combo_box("savein"))
{
    mpImpl->xEventLB = m_xBuilder->weld_tree_view("events");
    mpImpl->xAssignPB = m_xBuilder->weld_button("macro");
    mpImpl->xDeletePB = m_xBuilder->weld_button("delete");
    mpImpl->xAssignComponentPB = m_xBuilder->weld_button("component");

    mpImpl->xEventLB->set_size_request(mpImpl->xEventLB->get_approximate_digit_width() * 70,
                                       mpImpl->xEventLB->get_height_rows(20));

    InitResources();

    m_xSaveInListBox->connect_changed( LINK( this, SvxEventConfigPage,
                SelectHdl_Impl ) );

    uno::Reference< frame::XGlobalEventBroadcaster > xSupplier;

    xSupplier =
        frame::theGlobalEventBroadcaster::get(::comphelper::getProcessComponentContext());

    m_xAppEvents = xSupplier->getEvents();
    m_xSaveInListBox->append(OUString::boolean(true), utl::ConfigManager::getProductName());
    m_xSaveInListBox->set_active(0);
}

void SvxEventConfigPage::LateInit( const uno::Reference< frame::XFrame >& _rxFrame  )
{
    SetFrame( _rxFrame );
    ImplInitDocument();

    InitAndSetHandler( m_xAppEvents, m_xDocumentEvents, m_xDocumentModifiable );

    SelectHdl_Impl( *m_xSaveInListBox );
}

SvxEventConfigPage::~SvxEventConfigPage()
{
    disposeOnce();
}

void SvxEventConfigPage::ImplInitDocument()
{
    uno::Reference< frame::XFrame > xFrame( GetFrame() );
    OUString aModuleId = SvxConfigPage::GetFrameWithDefaultAndIdentify( xFrame );
    if ( !xFrame.is() )
        return;

    try
    {
        uno::Reference< frame::XModel > xModel;
        if ( !SvxConfigPage::CanConfig( aModuleId ) )
            return;

        uno::Reference< frame::XController > xController =
            xFrame->getController();

        if ( xController.is() )
        {
            xModel = xController->getModel();
        }

        if ( !xModel.is() )
            return;

        uno::Reference< document::XEventsSupplier > xSupplier( xModel, uno::UNO_QUERY );

        if ( xSupplier.is() )
        {
            m_xDocumentEvents = xSupplier->getEvents();
            m_xDocumentModifiable.set(xModel, css::uno::UNO_QUERY);

            OUString aTitle = ::comphelper::DocumentInfo::getDocumentTitle( xModel );

            m_xSaveInListBox->append(OUString::boolean(false), aTitle);
            m_xSaveInListBox->set_active(m_xSaveInListBox->get_count() - 1);
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("cui.customize");
    }
}

IMPL_LINK_NOARG( SvxEventConfigPage, SelectHdl_Impl, weld::ComboBox&, void )
{
    bool bApp = m_xSaveInListBox->get_active_id().toBoolean();

    mpImpl->xEventLB->freeze();
    if (bApp)
    {
        SetReadOnly( false );
        SvxMacroTabPage_::DisplayAppEvents( true );
    }
    else
    {
        bool isReadonly = false;

        uno::Reference< frame::XDesktop2 > xFramesSupplier = frame::Desktop::create(
            ::comphelper::getProcessComponentContext() );

        uno::Reference< frame::XFrame > xFrame =
            xFramesSupplier->getActiveFrame();

        if ( xFrame.is() )
        {
            uno::Reference< frame::XController > xController =
                xFrame->getController();

            if ( xController.is() )
            {
                uno::Reference< frame::XStorable > xStorable(
                    xController->getModel(), uno::UNO_QUERY );
                isReadonly = xStorable->isReadonly();
            }
        }

        SetReadOnly( isReadonly );
        SvxMacroTabPage_::DisplayAppEvents( false );
    }

    mpImpl->xEventLB->thaw();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
