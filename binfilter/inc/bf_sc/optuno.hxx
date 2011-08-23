/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SC_OPTUNO_HXX
#define SC_OPTUNO_HXX

#ifndef SC_DOCUNO_HXX
#include "docuno.hxx"
#endif

#ifndef SC_DOCOPTIO_HXX
#include "docoptio.hxx"
#endif
namespace binfilter {


class ScDocOptionsHelper
{
public:
    static const SfxItemPropertyMap* GetPropertyMap();

    static sal_Bool setPropertyValue( ScDocOptions& rOptions,
                                    const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue );
    static ::com::sun::star::uno::Any getPropertyValue(
                                    const ScDocOptions& rOptions,
                                    const ::rtl::OUString& PropertyName );
};


//	empty doc object to supply only doc options

class ScDocOptionsObj : public ScModelObj
{
private:
    ScDocOptions	aOptions;

public:
                            ScDocOptionsObj( const ScDocOptions& rOpt );
    virtual					~ScDocOptionsObj();

    // get/setPropertyValue overloaded to used stored options instead of document

    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
};


} //namespace binfilter
#endif

