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



#ifndef _SAX_ATTRLISTIMPL_HXX
#define _SAX_ATTRLISTIMPL_HXX

#include "sal/config.h"
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

namespace sax_expatwrap
{

struct AttributeList_impl;

//FIXME
class /*SAX_DLLPUBLIC*/ AttributeList :
    public ::cppu::WeakImplHelper2<
                ::com::sun::star::xml::sax::XAttributeList,
                ::com::sun::star::util::XCloneable >
{
public:
    AttributeList();
    AttributeList( const AttributeList & );
    virtual ~AttributeList();

    void addAttribute( const ::rtl::OUString &sName ,
        const ::rtl::OUString &sType , const ::rtl::OUString &sValue );
    void clear();
public:
    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength(void)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByName(const ::rtl::OUString& aName)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByName(const ::rtl::OUString& aName)
        throw( ::com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< XCloneable > SAL_CALL
        createClone()   throw(::com::sun::star::uno::RuntimeException);

private:
    struct AttributeList_impl *m_pImpl;
};

}

#endif

