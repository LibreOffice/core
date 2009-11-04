/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ToolBoxHelper.cxx,v $
 * $Revision: 1.10 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_TOOLBOXHELPER_HXX
#include "ToolBoxHelper.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include <svtools/miscopt.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _SVTOOLS_IMGDEF_HXX
#include <svtools/imgdef.hxx>
#endif
#include <vcl/event.hxx>

namespace dbaui
{
    DBG_NAME(OToolBoxHelper)
    OToolBoxHelper::OToolBoxHelper()
        : m_bIsHiContrast(sal_False)
        ,m_nSymbolsSize(-1 )
        ,m_pToolBox(NULL)
    {
        DBG_CTOR(OToolBoxHelper,NULL);

        OSL_ENSURE(m_nSymbolsSize != SvtMiscOptions().GetCurrentSymbolsSize(),"SymbolsSize should not be identical");
        SvtMiscOptions().AddListenerLink( LINK( this, OToolBoxHelper, ConfigOptionsChanged ) );
        Application::AddEventListener( LINK( this, OToolBoxHelper, SettingsChanged ) );
    }
    // -----------------------------------------------------------------------------
    OToolBoxHelper::~OToolBoxHelper()
    {
        SvtMiscOptions().RemoveListenerLink( LINK( this, OToolBoxHelper, ConfigOptionsChanged ) );
        Application::RemoveEventListener( LINK( this, OToolBoxHelper, SettingsChanged ) );
        DBG_DTOR(OToolBoxHelper,NULL);
    }

    // -----------------------------------------------------------------------------
    void OToolBoxHelper::checkImageList()
    {
        if ( m_pToolBox )
        {
            sal_Int16 nCurSymbolsSize = SvtMiscOptions().GetCurrentSymbolsSize();
            if ( nCurSymbolsSize != m_nSymbolsSize ||
                m_bIsHiContrast != m_pToolBox->GetBackground().GetColor().IsDark() )
            {
                m_nSymbolsSize  = nCurSymbolsSize;
                m_bIsHiContrast = m_pToolBox->GetBackground().GetColor().IsDark();


                m_pToolBox->SetImageList( getImageList(m_nSymbolsSize,m_bIsHiContrast) );
                Size aTbOldSize = m_pToolBox->GetSizePixel();
                adjustToolBoxSize(m_pToolBox);
                Size aTbNewSize = m_pToolBox->GetSizePixel();
                resizeControls(Size(aTbNewSize.Width() - aTbOldSize.Width(),
                                    aTbNewSize.Height() - aTbOldSize.Height())
                                );
            }
        }
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(OToolBoxHelper, ConfigOptionsChanged, SvtMiscOptions*, /*_pOptions*/)
    {
        if ( m_pToolBox )
        {
            SvtMiscOptions aOptions;
            // check if imagelist changed
            checkImageList();
            if ( aOptions.GetToolboxStyle() != m_pToolBox->GetOutStyle() )
                m_pToolBox->SetOutStyle(aOptions.GetToolboxStyle());
        }

        return 0L;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(OToolBoxHelper, SettingsChanged, VclWindowEvent*, _pEvt)
    {
        if ( m_pToolBox && _pEvt && _pEvt->GetId() == VCLEVENT_APPLICATION_DATACHANGED )
        {
            DataChangedEvent* pData = reinterpret_cast<DataChangedEvent*>(_pEvt->GetData());
            if ( pData && ((( pData->GetType() == DATACHANGED_SETTINGS  )   ||
            ( pData->GetType() == DATACHANGED_DISPLAY   ))  &&
            ( pData->GetFlags() & SETTINGS_STYLE        )))
                // check if imagelist changed
                checkImageList();
        }

        return 0L;
    }
    // -----------------------------------------------------------------------------
    void OToolBoxHelper::setToolBox(ToolBox* _pTB)
    {
        sal_Bool bFirstTime = (m_pToolBox == NULL);
        m_pToolBox = _pTB;
        if ( m_pToolBox )
        {
            //  m_bIsHiContrast = m_pToolBox->GetBackground().GetColor().IsDark();
            ConfigOptionsChanged(NULL);
            if ( bFirstTime )
                adjustToolBoxSize(m_pToolBox);
        }
    }
// -----------------------------------------------------------------------------
} // namespace
// -----------------------------------------------------------------------------

