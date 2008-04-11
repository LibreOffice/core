/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: featuredispatcher.hxx,v $
 * $Revision: 1.4 $
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

#ifndef FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX
#define FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

class EditView;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= ORichTextFeatureDispatcher
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::frame::XDispatch
                                    >   ORichTextFeatureDispatcher_Base;

    class ORichTextFeatureDispatcher    :public ::comphelper::OBaseMutex
                                        ,public ORichTextFeatureDispatcher_Base
    {
    private:
        ::com::sun::star::util::URL         m_aFeatureURL;
        ::cppu::OInterfaceContainerHelper   m_aStatusListeners;
        EditView*                           m_pEditView;
        bool                                m_bDisposed;

    protected:
              EditView*   getEditView()       { return m_pEditView; }
        const EditView*   getEditView() const { return m_pEditView; }

    protected:
        inline const ::com::sun::star::util::URL&       getFeatureURL() const { return m_aFeatureURL; }
        inline       ::cppu::OInterfaceContainerHelper& getStatusListeners() { return m_aStatusListeners; }
        inline       bool                               isDisposed() const { return m_bDisposed; }
        inline       void                               checkDisposed() const { if ( isDisposed() ) throw ::com::sun::star::lang::DisposedException(); }

    protected:
        ORichTextFeatureDispatcher( EditView& _rView, const ::com::sun::star::util::URL&  _rURL );
        ~ORichTextFeatureDispatcher( );

    public:
        /// clean up resources associated with this instance
        void    dispose();

        // invalidate the feature, re-retrieve it's state, and broadcast changes, if necessary
        void    invalidate();

    protected:
        // overridables
        virtual void    newStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener );
        virtual void    disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify );
        virtual void    invalidateFeatureState_Broadcast();

        // to be overridden, and filled with the info special do your derived class
        virtual ::com::sun::star::frame::FeatureStateEvent
                        buildStatusEvent() const;

        void    doNotify(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener,
                    const ::com::sun::star::frame::FeatureStateEvent& _rEvent
                ) const SAL_THROW(());

    protected:
        // XDispatch
        virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxControl, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxControl, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException);
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX

