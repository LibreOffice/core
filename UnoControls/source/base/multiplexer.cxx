/*************************************************************************
 *
 *  $RCSfile: multiplexer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:11:17 $
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

//____________________________________________________________________________________________________________
//  my own include
//____________________________________________________________________________________________________________

#ifndef _UNOCONTROLS_MULTIPLEXER_HXX
#include "multiplexer.hxx"
#endif

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif

//____________________________________________________________________________________________________________
//  includes of my own project
//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________
//  namespaces
//____________________________________________________________________________________________________________

using namespace ::cppu                  ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::awt   ;
using namespace ::com::sun::star::lang  ;

namespace unocontrols{

//____________________________________________________________________________________________________________
//  macros
//____________________________________________________________________________________________________________

#define MULTIPLEX( INTERFACE, METHOD, EVENTTYP, EVENT )                                                                             \
                                                                                                                                    \
    /* First get all interfaces from container with right type.*/                                                                   \
    OInterfaceContainerHelper* pContainer = m_aListenerHolder.getContainer( ::getCppuType((const Reference< INTERFACE## >*)0) );    \
    /* Do the follow only, if elements in container exist.*/                                                                        \
    if( pContainer != NULL )                                                                                                        \
    {                                                                                                                               \
        OInterfaceIteratorHelper aIterator( *pContainer );                                                                          \
        EVENTTYP aLocalEvent = EVENT##;                                                                                             \
        /* Remark: The control is the event source not the peer.*/                                                                  \
        /*         We must change the source of the event.      */                                                                  \
        aLocalEvent.Source = m_xControl ;                                                                                           \
        /* Is the control not destroyed? */                                                                                         \
        if( aLocalEvent.Source.is() == sal_True )                                                                                   \
        {                                                                                                                           \
            if( aIterator.hasMoreElements() )                                                                                       \
            {                                                                                                                       \
                INTERFACE## * pListener = (INTERFACE## *)aIterator.next();                                                          \
                try                                                                                                                 \
                {                                                                                                                   \
                    pListener->METHOD##( aLocalEvent );                                                                             \
                }                                                                                                                   \
                catch( RuntimeException& )                                                                                          \
                {                                                                                                                   \
                    /* Ignore all system exceptions from the listener! */                                                           \
                }                                                                                                                   \
            }                                                                                                                       \
        }                                                                                                                           \
    }

//____________________________________________________________________________________________________________
//  construct/destruct
//____________________________________________________________________________________________________________

OMRCListenerMultiplexerHelper::OMRCListenerMultiplexerHelper(   const   Reference< XWindow >&   xControl    ,
                                                                const   Reference< XWindow >&   xPeer       )
    : m_aListenerHolder ( m_aMutex  )
    , m_xControl        ( xControl  )
    , m_xPeer           ( xPeer     )
{
}

OMRCListenerMultiplexerHelper::OMRCListenerMultiplexerHelper( const OMRCListenerMultiplexerHelper& aCopyInstance )
    : m_aListenerHolder ( m_aMutex  )
{
}

OMRCListenerMultiplexerHelper::~OMRCListenerMultiplexerHelper()
{
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

Any SAL_CALL OMRCListenerMultiplexerHelper::queryInterface( const Type& rType ) throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface(   rType                                           ,
                                            static_cast< XWindowListener*       > ( this )  ,
                                            static_cast< XKeyListener*          > ( this )  ,
                                            static_cast< XFocusListener*        > ( this )  ,
                                            static_cast< XMouseListener*        > ( this )  ,
                                            static_cast< XMouseMotionListener*  > ( this )  ,
                                            static_cast< XPaintListener*        > ( this )  ,
                                            static_cast< XTopWindowListener*    > ( this )  ,
                                            static_cast< XTopWindowListener*    > ( this )
                                        )
                );

    // If searched interface supported by this class ...
    if ( aReturn.hasValue() == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return OWeakObject::queryInterface( rType );
    }
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL OMRCListenerMultiplexerHelper::acquire() throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL OMRCListenerMultiplexerHelper::release() throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::release();
}

//____________________________________________________________________________________________________________
//  operator
//____________________________________________________________________________________________________________

OMRCListenerMultiplexerHelper::operator Reference< XInterface >() const
{
    return ((OWeakObject*)this) ;
}

//____________________________________________________________________________________________________________
//  operator
//____________________________________________________________________________________________________________

//OMRCListenerMultiplexerHelper& OMRCListenerMultiplexerHelper::operator= ( const OMRCListenerMultiplexerHelper& aCopyInstance )
//{
//  return this ;
//}

//____________________________________________________________________________________________________________
//  container method
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::setPeer( const Reference< XWindow >& xPeer )
{
    MutexGuard aGuard( m_aMutex );
    if( m_xPeer != xPeer )
    {
        if( m_xPeer.is() )
        {
            // get all types from the listener added to the peer
            Sequence< Type >    aContainedTypes = m_aListenerHolder.getContainedTypes();
            const Type*         pArray          = aContainedTypes.getConstArray();
            sal_Int32           nCount          = aContainedTypes.getLength();
            // loop over all listener types and remove the listeners from the peer
            for( sal_Int32 i=0; i<nCount; i++ )
                impl_unadviseFromPeer( m_xPeer, pArray[i] );
        }
        m_xPeer = xPeer;
        if( m_xPeer.is() )
        {
            // get all types from the listener added to the peer
            Sequence< Type >    aContainedTypes = m_aListenerHolder.getContainedTypes();
            const Type*         pArray          = aContainedTypes.getConstArray();
            sal_Int32           nCount          = aContainedTypes.getLength();
            // loop over all listener types and add the listeners to the peer
            for( sal_Int32 i = 0; i < nCount; i++ )
                impl_adviseToPeer( m_xPeer, pArray[i] );
        }
    }
}

//____________________________________________________________________________________________________________
//  container method
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::disposeAndClear()
{
    EventObject aEvent ;
    aEvent.Source = m_xControl ;
    m_aListenerHolder.disposeAndClear( aEvent );
}

//____________________________________________________________________________________________________________
//  container method
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::advise( const   Type&                       aType       ,
                                            const   Reference< XInterface >&    xListener   )
{
    MutexGuard aGuard( m_aMutex );
    if( m_aListenerHolder.addInterface( aType, xListener ) == 1 )
    {
        // the first listener is added
        if( m_xPeer.is() )
        {
            impl_adviseToPeer( m_xPeer, aType );
        }
    }
}

//____________________________________________________________________________________________________________
//  container method
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::unadvise(   const   Type&                       aType       ,
                                                const   Reference< XInterface >&    xListener   )
{
    MutexGuard aGuard( m_aMutex );
    if( m_aListenerHolder.removeInterface( aType, xListener ) == 0 )
    {
        // the last listener is removed
        if ( m_xPeer.is() )
        {
            impl_unadviseFromPeer( m_xPeer, aType );
        }
    }
}

//____________________________________________________________________________________________________________
//  XEventListener
//____________________________________________________________________________________________________________

void SAL_CALL OMRCListenerMultiplexerHelper::disposing( const EventObject& aSource ) throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );
    // peer is disposed, clear the reference
    m_xPeer = Reference< XWindow >();
}

//____________________________________________________________________________________________________________
//  XFcousListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::focusGained(const FocusEvent& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
/*
    OInterfaceContainerHelper * pCont = aListenerHolder.getContainer( ::getCppuType((const Reference< XFocusListener >*)0) );
    if( pCont )
    {
        OInterfaceIteratorHelper    aIt( *pCont );
        FocusEvent aEvt = e;
        // Reamark: The control is the event source not the peer. We must change
        // the source of the event
        xControl.queryHardRef( ((XInterface*)NULL)->getSmartUik(), aEvt.Source );
        //.is the control not destroyed
        if( aEvt.Source.is() )
        {
            if( aIt.hasMoreElements() )
            {
                XFocusListener * pListener = (XFocusListener *)aIt.next();
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
    MULTIPLEX( XFocusListener, focusGained, FocusEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XFcousListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::focusLost(const FocusEvent& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XFocusListener, focusLost, FocusEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowResized(const WindowEvent& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XWindowListener, windowResized, WindowEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowMoved(const WindowEvent& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XWindowListener, windowMoved, WindowEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowShown(const EventObject& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XWindowListener, windowShown, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  XWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowHidden(const EventObject& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XWindowListener, windowHidden, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  XKeyListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::keyPressed(const KeyEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XKeyListener, keyPressed, KeyEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XKeyListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::keyReleased(const KeyEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XKeyListener, keyReleased, KeyEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XMouseListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::mousePressed(const MouseEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XMouseListener, mousePressed, MouseEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XMouseListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::mouseReleased(const MouseEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XMouseListener, mouseReleased, MouseEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XMouseListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::mouseEntered(const MouseEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XMouseListener, mouseEntered, MouseEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XMouseListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::mouseExited(const MouseEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XMouseListener, mouseExited, MouseEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XMouseMotionListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::mouseDragged(const MouseEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XMouseMotionListener, mouseDragged, MouseEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XMouseMotionListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::mouseMoved(const MouseEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XMouseMotionListener, mouseMoved, MouseEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XPaintListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowPaint(const PaintEvent& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XPaintListener, windowPaint, PaintEvent, aEvent )
}

//____________________________________________________________________________________________________________
//  XTopWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowOpened(const EventObject& aEvent) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XTopWindowListener, windowOpened, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  XTopWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowClosing( const EventObject& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XTopWindowListener, windowClosing, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  XTopWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowClosed( const EventObject& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XTopWindowListener, windowClosed, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  XTopWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowMinimized( const EventObject& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XTopWindowListener, windowMinimized, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  XTopWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowNormalized( const EventObject& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XTopWindowListener, windowNormalized, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  XTopWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowActivated( const EventObject& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XTopWindowListener, windowActivated, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  XTopWindowListener
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::windowDeactivated( const EventObject& aEvent ) throw( UNO3_RUNTIMEEXCEPTION )
{
    MULTIPLEX( XTopWindowListener, windowDeactivated, EventObject, aEvent )
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::impl_adviseToPeer(  const   Reference< XWindow >&   xPeer   ,
                                                        const   Type&                   aType   )
{
    // add a listener to the source (peer)
    if( aType == ::getCppuType((const Reference< XWindowListener >*)0) )
        xPeer->addWindowListener( this );
    else if( aType == ::getCppuType((const Reference< XKeyListener >*)0) )
        xPeer->addKeyListener( this );
    else if( aType == ::getCppuType((const Reference< XFocusListener >*)0) )
        xPeer->addFocusListener( this );
    else if( aType == ::getCppuType((const Reference< XMouseListener >*)0) )
        xPeer->addMouseListener( this );
    else if( aType == ::getCppuType((const Reference< XMouseMotionListener >*)0) )
        xPeer->addMouseMotionListener( this );
    else if( aType == ::getCppuType((const Reference< XPaintListener >*)0) )
        xPeer->addPaintListener( this );
    else if( aType == ::getCppuType((const Reference< XTopWindowListener >*)0) )
    {
        Reference< XTopWindow > xTop( xPeer, UNO_QUERY );
        if( xTop.is() )
            xTop->addTopWindowListener( this );
    }
    else
    {
        VOS_ENSHURE( sal_False, "unknown listener" );
    }
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void OMRCListenerMultiplexerHelper::impl_unadviseFromPeer(  const   Reference< XWindow >&   xPeer   ,
                                                            const   Type&                   aType   )
{
    // the last listener is removed, remove the listener from the source (peer)
    if( aType == ::getCppuType((const Reference< XWindowListener >*)0) )
        xPeer->removeWindowListener( this );
    else if( aType == ::getCppuType((const Reference< XKeyListener >*)0) )
        xPeer->removeKeyListener( this );
    else if( aType == ::getCppuType((const Reference< XFocusListener >*)0) )
        xPeer->removeFocusListener( this );
    else if( aType == ::getCppuType((const Reference< XMouseListener >*)0) )
        xPeer->removeMouseListener( this );
    else if( aType == ::getCppuType((const Reference< XMouseMotionListener >*)0) )
        xPeer->removeMouseMotionListener( this );
    else if( aType == ::getCppuType((const Reference< XPaintListener >*)0) )
        xPeer->removePaintListener( this );
    else if( aType == ::getCppuType((const Reference< XTopWindowListener >*)0) )
    {
        Reference< XTopWindow >  xTop( xPeer, UNO_QUERY );
        if( xTop.is() )
            xTop->removeTopWindowListener( this );
    }
    else
    {
        VOS_ENSHURE( sal_False, "unknown listener" );
    }
}

} // namespace unocontrols
