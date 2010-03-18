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

#include "ctp_panel.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//......................................................................................................................
namespace sd { namespace colortoolpanel
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
    using ::com::sun::star::drawing::framework::XConfigurationController;
    using ::com::sun::star::drawing::framework::XResourceId;
    using ::com::sun::star::uno::XComponentContext;
    /** === end UNO using === **/

    //==================================================================================================================
    //= class SingleColorPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    SingleColorPanel::SingleColorPanel( const Reference< XComponentContext >& i_rContext,
            const Reference< XConfigurationController >& i_rConfigController, const Reference< XResourceId >& i_rResourceId )
        :m_xContext( i_rContext )
        ,m_xResourceId( i_rResourceId )
    {
        // TODO: retrieve the resource object for the anchor, ask it for its XPane interface, retrieve the window
        // associated with it, create our own window as child of the pane's window
        (void)i_rConfigController;
    }

    //------------------------------------------------------------------------------------------------------------------
    SingleColorPanel::~SingleColorPanel()
    {
    }

    //--------------------------------------------------------------------
    Reference< XResourceId > SAL_CALL SingleColorPanel::getResourceId(  ) throw (RuntimeException)
    {
        return m_xResourceId;
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL SingleColorPanel::isAnchorOnly(  ) throw (RuntimeException)
    {
        return sal_False;
    }


//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................
