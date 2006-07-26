/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commoncontrol.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:54:06 $
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

#ifndef _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_
#include "commoncontrol.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

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
                m_xContext->controlValueChanged( &m_rAntiImpl );
                m_bModified = sal_False;
            }
            catch( const Exception& e )
            {
            #if OSL_DEBUG_LEVEL > 0
                ::rtl::OString sMessage( "ControlHelper::notifyModifiedValue: caught an exception!\n" );
                sMessage += "message:\n";
                sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), osl_getThreadTextEncoding() );
                OSL_ENSURE( false, sMessage );
            #else
                e; // make compiler happy
            #endif
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
        catch( const Exception& e )
        {
        #if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "ControlHelper::impl_activateNextControl_nothrow: caught an exception!\n" );
            sMessage += "message:\n";
            sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), osl_getThreadTextEncoding() );
            OSL_ENSURE( false, sMessage );
        #else
            e; // make compiler happy
        #endif
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
        catch( const Exception& e )
        {
        #if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "ControlHelper, GetFocusHdl: caught an exception!\n" );
            sMessage += "message:\n";
            sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), osl_getThreadTextEncoding() );
            OSL_ENSURE( false, sMessage );
        #else
            e; // make compiler happy
        #endif
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

