/*************************************************************************
 *
 *  $RCSfile: ToolBoxHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 13:26:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

namespace dbaui
{
    OToolBoxHelper::OToolBoxHelper()
        : m_bIsHiContrast(sal_False)
        ,m_pToolBox(NULL)
        ,m_nBitmapSet( getCurrentSymbolSet() )
    {
        SvtMiscOptions().AddListener( LINK( this, OToolBoxHelper, ConfigOptionsChanged ) );
        Application::AddEventListener( LINK( this, OToolBoxHelper, SettingsChanged ) );
    }
    // -----------------------------------------------------------------------------
    OToolBoxHelper::~OToolBoxHelper()
    {
        SvtMiscOptions().RemoveListener( LINK( this, OToolBoxHelper, ConfigOptionsChanged ) );
        Application::RemoveEventListener( LINK( this, OToolBoxHelper, SettingsChanged ) );
    }
    // -----------------------------------------------------------------------------
    sal_Int16 OToolBoxHelper::getCurrentSymbolSet()
    {
        sal_Int16   eOptSymbolSet = SvtMiscOptions().GetSymbolSet();

        if ( eOptSymbolSet == SFX_SYMBOLS_AUTO )
        {
            // Use system settings, we have to retrieve the toolbar icon size from the
            // Application class
            ULONG nStyleIconSize = Application::GetSettings().GetStyleSettings().GetToolbarIconSize();
            if ( nStyleIconSize == STYLE_TOOLBAR_ICONSIZE_LARGE )
                eOptSymbolSet = SFX_SYMBOLS_LARGE;
            else
                eOptSymbolSet = SFX_SYMBOLS_SMALL;
        }

        return eOptSymbolSet;
    }

    // -----------------------------------------------------------------------------
    void OToolBoxHelper::checkImageList()
    {
        if ( m_pToolBox )
        {
            sal_Int16 nCurBitmapSet = getCurrentSymbolSet();
            if ( nCurBitmapSet != m_nBitmapSet ||
                m_bIsHiContrast != m_pToolBox->GetBackground().GetColor().IsDark() )
            {
                m_nBitmapSet    = nCurBitmapSet;
                m_bIsHiContrast = m_pToolBox->GetBackground().GetColor().IsDark();


                m_pToolBox->SetImageList( ModuleRes( getImageListId(m_nBitmapSet,m_bIsHiContrast) ) );
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
    IMPL_LINK(OToolBoxHelper, ConfigOptionsChanged, SvtMiscOptions*, _pOptions)
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
    IMPL_LINK(OToolBoxHelper, SettingsChanged, void*, _pVoid)
    {
        if ( m_pToolBox )
        {
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

