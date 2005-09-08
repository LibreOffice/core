/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdmgr_tdenumeration.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:14:28 $
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

#ifndef _STOC_TDMGR_TDENUMERATION_HXX
#define _STOC_TDMGR_TDENUMERATION_HXX

#include <stack>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XTYPEDESCRIPTIONENUMERATION_HPP_
#include <com/sun/star/reflection/XTypeDescriptionEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XTYPEDESCRIPTIONENUMERATIONACCESS_HPP_
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace stoc_tdmgr
{

typedef std::stack< com::sun::star::uno::Reference<
            com::sun::star::reflection::XTypeDescriptionEnumerationAccess > >
                TDEnumerationAccessStack;

class TypeDescriptionEnumerationImpl
    : public cppu::WeakImplHelper1<
                com::sun::star::reflection::XTypeDescriptionEnumeration >
{
public:
    TypeDescriptionEnumerationImpl(
        const rtl::OUString & rModuleName,
        const com::sun::star::uno::Sequence<
            com::sun::star::uno::TypeClass > & rTypes,
        com::sun::star::reflection::TypeDescriptionSearchDepth eDepth,
        const TDEnumerationAccessStack & rTDEAS );
    virtual ~TypeDescriptionEnumerationImpl();

    // XEnumeration (base of XTypeDescriptionEnumeration)
    virtual sal_Bool SAL_CALL hasMoreElements()
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    // XTypeDescriptionEnumeration
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::reflection::XTypeDescription > SAL_CALL
    nextTypeDescription()
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::uno::RuntimeException );

private:
    com::sun::star::uno::Reference<
        com::sun::star::reflection::XTypeDescriptionEnumeration >
    queryCurrentChildEnumeration();

    osl::Mutex m_aMutex;
    rtl::OUString m_aModuleName;
    com::sun::star::uno::Sequence< com::sun::star::uno::TypeClass > m_aTypes;
    com::sun::star::reflection::TypeDescriptionSearchDepth m_eDepth;
    TDEnumerationAccessStack m_aChildren;
    com::sun::star::uno::Reference<
        com::sun::star::reflection::XTypeDescriptionEnumeration > m_xEnum;
};

} // namespace stoc_tdmgr

#endif /* _STOC_TDMGR_TDENUMERATION_HXX */

