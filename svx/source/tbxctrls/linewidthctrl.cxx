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

#include <string>

#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>

#include <svtools/toolbarmenu.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/valueset.hxx>

#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <helpids.h>

#include <svx/drawitem.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xtable.hxx>
#include <svx/linectrl.hxx>
#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <svx/unoapi.hxx>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

SFX_IMPL_TOOLBOX_CONTROL( SvxLineWidthToolBoxControl, XLineWidthItem );

SvxLineWidthToolBoxControl::SvxLineWidthToolBoxControl(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    addStatusListener( ".uno:MetricUnit");
}


SvxLineWidthToolBoxControl::~SvxLineWidthToolBoxControl()
{
}


void SvxLineWidthToolBoxControl::StateChanged(
    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    SvxMetricField* pFld = static_cast<SvxMetricField*>(
                           GetToolBox().GetItemWindow( GetId() ));
    DBG_ASSERT( pFld, "Window not found" );

    if ( nSID == SID_ATTR_METRIC )
    {
        pFld->RefreshDlgUnit();
    }
    else
    {
        if ( eState == SfxItemState::DISABLED )
        {
            pFld->set_sensitive(false);
        }
        else
        {
            pFld->set_sensitive(true);

            if ( eState == SfxItemState::DEFAULT )
            {
                DBG_ASSERT( dynamic_cast<const XLineWidthItem*>( pState) !=  nullptr, "wrong ItemType" );

                // Core-Unit handed over to MetricField
                // Should not happen in CreateItemWin ()!
                // CD!!! GetCoreMetric();
                pFld->SetCoreUnit( MapUnit::Map100thMM );

                pFld->Update( static_cast<const XLineWidthItem*>(pState) );
            }
            else
                pFld->Update( nullptr );
        }
    }
}

VclPtr<vcl::Window> SvxLineWidthToolBoxControl::CreateItemWindow(vcl::Window *pParent)
{
    VclPtr<SvxMetricField> pWindow = VclPtr<SvxMetricField>::Create(pParent, m_xFrame);
    pWindow->Show();

    return pWindow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
