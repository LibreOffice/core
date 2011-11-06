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



#ifndef INCLUDED_SDR_PRIMITIVE2D_PRIMITIVEFACTORY2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_PRIMITIVEFACTORY2D_HXX

#include <com/sun/star/graphic/XPrimitiveFactory2D.hpp>
#include <cppuhelper/compbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <svx/svxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// UNO API helper methods

namespace drawinglayer
{
    namespace primitive2d
    {
        SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL XPrimitiveFactory2DProvider_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw( ::com::sun::star::uno::Exception );
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// PrimitiveFactory2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        // typedef for PrimitiveFactory2DImplBase
        typedef cppu::WeakComponentImplHelper1< ::com::sun::star::graphic::XPrimitiveFactory2D > PrimitiveFactory2DImplBase;

        // base class for C++ implementation of com::sun::star::graphic::XPrimitiveFactory2D
        class PrimitiveFactory2D
        :   protected comphelper::OBaseMutex,
            public PrimitiveFactory2DImplBase
        {
        private:
        protected:
        public:
            // constructor
            PrimitiveFactory2D();

            // Methods from XPrimitiveFactory2D
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > > SAL_CALL createPrimitivesFromXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aParms ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > > SAL_CALL createPrimitivesFromXDrawPage( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aParms ) throw (::com::sun::star::uno::RuntimeException);

            // UNO API helpers
            SVX_DLLPUBLIC static rtl::OUString getImplementationName_Static();
            SVX_DLLPUBLIC static com::sun::star::uno::Sequence< rtl::OUString > getSupportedServiceNames_Static();
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_PRIMITIVEFACTORY2D_HXX

// eof
