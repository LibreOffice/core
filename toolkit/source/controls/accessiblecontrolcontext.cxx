/*************************************************************************
 *
 *  $RCSfile: accessiblecontrolcontext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 15:10:24 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef TOOLKIT_ACCESSIBLE_CONTROL_CONTEXT_HXX
#include <toolkit/controls/accessiblecontrolcontext.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

//........................................................................
namespace toolkit
{
//........................................................................

    using ::comphelper::OContextEntryGuard;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::accessibility;

    //====================================================================
    //= OAccessibleControlContext
    //====================================================================
    //--------------------------------------------------------------------
    OAccessibleControlContext::OAccessibleControlContext()
        :OAccessibleControlContext_Base( )
    {
        // nothing to do here, we have a late ctor
    }

    //--------------------------------------------------------------------
    OAccessibleControlContext::~OAccessibleControlContext()
    {
        ensureDisposed();
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE3( OAccessibleControlContext, OAccessibleControlContext_Base, OAccessibleImplementationAccess, OAccessibleControlContext_IBase )
    IMPLEMENT_FORWARD_XTYPEPROVIDER3( OAccessibleControlContext, OAccessibleControlContext_Base, OAccessibleImplementationAccess, OAccessibleControlContext_IBase )
        // (order matters: the first is the class name, the second is the class doing the ref counting)

    //--------------------------------------------------------------------
    void OAccessibleControlContext::Init( const Reference< XAccessible >& _rxCreator ) SAL_THROW( ( Exception ) )
    {
        OContextEntryGuard aGuard( this );

        // retrieve the model of the control
        OSL_ENSURE( !m_xControlModel.is(), "OAccessibleControlContext::Init: already know a control model....!???" );

        Reference< awt::XControl > xControl( _rxCreator, UNO_QUERY );
        if ( xControl.is() )
            m_xControlModel = m_xControlModel.query( xControl->getModel() );
        OSL_ENSURE( m_xControlModel.is(), "OAccessibleControlContext::Init: invalid creator (no control, or control without model!" );
        if ( !m_xControlModel.is() )
            throw DisposedException();  // caught by the caller (the create method)

        // start listening at the model
        startModelListening();

        // announce the XAccessible to our base class
        OAccessibleControlContext_Base::lateInit( _rxCreator );
    }

    //--------------------------------------------------------------------
    OAccessibleControlContext* OAccessibleControlContext::create( const Reference< XAccessible >& _rxCreator ) SAL_THROW( ( ) )
    {
        OAccessibleControlContext* pNew = NULL;
        try
        {
            pNew = new OAccessibleControlContext;
            pNew->Init( _rxCreator );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OAccessibleControlContext::create: caught an exception from the late ctor!" );
        }
        return pNew;
    }

    //--------------------------------------------------------------------
    void OAccessibleControlContext::startModelListening( ) SAL_THROW( ( Exception ) )
    {
        Reference< XComponent > xModelComp( m_xControlModel, UNO_QUERY );
        OSL_ENSURE( xModelComp.is(), "OAccessibleControlContext::startModelListening: invalid model!" );
        if ( xModelComp.is() )
            xModelComp->addEventListener( this );
    }

    //--------------------------------------------------------------------
    void OAccessibleControlContext::stopModelListening( ) SAL_THROW( ( Exception ) )
    {
        Reference< XComponent > xModelComp( m_xControlModel, UNO_QUERY );
        OSL_ENSURE( xModelComp.is(), "OAccessibleControlContext::stopModelListening: invalid model!" );
        if ( xModelComp.is() )
            xModelComp->removeEventListener( this );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleControlContext::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        // we do not have children
        return 0;
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleControlContext::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        // we do not have children
        throw IndexOutOfBoundsException();
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleControlContext::getAccessibleParent(  ) throw (RuntimeException)
    {
        OContextEntryGuard aGuard( this );
        OSL_ENSURE( implGetForeignControlledParent().is(), "OAccessibleControlContext::getAccessibleParent: somebody forgot to set a parent!" );
            // this parent of us is foreign controlled - somebody has to set it using the OAccessibleImplementationAccess
            // class, before integrating our instance into an AccessibleDocumentModel
        return implGetForeignControlledParent();
    }

    //--------------------------------------------------------------------
    sal_Int16 SAL_CALL OAccessibleControlContext::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::SHAPE;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAccessibleControlContext::getAccessibleDescription(  ) throw (RuntimeException)
    {
        OContextEntryGuard aGuard( this );
        return getModelStringProperty( "HelpText" );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAccessibleControlContext::getAccessibleName(  ) throw (RuntimeException)
    {
        OContextEntryGuard aGuard( this );
        return getModelStringProperty( "Name" );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL OAccessibleControlContext::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        return NULL;
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleStateSet > SAL_CALL OAccessibleControlContext::getAccessibleStateSet(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
            // no OContextEntryGuard here, as we do not want to throw an exception in case we're not alive anymore

        ::utl::AccessibleStateSetHelper* pStateSet = NULL;
        if ( isAlive() )
        {
            // no own states, only the ones which are foreign controlled
            pStateSet = new ::utl::AccessibleStateSetHelper( implGetForeignControlledStates() );
        }
        else
        {   // only the DEFUNC state if we're already disposed
            pStateSet = new ::utl::AccessibleStateSetHelper;
            pStateSet->AddState( AccessibleStateType::DEFUNC );
        }
        return pStateSet;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleControlContext::disposing( const EventObject& _rSource ) throw ( RuntimeException )
    {
        OSL_ENSURE( Reference< XPropertySet >( _rSource.Source, UNO_QUERY ).get() == m_xControlModel.get(),
            "OAccessibleControlContext::disposing: where did this come from?" );

        stopModelListening( );
        m_xControlModel.clear();
        m_xModelPropsInfo.clear();

        OAccessibleControlContext_Base::disposing();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OAccessibleControlContext::getModelStringProperty( const sal_Char* _pPropertyName )
    {
        ::rtl::OUString sReturn;
        try
        {
            if ( !m_xModelPropsInfo.is() && m_xControlModel.is() )
                m_xModelPropsInfo = m_xControlModel->getPropertySetInfo();

            ::rtl::OUString sPropertyName( ::rtl::OUString::createFromAscii( _pPropertyName ) );
            if ( m_xModelPropsInfo.is() && m_xModelPropsInfo->hasPropertyByName( sPropertyName ) )
                m_xControlModel->getPropertyValue( sPropertyName ) >>= sReturn;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OAccessibleControlContext::getModelStringProperty: caught an exception!" );
        }
        return sReturn;
    }

    //--------------------------------------------------------------------
    Window* OAccessibleControlContext::implGetWindow( Reference< awt::XWindow >* _pxUNOWindow ) const
    {
        Reference< awt::XControl > xControl( getAccessibleCreator(), UNO_QUERY );
        Reference< awt::XWindow > xWindow;
        if ( xControl.is() )
            xWindow = xWindow.query( xControl->getPeer() );

        Window* pWindow = xWindow.is() ? VCLUnoHelper::GetWindow( xWindow ) : NULL;

        if ( _pxUNOWindow )
            *_pxUNOWindow = xWindow;
        return pWindow;
    }

    //--------------------------------------------------------------------
    awt::Rectangle SAL_CALL OAccessibleControlContext::implGetBounds(  ) throw (RuntimeException)
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            // want to do some VCL stuff here ...
        OContextEntryGuard aGuard( this );

        OSL_ENSURE( sal_False, "OAccessibleControlContext::implGetBounds: performance issue: forced to calc the size myself!" );
        // In design mode (and this is what this class is for), the surrounding shape (if any) should handle this call
        // The problem is that in design mode, our size may not be correct (in the drawing layer, controls are
        // positioned/sized for painting only), and that calculation of our position is expensive

        // what we know (or can obtain from somewhere):
        // * the PosSize of our peer, relative to it's parent window
        // * the parent window which the PosSize is relative to
        // * our foreign controlled accessible parent
        // from this info, we can determine the the position of our peer relative to the foreign parent

        // our control
        Reference< awt::XWindow > xWindow;
        Window* pVCLWindow = implGetWindow( &xWindow );

        awt::Rectangle aBounds( 0, 0, 0, 0 );
        if ( xWindow.is() )
        {
            // ugly, but .... though the XWindow has a getPosSize, it is impossible to determine the
            // parent which this position/size is relative to. This means we must tunnel UNO and ask the
            // implementation
            Window* pVCLParent = pVCLWindow ? pVCLWindow->GetParent() : NULL;

            // the relative location of the window
            ::Point aWindowRelativePos( 0, 0);
            if ( pVCLWindow )
                aWindowRelativePos = pVCLWindow->GetPosPixel();

            // the screnn position of the "window parent" of the control
            ::Point aVCLParentScreenPos( 0, 0 );
            if ( pVCLParent )
                aVCLParentScreenPos = pVCLParent->GetPosPixel();

            // the screen position of the "accessible parent" of the control
            Reference< XAccessible > xParentAcc( implGetForeignControlledParent() );
            Reference< XAccessibleComponent > xParentAccComponent;
            if ( xParentAcc.is() )
                xParentAccComponent = xParentAccComponent.query( xParentAcc->getAccessibleContext() );
            awt::Point aAccParentScreenPos( 0, 0 );
            if ( xParentAccComponent.is() )
                aAccParentScreenPos = xParentAccComponent->getLocationOnScreen();

            // now the size of the control
            aBounds = xWindow->getPosSize();

            // correct the pos
            aBounds.X = aWindowRelativePos.X() + aVCLParentScreenPos.X() - aAccParentScreenPos.X;
            aBounds.Y = aWindowRelativePos.Y() + aVCLParentScreenPos.Y() - aAccParentScreenPos.Y;
        }

        return aBounds;
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleControlContext::getAccessibleAtPoint( const awt::Point& /* _rPoint */ ) throw (RuntimeException)
    {
        // no children at all
        return NULL;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleControlContext::grabFocus(  ) throw (RuntimeException)
    {
        OSL_ENSURE( sal_False, "OAccessibleControlContext::grabFocus: !isFocusTraversable, but grabFocus!" );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OAccessibleControlContext::getAccessibleKeyBinding(  ) throw (RuntimeException)
    {
        // we do not have any key bindings to activate a UNO control in design mode
        return Any();
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleControlContext::getForeground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            // want to do some VCL stuff here ...
        OContextEntryGuard aGuard( this );

        Window* pWindow = implGetWindow( );
        sal_Int32 nColor = 0;
        if ( pWindow )
        {
            if ( pWindow->IsControlForeground() )
                nColor = pWindow->GetControlForeground().GetColor();
            else
            {
                Font aFont;
                if ( pWindow->IsControlFont() )
                    aFont = pWindow->GetControlFont();
                else
                    aFont = pWindow->GetFont();
                nColor = aFont.GetColor().GetColor();
            }
        }
        return nColor;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleControlContext::getBackground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            // want to do some VCL stuff here ...
        OContextEntryGuard aGuard( this );

        Window* pWindow = implGetWindow( );
        sal_Int32 nColor = 0;
        if ( pWindow )
        {
            if ( pWindow->IsControlBackground() )
                nColor = pWindow->GetControlBackground().GetColor();
            else
                nColor = pWindow->GetBackground().GetColor().GetColor();
        }

        return nColor;
    }

//........................................................................
}   //namespace toolkit
//........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4.58.1  2003/04/11 17:26:19  mt
 *  #108656# Moved accessibility from drafts to final
 *
 *  Revision 1.4  2002/10/29 08:10:15  fs
 *  #65293# must-change not done: new methods for XAccessibleComponent implemented
 *
 *  Revision 1.3  2002/06/12 13:16:58  fs
 *  #100126# exception thrown in wrong situation
 *
 *  Revision 1.2  2002/05/17 15:29:11  tbe
 *  #97222# removed isShowing, isVisible, isFocusTraversable, addFocusListener, removeFocusListener
 *
 *  Revision 1.1  2002/04/26 14:31:31  fs
 *  initial checkin - fallback AccessibleContext for uno controls
 *
 *
 *  Revision 1.0 17.04.2002 12:50:51  fs
 ************************************************************************/

