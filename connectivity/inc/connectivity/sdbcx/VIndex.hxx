/*************************************************************************
 *
 *  $RCSfile: VIndex.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-30 07:21:14 $
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

#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#define _CONNECTIVITY_SDBCX_INDEX_HXX_

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _CONNECTIVITY_SDBCX_INDEXDESCRIPTOR_HXX_
#include "connectivity/sdbcx/VIndexDescriptor.hxx"
#endif

namespace connectivity
{
    namespace sdbcx
    {
        class OIndex;
        typedef ::comphelper::OPropertyArrayUsageHelper<OIndex> OIndex_PROP;

        class OIndex :  public OIndexDescriptor,
                        public ::com::sun::star::sdbcx::XDataDescriptorFactory,
                        public OIndex_PROP
        {
        protected:
            DECLARE_CTY_PROPERTY(OIndex_PROP,OIndex)
        public:
            DECLARE_CTY_DEFAULTS( OIndexDescriptor);
            OIndex(sal_Bool _bCase);
            OIndex( const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Catalog,
                    sal_Bool _isUnique,
                    sal_Bool _isPrimaryKeyIndex,
                    sal_Bool _isClustered,
                    sal_Bool _bCase);

            virtual ~OIndex( );

            DECLARE_SERVICE_INFO();

            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor( void ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_INDEX_HXX_

