/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventcontainer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 22:56:00 $
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


#include <osl/diagnose.h>
#ifndef _COM_SUN_STAR_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

#include <toolkit/helper/listenermultiplexer.hxx>

#include <cppuhelper/implbase2.hxx>
typedef ::cppu::WeakImplHelper2< ::com::sun::star::container::XNameContainer,
                                 ::com::sun::star::container::XContainer > NameContainerHelper;


namespace toolkit
{

// Hashtable to optimize
struct hashName_Impl
{
    size_t operator()(const ::rtl::OUString Str) const
    {
        return (size_t)Str.hashCode();
    }
};

struct eqName_Impl
{
    sal_Bool operator()(const ::rtl::OUString Str1, const ::rtl::OUString Str2) const
    {
        return ( Str1 == Str2 );
    }
};

typedef std::hash_map
<
    ::rtl::OUString,
    sal_Int32,
    hashName_Impl,
    eqName_Impl
>
NameContainerNameMap;


class NameContainer_Impl : public NameContainerHelper
{
    NameContainerNameMap mHashMap;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > mNames;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > mValues;
    sal_Int32 mnElementCount;
    ::com::sun::star::uno::Type mType;

    ContainerListenerMultiplexer maContainerListeners;

public:
    NameContainer_Impl( ::com::sun::star::uno::Type const & aType )
        : mnElementCount( 0 ),
          mType( aType ),
          maContainerListeners( *this )
    {
    }

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener )
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener )
        throw(::com::sun::star::uno::RuntimeException);
};

class ScriptEventContainer : public NameContainer_Impl
{
public:
    ScriptEventContainer( void );
};


}   // namespace toolkit_namecontainer

