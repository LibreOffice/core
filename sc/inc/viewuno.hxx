/*************************************************************************
 *
 *  $RCSfile: viewuno.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_VIEWUNO_HXX
#define SC_VIEWUNO_HXX

#ifndef _SFX_SFXBASECONTROLLER_HXX_
#include <sfx2/sfxbasecontroller.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _COM_SUN_STAR_VIEW_XCONTROLACCESS_HPP_
#include <com/sun/star/view/XControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEREFERRER_HPP_
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XVIEWSPLITABLE_HPP_
#include <com/sun/star/sheet/XViewSplitable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XVIEWFREEZABLE_HPP_
#include <com/sun/star/sheet/XViewFreezable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETVIEW_HPP_
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XVIEWPANE_HPP_
#include <com/sun/star/sheet/XViewPane.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

class ScTabViewShell;

#define SC_VIEWPANE_ACTIVE  0xFFFF


typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XEventListener >* XEventListenerPtr;
SV_DECL_PTRARR_DEL( EventListenerArr, XEventListenerPtr, 4, 4 );

typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::view::XSelectionChangeListener >* XSelectionChangeListenerPtr;
SV_DECL_PTRARR_DEL( XSelectionChangeListenerArr_Impl, XSelectionChangeListenerPtr, 4, 4 );


//  ScViewPaneBase not derived from OWeakObject
//  to avoid duplicate OWeakObject in ScTabViewObj

class ScViewPaneBase : public com::sun::star::sheet::XViewPane,
                       public com::sun::star::sheet::XCellRangeReferrer,
                       public com::sun::star::view::XControlAccess,
                       public com::sun::star::lang::XServiceInfo,
                       public com::sun::star::lang::XTypeProvider,
                       public SfxListener
{
private:
    ScTabViewShell*         pViewShell;
    USHORT                  nPane;          // ScSplitPos oder SC_VIEWPANE_ACTIVE

protected:
    ScTabViewShell*         GetViewShell() const    { return pViewShell; }

public:
                            ScViewPaneBase(ScTabViewShell* pViewSh, USHORT nP);
    virtual                 ~ScViewPaneBase();

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
                            ScViewPaneObj(ScTabViewShell* pViewSh, USHORT nP);
    virtual                 ~ScViewPaneObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);
};


//  OWeakObject is base of SfxBaseController -> use ScViewPaneBase

class ScTabViewObj : public ScViewPaneBase,
                     public SfxBaseController,
                     public com::sun::star::sheet::XSpreadsheetView,
                     public com::sun::star::container::XEnumerationAccess,
                     public com::sun::star::container::XIndexAccess,
                     public com::sun::star::view::XSelectionSupplier,
                     public com::sun::star::beans::XPropertySet,
                     public com::sun::star::sheet::XViewSplitable,
                     public com::sun::star::sheet::XViewFreezable,
                     public com::sun::star::lang::XUnoTunnel
{
private:
    SfxItemPropertySet                  aPropSet;
    XSelectionChangeListenerArr_Impl    aSelectionListeners;

    ScViewPaneObj*          GetObjectByIndex_Impl(USHORT nIndex) const;

public:
                            ScTabViewObj();
                            ScTabViewObj(ScTabViewShell* pViewSh);
    virtual                 ~ScTabViewObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

    void                    SelectionChanged();

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
};


#endif

