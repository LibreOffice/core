/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterPropertySetInfo.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _COMPHELPER_MASTERPROPERTYSETINFO_HXX_
#define _COMPHELPER_MASTERPROPERTYSETINFO_HXX_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <comphelper/PropertyInfoHash.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/TypeGeneration.hxx>
#include "comphelper/comphelperdllapi.h"

namespace comphelper
{
    class COMPHELPER_DLLPUBLIC MasterPropertySetInfo:
        public ::cppu::WeakImplHelper1<
        ::com::sun::star::beans::XPropertySetInfo >
    {
        friend class MasterPropertySet;
    protected:
        PropertyDataHash maMap;
        com::sun::star::uno::Sequence < com::sun::star::beans::Property > maProperties;
    public:
        MasterPropertySetInfo()
            throw();
        MasterPropertySetInfo( PropertyInfo * pMap )
            throw();
        virtual ~MasterPropertySetInfo()
            throw();
        void add( PropertyInfo* pMap, sal_Int32 nCount = -1, sal_uInt8 nMapId = 0 )
            throw();
        void add( PropertyInfoHash &rHash, sal_uInt8 nMapId )
            throw();
        void remove( const rtl::OUString& aName )
            throw();

        // XPropertySetInfo
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties()
            throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name )
            throw(::com::sun::star::uno::RuntimeException);
    };
}
#endif
