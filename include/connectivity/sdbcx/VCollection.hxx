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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VCOLLECTION_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VCOLLECTION_HXX

#include <cppuhelper/implbase10.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/container/XContainer.hpp>
#include <connectivity/StdTypeDefs.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <memory>


namespace connectivity
{
    namespace sdbcx
    {

        // the class OCollection is base class for collections :-)
        typedef ::cppu::ImplHelper10< ::com::sun::star::container::XNameAccess,
                                         ::com::sun::star::container::XIndexAccess,
                                         ::com::sun::star::container::XEnumerationAccess,
                                         ::com::sun::star::container::XContainer,
                                         ::com::sun::star::sdbc::XColumnLocate,
                                         ::com::sun::star::util::XRefreshable,
                                         ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                         ::com::sun::star::sdbcx::XAppend,
                                         ::com::sun::star::sdbcx::XDrop,
                                         ::com::sun::star::lang::XServiceInfo> OCollectionBase;

        typedef css::uno::Reference< css::beans::XPropertySet > ObjectType;

        class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE IObjectCollection
        {
        public:
            virtual ~IObjectCollection();
            virtual bool exists(const OUString& _sName ) = 0;
            virtual bool empty() = 0;
            virtual void swapAll() = 0;
            virtual void swap() = 0;
            virtual void clear() = 0;
            virtual void reFill(const TStringVector &_rVector) = 0;
            virtual void insert(const OUString& _sName, const ObjectType& _xObject) = 0;
            virtual bool rename(const OUString& _sOldName, const OUString& _sNewName) = 0;
            virtual sal_Int32 size() = 0;
            virtual ::com::sun::star::uno::Sequence< OUString > getElementNames() = 0;
            virtual OUString getName(sal_Int32 _nIndex) = 0;
            virtual void disposeAndErase(sal_Int32 _nIndex) = 0;
            virtual void disposeElements() = 0;
            virtual sal_Int32 findColumn( const OUString& columnName ) = 0;
            virtual ObjectType getObject(sal_Int32 _nIndex) = 0;
            virtual ObjectType getObject(const OUString& columnName) = 0;
            virtual void setObject(sal_Int32 _nIndex,const ObjectType& _xObject) = 0;
            virtual bool isCaseSensitive() const = 0;
        };

        //  OCollection

        class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE OCollection :
            public OCollectionBase
        {
        protected:
            ::std::unique_ptr<IObjectCollection>    m_pElements;

            ::cppu::OInterfaceContainerHelper       m_aContainerListeners;
            ::cppu::OInterfaceContainerHelper       m_aRefreshListeners;

        protected:
            ::cppu::OWeakObject&                    m_rParent;          // parent of the collection
            ::osl::Mutex&                           m_rMutex;           // mutex of the parent
            bool                                    m_bUseIndexOnly;    // is only TRUE when only an indexaccess is needed

            // the implementing class should refresh their elements
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException) = 0;

            // will be called when a object was requested by one of the accessing methods like getByIndex
            virtual ObjectType createObject(const OUString& _rName) = 0;

            // will be called when a new object should be generated by a call of createDataDescriptor
            // the returned object is empty will be filled outside and added to the collection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();

            /** appends an object described by a descriptor, under a given name
                @param _rForName
                    is the name under which the object should be appended. Guaranteed to not be empty.
                    This is passed for convenience only, since it's the result of a call of
                    getNameForObject for the given descriptor
                @param descriptor
                    describes the object to append
                @return
                    the new object which is to be inserted into the collection. This might be the result
                    of a call of <code>createObject( _rForName )</code>, or a clone of the descriptor.
            */
            virtual ObjectType appendObject( const OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );

            // called when XDrop was called
            virtual void dropObject(sal_Int32 _nPos, const OUString& _sElementName);

            /** returns the name for the object. The default implementation ask for the property NAME. If this doesn't satisfy, it has to be overridden.
                @param  _xObject    The object where the name should be extracted.
                @return The name of the object.
            */
            virtual OUString getNameForObject(const ObjectType& _xObject);

            /** clones the given descriptor

                The method calls createDescriptor to create a new, empty descriptor, and then copies all properties from
                _descriptor to the new object, which is returned.

                This method might come handy in derived classes for implementing appendObject, when the object
                is not actually appended to any backend (e.g. for the columns collection of a descriptor object itself,
                where there is not yet a database backend to append the column to).
            */
            ObjectType cloneDescriptor( const ObjectType& _descriptor );

            OCollection(::cppu::OWeakObject& _rParent,
                        bool _bCase,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector,
                        bool _bUseIndexOnly = false,
                        bool _bUseHardRef = true);

            /** clear the name map
                <p>Does <em>not</em> dispose the objects hold by the collection.</p>
            */
            void clear_NoDispose();

            /**  insert a new element into the collection
            */
            void insertElement(const OUString& _sElementName,const ObjectType& _xElement);

            /** return the object, if not existent it creates it.
                @param  _nIndex
                    The index of the object to create.
                @return ObjectType
            */
            ObjectType getObject(sal_Int32 _nIndex);

        public:
            virtual ~OCollection();
            DECLARE_SERVICE_INFO();

            void reFill(const TStringVector &_rVector);
            inline bool isCaseSensitive() const { return m_pElements->isCaseSensitive(); }
            void renameObject(const OUString& _sOldName, const OUString& _sNewName);

            // only the name is identical to ::cppu::OComponentHelper
            virtual void SAL_CALL disposing();
            // dispatch the refcounting to the parent
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;

            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // ::com::sun::star::container::XElementAccess
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // ::com::sun::star::container::XIndexAccess
            virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

                // ::com::sun::star::container::XNameAccess
            virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XEnumerationAccess
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
                        // ::com::sun::star::util::XRefreshable
            virtual void SAL_CALL refresh(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XAppend
            virtual void SAL_CALL appendByDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XDrop
            virtual void SAL_CALL dropByName( const OUString& elementName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL dropByIndex( sal_Int32 index ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // ::com::sun::star::container::XContainer
            virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        private:
            void notifyElementRemoved(const OUString& _sName);
            void disposeElements();
            void dropImpl(sal_Int32 _nIndex, bool _bReallyDrop = true);
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SDBCX_VCOLLECTION_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
