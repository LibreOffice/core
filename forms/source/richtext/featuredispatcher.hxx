/*************************************************************************
 *
 *  $RCSfile: featuredispatcher.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:11:43 $
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

#ifndef FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX
#define FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

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

