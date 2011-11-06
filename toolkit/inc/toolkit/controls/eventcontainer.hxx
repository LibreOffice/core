/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#include <osl/diagnose.h>
#ifndef _COM_SUN_STAR_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

#include <toolkit/helper/listenermultiplexer.hxx>

#include <hash_map>
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

