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

#ifndef SC_VIEWUNO_HXX
#define SC_VIEWUNO_HXX

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
#include "boost/ptr_container/ptr_vector.hpp"

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
                                    throw(::com::sun::star::uno::RuntimeException);

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XViewPane
    virtual sal_Int32 SAL_CALL getFirstVisibleColumn() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFirstVisibleColumn( sal_Int32 nFirstVisibleColumn )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getFirstVisibleRow() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFirstVisibleRow( sal_Int32 nFirstVisibleRow )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getVisibleRange()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeReferrer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getReferredCells() throw(::com::sun::star::uno::RuntimeException);

                            // XFormLayerAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController > SAL_CALL
                            getFormController( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::form::XForm >& Form )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL
                            isFormDesignMode(  )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormDesignMode( ::sal_Bool DesignMode )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XControlAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > SAL_CALL
                            getControl( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel >& xModel )
                                    throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::uno::RuntimeException);

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


//  ScViewPaneObj for direct use (including OWeakObject)

class ScViewPaneObj : public ScViewPaneBase, public cppu::OWeakObject
{
public:
                            ScViewPaneObj(ScTabViewShell* pViewSh, sal_uInt16 nP);
    virtual                 ~ScViewPaneObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw();
    virtual void SAL_CALL   release() throw();
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
    sal_Bool                                bDrawSelModeSet;
    sal_Bool                                bFilteredRangeSelection;

    ScViewPaneObj*          GetObjectByIndex_Impl(sal_uInt16 nIndex) const;
    sal_Int16                   GetZoom(void) const;
    void                    SetZoom(sal_Int16 Zoom);
    sal_Int16                   GetZoomType(void) const;
    void                    SetZoomType(sal_Int16 ZoomType);

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > GetClickedObject(const Point& rPoint) const;
    void                    StartMouseListening();
    void                    EndMouseListening();
    void                    StartActivationListening();
    void                    EndActivationListening();

    ScTabViewObj(); // disabled
public:
                            ScTabViewObj(ScTabViewShell* pViewSh);
    virtual                 ~ScTabViewObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw();
    virtual void SAL_CALL   release() throw();

    void                    SelectionChanged();
    void                    VisAreaChanged();
    // bSameTabButMoved = true if the same sheet as before is activated, used after moving/copying/inserting/deleting a sheet
    void                    SheetChanged( bool bSameTabButMoved = false );
    bool                    IsMouseListening() const;
    sal_Bool                MousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool                MouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    void                    RangeSelDone( const String& rText );
    void                    RangeSelAborted( const String& rText );
    void                    RangeSelChanged( const String& rText );

                            // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addSelectionChangeListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::view::XSelectionChangeListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeSelectionChangeListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::view::XSelectionChangeListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);

    //! XPrintable?

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

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

                            // XSpreadsheetView
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getActiveSheet() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setActiveSheet( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSpreadsheet >& xActiveSheet )
                                    throw(::com::sun::star::uno::RuntimeException);

                            //XEnhancedMouseClickBroadcaster

    virtual void SAL_CALL addEnhancedMouseClickHandler( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XEnhancedMouseClickHandler >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEnhancedMouseClickHandler( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XEnhancedMouseClickHandler >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException);

                            //XActivationBroadcaster

    virtual void SAL_CALL addActivationEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XActivationEventListener >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActivationEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XActivationEventListener >& aListener )
                                    throw (::com::sun::star::uno::RuntimeException);

                            // XViewSplitable
    virtual sal_Bool SAL_CALL getIsWindowSplit() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSplitHorizontal() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSplitVertical() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSplitColumn() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSplitRow() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   splitAtPosition( sal_Int32 nPixelX, sal_Int32 nPixelY )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XViewFreezable
    virtual sal_Bool SAL_CALL hasFrozenPanes() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   freezeAtPosition( sal_Int32 nColumns, sal_Int32 nRows )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XRangeSelection
    virtual void SAL_CALL   startRangeSelection( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& aArguments )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   abortRangeSelection() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addRangeSelectionListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XRangeSelectionListener >& aListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeRangeSelectionListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XRangeSelectionListener >& aListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addRangeSelectionChangeListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XRangeSelectionChangeListener >& aListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeRangeSelectionChangeListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XRangeSelectionChangeListener >& aListener )
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

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTabViewObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);

    // XTransferableSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getTransferable(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertTransferable( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans ) throw (::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::uno::RuntimeException);

    // XSelectedSheetsSupplier
    virtual ::com::sun::star::uno::Sequence<sal_Int32> SAL_CALL getSelectedSheets()
        throw(::com::sun::star::uno::RuntimeException);
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
            throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    virtual void Notify(SfxBroadcaster&, const SfxHint& rHint);

    // XSelectedSheetsSupplier
    virtual ::com::sun::star::uno::Sequence<sal_Int32> SAL_CALL getSelectedSheets()
        throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
