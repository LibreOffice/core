/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: commoncontrol.cxx,v $
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
#include "precompiled_extensions.hxx"
#include "commoncontrol.hxx"
#include "pcrcommon.hxx"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/combobox.hxx>
#include <toolkit/helper/vclunohelper.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::inspection::XPropertyControlContext;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::inspection::XPropertyControl;
    /** === end UNO using === **/

    //==================================================================
    //= ControlHelper
    //==================================================================
    //------------------------------------------------------------------
    ControlHelper::ControlHelper( Window* _pControlWindow, sal_Int16 _nControlType, XPropertyControl& _rAntiImpl, IModifyListener* _pModifyListener )
        :m_pControlWindow( _pControlWindow )
        ,m_nControlType( _nControlType )
        ,m_rAntiImpl( _rAntiImpl )
        ,m_pModifyListener( _pModifyListener )
        ,m_bModified( sal_False )
    {
        DBG_ASSERT( m_pControlWindow != NULL, "ControlHelper::ControlHelper: invalid window!" );
    }

    //------------------------------------------------------------------
    ControlHelper::~ControlHelper()
    {
    }

    //--------------------------------------------------------------------
    ::sal_Int16 SAL_CALL ControlHelper::getControlType() throw (RuntimeException)
    {
        return m_nControlType;
    }

    //--------------------------------------------------------------------
    Reference< XPropertyControlContext > SAL_CALL ControlHelper::getControlContext() throw (RuntimeException)
    {
        return m_xContext;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ControlHelper::setControlContext( const Reference< XPropertyControlContext >& _controlcontext ) throw (RuntimeException)
    {
        m_xContext = _controlcontext;
    }

    //--------------------------------------------------------------------
    Reference< XWindow > SAL_CALL ControlHelper::getControlWindow() throw (RuntimeException)
    {
        return VCLUnoHelper::GetInterface( m_pControlWindow );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL ControlHelper::isModified(  ) throw (RuntimeException)
    {
        return m_bModified;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ControlHelper::notifyModifiedValue(  ) throw (RuntimeException)
    {
        if ( isModified() && m_xContext.is() )
        {
            try
            {
                m_xContext->valueChanged( &m_rAntiImpl );
                m_bModified = sal_False;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //------------------------------------------------------------------
    void SAL_CALL ControlHelper::dispose()
    {
        DELETEZ( m_pControlWindow );
    }

    //------------------------------------------------------------------
    void ControlHelper::autoSizeWindow()
    {
        OSL_PRECOND( m_pControlWindow, "ControlHelper::autoSizeWindow: no window!" );
        if ( !m_pControlWindow )
            return;

        ComboBox aComboBox(m_pControlWindow, WB_DROPDOWN);
        aComboBox.SetPosSizePixel(Point(0,0), Size(100,100));
        m_pControlWindow->SetSizePixel(aComboBox.GetSizePixel());

        // TODO/UNOize: why do the controls this themselves? Shouldn't this be the task
        // of the the browser listbox/line?
    }

    //------------------------------------------------------------------
    void ControlHelper::impl_activateNextControl_nothrow() const
    {
        try
        {
            if ( m_xContext.is() )
                m_xContext->activateNextControl( const_cast< XPropertyControl* >( &m_rAntiImpl ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------
    bool ControlHelper::handlePreNotify(NotifyEvent& rNEvt)
    {
        if (EVENT_KEYINPUT == rNEvt.GetType())
        {
            const KeyCode& aKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
            sal_uInt16 nKey = aKeyCode.GetCode();

            if (nKey == KEY_RETURN && !aKeyCode.IsShift())
            {
                LoseFocusHdl(m_pControlWindow);
                impl_activateNextControl_nothrow();
                return true;
            }
        }
        return false;
    }

    //------------------------------------------------------------------
    IMPL_LINK( ControlHelper, ModifiedHdl, Window*, /*_pWin*/ )
    {
        if ( m_pModifyListener )
            m_pModifyListener->modified();
        return 0;
    }

    //------------------------------------------------------------------
    IMPL_LINK( ControlHelper, GetFocusHdl, Window*, /*_pWin*/ )
    {
        try
        {
            if ( m_xContext.is() )
                m_xContext->focusGained( &m_rAntiImpl );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return 0;
    }

    //------------------------------------------------------------------
    IMPL_LINK( ControlHelper, LoseFocusHdl, Window*, /*_pWin*/ )
    {
        // TODO/UNOize: should this be outside the default control's implementations? If somebody
        // has an own control implementation, which does *not* do this - would this be allowed?
        // If not, then we must move this logic out of here.
        notifyModifiedValue();
        return 0;
    }

//............................................................................
} // namespace pcr
//............................................................................

