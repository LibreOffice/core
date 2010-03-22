/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_sd.hxx"

#include "TaskPaneToolPanel.hxx"
#include "ToolPanelDeck.hxx"

#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::drawing::framework::XResourceId;
    /** === end UNO using === **/

    //==================================================================================================================
    //= TaskPaneToolPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TaskPaneToolPanel::TaskPaneToolPanel( ToolPanelDeck& i_rPanelDeck, const String& i_rPanelName, const Image& i_rImage,
            const SmartId& i_rHelpId )
        :m_pPanelDeck( &i_rPanelDeck )
        ,m_aPanelImage( i_rImage )
        ,m_sPanelName( i_rPanelName )
        ,m_aHelpId( i_rHelpId )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    TaskPaneToolPanel::~TaskPaneToolPanel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Window& TaskPaneToolPanel::getPanelWindowAnchor()
    {
        OSL_ENSURE( !isDisposed(), "already disposed!" );
        return m_pPanelDeck->GetPanelWindowAnchor();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TaskPaneToolPanel::GetDisplayName() const
    {
        return m_sPanelName;
    }

    //------------------------------------------------------------------------------------------------------------------
    Image TaskPaneToolPanel::GetImage() const
    {
        return m_aPanelImage;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneToolPanel::Dispose()
    {
        ENSURE_OR_RETURN_VOID( m_pPanelDeck, "disposed twice" );
        m_pPanelDeck = NULL;
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................
