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
#ifndef INCLUDED_SW_INC_UNOCHART_HXX
#define INCLUDED_SW_INC_UNOCHART_HXX

#include <map>
#include <set>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence2.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/DataSequenceRole.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/table/XCell.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <tools/link.hxx>
#include <vcl/timer.hxx>

#include <calbck.hxx>
#include <frmfmt.hxx>
#include <unocrsr.hxx>

class SfxItemPropertySet;
class SwDoc;
class SwTable;
class SwTableBox;
struct SwRangeDescriptor;
class SwSelBoxes;
class SwFrameFormat;

bool FillRangeDescriptor( SwRangeDescriptor &rDesc, const OUString &rCellRangeName );

class SwChartHelper
{
public:
    static void DoUpdateAllCharts( SwDoc* pDoc );
};

class SwChartLockController_Helper
{
    SwDoc   *pDoc;

    DECL_LINK_TYPED( DoUnlockAllCharts, Timer *, void );
    Timer   aUnlockTimer;   // timer to unlock chart controllers
    bool    bIsLocked;

    SwChartLockController_Helper( const SwChartLockController_Helper & ) = delete;
    SwChartLockController_Helper & operator = ( const SwChartLockController_Helper & ) = delete;

    void LockUnlockAllCharts( bool bLock );
    void LockAllCharts()    { LockUnlockAllCharts( true ); };
    void UnlockAllCharts()  { LockUnlockAllCharts( false ); };

public:
    SwChartLockController_Helper( SwDoc *pDocument );
    ~SwChartLockController_Helper();

    void StartOrContinueLocking();
    void Disconnect();
};

typedef cppu::WeakImplHelper
<
    css::chart2::data::XDataProvider,
    css::chart2::data::XRangeXMLConversion,
    css::lang::XComponent,
    css::lang::XServiceInfo
>
SwChartDataProviderBaseClass;

class SwChartDataProvider :
    public SwChartDataProviderBaseClass,
    public SwClient
{

    // used to keep weak-references to all data-sequences of a single table
    // see set definition below...
    struct lt_DataSequenceRef
    {
        bool operator()( css::uno::WeakReference< css::chart2::data::XDataSequence > xWRef1, css::uno::WeakReference< css::chart2::data::XDataSequence > xWRef2 ) const
        {
            css::uno::Reference< css::chart2::data::XDataSequence > xRef1( xWRef1 );
            css::uno::Reference< css::chart2::data::XDataSequence > xRef2( xWRef2 );
            return xRef1.get() < xRef2.get();
        }
    };
    typedef std::set< css::uno::WeakReference < css::chart2::data::XDataSequence >, lt_DataSequenceRef > Set_DataSequenceRef_t;

    // map of data-sequence sets for each table
    struct lt_SwTable_Ptr
    {
        bool operator()( const SwTable *p1, const SwTable *p2 ) const
        {
            return p1 < p2;
        }
    };
    typedef std::map< const SwTable *, Set_DataSequenceRef_t, lt_SwTable_Ptr > Map_Set_DataSequenceRef_t;

    // map of all data-sequences provided directly or indirectly (e.g. via
    // data-source) by this object. Since there is only one object of this type
    // for each document it should hold references to all used data-sequences for
    // all tables of the document.
    mutable Map_Set_DataSequenceRef_t       aDataSequences;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    const SwDoc *                           pDoc;
    bool                                bDisposed;

    SwChartDataProvider( const SwChartDataProvider & ) = delete;
    SwChartDataProvider & operator = ( const SwChartDataProvider & ) = delete;

    css::uno::Reference< css::chart2::data::XDataSource > SAL_CALL Impl_createDataSource( const css::uno::Sequence< css::beans::PropertyValue >& aArguments, bool bTestOnly = false )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException,
               std::exception);
    css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL Impl_createDataSequenceByRangeRepresentation( const OUString& aRangeRepresentation, bool bTestOnly = false )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException,
               std::exception);

    static OUString GetBrokenCellRangeForExport( const OUString &rCellRangeRepresentation );

protected:
    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwChartDataProvider( const SwDoc* pDoc );
    virtual ~SwChartDataProvider();

    // XDataProvider
    virtual sal_Bool SAL_CALL createDataSourcePossible( const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::chart2::data::XDataSource > SAL_CALL createDataSource( const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL detectArguments( const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible( const OUString& aRangeRepresentation ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation( const OUString& aRangeRepresentation ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sheet::XRangeSelection > SAL_CALL getRangeSelection(  ) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence>
        SAL_CALL createDataSequenceByValueArray(
            const OUString& aRole, const OUString& aRangeRepresentation )
                throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XRangeXMLConversion
    virtual OUString SAL_CALL convertRangeToXML( const OUString& aRangeRepresentation )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual OUString SAL_CALL convertRangeFromXML( const OUString& aXMLRange ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    void        AddDataSequence( const SwTable &rTable, css::uno::Reference< css::chart2::data::XDataSequence > &rxDataSequence );
    void        RemoveDataSequence( const SwTable &rTable, css::uno::Reference< css::chart2::data::XDataSequence > &rxDataSequence );

    // will send modifdied events for all data-sequences of the table
    void        InvalidateTable( const SwTable *pTable );
    bool        DeleteBox( const SwTable *pTable, const SwTableBox &rBox );
    void        DisposeAllDataSequences( const SwTable *pTable );

    // functionality needed to get notified about new added rows/cols
    void        AddRowCols( const SwTable &rTable, const SwSelBoxes& rBoxes, sal_uInt16 nLines, bool bBehind );
};

typedef cppu::WeakImplHelper
<
    css::chart2::data::XDataSource,
    css::lang::XServiceInfo
>
SwChartDataSourceBaseClass;

class SwChartDataSource :
    public SwChartDataSourceBaseClass
{
    css::uno::Sequence<
        css::uno::Reference< css::chart2::data::XLabeledDataSequence > > aLDS;

    SwChartDataSource( const SwChartDataSource & ) = delete;
    SwChartDataSource & operator = ( const SwChartDataSource & ) = delete;

public:
    SwChartDataSource( const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > &rLDS );
    virtual ~SwChartDataSource();

    // XDataSource
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;
};

typedef cppu::WeakImplHelper
<
    css::chart2::data::XDataSequence,
    css::chart2::data::XTextualDataSequence,
    css::chart2::data::XNumericalDataSequence,
    css::util::XCloneable,
    css::beans::XPropertySet,
    css::lang::XServiceInfo,
    css::lang::XUnoTunnel,
    css::util::XModifiable,
    css::lang::XEventListener,
    css::lang::XComponent
>
SwChartDataSequenceBaseClass;

class SwChartDataSequence :
    public SwChartDataSequenceBaseClass,
    public SwClient
{
    ::cppu::OInterfaceContainerHelper          aEvtListeners;
    ::cppu::OInterfaceContainerHelper          aModifyListeners;
    css::chart2::data::DataSequenceRole        aRole;

    OUString  aRowLabelText;
    OUString  aColLabelText;

    // holds a reference to the data-provider to guarantee its lifetime last as
    // long as the pointer may be used.
    css::uno::Reference< css::chart2::data::XDataProvider >    xDataProvider;
    SwChartDataProvider *                   pDataProvider;

    sw::UnoCursorPointer pTableCrsr;   // cursor spanned over cells to use

    const SfxItemPropertySet*   _pPropSet;

    bool    bDisposed;

    SwChartDataSequence( const SwChartDataSequence &rObj );
    SwChartDataSequence & operator = ( const SwChartDataSequence & ) = delete;

protected:
    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwChartDataSequence( SwChartDataProvider &rProvider,
                         SwFrameFormat   &rTblFmt,
                         std::shared_ptr<SwUnoCrsr> pTableCursor );
    virtual ~SwChartDataSequence();

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    // XDataSequence
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getData()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual OUString SAL_CALL getSourceRangeRepresentation()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL generateLabel( css::chart2::data::LabelOrigin eLabelOrigin )
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XTextualDataSequence
    virtual css::uno::Sequence< OUString > SAL_CALL getTextualData()
        throw (css::uno::RuntimeException,
               std::exception) override;

    // XNumericalDataSequence
    virtual css::uno::Sequence< double > SAL_CALL getNumericalData()
        throw (css::uno::RuntimeException,
               std::exception) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XModifiable
    virtual sal_Bool SAL_CALL isModified(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    SwFrameFormat*   GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }
    bool    DeleteBox( const SwTableBox &rBox );

    void        FillRangeDesc( SwRangeDescriptor &rRangeDesc ) const;
    bool        ExtendTo( bool bExtendCol, sal_Int32 nFirstNew, sal_Int32 nCount );
    std::vector< css::uno::Reference< css::table::XCell > > GetCells();
};

typedef cppu::WeakImplHelper
<
    css::chart2::data::XLabeledDataSequence2,
    css::lang::XServiceInfo,
    css::util::XModifyListener,
    css::lang::XComponent
>
SwChartLabeledDataSequenceBaseClass;

class SwChartLabeledDataSequence :
    public SwChartLabeledDataSequenceBaseClass
{
    ::cppu::OInterfaceContainerHelper                           aEvtListeners;
    ::cppu::OInterfaceContainerHelper                           aModifyListeners;

    css::uno::Reference< css::chart2::data::XDataSequence >     xData;
    css::uno::Reference< css::chart2::data::XDataSequence >     xLabels;

    bool    bDisposed;

    SwChartLabeledDataSequence( const SwChartLabeledDataSequence & ) = delete;
    SwChartLabeledDataSequence & operator = ( const SwChartLabeledDataSequence & ) = delete;

    void    SetDataSequence( css::uno::Reference< css::chart2::data::XDataSequence >& rxDest, const css::uno::Reference< css::chart2::data::XDataSequence >& rxSource );

public:
    SwChartLabeledDataSequence();
    virtual ~SwChartLabeledDataSequence();

    // XLabeledDataSequence
    virtual css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL getValues(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValues( const css::uno::Reference< css::chart2::data::XDataSequence >& xSequence ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL getLabel(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLabel( const css::uno::Reference< css::chart2::data::XDataSequence >& xSequence ) throw (css::uno::RuntimeException, std::exception) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
