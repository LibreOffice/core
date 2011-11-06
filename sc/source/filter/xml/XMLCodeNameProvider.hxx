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



#ifndef SC_XMLCODENAMEPROVIDER_HXX
#define SC_XMLCODENAMEPROVIDER_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase1.hxx>

class ScDocument;
class String;

class XMLCodeNameProvider : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XNameAccess >
{
    ScDocument* mpDoc;
    ::rtl::OUString msDocName;
    ::rtl::OUString msCodeNameProp;

    static sal_Bool _getCodeName( const ::com::sun::star::uno::Any& aAny,
                           String& rCodeName );

public:
    XMLCodeNameProvider( ScDocument* pDoc );
    virtual ~XMLCodeNameProvider();

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException );

    static void set( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& xNameAccess, ScDocument *pDoc );
};

#endif
