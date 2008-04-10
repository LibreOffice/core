/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataPointStyle.hxx,v $
 * $Revision: 1.3 $
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
#ifndef CHART_DATAPOINTSTYLE_HXX
#define CHART_DATAPOINTSTYLE_HXX

#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include "OStyle.hxx"

namespace chart
{

class DataPointStyle :
        public ::cppu::OWeakObject,
        public ::property::OStyle
{
public:
    DataPointStyle( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::container::XNameAccess > & xStyleFamily,
                    ::osl::Mutex & rMutex );
    virtual ~DataPointStyle();

    // ____ XInterface ____
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type & rType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

protected:
    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    ::osl::Mutex & GetMutex();

    /** const variant of getInfoHelper()
     */
    ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelperConst() const;

private:
    ::osl::Mutex & m_rMutex;
};

} //  namespace chart

// CHART_DATAPOINTSTYLE_HXX
#endif
