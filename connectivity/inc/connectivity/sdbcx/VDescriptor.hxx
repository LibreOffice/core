/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VDescriptor.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:17:33 $
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

#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#define _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_

#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif


namespace connectivity
{
    namespace sdbcx
    {
        // =========================================================================
        // = ODescriptor
        // =========================================================================
        typedef ::comphelper::OPropertyContainer ODescriptor_PBASE;
        class ODescriptor
                    :public ODescriptor_PBASE
                    ,public ::com::sun::star::lang::XUnoTunnel
        {
        protected:
            ::rtl::OUString         m_Name;

            /** helper for derived classes to implement OPropertyArrayUsageHelper::createArrayHelper

                This method just calls describeProperties, and flags all properties as READONLY if and
                only if we do *not* act as descriptor, but as final object.

                @seealso    isNew
            */
            ::cppu::IPropertyArrayHelper*   doCreateArrayHelper() const;

        private:
            comphelper::UStringMixEqual m_aCase;
            sal_Bool                    m_bNew;

        public:
            ODescriptor(::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bCase, sal_Bool _bNew = sal_False);

            virtual ~ODescriptor();

            sal_Bool isNew()  const         { return m_bNew;    }
            sal_Bool getNew() const         { return m_bNew;    }
            virtual void     setNew(sal_Bool _bNew);

            sal_Bool isCaseSensitive() const { return m_aCase.isCaseSensitive(); }

            virtual void construct();

            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // compare
            inline sal_Bool operator == ( const ::rtl::OUString & _rRH )
            {
                return m_aCase(m_Name,_rRH);
            }

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            static ODescriptor* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxSomeComp );
            // retrieves the ODescriptor implementation of a given UNO component, and returns its ->isNew flag
            static sal_Bool isNew( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDescriptor );
        };
    }

}
#endif // _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_

