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



#ifndef __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_
#define __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_

#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/container/XNameContainer.hpp>
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HXX_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#include <cppuhelper/implbase1.hxx>

namespace framework
{
    class GraphicNameAccess : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XNameAccess >
    {
        public:
            GraphicNameAccess();
            virtual ~GraphicNameAccess();

            void addElement( const rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rElement );

            // XNameAccess
            virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
                    ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                throw(::com::sun::star::uno::RuntimeException);

            // XElementAccess
            virtual sal_Bool SAL_CALL hasElements()
                throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
                throw(::com::sun::star::uno::RuntimeException);

        private:
            typedef BaseHash< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > > NameGraphicHashMap;
            NameGraphicHashMap m_aNameToElementMap;
            ::com::sun::star::uno::Sequence< rtl::OUString > m_aSeq;
    };
}

#endif // __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_
