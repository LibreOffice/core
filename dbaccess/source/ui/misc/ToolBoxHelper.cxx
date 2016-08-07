/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
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
                m_bIsHiContrast != m_pToolBox->GetSettings().GetStyleSettings().GetHighContrastMode() )
            {
                m_nSymbolsSize  = nCurSymbolsSize;
                m_bIsHiContrast = m_pToolBox->GetSettings().GetStyleSettings().GetHighContrastMode();


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
            //  m_bIsHiContrast = m_pToolBox->GetSettings().GetStyleSettings().GetHighContrastMode();
            ConfigOptionsChanged(NULL);
            if ( bFirstTime )
                adjustToolBoxSize(m_pToolBox);
        }
    }
// -----------------------------------------------------------------------------
} // namespace
// -----------------------------------------------------------------------------

