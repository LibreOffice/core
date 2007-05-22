/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedDefaultProperty.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:24:17 $
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
#ifndef CHART2_WRAPPEDDEFAULTPROPERTY_HXX
#define CHART2_WRAPPEDDEFAULTPROPERTY_HXX

#include "WrappedProperty.hxx"

namespace chart
{

class WrappedDefaultProperty : public WrappedProperty
{
public:
    explicit WrappedDefaultProperty(
        const ::rtl::OUString& rOuterName, const ::rtl::OUString& rInnerName,
        const ::com::sun::star::uno::Any& rNewOuterDefault );
    virtual ~WrappedDefaultProperty();

    virtual void setPropertyToDefault(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::PropertyState getPropertyState(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
private:
    ::com::sun::star::uno::Any m_aOuterDefaultValue;
};

} //  namespace chart

// CHART2_WRAPPEDPROPERTYNEWDEFAULT_HXX
#endif
