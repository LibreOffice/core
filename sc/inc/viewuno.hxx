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
#include <boost/ptr_container/ptr_vector.hpp>

class ScTabViewShell;
class ScPreviewShell;

#define SC_VIEWPANE_ACTIVE  0xFFFF

//  ScViewPaneBase not derived from OWeakObject
//  to avoid duplicate OWeakObject in ScTabViewObj

class ScViewPaneBase : public com::sun::star::sheet::XViewPane,
                       public com::sun::star::sheet::XCellRangeReferrer,
                       public com::sun::star::view::XFormLayerAccess,
                       public com::sun::star::lang::XServiceInfo,
                       public com::sun::star::lang::XTypeProvider,
                       public SfxListener
{
private:
    ScTabViewShell*         pViewShell;
    sal_uInt16                  nPane;          // ScSplitPos oder SC_VIEWPANE_ACTIVE

protected:
    ::com::sun::star::awt::Rectangle GetVisArea() const;

public:
                            ScViewPaneBase(ScTabViewShell* pViewSh, sal_uInt16 nP);
    virtual                 ~ScViewPaneBase();

    ScTabViewShell*         GetViewShell() const    { return pViewShell; }

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            // XViewPane
    virtual sal_Int32 SAL_CALL getFirstVisibleColumn() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setFirstVisibleColumn( sal_Int32 nFirstVisibleColumn )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getFirstVisibleRow() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setFirstVisibleRow( sal_Int32 nFirstVisibleRow )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getVisibleRange()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

                            // XCellRangeReferrer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getReferredCells() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XFormLayerAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController > SAL_CALL
                            getFormController( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::form::XForm >& Form )
                                    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL
                            isFormDesignMode(  )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setFormDesignMode( sal_Bool DesignMode )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XControlAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > SAL_CALL
                            getControl( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel >& xModel )
                                    throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

//  ScViewPaneObj for direct use (including OWeakObject)

class ScViewPaneObj : public ScViewPaneBase, public cppu::OWeakObject
{
public:
                            ScViewPaneObj(ScTabViewShell* pViewSh, sal_uInt16 nP);
    virtual                 ~ScViewPaneObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;
};

//  OWeakObject is base of SfxBaseController -> use ScViewPaneBase

class ScTabViewObj : public ScViewPaneBase,
                     public SfxBaseController,
                     public com::sun::star::sheet::XSpreadsheetView,
                     public com::sun::star::sheet::XEnhancedMouseClickBroadcaster,
                     public com::sun::star::sheet::XActivationBroadcaster,
                     public com::sun::star::container::XEnumerationAccess,
                     public com::sun::star::container::XIndexAccess,
                     public com::sun::star::view::XSelectionSupplier,
                     public com::sun::star::beans::XPropertySet,
                     public com::sun::star::sheet::XViewSplitable,
                     public com::sun::star::sheet::XViewFreezable,
                     public com::sun::star::sheet::XRangeSelection,
                     public com::sun::star::lang::XUnoTunnel,
                     public com::sun::star::datatransfer::XTransferableSupplier,
                     public com::sun::star::sheet::XSelectedSheetsSupplier
{
private:
    typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XRangeSelectionListener > XRangeSelectionListenerUnoRef;
    typedef boost::ptr_vector<XRangeSelectionListenerUnoRef> XRangeSelectionListenerVector;

    typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XRangeSelectionChangeListener > XRangeSelectionChangeListenerUnoRef;
    typedef boost::ptr_vector<XRangeSelectionChangeListenerUnoRef> XRangeSelectionChangeListenerVector;

    typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::view::XSelectionChangeListener > XSelectionChangeListenerUnoRef;
    typedef boost::ptr_vector<XSelectionChangeListenerUnoRef> XSelectionChangeListenerVector;

    typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener > XViewPropertyChangeListenerUnoRef;
    typedef boost::ptr_vector<XViewPropertyChangeListenerUnoRef> XViewPropertyChangeListenerVector;

    typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::awt::XEnhancedMouseClickHandler > XMouseClickHandlerUnoRef;
    typedef boost::ptr_vector<XMouseClickHandlerUnoRef> XMouseClickHandlerVector;

    typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XActivationEventListener > XActivationEventListenerUnoRef;
    typedef boost::ptr_vector<XActivationEventListenerUnoRef> XActivationEventListenerVector;

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

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > GetClickedObject(const Point& rPoint) const;
    void                    EndMouseListening();
    void                    EndActivationListening();
    bool                    mbLeftMousePressed;
    bool                    mbPendingSelectionChanged;
    ScTabViewObj(); // disabled
public:
                            ScTabViewObj(ScTabViewShell* pViewSh);
    virtual                 ~ScTabViewObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;

    void                    SelectionChanged();
    void                    VisAreaChanged();
    // bSameTabButMoved = true if the same sheet as before is activated, used after moving/copying/inserting/deleting a sheet
    void                    SheetChanged( bool bSameTabButMoved = false );
    bool                    IsMouseListening() const;
    bool                    MousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    bool                    MouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    void                    RangeSelDone( const OUString& rText );
    void                    RangeSelAborted( const OUString& rText );
    void                    RangeSelChanged( const OUString& rText );

                            // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addSelectionChangeListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::view::XSelectionChangeListener >& xListener )
                                    throw (::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeSelectionChangeListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::view::XSelectionChangeListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //! XPrintable?

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException,
                                    std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException,
                                    std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                       ::com::sun::star::lang::WrappedTargetException,
                                       ::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSpreadsheetView
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getActiveSheet() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setActiveSheet( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSpreadsheet >& xActiveSheet )
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;

                            //XEnhancedMouseClickBroadcaster

    virtual void SAL_CALL addEnhancedMouseClickHandler( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XEnhancedMouseClickHandler >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEnhancedMouseClickHandler( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XEnhancedMouseClickHandler >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            //XActivationBroadcaster

    virtual void SAL_CALL addActivationEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XActivationEventListener >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeActivationEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XActivationEventListener >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XViewSplitable
    virtual sal_Bool SAL_CALL getIsWindowSplit() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSplitHorizontal() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSplitVertical() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSplitColumn()
        throw(::com::sun::star::uno::RuntimeException,
              std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSplitRow()
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   splitAtPosition( sal_Int32 nPixelX, sal_Int32 nPixelY )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

                            // XViewFreezable
    virtual sal_Bool SAL_CALL hasFrozenPanes() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   freezeAtPosition( sal_Int32 nColumns, sal_Int32 nRows )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

                            // XRangeSelection
    virtual void SAL_CALL   startRangeSelection( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& aArguments )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   abortRangeSelection() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addRangeSelectionListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XRangeSelectionListener >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeRangeSelectionListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XRangeSelectionListener >& aListener )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addRangeSelectionChangeListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XRangeSelectionChangeListener >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeRangeSelectionChangeListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XRangeSelectionChangeListener >& aListener )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTabViewObj* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTransferableSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getTransferable()
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertTransferable( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans )
                                throw(::com::sun::star::datatransfer::UnsupportedFlavorException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

    // XSelectedSheetsSupplier
    virtual ::com::sun::star::uno::Sequence<sal_Int32> SAL_CALL getSelectedSheets()
        throw(::com::sun::star::uno::RuntimeException,
              std::exception) SAL_OVERRIDE;
};

class ScPreviewObj : public SfxBaseController,
                     public SfxListener,
                     public com::sun::star::sheet::XSelectedSheetsSupplier
{
    ScPreviewShell* mpViewShell;
public:
    ScPreviewObj(ScPreviewShell* pViewSh);
    virtual ~ScPreviewObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type & rType)
            throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL release() throw() SAL_OVERRIDE;

    virtual void Notify(SfxBroadcaster&, const SfxHint& rHint) SAL_OVERRIDE;

    // XSelectedSheetsSupplier
    virtual ::com::sun::star::uno::Sequence<sal_Int32> SAL_CALL getSelectedSheets()
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
