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
#ifndef _UNOTXVW_HXX
#define _UNOTXVW_HXX

#include <bf_sfx2/sfxbasecontroller.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XRubySelection.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/view/XScreenCursor.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XLineCursor.hpp>
#include <com/sun/star/view/XViewCursor.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase8.hxx>	// helper for implementations
#include <bf_svtools/itemprop.hxx>
#include "calbck.hxx"
#include "TextCursorHelper.hxx"
#include <comphelper/uno3.hxx>
class SvEmbeddedObjectRef; 
namespace binfilter {

class SwView;


typedef ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > * XSelectionChangeListenerPtr;
SV_DECL_PTRARR_DEL( SelectionChangeListenerArr, XSelectionChangeListenerPtr, 4, 4 )//STRIP008 ;

/******************************************************************************
 *
 ******************************************************************************/
class SwXTextView :
    public ::com::sun::star::view::XSelectionSupplier,
    public ::com::sun::star::lang::XServiceInfo,
    public ::com::sun::star::view::XControlAccess,
    public ::com::sun::star::text::XTextViewCursorSupplier,
    public ::com::sun::star::text::XRubySelection,
    public ::com::sun::star::view::XViewSettingsSupplier,
    public SfxBaseController
{
    SelectionChangeListenerArr aSelChangedListeners;

    SwView* 				pView;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > * 		pxViewSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextViewCursor > * 	pxTextViewCursor;


    SdrObject* GetControl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & Model,
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& xToFill  );

protected:
    virtual ~SwXTextView();
public:
    SwXTextView(SwView* pSwView);
    

    virtual 	::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XSelectionSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL select(const ::com::sun::star::uno::Any& aInterface) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > & xListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > & xListener) throw( ::com::sun::star::uno::RuntimeException );

    //XControlAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  SAL_CALL getControl(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & Model) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException );

    //XTextViewCursorSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextViewCursor >  SAL_CALL getViewCursor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XViewSettings
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getViewSettings(void) throw( ::com::sun::star::uno::RuntimeException );

    //XRubySelection
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue > > SAL_CALL getRubyList( sal_Bool bAutomatic )
                throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setRubyList(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue > >& RubyList, sal_Bool bAutomatic )
            throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    void					NotifySelChanged();
    void                    NotifyDBChanged();

    SwView* 				GetView() {return pView;}
    void                    Invalidate();

    // temporary document used for PDF export of selections/multi-selections
    SfxObjectShellRef       BuildTmpSelectionDoc( SvEmbeddedObjectRef &rRef );
};

/* -----------------17.09.98 12:52-------------------
 *
 * --------------------------------------------------*/
class SwUnoCrsr;
typedef cppu::WeakImplHelper8<
                            ::com::sun::star::text::XTextViewCursor,
                            ::com::sun::star::lang::XServiceInfo,
                            ::com::sun::star::text::XPageCursor,
                            ::com::sun::star::view::XScreenCursor,
                            ::com::sun::star::view::XViewCursor,
                            ::com::sun::star::view::XLineCursor,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::beans::XPropertyState
                            > SwXTextViewCursor_Base;

class SwXTextViewCursor : public SwXTextViewCursor_Base,
public SwClient,
public OTextCursorHelper
{
    SwView* 			pView;
    SfxItemPropertySet 	aPropSet;
protected:
    virtual ~SwXTextViewCursor();
public:
    SwXTextViewCursor(SwView* pVw);
    

    DECLARE_XINTERFACE();

    //XTextViewCursor
    virtual sal_Bool SAL_CALL isVisible(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setVisible(sal_Bool bVisible) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTextCursor - neu
    virtual void SAL_CALL collapseToStart(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL collapseToEnd(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL isCollapsed(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL goLeft( sal_Int16 nCount, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL goRight( sal_Int16 nCount, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoStart( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoEnd( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);

    //XPageCursor
    virtual sal_Bool SAL_CALL jumpToFirstPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToLastPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToPage(sal_Int16 nPage) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToNextPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToPreviousPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToEndOfPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToStartOfPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL getPage(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const ::rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XScreenCursor
    virtual sal_Bool SAL_CALL screenDown(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL screenUp(void) throw( ::com::sun::star::uno::RuntimeException );

    //XViewCursor
    virtual sal_Bool SAL_CALL goDown(sal_Int16 nCount, sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL goUp(sal_Int16 nCount, sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );
//    virtual sal_Bool goLeft(sal_Int16 nCount, sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );
//    virtual sal_Bool goRight(sal_Int16 nCount, sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );

    //XLineCursor
    virtual sal_Bool SAL_CALL isAtStartOfLine(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL isAtEndOfLine(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL gotoEndOfLine(sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL gotoStartOfLine(sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    void	Invalidate(){pView = 0;}

    // ITextCursorHelper
    virtual const SwPaM*		GetPaM() const;
    virtual SwPaM*				GetPaM();
    virtual const SwDoc* 		GetDoc() const;
    virtual SwDoc* 				GetDoc();
};
} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
