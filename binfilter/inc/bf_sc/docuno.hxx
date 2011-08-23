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

#ifndef SC_DOCUNO_HXX
#define SC_DOCUNO_HXX

#include "global.hxx"			// ScRange, ScAddress

#include <bf_sfx2/sfxbasemodel.hxx>

#include <bf_svtools/lstner.hxx>

#include <bf_svx/fmdmod.hxx>

#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/sheet/XGoalSeek.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/sheet/XScenarios.hpp>
#include <com/sun/star/sheet/XConsolidatable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XDocumentAuditing.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>

#include <bf_svtools/itemprop.hxx>
namespace binfilter {
class SvEmbeddedObject;

class ScDocShell;
class ScAnnotationObj;
class ScMarkData;
class ScPrintFuncCache;
class ScPrintSelectionStatus;
class ScTableColumnObj;
class ScTableRowObj;
class ScTableSheetObj;
class SvxFmDrawPage;
class SvxDrawPage;

class ScModelObj : public SfxBaseModel,
                    public ::com::sun::star::sheet::XSpreadsheetDocument,
                    public ::com::sun::star::document::XActionLockable,
                    public ::com::sun::star::sheet::XCalculatable,
                    public ::com::sun::star::util::XProtectable,
                    public ::com::sun::star::drawing::XDrawPagesSupplier,
                    public ::com::sun::star::sheet::XGoalSeek,
                    public ::com::sun::star::sheet::XConsolidatable,
                    public ::com::sun::star::sheet::XDocumentAuditing,
                    public ::com::sun::star::style::XStyleFamiliesSupplier,
                    public ::com::sun::star::view::XRenderable,
                    public ::com::sun::star::document::XLinkTargetSupplier,
                    public ::com::sun::star::beans::XPropertySet,
                    public SvxFmMSFactory,	// derived from XMultiServiceFactory
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::com::sun::star::lang::XServiceInfo
{
private:
    SfxItemPropertySet		aPropSet;
    ScDocShell*				pDocShell;
    ScPrintFuncCache*		pPrintFuncCache;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation> xNumberAgg;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xDrawGradTab;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xDrawHatchTab;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xDrawBitmapTab;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xDrawTrGradTab;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xDrawMarkerTab;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xDrawDashTab;

public:
                            ScModelObj(ScDocShell* pDocSh);
    virtual					~ScModelObj();

    //	create ScModelObj and set at pDocSh (SetBaseModel)
    static void				CreateAndSet(ScDocShell* pDocSh);

    ScDocument*				GetDocument() const;
    SvEmbeddedObject*		GetEmbeddedObject() const;

    BOOL					AdjustRowHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab );

    void                    BeforeXMLLoading();
    void                    AfterXMLLoading(sal_Bool bRet);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

//?	virtual UString			getClassName(void);

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XSpreadsheetDocument
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheets > SAL_CALL
                            getSheets() throw(::com::sun::star::uno::RuntimeException);

                                // XStyleFamiliesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getStyleFamilies() throw(::com::sun::star::uno::RuntimeException);

                            // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getRenderer( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	render( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getLinks() throw(::com::sun::star::uno::RuntimeException);

                            // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addActionLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeActionLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setActionLocks( sal_Int16 nLock )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL resetActionLocks() throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL   lockControllers() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL   unlockControllers() throw (::com::sun::star::uno::RuntimeException);

                                // XCalculatable
    virtual void SAL_CALL	calculate() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	calculateAll() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAutomaticCalculationEnabled()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	enableAutomaticCalculation( sal_Bool bEnabled )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XProtectable
    virtual void SAL_CALL	protect( const ::rtl::OUString& aPassword )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	unprotect( const ::rtl::OUString& aPassword )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isProtected() throw(::com::sun::star::uno::RuntimeException);

                            //	XDrawPagesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > SAL_CALL
                            getDrawPages() throw(::com::sun::star::uno::RuntimeException);

    //!	XPrintable??

                            //	XGoalSeek
    virtual ::com::sun::star::sheet::GoalResult SAL_CALL seekGoal(
                                const ::com::sun::star::table::CellAddress& aFormulaPosition,
                                const ::com::sun::star::table::CellAddress& aVariablePosition,
                                const ::rtl::OUString& aGoalValue )
                                    throw(::com::sun::star::uno::RuntimeException);

                            //	XConsolidatable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XConsolidationDescriptor >
                            SAL_CALL createConsolidationDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL consolidate( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XConsolidationDescriptor >& xDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException);

                            //	XDocumentAuditing
    virtual void SAL_CALL	refreshArrows() throw(::com::sun::star::uno::RuntimeException);

                            //  XViewDataSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getViewData(  ) 
                                throw (::com::sun::star::uno::RuntimeException);

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

                            // XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            createInstance( const ::rtl::OUString& aServiceSpecifier )
                                throw(::com::sun::star::uno::Exception,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::uno::Any >& Arguments )
                                throw(::com::sun::star::uno::Exception,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

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
    static ScModelObj* getImplementation( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::uno::XInterface> xObj );

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScDrawPagesObj : public cppu::WeakImplHelper2<
                                ::com::sun::star::drawing::XDrawPages,
                                ::com::sun::star::lang::XServiceInfo>,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;

::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                            GetObjectByIndex_Impl(INT32 nIndex) const;

public:
                            ScDrawPagesObj(ScDocShell* pDocSh);
    virtual					~ScDrawPagesObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XDrawPages
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
                            insertNewByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	remove( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XDrawPage >& xPage )
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


class ScTableSheetsObj : public cppu::WeakImplHelper4<
                                ::com::sun::star::sheet::XSpreadsheets,
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::container::XIndexAccess,
                                ::com::sun::star::lang::XServiceInfo>,
                         public SfxListener
{
private:
    ScDocShell*				pDocShell;

    ScTableSheetObj*		GetObjectByIndex_Impl(USHORT nIndex) const;
    ScTableSheetObj*		GetObjectByName_Impl(const ::rtl::OUString& aName) const;

public:
                            ScTableSheetsObj(ScDocShell* pDocSh);
    virtual					~ScTableSheetsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XSpreadsheets
    virtual void SAL_CALL	insertNewByName( const ::rtl::OUString& aName, sal_Int16 nPosition )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	moveByName( const ::rtl::OUString& aName, sal_Int16 nDestination )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	copyByName( const ::rtl::OUString& aName,
                                const ::rtl::OUString& aCopy, sal_Int16 nDestination )
                                    throw(::com::sun::star::uno::RuntimeException);

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

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

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

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScTableColumnsObj : public cppu::WeakImplHelper5<
                                ::com::sun::star::table::XTableColumns,
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::container::XNameAccess,
                                ::com::sun::star::beans::XPropertySet,
                                ::com::sun::star::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*				pDocShell;
    USHORT					nTab;
    USHORT					nStartCol;
    USHORT					nEndCol;

    ScTableColumnObj*		GetObjectByIndex_Impl(USHORT nIndex) const;
    ScTableColumnObj*		GetObjectByName_Impl(const ::rtl::OUString& aName) const;

public:
                            ScTableColumnsObj(ScDocShell* pDocSh, USHORT nT,
                                                USHORT nSC, USHORT nEC);
    virtual					~ScTableColumnsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XTableColumns
    virtual void SAL_CALL	insertByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException);

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

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

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

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScTableRowsObj : public cppu::WeakImplHelper4<
                                ::com::sun::star::table::XTableRows,
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::beans::XPropertySet,
                                ::com::sun::star::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*				pDocShell;
    USHORT					nTab;
    USHORT					nStartRow;
    USHORT					nEndRow;

    ScTableRowObj*			GetObjectByIndex_Impl(USHORT nIndex) const;

public:
                            ScTableRowsObj(ScDocShell* pDocSh, USHORT nT,
                                                USHORT nSR, USHORT nER);
    virtual					~ScTableRowsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XTableRows
    virtual void SAL_CALL	insertByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException);

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

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScSpreadsheetSettingsObj : public cppu::WeakImplHelper2<
                                    ::com::sun::star::beans::XPropertySet,
                                    ::com::sun::star::lang::XServiceInfo>,
                                 public SfxListener
{
private:
    ScDocShell*				pDocShell;

public:
    virtual					~ScSpreadsheetSettingsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScAnnotationsObj : public cppu::WeakImplHelper3<
                                ::com::sun::star::sheet::XSheetAnnotations,
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*				pDocShell;
    USHORT					nTab;			// Collection haengt am Sheet

    BOOL					GetAddressByIndex_Impl( ULONG nIndex, ScAddress& rPos ) const;
    ScAnnotationObj*		GetObjectByIndex_Impl(USHORT nIndex) const;

public:
                            ScAnnotationsObj(ScDocShell* pDocSh, USHORT nT);
    virtual					~ScAnnotationsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XSheetAnnotations
    virtual void SAL_CALL	insertNew( const ::com::sun::star::table::CellAddress& aPosition,
                                const ::rtl::OUString& aText )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException);

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

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScScenariosObj : public cppu::WeakImplHelper4<
                                ::com::sun::star::sheet::XScenarios,
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::container::XIndexAccess,
                                ::com::sun::star::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*				pDocShell;
    USHORT					nTab;

    BOOL					GetScenarioIndex_Impl( const ::rtl::OUString& rName, USHORT& rIndex );
    ScTableSheetObj*		GetObjectByIndex_Impl(USHORT nIndex);
    ScTableSheetObj*		GetObjectByName_Impl(const ::rtl::OUString& aName);

public:
                            ScScenariosObj(ScDocShell* pDocSh, USHORT nT);
    virtual					~ScScenariosObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XScenarios
    virtual void SAL_CALL	addNewByName( const ::rtl::OUString& aName,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::table::CellRangeAddress >& aRanges,
                                const ::rtl::OUString& aComment )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

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

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

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




} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
