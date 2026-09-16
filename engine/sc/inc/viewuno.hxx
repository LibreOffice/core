/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <rtl/ref.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>
#include <tools/gen.hxx>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XViewSplitable.hpp>
#include <com/sun/star/sheet/XViewFreezable.hpp>
#include <com/sun/star/sheet/XSelectedSheetsSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XEnhancedMouseClickBroadcaster.hpp>
#include <com/sun/star/sheet/XActivationBroadcaster.hpp>
#include <com/sun/star/sheet/XViewPane.hpp>
#include <com/sun/star/sheet/XRangeSelection.hpp>
#include <com/sun/star/sheet/XSheetRange.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/datatransfer/XTransferableSupplier.hpp>
#include <comphelper/servicehelper.hxx>

#include "types.hxx"

namespace com::sun::star::view { class XSelectionChangeListener; }

class ScTabViewShell;
class ScPreviewShell;
class ScTableSheetObj;

#define SC_VIEWPANE_ACTIVE  0xFFFF

//  ScViewPaneBase not derived from OWeakObject
//  to avoid duplicate OWeakObject in ScTabViewObj

class ScViewPaneBase : public css::sheet::XViewPane,
                       public css::sheet::XCellRangeReferrer,
                       public css::view::XFormLayerAccess,
                       public css::lang::XServiceInfo,
                       public css::lang::XTypeProvider,
                       public SfxListener
{
private:
    ScTabViewShell*         pViewShell;
    sal_uInt16                  nPane;          // ScSplitPos or SC_VIEWPANE_ACTIVE

protected:
    css::awt::Rectangle GetVisArea() const;

public:
                            ScViewPaneBase(ScTabViewShell* pViewSh, sal_uInt16 nP);
    virtual                 ~ScViewPaneBase() override;

    ScTabViewShell*         GetViewShell() const    { return pViewShell; }

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XViewPane
    virtual sal_Int32 getFirstVisibleColumn() override;
    virtual void   setFirstVisibleColumn( sal_Int32 nFirstVisibleColumn ) override;
    virtual sal_Int32 getFirstVisibleRow() override;
    virtual void   setFirstVisibleRow( sal_Int32 nFirstVisibleRow ) override;
    virtual css::table::CellRangeAddress getVisibleRange() override;

                            // XCellRangeReferrer
    virtual cpo::uno::Reference< css::table::XCellRange >
                            getReferredCells() override;

                            // XFormLayerAccess
    virtual cpo::uno::Reference< css::form::runtime::XFormController >
                            getFormController( const cpo::uno::Reference< css::form::XForm >& Form ) override;
    virtual bool
                            isFormDesignMode(  ) override;
    virtual void   setFormDesignMode( bool DesignMode ) override;

                            // XControlAccess
    virtual cpo::uno::Reference< css::awt::XControl >
                            getControl( const cpo::uno::Reference< css::awt::XControlModel >& xModel ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

                            // XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;
};

//  ScViewPaneObj for direct use (including OWeakObject)

class ScViewPaneObj final : public ScViewPaneBase, public cppu::OWeakObject
{
public:
                            ScViewPaneObj(ScTabViewShell* pViewSh, sal_uInt16 nP);
    virtual                 ~ScViewPaneObj() override;

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void   acquire() noexcept override;
    virtual void   release() noexcept override;
};

//  OWeakObject is base of SfxBaseController -> use ScViewPaneBase

class ScTabViewObj final : public ScViewPaneBase,
                     public SfxBaseController,
                     public css::sheet::XSpreadsheetView,
                     public css::sheet::XEnhancedMouseClickBroadcaster,
                     public css::sheet::XActivationBroadcaster,
                     public css::container::XEnumerationAccess,
                     public css::container::XIndexAccess,
                     public css::view::XSelectionSupplier,
                     public css::beans::XPropertySet,
                     public css::sheet::XViewSplitable,
                     public css::sheet::XViewFreezable,
                     public css::sheet::XRangeSelection,
                     public css::sheet::XSheetRange,
                     public css::datatransfer::XTransferableSupplier,
                     public css::sheet::XSelectedSheetsSupplier
{
private:
    typedef std::vector<cpo::uno::Reference<css::sheet::XRangeSelectionListener>> RangeSelListeners;

    SfxItemPropertySet                      aPropSet;
    std::vector<cpo::uno::Reference<css::view::XSelectionChangeListener> >
                                            aSelectionChgListeners;
    RangeSelListeners                       aRangeSelListeners;
    std::vector<cpo::uno::Reference<css::sheet::XRangeSelectionChangeListener> >
                                            aRangeChgListeners;
    std::vector<cpo::uno::Reference<css::beans::XPropertyChangeListener> >
                                            aPropertyChgListeners;
    std::vector<cpo::uno::Reference<css::awt::XEnhancedMouseClickHandler> >
                                            aMouseClickHandlers;
    std::vector<cpo::uno::Reference<css::sheet::XActivationEventListener> >
                                            aActivationListeners;
    SCTAB                                   nPreviousTab;
    bool                                    bDrawSelModeSet;
    bool                                    bFilteredRangeSelection;

    rtl::Reference<ScViewPaneObj> GetObjectByIndex_Impl(sal_uInt16 nIndex) const;
    sal_Int16                   GetZoom() const;
    void                    SetZoom(sal_Int16 Zoom);
    sal_Int16                   GetZoomType() const;
    void                    SetZoomType(sal_Int16 ZoomType);

    cpo::uno::Reference< cpo::uno::XInterface > GetClickedObject(const Point& rPoint) const;
    void                    EndMouseListening();
    void                    EndActivationListening();
    bool                    mbLeftMousePressed;
public:
                            ScTabViewObj(ScTabViewShell* pViewSh);
                            ScTabViewObj() = delete;
    virtual                 ~ScTabViewObj() override;

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void   acquire() noexcept override;
    virtual void   release() noexcept override;

    void                    SelectionChanged();
    void                    VisAreaChanged();
    // bSameTabButMoved = true if the same sheet as before is activated, used after moving/copying/inserting/deleting a sheet
    void                    SheetChanged( bool bSameTabButMoved );
    bool                    IsMouseListening() const;
    /// @throws cpo::uno::RuntimeException
    bool                    MousePressed( const css::awt::MouseEvent& e );
    /// @throws cpo::uno::RuntimeException
    bool                    MouseReleased( const css::awt::MouseEvent& e );

    void                    RangeSelDone( const OUString& rText );
    void                    RangeSelAborted( const OUString& rText );
    void                    RangeSelChanged( const OUString& rText );

                            // XSelectionSupplier
    virtual bool select( const cpo::uno::Any& aSelection ) override;
    virtual cpo::uno::Any getSelection() override;
    virtual void   addSelectionChangeListener( const cpo::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void   removeSelectionChangeListener( const cpo::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

                            // XSheetRange
    virtual cpo::uno::Any getSelectionFromString( const OUString& aStrRange ) override;

    //! XPrintable?

                            // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XSpreadsheetView
    virtual cpo::uno::Reference< css::sheet::XSpreadsheet >
                            getActiveSheet() override;
    virtual void   setActiveSheet( const cpo::uno::Reference< css::sheet::XSpreadsheet >& xActiveSheet ) override;

                            //XEnhancedMouseClickBroadcaster

    virtual void addEnhancedMouseClickHandler( const cpo::uno::Reference<
                                css::awt::XEnhancedMouseClickHandler >& aListener ) override;
    virtual void removeEnhancedMouseClickHandler( const cpo::uno::Reference< css::awt::XEnhancedMouseClickHandler >& aListener ) override;

                            //XActivationBroadcaster

    virtual void addActivationEventListener( const cpo::uno::Reference< css::sheet::XActivationEventListener >& aListener ) override;
    virtual void removeActivationEventListener( const cpo::uno::Reference< css::sheet::XActivationEventListener >& aListener ) override;

                            // XViewSplitable
    virtual bool getIsWindowSplit() override;
    virtual sal_Int32 getSplitHorizontal() override;
    virtual sal_Int32 getSplitVertical() override;
    virtual sal_Int32 getSplitColumn() override;
    virtual sal_Int32 getSplitRow() override;
    virtual void   splitAtPosition( sal_Int32 nPixelX, sal_Int32 nPixelY ) override;

                            // XViewFreezable
    virtual bool hasFrozenPanes() override;
    virtual void   freezeAtPosition( sal_Int32 nColumns, sal_Int32 nRows ) override;

                            // XRangeSelection
    virtual void   startRangeSelection( const cpo::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;
    virtual void   abortRangeSelection() override;
    virtual void   addRangeSelectionListener( const cpo::uno::Reference< css::sheet::XRangeSelectionListener >& aListener ) override;
    virtual void   removeRangeSelectionListener( const cpo::uno::Reference< css::sheet::XRangeSelectionListener >& aListener ) override;
    virtual void   addRangeSelectionChangeListener( const cpo::uno::Reference< css::sheet::XRangeSelectionChangeListener >& aListener ) override;
    virtual void   removeRangeSelectionChangeListener( const cpo::uno::Reference< css::sheet::XRangeSelectionChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

                            // XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    // XTransferableSupplier
    virtual cpo::uno::Reference< css::datatransfer::XTransferable > getTransferable() override;
    virtual void insertTransferable( const cpo::uno::Reference< css::datatransfer::XTransferable >& xTrans ) override;

    // XSelectedSheetsSupplier
    virtual cpo::uno::Sequence<sal_Int32> getSelectedSheets() override;

    rtl::Reference< ScTableSheetObj > getActiveScSheet();
};

class ScPreviewObj final : public SfxBaseController,
                     public SfxListener,
                     public css::sheet::XSelectedSheetsSupplier
{
    ScPreviewShell* mpViewShell;
public:
    ScPreviewObj(ScPreviewShell* pViewSh);
    virtual ~ScPreviewObj() override;

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType) override;

    virtual void acquire() noexcept override;
    virtual void release() noexcept override;

    virtual void Notify(SfxBroadcaster&, const SfxHint& rHint) override;

    // XSelectedSheetsSupplier
    virtual cpo::uno::Sequence<sal_Int32> getSelectedSheets() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
