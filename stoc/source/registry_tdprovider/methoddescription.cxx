/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: methoddescription.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:03:42 $
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

#include "methoddescription.hxx"

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/reflection/XParameter.hpp"
#include "com/sun/star/reflection/XTypeDescription.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/mutex.hxx"
#include "registry/reader.hxx"
#include "registry/types.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace css = com::sun::star;

using stoc::registry_tdprovider::MethodDescription;

namespace {

class Parameter: public cppu::WeakImplHelper1< css::reflection::XParameter > {
public:
    Parameter(
        css::uno::Reference< css::container::XHierarchicalNameAccess > const &
            manager,
        rtl::OUString const & name, rtl::OUString const & typeName,
        RTParamMode mode, sal_Int32 position):
        m_manager(manager), m_name(name),
        m_typeName(typeName.replace('/', '.')), m_mode(mode),
        m_position(position) {}

    virtual ~Parameter() {}

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    { return m_name; }

    virtual css::uno::Reference< css::reflection::XTypeDescription > SAL_CALL
    getType() throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isIn() throw (css::uno::RuntimeException)
    { return (m_mode & RT_PARAM_IN) != 0; }

    virtual sal_Bool SAL_CALL isOut() throw (css::uno::RuntimeException)
    { return (m_mode & RT_PARAM_OUT) != 0; }

    virtual sal_Int32 SAL_CALL getPosition() throw (css::uno::RuntimeException)
    { return m_position; }

    virtual sal_Bool SAL_CALL isRestParameter()
        throw (css::uno::RuntimeException)
    { return (m_mode & RT_PARAM_REST) != 0; }

private:
    Parameter(Parameter &); // not implemented
    void operator =(Parameter); // not implemented

    css::uno::Reference< css::container::XHierarchicalNameAccess > m_manager;
    rtl::OUString m_name;
    rtl::OUString m_typeName;
    RTParamMode m_mode;
    sal_Int32 m_position;
};

css::uno::Reference< css::reflection::XTypeDescription > Parameter::getType()
    throw (css::uno::RuntimeException)
{
    try {
        return css::uno::Reference< css::reflection::XTypeDescription >(
            m_manager->getByHierarchicalName(m_typeName),
            css::uno::UNO_QUERY_THROW);
    } catch (css::container::NoSuchElementException & e) {
        throw new css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.container.NoSuchElementException: "))
             + e.Message),
            static_cast< cppu::OWeakObject * >(this));
    }
}

}

MethodDescription::MethodDescription(
    css::uno::Reference< css::container::XHierarchicalNameAccess > const &
        manager,
    rtl::OUString const & name,
    com::sun::star::uno::Sequence< sal_Int8 > const & bytes,
    sal_uInt16 index):
    FunctionDescription(manager, bytes, index), m_name(name),
    m_parametersInit(false)
{}

MethodDescription::~MethodDescription() {}

css::uno::Sequence< css::uno::Reference< css::reflection::XParameter > >
MethodDescription::getParameters() const {
    osl::MutexGuard guard(m_mutex);
    if (!m_parametersInit) {
        typereg::Reader reader(getReader());
        sal_uInt16 n = reader.getMethodParameterCount(m_index);
        m_parameters.realloc(n);
        for (sal_uInt16 i = 0; i < n; ++i) {
            m_parameters[i] = new Parameter(
                m_manager, reader.getMethodParameterName(m_index, i),
                reader.getMethodParameterTypeName(m_index, i),
                reader.getMethodParameterFlags(m_index, i), i);
        }
        m_parametersInit = true;
    }
    return m_parameters;
}
