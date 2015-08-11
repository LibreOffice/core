/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_DATATYPEREPOSITORY_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_DATATYPEREPOSITORY_HXX

#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <map>


namespace xforms
{

    class OXSDDataType;

    typedef ::cppu::WeakImplHelper <   ::com::sun::star::xforms::XDataTypeRepository
                                    >   ODataTypeRepository_Base;
    class ODataTypeRepository : public ODataTypeRepository_Base
    {
    private:
        typedef ::rtl::Reference< OXSDDataType >            DataType;
        typedef ::std::map< OUString, DataType >     Repository;

        ::osl::Mutex                                        m_aMutex;
        Repository                                          m_aRepository;

    public:
        ODataTypeRepository( );

    protected:
        virtual ~ODataTypeRepository( );

        // XDataTypeRepository
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType > SAL_CALL getBasicDataType( sal_Int16 dataTypeClass ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType > SAL_CALL cloneDataType( const OUString& sourceName, const OUString& newName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL revokeDataType( const OUString& typeName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::util::VetoException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType > SAL_CALL getDataType( const OUString& typeName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XEnumerationAccess (base of XDataTypeRepository)
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XNameAccess (base of XDataTypeRepository)
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XElementAccess (base of XEnumerationAccess and XNameAccess)
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        ODataTypeRepository( const ODataTypeRepository& ) SAL_DELETED_FUNCTION;
        ODataTypeRepository& operator=( const ODataTypeRepository& ) SAL_DELETED_FUNCTION;

    private:
        /** locates the type with the given name in our repository, or throws an exception if there is no such type
        */
        Repository::iterator    implLocate( const OUString& _rName, bool _bAllowMiss = false );
    };


} // namespace xforms


#endif // INCLUDED_FORMS_SOURCE_XFORMS_DATATYPEREPOSITORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
