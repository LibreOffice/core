/*************************************************************************
 *
 *  $RCSfile: weakeventlistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 15:58:31 $
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

#ifndef COMPHELPER_WEAKEVENTLISTENER_HXX
#define COMPHELPER_WEAKEVENTLISTENER_HXX

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XWEAK_HPP_
#include <com/sun/star/uno/XWeak.hpp>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= OWeakListenerAdapterBase
    //=====================================================================
    /** (the base for) an adapter which allows to add as listener to a foreign component, without
        being held hard.

        <p>The idea is that this adapter is added as listener to a foreign component, which usually
        holds it's listener hard. The adapter itself knows the real listener as weak reference,
        thus not affecting it's life time.</p>
    */
    class OWeakListenerAdapterBase : public OBaseMutex
    {
    private:
        ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >
                m_aListener;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                m_xBroadcaster;

    protected:
        inline ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                getListener( ) const
        {
            return m_aListener.get();
        }

        inline const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&
                getBroadcaster( ) const
        {
            return m_xBroadcaster;
        }

        inline void resetListener( )
        {
            m_aListener = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >();
        }


    protected:
        inline OWeakListenerAdapterBase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak >& _rxListener,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxBroadcaster
        )
            :m_aListener    (  _rxListener )
            ,m_xBroadcaster ( _rxBroadcaster )
        {
        }

    protected:
        virtual ~OWeakListenerAdapterBase();
    };


    //=====================================================================
    //= OWeakListenerAdapter
    //=====================================================================
    template< class BROADCASTER, class LISTENER >
    /** yet another base for weak listener adapters, this time with some type safety

        <p>Note that derived classes need to overwrite all virtual methods of their interface
        except XEventListener::disposing, and forward it to their master listener.</p>

        <p>Addtionally, derived classes need to add themself as listener to the broadcaster,
        as this can't be done in a generic way</p>
    */
    class OWeakListenerAdapter
            :public ::cppu::WeakComponentImplHelper1 < LISTENER >
            ,public OWeakListenerAdapterBase
    {
    protected:
        /** ctor
            <p>Note that derived classes still need to add themself as listener to the broadcaster,
            as this can't be done in a generic way</p>
        */
        OWeakListenerAdapter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak >& _rxListener,
            const ::com::sun::star::uno::Reference< BROADCASTER >& _rxBroadcaster
        );

    protected:
        inline  ::com::sun::star::uno::Reference< LISTENER > getListener( ) const
        {
            return  ::com::sun::star::uno::Reference< LISTENER >( OWeakListenerAdapterBase::getListener(), ::com::sun::star::uno::UNO_QUERY );
        }

        // XEventListener overridables
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        // OComponentHelper overridables
        // to be overridden, again - the derived class should revoke the listener from the broadcaster
        virtual void SAL_CALL disposing( ) = 0;
    };

    //=====================================================================
    //= OWeakEventListenerAdapter
    //=====================================================================
    typedef OWeakListenerAdapter    <   ::com::sun::star::lang::XComponent
                                    ,   ::com::sun::star::lang::XEventListener
                                    >   OWeakEventListenerAdapter_Base;
    /** the most simple listener adapter: for XEventListeners at XComponents
    */
    class OWeakEventListenerAdapter : public OWeakEventListenerAdapter_Base
    {
    public:
        OWeakEventListenerAdapter(
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak > _rxListener,
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > _rxBroadcaster
        );

        // nothing to do except an own ctor - the forwarding of the "disposing" is already done
        // in the base class

    protected:
        virtual void SAL_CALL disposing( );
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#define INCLUDED_BY_WEAKEVENTLISTENER_HXX
#include "weakeventlistener_impl.hxx"

#endif// COMPHELPER_WEAKEVENTLISTENER_HXX


