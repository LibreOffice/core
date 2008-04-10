/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StyleFamily.hxx,v $
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
#ifndef CHART_STYLEFAMILY_HXX
#define CHART_STYLEFAMILY_HXX

#include <cppuhelper/implbase3.hxx>
#include <osl/mutex.hxx>

#include "ServiceMacros.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <map>

namespace com { namespace sun { namespace star {
    namespace style {
        class XStyle;
    }
}}}

namespace chart
{

class StyleFamily : public
    ::cppu::WeakImplHelper3<
        ::com::sun::star::container::XNameContainer,
        ::com::sun::star::container::XIndexAccess,
        ::com::sun::star::lang::XServiceInfo
    >
{
public:
    StyleFamily();
    virtual ~StyleFamily();

    /** adds a style.

        @param rStyle the style object supporting the service
               ::com::sun::star::style::Style.  The name under which this object
               can be accessed through XNameAccess is the one you get from
               XNamed::getName() (XNamed is a base interface for XStyle.)

        @return true, if the style was added.  In case there already exists a
                style with the name given in getName(), nothing is done and
                false is returned.
     */
    bool AddStyle( const ::com::sun::star::uno::Reference<
                             ::com::sun::star::style::XStyle > & rStyle );

protected:

    // container::XNameAccess (also base of XNameReplace)
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    // container::XNameContainer (optional, base of XNameReplace)
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // container::XIndexAccess (optional)
    virtual sal_Int32 SAL_CALL getCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // container::XElementAccess (base of XNameAccess and XIndexAccess)
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException);

    // container::XNameReplace (base of XNameAccess)
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    APPHELPER_XSERVICEINFO_DECL()

    typedef     ::std::map< ::rtl::OUString,
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::style::XStyle > >
        tStyleContainer;

private:
    tStyleContainer    m_aStyles;
    ::osl::Mutex       m_aMutex;
};

} //  namespace chart

// CHART_STYLEFAMILY_HXX
#endif
