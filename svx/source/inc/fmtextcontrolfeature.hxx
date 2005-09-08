/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtextcontrolfeature.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:20:28 $
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

#ifndef SVX_SOURCE_INC_FMTEXTCONTROLFEATURE_HXX
#define SVX_SOURCE_INC_FMTEXTCONTROLFEATURE_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef SVX_SOURCE_INC_FMSLOTINVALIDATOR_HXX
#include "fmslotinvalidator.hxx"
#endif

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= FmTextControlFeature
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::frame::XStatusListener
                                    >   FmTextControlFeature_Base;

    class FmTextControlFeature : public FmTextControlFeature_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
                                        m_xDispatcher;
        ::com::sun::star::util::URL     m_aFeatureURL;
        ::com::sun::star::uno::Any      m_aFeatureState;
        SfxSlotId                       m_nSlotId;
        ISlotInvalidator*               m_pInvalidator;
        bool                            m_bFeatureEnabled;

    public:
        /** constructs an FmTextControlFeature object
        @param _rxDispatcher
            the dispatcher which the instance should work with
        @param _rFeatureURL
            the URL which the instance should be responsible for
        */
        FmTextControlFeature(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& _rxDispatcher,
            const ::com::sun::star::util::URL& _rFeatureURL,
            SfxSlotId _nId,
            ISlotInvalidator* _pInvalidator
        );

        /// determines whether the feature we're responsible for is currently enabled
        inline  bool                                isFeatureEnabled( ) const { return m_bFeatureEnabled; }
        inline  const ::com::sun::star::uno::Any&   getFeatureState( ) const { return m_aFeatureState; }

        /** dispatches the feature URL to the dispatcher
        */
        void    dispatch() const SAL_THROW(());

        /** dispatches the feature URL to the dispatcher, with passing the given arguments
        */
        void    dispatch( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArgs ) const SAL_THROW(());

        /// releases any resources associated with this instance
        void    dispose() SAL_THROW(());

    protected:
        ~FmTextControlFeature();

    protected:
        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& State ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_SOURCE_INC_FMTEXTCONTROLFEATURE_HXX

