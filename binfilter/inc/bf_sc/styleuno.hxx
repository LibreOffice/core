/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SC_STYLEUNO_HXX
#define SC_STYLEUNO_HXX

#include <bf_svtools/itemprop.hxx>

#include <bf_svtools/lstner.hxx>

#include <rsc/rscsfx.hxx>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase7.hxx>
namespace binfilter {

class ScDocShell;

class SfxStyleSheetBase;
class ScStyleFamilyObj;
class ScStyleObj;


class ScStyleNameConversion
{
public:
    static String DisplayToProgrammaticName( const String& rDispName, UINT16 nType );
    static String ProgrammaticToDisplayName( const String& rProgName, UINT16 nType );
};


class ScStyleFamiliesObj : public ::cppu::WeakImplHelper4<
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::container::XNameAccess,
                            ::com::sun::star::style::XStyleLoader,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;

    ScStyleFamilyObj*		GetObjectByType_Impl(UINT16 Type) const;
    ScStyleFamilyObj*		GetObjectByIndex_Impl(UINT32 nIndex) const;
    ScStyleFamilyObj*		GetObjectByName_Impl(const ::rtl::OUString& aName) const;

public:
                            ScStyleFamiliesObj(ScDocShell* pDocSh);
    virtual					~ScStyleFamiliesObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XStyleLoader
    virtual void SAL_CALL	loadStylesFromURL( const ::rtl::OUString& URL,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aOptions )
                                        throw(::com::sun::star::io::IOException,
                                            ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getStyleLoaderOptions() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScStyleFamilyObj : public ::cppu::WeakImplHelper3<
                            ::com::sun::star::container::XNameContainer,
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;
    SfxStyleFamily 			eFamily;		// Familie

    ScStyleObj*				GetObjectByIndex_Impl(UINT32 nIndex);
    ScStyleObj*				GetObjectByName_Impl(const ::rtl::OUString& Name);

public:
                            ScStyleFamilyObj(ScDocShell* pDocSh, SfxStyleFamily eFam);
    virtual					~ScStyleFamilyObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNameContainer
    virtual void SAL_CALL	insertByName( const ::rtl::OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::ElementExistException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeByName( const ::rtl::OUString& Name )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameReplace
    virtual void SAL_CALL	replaceByName( const ::rtl::OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScStyleObj : public ::cppu::WeakImplHelper7<
                    ::com::sun::star::style::XStyle,
                    ::com::sun::star::beans::XPropertySet,
                    ::com::sun::star::beans::XMultiPropertySet,
                    ::com::sun::star::beans::XPropertyState,
                    ::com::sun::star::beans::XMultiPropertyStates,
                    ::com::sun::star::lang::XUnoTunnel,
                    ::com::sun::star::lang::XServiceInfo >,
                public SfxListener
{
private:
    SfxItemPropertySet		aPropSet;
    ScDocShell*				pDocShell;
    SfxStyleFamily 			eFamily;		// Familie
    String 					aStyleName;

    SfxStyleSheetBase*		GetStyle_Impl();
    const SfxItemSet*		GetStyleItemSet_Impl( const String& rPropName, const SfxItemPropertyMap*& rpMapEntry );
    void					SetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                                    const ::com::sun::star::uno::Any* pValue )
                                            throw(::com::sun::star::lang::IllegalArgumentException,
                                                    ::com::sun::star::uno::RuntimeException);

public:
                            ScStyleObj(ScDocShell* pDocSh, SfxStyleFamily eFam, const String& rName);
    virtual					~ScStyleObj();

                            // per getImplementation gerufen:
    sal_Bool				IsInserted() const		{ return pDocShell != NULL; }
    SfxStyleFamily 			GetFamily() const 		{ return eFamily; }
    void					InitDoc( ScDocShell* pNewDocSh, const String& rNewName );

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >
                            CreateEmptyNumberingRules();

                            // XStyle
    virtual sal_Bool SAL_CALL isUserDefined() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isInUse() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString	SAL_CALL getParentStyle() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setParentStyle( const ::rtl::OUString& aParentStyle )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setName( const ::rtl::OUString& aName )
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

                            // XMultiPropertySet
    virtual void SAL_CALL	setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
                                throw (::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
                            getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL
                            getPropertyStates( const ::com::sun::star::uno::Sequence<
                                        ::rtl::OUString >& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyToDefault( const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
                                    const ::rtl::OUString& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XMultiPropertyStates
    // getPropertyStates already defined for XPropertyState
    virtual void SAL_CALL	setAllPropertiesToDefault() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertiesToDefault( const ::com::sun::star::uno::Sequence<
                                        ::rtl::OUString >& aPropertyNames )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
                            getPropertyDefaults( const ::com::sun::star::uno::Sequence<
                                        ::rtl::OUString >& aPropertyNames )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScStyleObj* getImplementation( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::uno::XInterface> xObj );

};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
