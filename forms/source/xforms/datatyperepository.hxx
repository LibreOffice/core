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



#ifndef OFFAPI_COM_SUN_STAR_XFORMS_DATATYPEREPOSITORY_HXX
#define OFFAPI_COM_SUN_STAR_XFORMS_DATATYPEREPOSITORY_HXX

/** === begin UNO includes === **/
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
/** === end UNO includes === **/
#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>

#include <map>

//........................................................................
namespace xforms
{
//........................................................................

    class OXSDDataType;
    //====================================================================
    //= ODataTypeRepository
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::xforms::XDataTypeRepository
                                    >   ODataTypeRepository_Base;
    class ODataTypeRepository : public ODataTypeRepository_Base
    {
    private:
        typedef ::rtl::Reference< OXSDDataType >            DataType;
        typedef ::std::map< ::rtl::OUString, DataType >     Repository;

        ::osl::Mutex                                        m_aMutex;
        Repository                                          m_aRepository;

    public:
        ODataTypeRepository( );

    protected:
        ~ODataTypeRepository( );

        // XDataTypeRepository
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType > SAL_CALL getBasicDataType( sal_Int16 dataTypeClass ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType > SAL_CALL cloneDataType( const ::rtl::OUString& sourceName, const ::rtl::OUString& newName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL revokeDataType( const ::rtl::OUString& typeName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::util::VetoException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType > SAL_CALL getDataType( const ::rtl::OUString& typeName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        // XEnumerationAccess (base of XDataTypeRepository)
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (::com::sun::star::uno::RuntimeException);

        // XNameAccess (base of XDataTypeRepository)
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess (base of XEnumerationAccess and XNameAccess)
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        ODataTypeRepository( const ODataTypeRepository& );              // never implemented
        ODataTypeRepository& operator=( const ODataTypeRepository& );   // never implemented

    private:
        /** locates the type with the given name in our repository, or throws an exception if there is no such type
        */
        Repository::iterator    implLocate( const ::rtl::OUString& _rName, bool _bAllowMiss = false ) SAL_THROW( ( ::com::sun::star::container::NoSuchElementException ) );
    };

//........................................................................
} // namespace xforms
//........................................................................

#endif // OFFAPI_COM_SUN_STAR_XFORMS_DATATYPEREPOSITORY_HXX

