/*************************************************************************
 *
 *  $RCSfile: accessiblecontexthelper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: sb $ $Date: 2002-07-22 07:00:37 $
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

#ifndef COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX
#include <comphelper/accessiblecontexthelper.hxx>
#endif
#ifndef INCLUDED_COMPHELPER_ACCESSIBLEEVENTBUFFER_HXX
#include <comphelper/accessibleeventbuffer.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::drafts::com::sun::star::accessibility;

    //=====================================================================
    //= OContextHelper_Impl
    //=====================================================================
    /** implementation class for OAccessibleContextHelper. No own thread safety!
    */
    class OContextHelper_Impl
    {
    private:
        OAccessibleContextHelper*           m_pAntiImpl;        // the owning instance
        IMutex*                             m_pExternalLock;    // the optional additional external lock

        ::cppu::OInterfaceContainerHelper*  m_pEventListeners;
        WeakReference< XAccessible >        m_aCreator;     // the XAccessible which created our XAccessibleContext

    public:
        ::cppu::OInterfaceContainerHelper*  getListenerContainer( sal_Bool _bCreate = sal_True );
            // not const - will create if necessary

        inline  Reference< XAccessible >    getCreator( ) const { return m_aCreator; }
        inline  void                        setCreator( const Reference< XAccessible >& _rAcc );

        inline  IMutex*                     getExternalLock( )                  { return m_pExternalLock; }
        inline  void                        setExternalLock( IMutex* _pLock )   { m_pExternalLock = _pLock; }

    public:
        OContextHelper_Impl( OAccessibleContextHelper* _pAntiImpl )
            :m_pAntiImpl( _pAntiImpl )
            ,m_pExternalLock( NULL )
            ,m_pEventListeners( NULL )
        {
        }
    };

    //---------------------------------------------------------------------
    inline  void OContextHelper_Impl::setCreator( const Reference< XAccessible >& _rAcc )
    {
        m_aCreator = _rAcc;
    }

    //---------------------------------------------------------------------
    ::cppu::OInterfaceContainerHelper* OContextHelper_Impl::getListenerContainer( sal_Bool _bCreate )
    {
        if ( !m_pEventListeners && _bCreate )
            m_pEventListeners = new ::cppu::OInterfaceContainerHelper( m_pAntiImpl->GetMutex( OAccessibleContextHelper::OAccessControl() ) );
        return m_pEventListeners;
    }

    //=====================================================================
    //= OAccessibleContextHelper
    //=====================================================================
    //---------------------------------------------------------------------
    OAccessibleContextHelper::OAccessibleContextHelper( )
        :OAccessibleContextHelper_Base( GetMutex() )
        ,m_pImpl( NULL )
    {
        m_pImpl = new OContextHelper_Impl( this );
    }

    //---------------------------------------------------------------------
    OAccessibleContextHelper::OAccessibleContextHelper( IMutex* _pExternalLock )
        :OAccessibleContextHelper_Base( GetMutex() )
        ,m_pImpl( NULL )
    {
        m_pImpl = new OContextHelper_Impl( this );
        m_pImpl->setExternalLock( _pExternalLock );
    }

    //---------------------------------------------------------------------
    void OAccessibleContextHelper::forgetExternalLock()
    {
        m_pImpl->setExternalLock( NULL );
    }

    //---------------------------------------------------------------------
    OAccessibleContextHelper::~OAccessibleContextHelper( )
    {
        forgetExternalLock();
            // this ensures that the lock, which may be already destroyed as part of the derivee,
            // is not used anymore

        ensureDisposed();

        delete m_pImpl;
        m_pImpl = NULL;
    }

    //---------------------------------------------------------------------
    IMutex* OAccessibleContextHelper::getExternalLock( )
    {
        return m_pImpl->getExternalLock();
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAccessibleContextHelper::disposing()
    {
        // notify our listeners that we're going to be defunc
        NotifyAccessibleEvent(
            AccessibleEventId::ACCESSIBLE_STATE_EVENT,
            Any(),
            makeAny( AccessibleStateType::DEFUNC )
        );

        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        ::cppu::OInterfaceContainerHelper* pListeners = m_pImpl->getListenerContainer( sal_False );
        if ( pListeners )
        {
            EventObject aDisposee( *this );
            aGuard.clear();
            pListeners->disposeAndClear( aDisposee );
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAccessibleContextHelper::addEventListener( const Reference< XAccessibleEventListener >& _rxListener ) throw (RuntimeException)
    {
        OContextEntryGuard aGuard( this );
        if ( _rxListener.is() )
            m_pImpl->getListenerContainer()->addInterface( _rxListener );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAccessibleContextHelper::removeEventListener( const Reference< XAccessibleEventListener >& _rxListener ) throw (RuntimeException)
    {
        OContextEntryGuard aGuard( this );
        if ( _rxListener.is() )
            m_pImpl->getListenerContainer()->removeInterface( _rxListener );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAccessibleContextHelper::NotifyAccessibleEvent( const sal_Int16 _nEventId,
        const Any& _rOldValue, const Any& _rNewValue )
    {
        // copy our current listeners
        ::cppu::OInterfaceContainerHelper*  pListeners = m_pImpl->getListenerContainer( sal_False );
        Sequence< Reference< XInterface > > aListeners;
        if ( pListeners )
            aListeners = pListeners->getElements();

        if ( aListeners.getLength() )
        {
            AccessibleEventObject aEvent;
            aEvent.Source = *this;
            OSL_ENSURE( aEvent.Source.is(), "OAccessibleContextHelper::NotifyAccessibleEvent: invalid creator!" );
            aEvent.EventId = _nEventId;
            aEvent.OldValue = _rOldValue;
            aEvent.NewValue = _rNewValue;

            const Reference< XInterface >*  pLoop = aListeners.getConstArray();
            const Reference< XInterface >*  pLoopEnd = pLoop + aListeners.getLength();

            while ( pLoop != pLoopEnd )
            {
                try
                {
                    while ( pLoop != pLoopEnd )
                    {
                        XAccessibleEventListener* pListener = static_cast< XAccessibleEventListener* > (
                            pLoop->get() );
                        // note that this cast is valid:
                        // We added the interface to our listener container, and at this time it was an
                        // XAccessibleEventListener. As we did not query for XInterface, but instead used
                        // the XInterface which is the base of XAccessibleEventListener, we can now safely
                        // cast.

                        if ( pListener )
                            pListener->notifyEvent( aEvent );

                        ++pLoop;
                    }
                }
                catch( const Exception& e )
                {
                    e;  // make compiler happy
                    // skip this listener and continue with the next one
                    ++pLoop;
                }
            }
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAccessibleContextHelper::NotifyAccessibleEvent( const sal_Int16 _nEventId,
        const Any& _rOldValue, const Any& _rNewValue,
        AccessibleEventBuffer & _rBuffer )
    {
        // copy our current listeners
        ::cppu::OInterfaceContainerHelper*  pListeners = m_pImpl->getListenerContainer( sal_False );
        Sequence< Reference< XInterface > > aListeners;
        if ( pListeners )
            aListeners = pListeners->getElements();

        if ( aListeners.getLength() )
        {
            AccessibleEventObject aEvent;
            aEvent.Source = *this;
            OSL_ENSURE( aEvent.Source.is(), "OAccessibleContextHelper::NotifyAccessibleEvent: invalid creator!" );
            aEvent.EventId = _nEventId;
            aEvent.OldValue = _rOldValue;
            aEvent.NewValue = _rNewValue;

            _rBuffer.addEvent( aEvent, aListeners );
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OAccessibleContextHelper::isAlive() const
    {
        return !GetBroadcastHelper().bDisposed && !GetBroadcastHelper().bInDispose;
    }

    //---------------------------------------------------------------------
    void OAccessibleContextHelper::ensureAlive() const SAL_THROW( ( DisposedException ) )
    {
        if( !isAlive() )
            throw DisposedException();
    }

    //---------------------------------------------------------------------
    void OAccessibleContextHelper::ensureDisposed( )
    {
        if ( !GetBroadcastHelper().bDisposed )
        {
            OSL_ENSURE( 0 == m_refCount, "OAccessibleContextHelper::ensureDisposed: this method _has_ to be called from without your dtor only!" );
            acquire();
            dispose();
        }
    }

    //---------------------------------------------------------------------
    void OAccessibleContextHelper::lateInit( const Reference< XAccessible >& _rxAccessible )
    {
        m_pImpl->setCreator( _rxAccessible );
    }

    //---------------------------------------------------------------------
    Reference< XAccessible > OAccessibleContextHelper::getAccessibleCreator( ) const
    {
        return m_pImpl->getCreator();
    }

    //---------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleContextHelper::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        // -1 for child not found/no parent (according to specification)
        sal_Int32 nRet = -1;

        try
        {

            Reference< XAccessibleContext > xParentContext( implGetParentContext() );

            //  iterate over parent's children and search for this object
            if ( xParentContext.is() )
            {
                // our own XAccessible for comparing with the children of our parent
                Reference< XAccessible > xCreator( m_pImpl->getCreator() );

                OSL_ENSURE( xCreator.is(), "OAccessibleContextHelper::getAccessibleIndexInParent: invalid creator!" );
                    // two ideas why this could be NULL:
                    // * nobody called our late ctor (init), so we never had a creator at all -> bad
                    // * the creator is already dead. In this case, we should have been disposed, and
                    //   never survived the above OContextEntryGuard.
                    // in all other situations the creator should be non-NULL

                if ( xCreator.is() )
                {
                    sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
                    for ( sal_Int32 nChild = 0; ( nChild < nChildCount ) && ( -1 == nRet ); ++nChild )
                    {
                        Reference< XAccessible > xChild( xParentContext->getAccessibleChild( nChild ) );
                        if ( xChild.get() == xCreator.get() )
                            nRet = nChild;
                    }
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OAccessibleContextHelper::getAccessibleIndexInParent: caught an exception!" );
        }

        return nRet;
    }

    //---------------------------------------------------------------------
    Locale SAL_CALL OAccessibleContextHelper::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        // simply ask the parent
        Reference< XAccessible > xParent = getAccessibleParent();
        Reference< XAccessibleContext > xParentContext;
        if ( xParent.is() )
            xParentContext = xParent->getAccessibleContext();

        if ( !xParentContext.is() )
            throw IllegalAccessibleComponentStateException( ::rtl::OUString(), *this );

        return xParentContext->getLocale();
    }

    //---------------------------------------------------------------------
    Reference< XAccessibleContext > OAccessibleContextHelper::implGetParentContext() SAL_THROW( ( RuntimeException ) )
    {
        Reference< XAccessible > xParent = getAccessibleParent();
        Reference< XAccessibleContext > xParentContext;
        if ( xParent.is() )
            xParentContext = xParent->getAccessibleContext();
        return xParentContext;
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.7  2002/05/08 15:38:36  fs
 *  #99218# allow abstract external locks in addition to the own mutex
 *
 *  Revision 1.6  2002/05/08 07:54:46  fs
 *  #98750# no use the context (not the XAccessible) as event source, again, as usual in the UNO world
 *
 *  Revision 1.5  2002/04/30 07:42:27  hr
 *  #65293#: removed not needed vcl/svapp.hxx includes to reduce dependencies
 *
 *  Revision 1.4  2002/04/26 14:24:28  fs
 *  #98750# +getAccessibleCreator / use the creator (XAccessible) as event source
 *
 *  Revision 1.3  2002/04/26 07:25:50  fs
 *  #98750# corrected NotifyAccessibleEvent
 *
 *  Revision 1.2  2002/04/26 05:52:18  fs
 *  #98750# use correct broadcasthelper (in the WeagAggComponentImpl* base)
 *
 *  Revision 1.1  2002/04/23 11:10:30  fs
 *  initial checkin - helper for implementing an XAccessibleContext
 *
 *
 *  Revision 1.0 17.04.2002 16:06:46  fs
 ************************************************************************/

