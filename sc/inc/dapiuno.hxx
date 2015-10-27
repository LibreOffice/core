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

#ifndef INCLUDED_SC_INC_DAPIUNO_HXX
#define INCLUDED_SC_INC_DAPIUNO_HXX

#include "global.hxx"
#include "dpobject.hxx"
#include "rangeutl.hxx"
#include "cellsuno.hxx"

#include <svl/lstner.hxx>
#include <svl/itemprop.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XDataPilotDataLayoutFieldSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotField.hpp>
#include <com/sun/star/sheet/XDataPilotFieldGrouping.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <com/sun/star/sheet/XDataPilotTable2.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>

#include <cppuhelper/implbase.hxx>

#include <vector>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
    struct DataPilotTablePositionData;
}}}}

class ScDocShell;
class ScDPSaveDimension;
struct ScDPNumGroupInfo;

class ScDataPilotTableObj;
class ScDataPilotFieldObj;
class ScDataPilotItemObj;

class ScDataPilotConversion
{
public:
    static css::sheet::GeneralFunction   FirstFunc( sal_uInt16 nBits );
    static sal_uInt16           FunctionBit( css::sheet::GeneralFunction eFunc );

    static void         FillGroupInfo(
                            css::sheet::DataPilotFieldGroupInfo& rInfo,
                            const ScDPNumGroupInfo& rGroupInfo );
};

/** DataPilotTables collection per sheet. */
class ScDataPilotTablesObj : public cppu::WeakImplHelper<
                                        css::sheet::XDataPilotTables,
                                        css::container::XEnumerationAccess,
                                        css::container::XIndexAccess,
                                        css::lang::XServiceInfo>,
                                    public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;

    ScDataPilotTableObj*    GetObjectByIndex_Impl( sal_Int32 nIndex );
    ScDataPilotTableObj*    GetObjectByName_Impl(const OUString& aName);

public:
                            ScDataPilotTablesObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScDataPilotTablesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XDataPilotTables
    virtual css::uno::Reference< css::sheet::XDataPilotDescriptor > SAL_CALL
                            createDataPilotDescriptor() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   insertNewByName( const OUString& aName,
                                    const css::table::CellAddress& aOutputAddress,
                                    const css::uno::Reference< css::sheet::XDataPilotDescriptor >& xDescriptor )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;
    virtual void SAL_CALL   removeByName( const OUString& aName )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

//  ScDataPilotDescriptorBase is never instantiated directly
class ScDataPilotDescriptorBase : public css::sheet::XDataPilotDescriptor,
                                  public css::beans::XPropertySet,
                                  public css::sheet::XDataPilotDataLayoutFieldSupplier,
                                  public css::lang::XServiceInfo,
                                  public css::lang::XUnoTunnel,
                                  public css::lang::XTypeProvider,
                                  public cppu::OWeakObject,
                                  public SfxListener
{
private:
    SfxItemPropertySet  maPropSet;
    ScDocShell*         pDocShell;

public:
                            ScDataPilotDescriptorBase(ScDocShell* pDocSh);
    virtual                 ~ScDataPilotDescriptorBase();

    virtual css::uno::Any SAL_CALL queryInterface(
                                const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ScDocShell*             GetDocShell() const     { return pDocShell; }

                            // in the derivatives:
    virtual ScDPObject* GetDPObject() const = 0;
    virtual void SetDPObject(ScDPObject* pDPObj) = 0;

                            // XDataPilotDescriptor
                            //  getName, setName, getTag, setTag in derived classes

    virtual css::table::CellRangeAddress SAL_CALL getSourceRange()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSourceRange( const css::table::CellRangeAddress& aSourceRange )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetFilterDescriptor > SAL_CALL
                            getFilterDescriptor() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getDataPilotFields() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getColumnFields() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getRowFields() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getPageFields() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getDataFields() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getHiddenFields() throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw (css::beans::UnknownPropertyException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XDataPilotDataLayoutFieldSupplier
    virtual css::uno::Reference< css::sheet::XDataPilotField >
                            SAL_CALL getDataLayoutField()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    SC_DLLPUBLIC static ScDataPilotDescriptorBase* getImplementation(const css::uno::Reference<css::sheet::XDataPilotDescriptor>& rObj);

                            // XTypeProvider (override in ScDataPilotTableObj)
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo is in derived classes
};

class ScDataPilotDescriptor : public ScDataPilotDescriptorBase
{
private:
    ScDPObject*             mpDPObject;

public:
                            ScDataPilotDescriptor(ScDocShell* pDocSh);
    virtual                 ~ScDataPilotDescriptor();

    virtual ScDPObject* GetDPObject() const override;
    virtual void SetDPObject(ScDPObject* pDPObj) override;

                            // rest of XDataPilotDescriptor (incl. XNamed)
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTag() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setTag( const OUString& aTag )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScDataPilotTableObj : public ScDataPilotDescriptorBase,
                            public css::sheet::XDataPilotTable2,
                            public css::util::XModifyBroadcaster
{
private:
    SCTAB                   nTab;
    OUString                aName;
    XModifyListenerArr_Impl aModifyListeners;

    void                    Refreshed_Impl();

public:
                            ScDataPilotTableObj(ScDocShell* pDocSh, SCTAB nT, const OUString& rN);
    virtual                 ~ScDataPilotTableObj();

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual ScDPObject* GetDPObject() const override;
    virtual void SetDPObject(ScDPObject* pDPObj) override;

                            // rest of XDataPilotDescriptor (incl. XNamed)
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTag() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setTag( const OUString& aTag )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XDataPilotTable
    virtual css::table::CellRangeAddress SAL_CALL getOutputRange()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   refresh() throw(css::uno::RuntimeException, std::exception) override;

                            // XDataPilotTable2
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > >
        SAL_CALL getDrillDownData(const css::table::CellAddress& aAddr)
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual css::sheet::DataPilotTablePositionData
        SAL_CALL getPositionData(const css::table::CellAddress& aAddr)
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL insertDrillDownSheet(const css::table::CellAddress& aAddr)
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual css::table::CellRangeAddress SAL_CALL getOutputRangeByType( sal_Int32 nType )
                                throw(css::lang::IllegalArgumentException,
                                      css::uno::RuntimeException, std::exception) override;

                            // XModifyBroadcaster
    virtual void SAL_CALL   addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XTypeProvider (override)
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

struct ScFieldIdentifier
{
    OUString     maFieldName;    /// Source field name.
    sal_Int32           mnFieldIdx;     /// Field index (if several fields with same name exist).
    bool                mbDataLayout;   /// True = data layout field collecting all data fields as items.

    inline explicit     ScFieldIdentifier() :
                            mnFieldIdx( 0 ), mbDataLayout( false ) {}

    inline explicit     ScFieldIdentifier( const OUString& rFieldName, sal_Int32 nFieldIdx, bool bDataLayout ) :
                            maFieldName( rFieldName ), mnFieldIdx( nFieldIdx ), mbDataLayout( bDataLayout ) {}
};

/** Base class of all implementation objects based on a DataPilot descriptor
    or DataPilot table object. Wraps acquiring and releasing the parent. */
class ScDataPilotChildObjBase
{
protected:
    explicit            ScDataPilotChildObjBase( ScDataPilotDescriptorBase& rParent );
    explicit            ScDataPilotChildObjBase( ScDataPilotDescriptorBase& rParent, const ScFieldIdentifier& rFieldId );
    virtual             ~ScDataPilotChildObjBase();

    /** Returns the wrapped DataPilot object (calls GetDPObject() at parent). */
    ScDPObject*         GetDPObject() const;
    /** Sets the passed DataPilot object (calls SetDPObject() at parent). */
    void                SetDPObject( ScDPObject* pDPObject );

    /** Returns the DataPilot dimension object related to the field described by maFieldId. */
    ScDPSaveDimension*  GetDPDimension( ScDPObject** ppDPObject = 0 ) const;

    /** Returns the number of members for the field described by maFieldId. */
    sal_Int32           GetMemberCount() const;
    /** Returns the collection of members for the field described by maFieldId. */
    css::uno::Reference< css::container::XNameAccess >
                        GetMembers() const;

    ScDocShell* GetDocShell() const;
protected:
    ScDataPilotDescriptorBase& mrParent;
    ScFieldIdentifier   maFieldId;

private:
    ScDataPilotChildObjBase& operator=( const ScDataPilotChildObjBase& ) = delete;
};

typedef ::cppu::WeakImplHelper
<
    css::container::XEnumerationAccess,
    css::container::XIndexAccess,
    css::container::XNameAccess,
    css::lang::XServiceInfo
>
ScDataPilotFieldsObjImpl;

/** Collection of all DataPilot fields, or of all fields from a specific dimension. */
class ScDataPilotFieldsObj : public ScDataPilotChildObjBase, public ScDataPilotFieldsObjImpl
{
public:
    explicit            ScDataPilotFieldsObj(
                            ScDataPilotDescriptorBase& rParent );

    explicit            ScDataPilotFieldsObj(
                            ScDataPilotDescriptorBase& rParent,
                            css::sheet::DataPilotFieldOrientation eOrient );

    virtual             ~ScDataPilotFieldsObj();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

private:
    ScDataPilotFieldObj*    GetObjectByIndex_Impl( sal_Int32 nIndex ) const;
    ScDataPilotFieldObj*    GetObjectByName_Impl( const OUString& rName ) const;

private:
    css::uno::Any maOrient;    /// Field orientation, no value = all fields.
};

typedef ::cppu::WeakImplHelper
<
    css::container::XNamed,
    css::beans::XPropertySet,
    css::sheet::XDataPilotField,
    css::sheet::XDataPilotFieldGrouping,
    css::lang::XServiceInfo
>
ScDataPilotFieldObjImpl;

/** Implementation of a single DataPilot field. */
class ScDataPilotFieldObj : public ScDataPilotChildObjBase, public ScDataPilotFieldObjImpl
{
public:
                        ScDataPilotFieldObj(
                            ScDataPilotDescriptorBase& rParent,
                            const ScFieldIdentifier& rIdent );

                        ScDataPilotFieldObj(
                            ScDataPilotDescriptorBase& rParent,
                            const ScFieldIdentifier& rIdent,
                            const css::uno::Any& rOrient );

    virtual             ~ScDataPilotFieldObj();

                            // XNamed
    virtual OUString SAL_CALL getName()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL   setName(const OUString& aName)
        throw (css::uno::RuntimeException,
               std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw (css::beans::UnknownPropertyException,
                                       css::beans::PropertyVetoException,
                                       css::lang::IllegalArgumentException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName )
                                throw (css::beans::UnknownPropertyException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XDatePilotField
    virtual css::uno::Reference<css::container::XIndexAccess> SAL_CALL
                                getItems() throw (css::uno::RuntimeException, std::exception) override;

                            // only called from property-functions:
    css::sheet::DataPilotFieldOrientation getOrientation() const;
    void setOrientation(css::sheet::DataPilotFieldOrientation Orientation);
    css::sheet::GeneralFunction getFunction() const;
    void setFunction(css::sheet::GeneralFunction Function);
    css::uno::Sequence< css::sheet::GeneralFunction > getSubtotals() const;
    void setSubtotals(const css::uno::Sequence< css::sheet::GeneralFunction >& rFunctions);
    static OUString getCurrentPage();
    void setCurrentPage(const OUString& sPage);
    void setUseCurrentPage(bool bUse);
    const css::sheet::DataPilotFieldAutoShowInfo* getAutoShowInfo();
    void setAutoShowInfo(const css::sheet::DataPilotFieldAutoShowInfo* pInfo);
    const css::sheet::DataPilotFieldLayoutInfo* getLayoutInfo();
    void setLayoutInfo(const css::sheet::DataPilotFieldLayoutInfo* pInfo);
    const css::sheet::DataPilotFieldReference* getReference();
    void setReference(const css::sheet::DataPilotFieldReference* pInfo);
    const css::sheet::DataPilotFieldSortInfo* getSortInfo();
    void setSortInfo(const css::sheet::DataPilotFieldSortInfo* pInfo);
    bool getShowEmpty() const;
    void setShowEmpty(bool bShow);
    bool getRepeatItemLabels() const;
    void setRepeatItemLabels(bool bShow);

    bool hasGroupInfo();
    css::sheet::DataPilotFieldGroupInfo getGroupInfo();
    void setGroupInfo(const css::sheet::DataPilotFieldGroupInfo* pInfo);

                            // XDataPilotFieldGrouping
    static bool HasString(const css::uno::Sequence< OUString >& aItems, const OUString& aString);
    virtual css::uno::Reference < css::sheet::XDataPilotField > SAL_CALL
        createNameGroup(const css::uno::Sequence< OUString >& aItems)
             throw (css::uno::RuntimeException,
                    css::lang::IllegalArgumentException,
                    std::exception) override;
    virtual css::uno::Reference < css::sheet::XDataPilotField > SAL_CALL
        createDateGroup(const css::sheet::DataPilotFieldGroupInfo& rInfo)
             throw (css::uno::RuntimeException,
                    css::lang::IllegalArgumentException,
                    std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference< css::container::XIndexAccess >
                        mxItems;
    SfxItemPropertySet  maPropSet;
    css::uno::Any maOrient;
};

typedef ::std::vector< OUString > ScFieldGroupMembers;

struct ScFieldGroup
{
    OUString     maName;
    ScFieldGroupMembers maMembers;
};

typedef ::std::vector< ScFieldGroup > ScFieldGroups;

typedef ::cppu::WeakImplHelper
<
    css::container::XNameContainer,
    css::container::XEnumerationAccess,
    css::container::XIndexAccess,
    css::lang::XServiceInfo
>
ScDataPilotFieldGroupsObjImpl;

/** Implementation of all grouped items in a DataPilot field.

    This is a stand-alone object without any connection to the base DataPilot
    field. Grouping info has to be written back with the GroupInfo property of
    the DataPilot field after modifying this object.
 */
class ScDataPilotFieldGroupsObj : public ScDataPilotFieldGroupsObjImpl
{
public:
    explicit            ScDataPilotFieldGroupsObj( const ScFieldGroups& rGroups );
    virtual             ~ScDataPilotFieldGroupsObj();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                                // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                throw (css::lang::IllegalArgumentException,
                                    css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                throw (css::lang::IllegalArgumentException,
                                    css::container::ElementExistException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name )
                                throw (css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                        // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    // implementation
    ScFieldGroup&       getFieldGroup( const OUString& rName ) throw(css::uno::RuntimeException);
    void                renameFieldGroup( const OUString& rOldName, const OUString& rNewName ) throw(css::uno::RuntimeException);

private:
    ScFieldGroups::iterator implFindByName( const OUString& rName );

private:
    ScFieldGroups       maGroups;
};

typedef ::cppu::WeakImplHelper
<
    css::container::XNameContainer,
    css::container::XEnumerationAccess,
    css::container::XIndexAccess,
    css::container::XNamed,
    css::lang::XServiceInfo
>
ScDataPilotFieldGroupObjImpl;

class ScDataPilotFieldGroupObj : public ScDataPilotFieldGroupObjImpl
{
public:
    explicit            ScDataPilotFieldGroupObj( ScDataPilotFieldGroupsObj& rParent, const OUString& rGroupName );
    virtual             ~ScDataPilotFieldGroupObj();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                                // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                throw (css::lang::IllegalArgumentException,
                                    css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                                // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                throw (css::lang::IllegalArgumentException,
                                    css::container::ElementExistException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name )
                                throw (css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

private:
    ScDataPilotFieldGroupsObj& mrParent;
    OUString     maGroupName;
};

typedef ::cppu::WeakImplHelper
<
    css::container::XNamed,
    css::lang::XServiceInfo
>
ScDataPilotFieldGroupItemObjImpl;

class ScDataPilotFieldGroupItemObj : public ScDataPilotFieldGroupItemObjImpl
{
public:
    explicit            ScDataPilotFieldGroupItemObj( ScDataPilotFieldGroupObj& rParent, const OUString& rName );
    virtual             ~ScDataPilotFieldGroupItemObj();

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

private:
    ScDataPilotFieldGroupObj& mrParent;
    OUString     maName;
};

typedef ::cppu::WeakImplHelper
<
    css::container::XEnumerationAccess,
    css::container::XIndexAccess,
    css::container::XNameAccess,
    css::lang::XServiceInfo
>
ScDataPilotItemsObjImpl;

class ScDataPilotItemsObj : public ScDataPilotChildObjBase, public ScDataPilotItemsObjImpl
{
public:
    explicit            ScDataPilotItemsObj( ScDataPilotDescriptorBase& rParent, const ScFieldIdentifier& rFieldId );
    virtual             ~ScDataPilotItemsObj();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

private:
    ScDataPilotItemObj* GetObjectByIndex_Impl( sal_Int32 nIndex ) const;
};

typedef ::cppu::WeakImplHelper
<
    css::container::XNamed,
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
ScDataPilotItemObjImpl;

class ScDataPilotItemObj : public ScDataPilotChildObjBase, public ScDataPilotItemObjImpl
{
public:
    explicit            ScDataPilotItemObj(
                            ScDataPilotDescriptorBase& rParent,
                            const ScFieldIdentifier& rFieldId,
                            sal_Int32 nIndex );

    virtual             ~ScDataPilotItemObj();

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw (css::beans::UnknownPropertyException,
                                       css::beans::PropertyVetoException,
                                       css::lang::IllegalArgumentException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw (css::beans::UnknownPropertyException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

private:
    SfxItemPropertySet  maPropSet;
    sal_Int32           mnIndex;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
