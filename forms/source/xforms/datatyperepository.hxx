/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datatyperepository.hxx,v $
 * $Revision: 1.4 $
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

