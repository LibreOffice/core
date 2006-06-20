/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ToolBoxHelper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:20:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
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
        SvtMiscOptions().AddListener( LINK( this, OToolBoxHelper, ConfigOptionsChanged ) );
        Application::AddEventListener( LINK( this, OToolBoxHelper, SettingsChanged ) );
    }
    // -----------------------------------------------------------------------------
    OToolBoxHelper::~OToolBoxHelper()
    {
        SvtMiscOptions().RemoveListener( LINK( this, OToolBoxHelper, ConfigOptionsChanged ) );
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


                m_pToolBox->SetImageList( ModuleRes( getImageListId(m_nSymbolsSize,m_bIsHiContrast) ) );
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

