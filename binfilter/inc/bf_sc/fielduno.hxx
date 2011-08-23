/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SC_FIELDUNO_HXX
#define SC_FIELDUNO_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"			// ScRange, ScAddress
#endif
#ifndef SC_MUTEXHLP_HXX
#include "mutexhlp.hxx"
#endif
 
#ifndef _SFXLSTNER_HXX //autogen
#include <bf_svtools/lstner.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <bf_svtools/itemprop.hxx>
#endif
#ifndef _MyEDITDATA_HXX //autogen
#include <bf_svx/editdata.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif

#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_ 
#include <osl/mutex.hxx>
#endif
namespace binfilter {

class SvxEditSource;
class SvxFieldItem;
class ScCellFieldObj;
class ScHeaderFieldObj;
class ScHeaderFooterContentObj;


//------------------------------------------------------------------


class ScCellFieldsObj : public cppu::WeakImplHelper5<
                            ::com::sun::star::container::XEnumerationAccess,
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::container::XContainer,
                            ::com::sun::star::util::XRefreshable,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;
    ScAddress				aCellPos;
    SvxEditSource*			pEditSource;
    ///	List of refresh listeners.
    cppu::OInterfaceContainerHelper* mpRefreshListeners;
    /// mutex to lock the InterfaceContainerHelper
    osl::Mutex              aMutex;

    ScCellFieldObj*			GetObjectByIndex_Impl(INT32 Index) const;

public:
                            ScCellFieldsObj(ScDocShell* pDocSh, const ScAddress& rPos);
    virtual					~ScCellFieldsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XContainer
    virtual void SAL_CALL	addContainerListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XContainerListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeContainerListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XContainerListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL refresh(  ) 
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference< 
                                ::com::sun::star::util::XRefreshListener >& l )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XRefreshListener >& l )
                                    throw (::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScCellFieldObj : public ScMutexHelper,
                        public ::cppu::OComponentHelper,
                        public ::com::sun::star::text::XTextField,
                        public ::com::sun::star::beans::XPropertySet,
                        public ::com::sun::star::lang::XUnoTunnel,
                        public ::com::sun::star::lang::XServiceInfo,
                        public SfxListener
{
private:
    SfxItemPropertySet		aPropSet;
    ScDocShell*				pDocShell;
    ScAddress				aCellPos;
    SvxEditSource*			pEditSource;
    ESelection				aSelection;

    String					aUrl;				// Inhalt, wenn noch nicht eingefuegt (nur dann!)
    String					aRepresentation;
    String					aTarget;

public:
                            ScCellFieldObj(ScDocShell* pDocSh, const ScAddress& rPos,
                                            const ESelection& rSel);
    virtual					~ScCellFieldObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // per getImplementation gerufen:
    void					DeleteField();
    BOOL					IsInserted() const		{ return pEditSource != NULL; }
    SvxFieldItem			CreateFieldItem();
    void					InitDoc( ScDocShell* pDocSh, const ScAddress& rPos,
                                        const ESelection& rSel );

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

                            // XTextField
    virtual ::rtl::OUString SAL_CALL getPresentation( sal_Bool bShowCommand )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTextContent
    virtual void SAL_CALL	attach( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::text::XTextRange >& xTextRange )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                            ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getAnchor() throw(::com::sun::star::uno::RuntimeException);

                            // XComponent
    virtual void SAL_CALL	dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::lang::XEventListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::lang::XEventListener >& aListener )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScCellFieldObj* getImplementation( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextContent> xObj );

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};

//------------------------------------------------------------------

class ScHeaderFieldsObj : public cppu::WeakImplHelper5<
                            ::com::sun::star::container::XEnumerationAccess,
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::container::XContainer,
                            ::com::sun::star::util::XRefreshable,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ScHeaderFooterContentObj*	pContentObj;
    USHORT						nPart;
    UINT16						nType;
    SvxEditSource*				pEditSource;

    ///	List of refresh listeners.
    cppu::OInterfaceContainerHelper* mpRefreshListeners;
    /// mutex to lock the InterfaceContainerHelper
    osl::Mutex                  aMutex;

    ScHeaderFieldObj*		GetObjectByIndex_Impl(INT32 Index) const;

public:
                            ScHeaderFieldsObj(ScHeaderFooterContentObj* pContent,
                                                USHORT nP, USHORT nT);
    virtual					~ScHeaderFieldsObj();

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XContainer
    virtual void SAL_CALL	addContainerListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XContainerListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeContainerListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XContainerListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL refresh(  ) 
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference< 
                                ::com::sun::star::util::XRefreshListener >& l )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XRefreshListener >& l )
                                    throw (::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScHeaderFieldObj : public ScMutexHelper,
                            public ::cppu::OComponentHelper,
                            public ::com::sun::star::text::XTextField,
                            public ::com::sun::star::beans::XPropertySet,
                            public ::com::sun::star::lang::XUnoTunnel,
                            public ::com::sun::star::lang::XServiceInfo
{
private:
    SfxItemPropertySet			aPropSet;
    ScHeaderFooterContentObj*	pContentObj;
    USHORT						nPart;
    UINT16						nType;
    SvxEditSource*				pEditSource;
    ESelection					aSelection;
    sal_Int16					nFileFormat;		// enum SvxFileFormat, valid if not inserted

public:
                            ScHeaderFieldObj(ScHeaderFooterContentObj* pContent, USHORT nP,
                                            USHORT nT, const ESelection& rSel);
    virtual					~ScHeaderFieldObj();

                            // per getImplementation gerufen:
    void					DeleteField();
    BOOL					IsInserted() const		{ return pEditSource != NULL; }
    SvxFieldItem			CreateFieldItem();
    void					InitDoc( ScHeaderFooterContentObj* pContent, USHORT nP,
                                        const ESelection& rSel );

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

                            // XTextField
    virtual ::rtl::OUString SAL_CALL getPresentation( sal_Bool bShowCommand )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTextContent
    virtual void SAL_CALL	attach( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::text::XTextRange >& xTextRange )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                            ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getAnchor() throw(::com::sun::star::uno::RuntimeException);

                            // XComponent
    virtual void SAL_CALL	dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::lang::XEventListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::lang::XEventListener >& aListener )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScHeaderFieldObj* getImplementation( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextContent> xObj );

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};

} //namespace binfilter
#endif

