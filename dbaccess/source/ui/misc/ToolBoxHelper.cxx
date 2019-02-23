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

#include <dbaccess/ToolBoxHelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <svtools/miscopt.hxx>
#include <UITools.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <osl/diagnose.h>

namespace dbaui
{
    OToolBoxHelper::OToolBoxHelper()
        : m_nSymbolsSize(-1)
        , m_pToolBox(nullptr)
    {

        OSL_ENSURE(m_nSymbolsSize != SvtMiscOptions().GetCurrentSymbolsSize(),"SymbolsSize should not be identical");
        SvtMiscOptions().AddListenerLink( LINK( this, OToolBoxHelper, ConfigOptionsChanged ) );
        Application::AddEventListener( LINK( this, OToolBoxHelper, SettingsChanged ) );
    }
    OToolBoxHelper::~OToolBoxHelper()
    {
        SvtMiscOptions().RemoveListenerLink( LINK( this, OToolBoxHelper, ConfigOptionsChanged ) );
        Application::RemoveEventListener( LINK( this, OToolBoxHelper, SettingsChanged ) );
    }

    void OToolBoxHelper::checkImageList()
    {
        if ( m_pToolBox )
        {
            sal_Int16 nCurSymbolsSize = SvtMiscOptions().GetCurrentSymbolsSize();
            if ( nCurSymbolsSize != m_nSymbolsSize )
            {
                m_nSymbolsSize  = nCurSymbolsSize;
                setImageList(m_nSymbolsSize);

                Size aTbOldSize = m_pToolBox->GetSizePixel();
                adjustToolBoxSize(m_pToolBox);
                Size aTbNewSize = m_pToolBox->GetSizePixel();
                resizeControls(Size(aTbNewSize.Width() - aTbOldSize.Width(),
                                    aTbNewSize.Height() - aTbOldSize.Height())
                                );
            }
        }
    }

    IMPL_LINK_NOARG(OToolBoxHelper, ConfigOptionsChanged, LinkParamNone*, void)
    {
        if ( m_pToolBox )
        {
            SvtMiscOptions aOptions;
            // check if imagelist changed
            checkImageList();
            if ( aOptions.GetToolboxStyle() != m_pToolBox->GetOutStyle() )
                m_pToolBox->SetOutStyle(aOptions.GetToolboxStyle());
        }
    }
    IMPL_LINK(OToolBoxHelper, SettingsChanged, VclSimpleEvent&, _rEvt, void)
    {
        if ( m_pToolBox && _rEvt.GetId() == VclEventId::ApplicationDataChanged )
        {
            DataChangedEvent* pData = static_cast<DataChangedEvent*>(static_cast<VclWindowEvent&>(_rEvt).GetData());
            if ( pData && ((( pData->GetType() == DataChangedEventType::SETTINGS  )   ||
            ( pData->GetType() == DataChangedEventType::DISPLAY   ))  &&
            ( pData->GetFlags() & AllSettingsFlags::STYLE        )))
                // check if imagelist changed
                checkImageList();
        }
    }
    void OToolBoxHelper::setToolBox(ToolBox* _pTB)
    {
        bool bFirstTime = (m_pToolBox == nullptr);
        m_pToolBox = _pTB;
        if ( m_pToolBox )
        {
            ConfigOptionsChanged(nullptr);
            if ( bFirstTime )
                adjustToolBoxSize(m_pToolBox);
        }
    }
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
