/*************************************************************************
 *
 *  $RCSfile: rdbtdp_tdenumeration.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-09 10:24:08 $
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

#ifndef _STOC_RDBTDP_TDENUMERATION_HXX
#define _STOC_RDBTDP_TDENUMERATION_HXX

#include <list>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _REGISTRY_REFLTYPE_HXX_
#include <registry/refltype.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_INVALIDTYPENAMEEXCEPTION_HPP_
#include <com/sun/star/reflection/InvalidTypeNameException.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_NOSUCHTYPENAMEEXCEPTION_HPP_
#include <com/sun/star/reflection/NoSuchTypeNameException.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_TYPEDESCRIPTIONSEARCHDEPTH_HPP_
#include <com/sun/star/reflection/TypeDescriptionSearchDepth.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XTYPEDESCRIPTIONENUMERATION_HPP_
#include <com/sun/star/reflection/XTypeDescriptionEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPECLASS_HPP_
#include <com/sun/star/uno/TypeClass.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif

class RegistryTypeReaderLoader;

namespace stoc_rdbtdp
{

typedef ::std::list< ::com::sun::star::uno::Reference<
    ::com::sun::star::reflection::XTypeDescription > > TypeDescriptionList;

class TypeDescriptionEnumerationImpl
    : public cppu::WeakImplHelper1<
                com::sun::star::reflection::XTypeDescriptionEnumeration >
{
public:
    static rtl::Reference< TypeDescriptionEnumerationImpl > createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const rtl::OUString & rModuleName,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::TypeClass > & rTypes,
        ::com::sun::star::reflection::TypeDescriptionSearchDepth eDepth,
        const RegistryTypeReaderLoader & rLoader,
        const RegistryKeyList & rBaseKeys )
            throw ( ::com::sun::star::reflection::NoSuchTypeNameException,
                    ::com::sun::star::reflection::InvalidTypeNameException,
                    ::com::sun::star::uno::RuntimeException );

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
    // Note: keys must be open (XRegistryKey->openKey(...)).
    TypeDescriptionEnumerationImpl(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const RegistryKeyList & rModuleKeys,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::TypeClass > & rTypes,
        ::com::sun::star::reflection::TypeDescriptionSearchDepth eDepth,
        const RegistryTypeReaderLoader & rLoader );

    static bool match( ::RTTypeClass eType1,
                       ::com::sun::star::uno::TypeClass eType2 );
    bool queryMore();
    ::com::sun::star::uno::Reference<
        ::com::sun::star::reflection::XTypeDescription > queryNext();
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XHierarchicalNameAccess > getTDMgr();

    // members
    osl::Mutex m_aMutex;
    RegistryKeyList     m_aModuleKeys;
    RegistryKeyList     m_aCurrentModuleSubKeys;
    TypeDescriptionList m_aTypeDescs;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::TypeClass > m_aTypes;
    ::com::sun::star::reflection::TypeDescriptionSearchDepth m_eDepth;
    RegistryTypeReaderLoader m_aLoader;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XHierarchicalNameAccess > m_xTDMgr;
};

} // namespace stoc_rdbtdp

#endif /* _STOC_RDBTDP_TDENUMERATION_HXX */

