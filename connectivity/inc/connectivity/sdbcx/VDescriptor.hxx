/*************************************************************************
 *
 *  $RCSfile: VDescriptor.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:19 $
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

#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#define _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_

#ifndef _CONNECTIVITY_SIMPLEPROPERTYCONTAINER_HXX_
#include "connectivity/simplepropertycontainer.hxx"
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


namespace connectivity
{
    namespace sdbcx
    {
        class ODescriptor : public connectivity::OSimplePropertyContainer
        {
        protected:
            ::rtl::OUString         m_Name;
        private:
            sal_Bool                m_bNew;
            utl::UStringMixEqual    m_aCase;

        public:
            ODescriptor(::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bCase, sal_Bool _bNew = sal_False)
                : connectivity::OSimplePropertyContainer(_rBHelper)
                , m_bNew(_bNew),m_aCase(_bCase){}

            sal_Bool isNew()  const { return m_bNew; }
            sal_Bool getNew() const { return m_bNew; }

            sal_Bool isCaseSensitive() const { return m_aCase.isCaseSensitive(); }

            virtual void construct();

            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
            {
                ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                                ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                                ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));
                return aTypes.getTypes();
            }

            inline sal_Bool operator == ( const ::rtl::OUString & _rRH )
            {
                return m_aCase(m_Name,_rRH);
            }
        };
    }

}
#endif // _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_

