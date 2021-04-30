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

#pragma once

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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/datatransfer/XTransferableSupplier.hpp>
#include <comphelper/servicehelper.hxx>

#include "types.hxx"

namespace com::sun::star::view { class XSelectionChangeListener; }

class ScTabViewShell;
class ScPreviewShell;

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

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XViewPane
    virtual sal_Int32 SAL_CALL getFirstVisibleColumn() override;
    virtual void SAL_CALL   setFirstVisibleColumn( sal_Int32 nFirstVisibleColumn ) override;
    virtual sal_Int32 SAL_CALL getFirstVisibleRow() override;
    virtual void SAL_CALL   setFirstVisibleRow( sal_Int32 nFirstVisibleRow ) override;
    virtual css::table::CellRangeAddress SAL_CALL getVisibleRange() override;

                            // XCellRangeReferrer
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getReferredCells() override;

                            // XFormLayerAccess
    virtual css::uno::Reference< css::form::runtime::XFormController > SAL_CALL
                            getFormController( const css::uno::Reference< css::form::XForm >& Form ) override;
    virtual sal_Bool SAL_CALL
                            isFormDesignMode(  ) override;
    virtual void SAL_CALL   setFormDesignMode( sal_Bool DesignMode ) override;

                            // XControlAccess
    virtual css::uno::Reference< css::awt::XControl > SAL_CALL
                            getControl( const css::uno::Reference< css::awt::XControlModel >& xModel ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
};

//  ScViewPaneObj for direct use (including OWeakObject)

class ScViewPaneObj final : public ScViewPaneBase, public cppu::OWeakObject
{
public:
                            ScViewPaneObj(ScTabViewShell* pViewSh, sal_uInt16 nP);
    virtual                 ~ScViewPaneObj() override;

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL   acquire() noexcept override;
    virtual void SAL_CALL   release() noexcept override;
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
                     public css::lang::XUnoTunnel,
                     public css::datatransfer::XTransferableSupplier,
                     public css::sheet::XSelectedSheetsSupplier
{
private:
    SfxItemPropertySet                      aPropSet;
    std::vector<css::uno::Reference<css::view::XSelectionChangeListener> >
                                            aSelectionChgListeners;
    std::vector<css::uno::Reference<css::sheet::XRangeSelectionListener> >
                                            aRangeSelListeners;
    std::vector<css::uno::Reference<css::sheet::XRangeSelectionChangeListener> >
                                            aRangeChgListeners;
    std::vector<css::uno::Reference<css::beans::XPropertyChangeListener> >
                                            aPropertyChgListeners;
    std::vector<css::uno::Reference<css::awt::XEnhancedMouseClickHandler> >
                                            aMouseClickHandlers;
    std::vector<css::uno::Reference<css::sheet::XActivationEventListener> >
                                            aActivationListeners;
    SCTAB                                   nPreviousTab;
    bool                                    bDrawSelModeSet;
    bool                                    bFilteredRangeSelection;

    rtl::Reference<ScViewPaneObj> GetObjectByIndex_Impl(sal_uInt16 nIndex) const;
    sal_Int16                   GetZoom() const;
    void                    SetZoom(sal_Int16 Zoom);
    sal_Int16                   GetZoomType() const;
    void                    SetZoomType(sal_Int16 ZoomType);

    css::uno::Reference< css::uno::XInterface > GetClickedObject(const Point& rPoint) const;
    void                    EndMouseListening();
    void                    EndActivationListening();
    bool                    mbLeftMousePressed;
public:
                            ScTabViewObj(ScTabViewShell* pViewSh);
                            ScTabViewObj() = delete;
    virtual                 ~ScTabViewObj() override;

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL   acquire() noexcept override;
    virtual void SAL_CALL   release() noexcept override;

    void                    SelectionChanged();
    void                    VisAreaChanged();
    // bSameTabButMoved = true if the same sheet as before is activated, used after moving/copying/inserting/deleting a sheet
    void                    SheetChanged( bool bSameTabButMoved );
    bool                    IsMouseListening() const;
    /// @throws css::uno::RuntimeException
    bool                    MousePressed( const css::awt::MouseEvent& e );
    /// @throws css::uno::RuntimeException
    bool                    MouseReleased( const css::awt::MouseEvent& e );

    void                    RangeSelDone( const OUString& rText );
    void                    RangeSelAborted( const OUString& rText );
    void                    RangeSelChanged( const OUString& rText );

                            // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) override;
    virtual css::uno::Any SAL_CALL getSelection() override;
    virtual void SAL_CALL   addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void SAL_CALL   removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

    //! XPrintable?

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XSpreadsheetView
    virtual css::uno::Reference< css::sheet::XSpreadsheet > SAL_CALL
                            getActiveSheet() override;
    virtual void SAL_CALL   setActiveSheet( const css::uno::Reference< css::sheet::XSpreadsheet >& xActiveSheet ) override;

                            //XEnhancedMouseClickBroadcaster

    virtual void SAL_CALL addEnhancedMouseClickHandler( const css::uno::Reference<
                                css::awt::XEnhancedMouseClickHandler >& aListener ) override;
    virtual void SAL_CALL removeEnhancedMouseClickHandler( const css::uno::Reference< css::awt::XEnhancedMouseClickHandler >& aListener ) override;

                            //XActivationBroadcaster

    virtual void SAL_CALL addActivationEventListener( const css::uno::Reference< css::sheet::XActivationEventListener >& aListener ) override;
    virtual void SAL_CALL removeActivationEventListener( const css::uno::Reference< css::sheet::XActivationEventListener >& aListener ) override;

                            // XViewSplitable
    virtual sal_Bool SAL_CALL getIsWindowSplit() override;
    virtual sal_Int32 SAL_CALL getSplitHorizontal() override;
    virtual sal_Int32 SAL_CALL getSplitVertical() override;
    virtual sal_Int32 SAL_CALL getSplitColumn() override;
    virtual sal_Int32 SAL_CALL getSplitRow() override;
    virtual void SAL_CALL   splitAtPosition( sal_Int32 nPixelX, sal_Int32 nPixelY ) override;

                            // XViewFreezable
    virtual sal_Bool SAL_CALL hasFrozenPanes() override;
    virtual void SAL_CALL   freezeAtPosition( sal_Int32 nColumns, sal_Int32 nRows ) override;

                            // XRangeSelection
    virtual void SAL_CALL   startRangeSelection( const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;
    virtual void SAL_CALL   abortRangeSelection() override;
    virtual void SAL_CALL   addRangeSelectionListener( const css::uno::Reference< css::sheet::XRangeSelectionListener >& aListener ) override;
    virtual void SAL_CALL   removeRangeSelectionListener( const css::uno::Reference< css::sheet::XRangeSelectionListener >& aListener ) override;
    virtual void SAL_CALL   addRangeSelectionChangeListener( const css::uno::Reference< css::sheet::XRangeSelectionChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeRangeSelectionChangeListener( const css::uno::Reference< css::sheet::XRangeSelectionChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

                            // XUnoTunnel
    UNO3_GETIMPLEMENTATION_DECL(ScTabViewObj)

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XTransferableSupplier
    virtual css::uno::Reference< css::datatransfer::XTransferable > SAL_CALL getTransferable() override;
    virtual void SAL_CALL insertTransferable( const css::uno::Reference< css::datatransfer::XTransferable >& xTrans ) override;

    // XSelectedSheetsSupplier
    virtual css::uno::Sequence<sal_Int32> SAL_CALL getSelectedSheets() override;
};

class ScPreviewObj final : public SfxBaseController,
                     public SfxListener,
                     public css::sheet::XSelectedSheetsSupplier
{
    ScPreviewShell* mpViewShell;
public:
    ScPreviewObj(ScPreviewShell* pViewSh);
    virtual ~ScPreviewObj() override;

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType) override;

    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    virtual void Notify(SfxBroadcaster&, const SfxHint& rHint) override;

    // XSelectedSheetsSupplier
    virtual css::uno::Sequence<sal_Int32> SAL_CALL getSelectedSheets() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
