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



#ifndef _TOOLKIT_AWT_VCLXCONTAINER_HXX_
#define _TOOLKIT_AWT_VCLXCONTAINER_HXX_


#include <com/sun/star/awt/XVclContainer.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <toolkit/awt/vclxwindow.hxx>


//  ----------------------------------------------------
//  class VCLXContainer
//  ----------------------------------------------------

class TOOLKIT_DLLPUBLIC VCLXContainer : public ::com::sun::star::awt::XVclContainer,
                        public ::com::sun::star::awt::XVclContainerPeer,
                        public VCLXWindow
{
public:
                    VCLXContainer();
                    ~VCLXContainer();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XVclContainer
    void SAL_CALL addVclContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeVclContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > SAL_CALL getWindows(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XVclContainerPeer
    void SAL_CALL enableDialogControl( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setTabOrder( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& WindowOrder, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Tabs, sal_Bool GroupControl ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& Windows ) throw(::com::sun::star::uno::RuntimeException);

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) { return ImplGetPropertyIds( aIds ); }
};




#endif // _TOOLKIT_AWT_VCLXCONTAINER_HXX_

