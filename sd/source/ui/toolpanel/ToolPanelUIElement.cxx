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

#include "ToolPanelUIElement.hxx"
#include "MethodGuard.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/lang/XComponent.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

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
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::ui::XToolPanel;
    using ::com::sun::star::lang::DisposedException;
    /** === end UNO using === **/
    namespace UIElementType = ::com::sun::star::ui::UIElementType;

    typedef MethodGuard< ToolPanelUIElement > UIElementMethodGuard;

    //==================================================================================================================
    //= ToolPanelUIElement
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanelUIElement::ToolPanelUIElement( const Reference< XFrame >& i_rFrame, const ::rtl::OUString& i_rResourceURL,
            const Reference< XToolPanel >& i_rToolPanel )
        :ToolPanelUIElement_Base( m_aMutex )
        ,m_xFrame( i_rFrame )
        ,m_sResourceURL( i_rResourceURL )
        ,m_xToolPanel( i_rToolPanel )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelUIElement::~ToolPanelUIElement()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelUIElement::checkDisposed()
    {
        if ( !m_xToolPanel.is() )
            throw DisposedException( ::rtl::OUString(), *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XFrame > SAL_CALL ToolPanelUIElement::getFrame() throw (RuntimeException)
    {
        UIElementMethodGuard aGuard( *this );
        return m_xFrame;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ToolPanelUIElement::getResourceURL() throw (RuntimeException)
    {
        UIElementMethodGuard aGuard( *this );
        return m_sResourceURL;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int16 SAL_CALL ToolPanelUIElement::getType() throw (RuntimeException)
    {
        UIElementMethodGuard aGuard( *this );
        return UIElementType::TOOLPANEL;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ToolPanelUIElement::getRealInterface(  ) throw (RuntimeException)
    {
        UIElementMethodGuard aGuard( *this );
        return m_xToolPanel.get();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ToolPanelUIElement::disposing()
    {
        try
        {
            Reference< XComponent > xPanelComponent( m_xToolPanel, UNO_QUERY_THROW );
            xPanelComponent->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................
