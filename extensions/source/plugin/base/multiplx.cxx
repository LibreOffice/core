/*************************************************************************
 *
 *  $RCSfile: multiplx.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:51 $
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
#include <vos/diagnose.hxx>
#include <plugin/multiplx.hxx>

//  ---------------------------------------------------------------------
//  class MRCListenerMultiplexerHelper
//  ---------------------------------------------------------------------
MRCListenerMultiplexerHelper::MRCListenerMultiplexerHelper
(
      const Reference< ::com::sun::star::awt::XWindow >  & rControl
    , const Reference< ::com::sun::star::awt::XWindow >  & rPeer
)
    : aListenerHolder( aMutex )
    , xPeer( rPeer )
    , xControl( Reference< ::com::sun::star::awt::XControl >( rControl, UNO_QUERY ) )
{
}


void MRCListenerMultiplexerHelper::setPeer( const Reference< ::com::sun::star::awt::XWindow >  & rPeer )
{
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );
    if( xPeer != rPeer )
    {
        if( xPeer.is() )
        {
            // get all uiks from the listener added to the peer
            Sequence<Type> aContainedTypes = aListenerHolder.getContainedTypes();
            const Type* pArray = aContainedTypes.getConstArray();
            sal_Int32 nCount = aContainedTypes.getLength();
            // loop over all listener types and remove the listeners from the peer
            for( sal_Int32 i = 0; i < nCount; i++ )
                unadviseFromPeer( xPeer, pArray[i] );
        }
        xPeer = rPeer;
        if( xPeer.is() )
        {
            // get all uiks from the listener added to the peer
            Sequence<Type> aContainedTypes = aListenerHolder.getContainedTypes();
            const Type * pArray = aContainedTypes.getConstArray();
            sal_Int32 nCount = aContainedTypes.getLength();
            // loop over all listener types and add the listeners to the peer
            for( sal_Int32 i = 0; i < nCount; i++ )
                adviseToPeer( xPeer, pArray[i] );
        }
    }
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::disposeAndClear()
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = xControl;
    aListenerHolder.disposeAndClear( aEvt );
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::adviseToPeer( const Reference< ::com::sun::star::awt::XWindow >  & rPeer, const Type & type )
{
    // add a listener to the source (peer)
    if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XWindowListener >*)0) )
        rPeer->addWindowListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XKeyListener >*)0) )
        rPeer->addKeyListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XFocusListener >*)0) )
        rPeer->addFocusListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XMouseListener >*)0) )
        rPeer->addMouseListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XMouseMotionListener >*)0) )
        rPeer->addMouseMotionListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XPaintListener >*)0) )
        rPeer->addPaintListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XTopWindowListener >*)0) )
    {
        Reference< ::com::sun::star::awt::XTopWindow >  xTop( rPeer, UNO_QUERY );
        if( xTop.is() )
            xTop->addTopWindowListener( this );
    }
    else
    {
        VOS_ENSHURE( sal_False, "unknown listener" );
    }
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::unadviseFromPeer( const Reference< ::com::sun::star::awt::XWindow >  & rPeer, const Type & type )
{
    // the last listener is removed, remove the listener from the source (peer)
    if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XWindowListener >*)0) )
        rPeer->removeWindowListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XKeyListener >*)0) )
        rPeer->removeKeyListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XFocusListener >*)0) )
        rPeer->removeFocusListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XMouseListener >*)0) )
        rPeer->removeMouseListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XMouseMotionListener >*)0) )
        rPeer->removeMouseMotionListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XPaintListener >*)0) )
        rPeer->removePaintListener( this );
    else if( type == ::getCppuType((const Reference< ::com::sun::star::awt::XTopWindowListener >*)0) )
    {
        Reference< ::com::sun::star::awt::XTopWindow >  xTop( rPeer, UNO_QUERY );
        if( xTop.is() )
            xTop->removeTopWindowListener( this );
    }
    else
    {
        VOS_ENSHURE( sal_False, "unknown listener" );
    }
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::advise( const Type & type, const Reference< XInterface > & listener)
{
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );
    if( 1 == aListenerHolder.addInterface( type, listener ) )
    {
        // the first listener is added
        if( xPeer.is() )
            adviseToPeer( xPeer, type );
    }
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::unadvise(const Type & type, const Reference< XInterface > & listener)
{
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );
    ::cppu::OInterfaceContainerHelper * pCont = aListenerHolder.getContainer( type );
    if( pCont )
    {
        if( 0 == pCont->removeInterface( listener ) && xPeer.is() )
            // the last listener is removed
            unadviseFromPeer( xPeer, type );
    }
}

// ::com::sun::star::lang::XEventListener
void MRCListenerMultiplexerHelper::disposing(const ::com::sun::star::lang::EventObject& )
{
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );
    // peer is disposed, clear the reference
    xPeer = Reference< ::com::sun::star::awt::XWindow > ();
}

#define MULTIPLEX( InterfaceName, MethodName, EventName )                   \
::cppu::OInterfaceContainerHelper * pCont;                                          \
pCont = aListenerHolder.getContainer( ::getCppuType((const Reference< InterfaceName >*)0) );        \
if( pCont )                                                                 \
{                                                                           \
    ::cppu::OInterfaceIteratorHelper    aIt( *pCont );                              \
    EventName aEvt = e;                                                     \
    /* Remark: The control is the event source not the peer. We must change */  \
    /* the source of the event */                                               \
    aEvt.Source = xControl;\
    /*.is the control not destroyed */                                      \
    if( aEvt.Source.is() )                                                  \
    {                                                                       \
        if( aIt.hasMoreElements() )                                         \
        {                                                                   \
            InterfaceName * pListener = (InterfaceName *)aIt.next();        \
            try                                                             \
            {                                                               \
                pListener->MethodName( aEvt );                              \
            }                                                               \
            catch( RuntimeException& )                                  \
            {                                                               \
                /* ignore all usr system exceptions from the listener */    \
            }                                                               \
        }                                                                   \
    }                                                                       \
}

// ::com::sun::star::awt::XFocusListener
void MRCListenerMultiplexerHelper::focusGained(const ::com::sun::star::awt::FocusEvent& e)
{
/*
    OInterfaceContainerHelper * pCont = aListenerHolder.getContainer( ::getCppuType((const Reference< ::com::sun::star::awt::XFocusListener >*)0) );
    if( pCont )
    {
        OInterfaceIteratorHelper    aIt( *pCont );
        ::com::sun::star::awt::FocusEvent aEvt = e;
        // Reamark: The control is the event source not the peer. We must change
        // the source of the event
        xControl.queryHardRef( ((XInterface*)NULL)->getSmartUik(), aEvt.Source );
        //.is the control not destroyed
        if( aEvt.Source.is() )
        {
            if( aIt.hasMoreElements() )
            {
                ::com::sun::star::awt::XFocusListener * pListener = (::com::sun::star::awt::XFocusListener *)aIt.next();
                TRY
                {
                    pListener->focusGained( aEvt );
                }
                CATCH( RuntimeException, e )
                {
                    // ignore all usr system exceptions from the listener
                }
                END_CATCH;
            }
        }
    }
*/
    MULTIPLEX( ::com::sun::star::awt::XFocusListener, focusGained, ::com::sun::star::awt::FocusEvent )
}

// ::com::sun::star::awt::XFocusListener
void MRCListenerMultiplexerHelper::focusLost(const ::com::sun::star::awt::FocusEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XFocusListener, focusLost, ::com::sun::star::awt::FocusEvent )
}

// ::com::sun::star::awt::XWindowListener
void MRCListenerMultiplexerHelper::windowResized(const ::com::sun::star::awt::WindowEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XWindowListener, windowResized, ::com::sun::star::awt::WindowEvent )
}

// ::com::sun::star::awt::XWindowListener
void MRCListenerMultiplexerHelper::windowMoved(const ::com::sun::star::awt::WindowEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XWindowListener, windowMoved, ::com::sun::star::awt::WindowEvent )
}

// ::com::sun::star::awt::XWindowListener
void MRCListenerMultiplexerHelper::windowShown(const ::com::sun::star::lang::EventObject& e)
{
    MULTIPLEX( ::com::sun::star::awt::XWindowListener, windowShown, ::com::sun::star::lang::EventObject )
}

// ::com::sun::star::awt::XWindowListener
void MRCListenerMultiplexerHelper::windowHidden(const ::com::sun::star::lang::EventObject& e)
{
    MULTIPLEX( ::com::sun::star::awt::XWindowListener, windowHidden, ::com::sun::star::lang::EventObject )
}

// ::com::sun::star::awt::XKeyListener
void MRCListenerMultiplexerHelper::keyPressed(const ::com::sun::star::awt::KeyEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XKeyListener, keyPressed, ::com::sun::star::awt::KeyEvent )
}

// ::com::sun::star::awt::XKeyListener
void MRCListenerMultiplexerHelper::keyReleased(const ::com::sun::star::awt::KeyEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XKeyListener, keyReleased, ::com::sun::star::awt::KeyEvent )
}

// ::com::sun::star::awt::XMouseListener
void MRCListenerMultiplexerHelper::mousePressed(const ::com::sun::star::awt::MouseEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XMouseListener, mousePressed, ::com::sun::star::awt::MouseEvent )
}

// ::com::sun::star::awt::XMouseListener
void MRCListenerMultiplexerHelper::mouseReleased(const ::com::sun::star::awt::MouseEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XMouseListener, mouseReleased, ::com::sun::star::awt::MouseEvent )
}

// ::com::sun::star::awt::XMouseListener
void MRCListenerMultiplexerHelper::mouseEntered(const ::com::sun::star::awt::MouseEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XMouseListener, mouseEntered, ::com::sun::star::awt::MouseEvent )
}

// ::com::sun::star::awt::XMouseListener
void MRCListenerMultiplexerHelper::mouseExited(const ::com::sun::star::awt::MouseEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XMouseListener, mouseExited, ::com::sun::star::awt::MouseEvent )
}

// ::com::sun::star::awt::XMouseMotionListener
void MRCListenerMultiplexerHelper::mouseDragged(const ::com::sun::star::awt::MouseEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XMouseMotionListener, mouseDragged, ::com::sun::star::awt::MouseEvent )
}

// ::com::sun::star::awt::XMouseMotionListener
void MRCListenerMultiplexerHelper::mouseMoved(const ::com::sun::star::awt::MouseEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XMouseMotionListener, mouseMoved, ::com::sun::star::awt::MouseEvent )
}

// ::com::sun::star::awt::XPaintListener
void MRCListenerMultiplexerHelper::windowPaint(const ::com::sun::star::awt::PaintEvent& e)
{
    MULTIPLEX( ::com::sun::star::awt::XPaintListener, windowPaint, ::com::sun::star::awt::PaintEvent )
}

// ::com::sun::star::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowOpened(const ::com::sun::star::lang::EventObject& e)
{
    MULTIPLEX( ::com::sun::star::awt::XTopWindowListener, windowOpened, ::com::sun::star::lang::EventObject )
}

// ::com::sun::star::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowClosing( const ::com::sun::star::lang::EventObject& e )
{
    MULTIPLEX( ::com::sun::star::awt::XTopWindowListener, windowClosing, ::com::sun::star::lang::EventObject )
}

// ::com::sun::star::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowClosed( const ::com::sun::star::lang::EventObject& e )
{
    MULTIPLEX( ::com::sun::star::awt::XTopWindowListener, windowClosed, ::com::sun::star::lang::EventObject )
}

// ::com::sun::star::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowMinimized( const ::com::sun::star::lang::EventObject& e )
{
    MULTIPLEX( ::com::sun::star::awt::XTopWindowListener, windowMinimized, ::com::sun::star::lang::EventObject )
}

// ::com::sun::star::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowNormalized( const ::com::sun::star::lang::EventObject& e )
{
    MULTIPLEX( ::com::sun::star::awt::XTopWindowListener, windowNormalized, ::com::sun::star::lang::EventObject )
}

// ::com::sun::star::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowActivated( const ::com::sun::star::lang::EventObject& e )
{
    MULTIPLEX( ::com::sun::star::awt::XTopWindowListener, windowActivated, ::com::sun::star::lang::EventObject )
}

// ::com::sun::star::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowDeactivated( const ::com::sun::star::lang::EventObject& e )
{
    MULTIPLEX( ::com::sun::star::awt::XTopWindowListener, windowDeactivated, ::com::sun::star::lang::EventObject )
}
