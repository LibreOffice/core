/*************************************************************************
 *
 *  $RCSfile: fmtextcontrolfeature.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 15:50:17 $
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

