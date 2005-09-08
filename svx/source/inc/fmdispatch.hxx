/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmdispatch.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:15:22 $
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

#ifndef SVX_FMDISPATCH_HXX
#define SVX_FMDISPATCH_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif


//........................................................................
namespace svx
{
//........................................................................

    class FormControllerHelper;

    //====================================================================
    //= OSingleFeatureDispatcher
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::frame::XDispatch
                                    >   OSingleFeatureDispatcher_Base;

    class OSingleFeatureDispatcher : public OSingleFeatureDispatcher_Base
    {
    private:
        ::osl::Mutex&                       m_rMutex;
        ::cppu::OInterfaceContainerHelper   m_aStatusListeners;
        const FormControllerHelper&         m_rController;
        const ::com::sun::star::util::URL   m_aFeatureURL;
        ::com::sun::star::uno::Any          m_aLastKnownState;
        const sal_Int32                     m_nFeatureId;
        sal_Bool                            m_bLastKnownEnabled;
        sal_Bool                            m_bDisposed;

    public:
        /** constructs the dispatcher

            @param _rFeatureURL
                the URL of the feature which this instance is responsible for

            @param _nFeatureId
                the feature which this instance is responsible for

            @param _rController
                the controller which is responsible for providing the state of feature of this instance,
                and for executing it. After disposing the dispatcher instance, the controller will
                not be accessed anymore

            @see dispose
        */
        OSingleFeatureDispatcher(
            const ::com::sun::star::util::URL& _rFeatureURL,
            sal_Int32 _nFeatureId,
            const FormControllerHelper& _rController,
                  ::osl::Mutex& _rMutex
        );

        /** disposes the dispatcher instance

            All status listeners will, after receiving an <member scope="com::sun::star::lang">XEventListener::disposing</member>
            call, be released.

            The controller provided in the in constructor will not be used anymore after returning from this call.

            No further requests to dispatch slots will be accepted.

            Multiple calls are allowed: if the object already was disposed, then subsequent calls are
            silently ignored.
        */
        void    dispose();

        /** notifies all our listeners of the current state
        */
        void    updateAllListeners();

    protected:
        // XDispatch
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& _rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArguments ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxControl, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxControl, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        /** notifies our current state to one or all listeners

            @param _rxListener
                the listener to notify. May be NULL, in this case all our listeners will be
                notified with the current state

            @param _rFreeForNotification
                a guard which currently locks our mutex, and which is to be cleared
                for actually doing the notification(s)
        */
        void    notifyStatus(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener,
                    ::osl::ClearableMutexGuard& _rFreeForNotification
                );

    private:
        /** checks whether our instance is alive

            If the instance already received a <member>dispose</member> call, then a
            <type scope="com::sun::star::lang">DisposedException</type> is thrown.

            @precond
                our Mutex is locked - else calling the method would not make sense, since
                it's result could be out-of-date as soon as it's returned to the caller.
        */
        void    checkAlive() const SAL_THROW((::com::sun::star::lang::DisposedException));

        /** retrieves the current status of our feature, in a format which can be used
            for UNO notifications

            @precond
                our mutex is locked
        */
        void    getUnoState( ::com::sun::star::frame::FeatureStateEvent& /* [out] */ _rState ) const;

    private:
        OSingleFeatureDispatcher();                                             // never implemented
        OSingleFeatureDispatcher( const OSingleFeatureDispatcher& );            // never implemented
        OSingleFeatureDispatcher& operator=( const OSingleFeatureDispatcher& ); // never implemented
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif
