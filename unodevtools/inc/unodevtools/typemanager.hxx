/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typemanager.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 12:24:14 $
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

#ifndef INCLUDED_UNODEVTOOLS_TYPEMANAGER_HXX
#define INCLUDED_UNODEVTOOLS_TYPEMANAGER_HXX

#ifndef INCLUDED_CODEMAKER_TYPEMANAGER_HXX
#include <codemaker/typemanager.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif

#include <hash_map>
#include <vector>

class RegistryKey;

namespace typereg { class Reader; }

#if defined( _MSC_VER ) && ( _MSC_VER < 1200 )
typedef ::std::__hash_map__
<
    ::rtl::OString, // Typename
    RTTypeClass,    // TypeClass
    HashString,
    EqualString,
    NewAlloc
> T2TypeClassMap;
#else
typedef ::std::hash_map
<
    ::rtl::OString, // Typename
    RTTypeClass,    // TypeClass
    HashString,
    EqualString
> T2TypeClassMap;
#endif

namespace unodevtools {

struct UnoTypeManagerImpl
{
    UnoTypeManagerImpl() {}

    T2TypeClassMap  m_t2TypeClass;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XHierarchicalNameAccess> m_tdmgr;
};

class UnoTypeManager : public TypeManager
{
public:
    UnoTypeManager();
    ~UnoTypeManager();

    UnoTypeManager( const UnoTypeManager& value )
        : TypeManager(value)
        , m_pImpl( value.m_pImpl )
    {}

    sal_Bool init(const ::std::vector< ::rtl::OUString > registries);

    sal_Bool    isValidType(const ::rtl::OString& name) const;
    ::rtl::OString getTypeName(RegistryKey& rTypeKey) const;
    typereg::Reader getTypeReader(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const;
    typereg::Reader getTypeReader(RegistryKey& rTypeKey) const;
    RTTypeClass getTypeClass(const ::rtl::OString& name) const;
    RTTypeClass getTypeClass(RegistryKey& rTypeKey) const;

protected:
    void release();

    UnoTypeManagerImpl* m_pImpl;
};

}

#endif // _UNODEVTOOLS_TYPEMANAGER_HXX_
