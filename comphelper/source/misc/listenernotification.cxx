/*************************************************************************
 *
 *  $RCSfile: listenernotification.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 21:08:50 $
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

#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#include <comphelper/listenernotification.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
/** === end UNO includes === **/

//........................................................................
namespace comphelper
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //====================================================================
    //= OListenerContainer
    //====================================================================
    //--------------------------------------------------------------------
    OListenerContainer::OListenerContainer( ::osl::Mutex& _rMutex )
        :m_aListeners( _rMutex )
    {
    }

    //--------------------------------------------------------------------
    void OListenerContainer::addListener( const Reference< XEventListener >& _rxListener )
    {
        OSL_PRECOND( _rxListener.is(), "OListenerContainer::addListener: a NULL listener?!" );
        if ( _rxListener.is() )
            m_aListeners.addInterface( _rxListener );
    }

    //--------------------------------------------------------------------
    void OListenerContainer::removeListener( const Reference< XEventListener >& _rxListener )
    {
#if OSL_DEBUG_LEVEL > 0
        ::cppu::OInterfaceIteratorHelper aIter( m_aListeners );
        bool bFound = false;
        while ( aIter.hasMoreElements() && !bFound )
        {
            bFound = ( Reference< XInterface >( aIter.next() ) == _rxListener );
        }
        OSL_ENSURE( bFound, "OListenerContainer::removeListener: sure your listener handling is correct? The given listener is not registered!" );
#endif
        m_aListeners.removeInterface( _rxListener );
    }

    //--------------------------------------------------------------------
    void OListenerContainer::disposing( const EventObject& _rEventSource )
    {
        m_aListeners.disposeAndClear( _rEventSource );
    }

    //--------------------------------------------------------------------
    bool OListenerContainer::notify( const EventObject& _rEvent ) SAL_THROW(( Exception ))
    {
        ::cppu::OInterfaceIteratorHelper aIter( m_aListeners );
        bool bCancelled = false;
        while ( aIter.hasMoreElements() && !bCancelled )
        {
            Reference< XEventListener > xListener( static_cast< XEventListener* >( aIter.next() ) );
            if ( !xListener.is() )
                continue;

            try
            {
                bCancelled = !implNotify( xListener, _rEvent );
            }
            catch( const DisposedException& e )
            {
                // DisposedExceptions from the listener might indicate a
                // broken connection to a different environment.

                OSL_ENSURE( e.Context.is(), "OListenerContainer::notify: caught dispose exception with empty Context field" );

                // If the exception stems from the listener then remove it
                // from the list of listeners.  If the Context field of the
                // exception is empty this is interpreted to indicate the
                // listener as well.
                if ( e.Context == xListener || !e.Context.is() )
                    aIter.remove();
            }
        }

        return !bCancelled;
    }

//........................................................................
}   // namespace comphelper
//........................................................................

