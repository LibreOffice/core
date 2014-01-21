/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef __FRAMEWORK_UIELEMENT_GENERICSTATUSBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_GENERICSTATUSBARCONTROLLER_HXX_

#include <svtools/statusbarcontroller.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>

namespace framework
{

struct AddonStatusbarItemData;

class GenericStatusbarController : public svt::StatusbarController
{
    public:
        GenericStatusbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                    const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                    const com::sun::star::uno::Reference< com::sun::star::ui::XStatusbarItem >& rxItem,
                                    AddonStatusbarItemData *pItemData );
        virtual ~GenericStatusbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );
        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
                                     const ::com::sun::star::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nStyle ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        sal_Bool m_bEnabled;
        sal_Bool m_bOwnerDraw;
        AddonStatusbarItemData *m_pItemData;
        ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > m_xGraphic;
};

}

#endif
