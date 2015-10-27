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

#ifndef INCLUDED_SC_INC_VIEWUNO_HXX
#define INCLUDED_SC_INC_VIEWUNO_HXX

#include <sfx2/sfxbasecontroller.hxx>
#include <svl/itemprop.hxx>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
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

#include "address.hxx"

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
    sal_uInt16                  nPane;          // ScSplitPos oder SC_VIEWPANE_ACTIVE

protected:
    css::awt::Rectangle GetVisArea() const;

public:
                            ScViewPaneBase(ScTabViewShell* pViewSh, sal_uInt16 nP);
    virtual                 ~ScViewPaneBase();

    ScTabViewShell*         GetViewShell() const    { return pViewShell; }

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XViewPane
    virtual sal_Int32 SAL_CALL getFirstVisibleColumn() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFirstVisibleColumn( sal_Int32 nFirstVisibleColumn )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getFirstVisibleRow() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFirstVisibleRow( sal_Int32 nFirstVisibleRow )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::table::CellRangeAddress SAL_CALL getVisibleRange()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;

                            // XCellRangeReferrer
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getReferredCells() throw(css::uno::RuntimeException, std::exception) override;

                            // XFormLayerAccess
    virtual css::uno::Reference< css::form::runtime::XFormController > SAL_CALL
                            getFormController( const css::uno::Reference< css::form::XForm >& Form )
                                    throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
                            isFormDesignMode(  )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormDesignMode( sal_Bool DesignMode )
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XControlAccess
    virtual css::uno::Reference< css::awt::XControl > SAL_CALL
                            getControl( const css::uno::Reference< css::awt::XControlModel >& xModel )
                                    throw(css::container::NoSuchElementException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;
};

//  ScViewPaneObj for direct use (including OWeakObject)

class ScViewPaneObj : public ScViewPaneBase, public cppu::OWeakObject
{
public:
                            ScViewPaneObj(ScTabViewShell* pViewSh, sal_uInt16 nP);
    virtual                 ~ScViewPaneObj();

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;
};

//  OWeakObject is base of SfxBaseController -> use ScViewPaneBase

class ScTabViewObj : public ScViewPaneBase,
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
    typedef std::vector<css::uno::Reference<css::sheet::XRangeSelectionListener> > XRangeSelectionListenerVector;
    typedef std::vector<css::uno::Reference<css::sheet::XRangeSelectionChangeListener> > XRangeSelectionChangeListenerVector;
    typedef std::vector<css::uno::Reference<css::view::XSelectionChangeListener> > XSelectionChangeListenerVector;
    typedef std::vector<css::uno::Reference<css::beans::XPropertyChangeListener> > XViewPropertyChangeListenerVector;
    typedef std::vector<css::uno::Reference<css::awt::XEnhancedMouseClickHandler> > XMouseClickHandlerVector;
    typedef std::vector<css::uno::Reference<css::sheet::XActivationEventListener> > XActivationEventListenerVector;

    SfxItemPropertySet                      aPropSet;
    XSelectionChangeListenerVector          aSelectionChgListeners;
    XRangeSelectionListenerVector           aRangeSelListeners;
    XRangeSelectionChangeListenerVector     aRangeChgListeners;
    XViewPropertyChangeListenerVector       aPropertyChgListeners;
    XMouseClickHandlerVector                aMouseClickHandlers;
    XActivationEventListenerVector          aActivationListeners;
    SCTAB                                   nPreviousTab;
    bool                                    bDrawSelModeSet;
    bool                                    bFilteredRangeSelection;

    ScViewPaneObj*          GetObjectByIndex_Impl(sal_uInt16 nIndex) const;
    sal_Int16                   GetZoom() const;
    void                    SetZoom(sal_Int16 Zoom);
    sal_Int16                   GetZoomType() const;
    void                    SetZoomType(sal_Int16 ZoomType);

    css::uno::Reference< css::uno::XInterface > GetClickedObject(const Point& rPoint) const;
    void                    EndMouseListening();
    void                    EndActivationListening();
    bool                    mbLeftMousePressed;
    bool                    mbPendingSelectionChanged;
    ScTabViewObj(); // disabled
public:
                            ScTabViewObj(ScTabViewShell* pViewSh);
    virtual                 ~ScTabViewObj();

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    void                    SelectionChanged();
    void                    VisAreaChanged();
    // bSameTabButMoved = true if the same sheet as before is activated, used after moving/copying/inserting/deleting a sheet
    void                    SheetChanged( bool bSameTabButMoved = false );
    bool                    IsMouseListening() const;
    bool                    MousePressed( const css::awt::MouseEvent& e ) throw (css::uno::RuntimeException);
    bool                    MouseReleased( const css::awt::MouseEvent& e ) throw (css::uno::RuntimeException);

    void                    RangeSelDone( const OUString& rText );
    void                    RangeSelAborted( const OUString& rText );
    void                    RangeSelChanged( const OUString& rText );

                            // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection )
                                throw(css::lang::IllegalArgumentException,
                                      css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::uno::Any SAL_CALL getSelection()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener )
                                    throw (css::uno::RuntimeException,
                                           std::exception) override;
    virtual void SAL_CALL   removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener )
                                    throw(css::uno::RuntimeException, std::exception) override;

    //! XPrintable?

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException,
                                    std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException,
                                    std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw (css::beans::UnknownPropertyException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
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
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XSpreadsheetView
    virtual css::uno::Reference< css::sheet::XSpreadsheet > SAL_CALL
                            getActiveSheet() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setActiveSheet( const css::uno::Reference< css::sheet::XSpreadsheet >& xActiveSheet )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;

                            //XEnhancedMouseClickBroadcaster

    virtual void SAL_CALL addEnhancedMouseClickHandler( const css::uno::Reference<
                                css::awt::XEnhancedMouseClickHandler >& aListener )
                                    throw (css::uno::RuntimeException,
                                           std::exception) override;
    virtual void SAL_CALL removeEnhancedMouseClickHandler( const css::uno::Reference< css::awt::XEnhancedMouseClickHandler >& aListener )
                                    throw (css::uno::RuntimeException, std::exception) override;

                            //XActivationBroadcaster

    virtual void SAL_CALL addActivationEventListener( const css::uno::Reference< css::sheet::XActivationEventListener >& aListener )
                                    throw (css::uno::RuntimeException,
                                           std::exception) override;
    virtual void SAL_CALL removeActivationEventListener( const css::uno::Reference< css::sheet::XActivationEventListener >& aListener )
                                    throw (css::uno::RuntimeException, std::exception) override;

                            // XViewSplitable
    virtual sal_Bool SAL_CALL getIsWindowSplit() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSplitHorizontal() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSplitVertical() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSplitColumn()
        throw(css::uno::RuntimeException,
              std::exception) override;
    virtual sal_Int32 SAL_CALL getSplitRow()
        throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   splitAtPosition( sal_Int32 nPixelX, sal_Int32 nPixelY )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;

                            // XViewFreezable
    virtual sal_Bool SAL_CALL hasFrozenPanes() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   freezeAtPosition( sal_Int32 nColumns, sal_Int32 nRows )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;

                            // XRangeSelection
    virtual void SAL_CALL   startRangeSelection( const css::uno::Sequence< css::beans::PropertyValue >& aArguments )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   abortRangeSelection() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addRangeSelectionListener( const css::uno::Reference< css::sheet::XRangeSelectionListener >& aListener )
                                    throw (css::uno::RuntimeException,
                                           std::exception) override;
    virtual void SAL_CALL   removeRangeSelectionListener( const css::uno::Reference< css::sheet::XRangeSelectionListener >& aListener )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addRangeSelectionChangeListener( const css::uno::Reference< css::sheet::XRangeSelectionChangeListener >& aListener )
                                    throw (css::uno::RuntimeException,
                                           std::exception) override;
    virtual void SAL_CALL   removeRangeSelectionChangeListener( const css::uno::Reference< css::sheet::XRangeSelectionChangeListener >& aListener )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTabViewObj* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;

    // XTransferableSupplier
    virtual css::uno::Reference< css::datatransfer::XTransferable > SAL_CALL getTransferable()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL insertTransferable( const css::uno::Reference< css::datatransfer::XTransferable >& xTrans )
                                throw(css::datatransfer::UnsupportedFlavorException,
                                      css::uno::RuntimeException,
                                      std::exception) override;

    // XSelectedSheetsSupplier
    virtual css::uno::Sequence<sal_Int32> SAL_CALL getSelectedSheets()
        throw(css::uno::RuntimeException,
              std::exception) override;
};

class ScPreviewObj : public SfxBaseController,
                     public SfxListener,
                     public css::sheet::XSelectedSheetsSupplier
{
    ScPreviewShell* mpViewShell;
public:
    ScPreviewObj(ScPreviewShell* pViewSh);
    virtual ~ScPreviewObj();

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType)
            throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    virtual void Notify(SfxBroadcaster&, const SfxHint& rHint) override;

    // XSelectedSheetsSupplier
    virtual css::uno::Sequence<sal_Int32> SAL_CALL getSelectedSheets()
        throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
