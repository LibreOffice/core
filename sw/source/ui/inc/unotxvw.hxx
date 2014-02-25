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
#ifndef INCLUDED_SW_SOURCE_UI_INC_UNOTXVW_HXX
#define INCLUDED_SW_SOURCE_UI_INC_UNOTXVW_HXX
#include <sfx2/sfxbasecontroller.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XRubySelection.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/view/XScreenCursor.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XLineCursor.hpp>
#include <com/sun/star/view/XViewCursor.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XTransferableSupplier.hpp>
#include <cppuhelper/implbase8.hxx>
#include <svl/itemprop.hxx>
#include "calbck.hxx"
#include "TextCursorHelper.hxx"
#include <comphelper/uno3.hxx>

#include <sfx2/objsh.hxx>

class SdrObject;
class SwView;

class SwXTextView :
    public ::com::sun::star::view::XSelectionSupplier,
    public ::com::sun::star::lang::XServiceInfo,
    public ::com::sun::star::view::XFormLayerAccess,
    public ::com::sun::star::text::XTextViewCursorSupplier,
    public ::com::sun::star::text::XRubySelection,
    public ::com::sun::star::view::XViewSettingsSupplier,
    public ::com::sun::star::beans::XPropertySet,
    public ::com::sun::star::datatransfer::XTransferableSupplier,
    public SfxBaseController
{
    ::cppu::OInterfaceContainerHelper m_SelChangedListeners;

    SwView*                     m_pView;
    const SfxItemPropertySet*   m_pPropSet;   // property map for SwXTextView properties
                                        // (not related to pxViewSettings!)

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *         pxViewSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextViewCursor > *   pxTextViewCursor;


    SdrObject* GetControl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & Model,
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& xToFill  );

protected:
    virtual ~SwXTextView();
public:
    SwXTextView(SwView* pSwView);


    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XSelectionSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL select(const ::com::sun::star::uno::Any& rInterface)
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL addSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > & xListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > & xListener) throw( ::com::sun::star::uno::RuntimeException );

    // XFormLayerAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController > SAL_CALL getFormController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& Form ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isFormDesignMode(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFormDesignMode( ::sal_Bool DesignMode ) throw (::com::sun::star::uno::RuntimeException);

    // XControlAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  SAL_CALL getControl(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & Model) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException );

    //XTextViewCursorSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextViewCursor >  SAL_CALL getViewCursor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XViewSettings
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getViewSettings(void) throw( ::com::sun::star::uno::RuntimeException );

    //XRubySelection
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue > > SAL_CALL getRubyList( sal_Bool bAutomatic )
                throw (::com::sun::star::uno::RuntimeException,
                       std::exception);

    virtual void SAL_CALL setRubyList(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue > >& RubyList, sal_Bool bAutomatic )
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTransferableSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getTransferable(  )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL insertTransferable( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans ) throw (::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::uno::RuntimeException);

    void                    NotifySelChanged();
    void                    NotifyDBChanged();

    SwView*                 GetView() {return m_pView;}
    void                    Invalidate();

    // temporary document used for PDF export of selections/multi-selections
    SfxObjectShellLock      BuildTmpSelectionDoc();
};

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
    SwView*                         m_pView;
    const SfxItemPropertySet*       m_pPropSet;
protected:
    sal_Bool    IsTextSelection( sal_Bool bAllowTables = sal_True ) const;
    virtual     ~SwXTextViewCursor();
public:
    SwXTextViewCursor(SwView* pVw);

    DECLARE_XINTERFACE()

    //XTextViewCursor
    virtual sal_Bool SAL_CALL isVisible(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setVisible(sal_Bool bVisible) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTextCursor - neu
    virtual void SAL_CALL collapseToStart()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL collapseToEnd()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL isCollapsed()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL gotoStart( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL gotoRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);

    //XPageCursor
    virtual sal_Bool SAL_CALL jumpToFirstPage()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL jumpToLastPage()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL jumpToPage(sal_Int16 nPage) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToNextPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToPreviousPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToEndOfPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToStartOfPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL getPage()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XScreenCursor
    virtual sal_Bool SAL_CALL screenDown()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL screenUp(void) throw( ::com::sun::star::uno::RuntimeException );

    //XViewCursor
    virtual sal_Bool SAL_CALL goDown(sal_Int16 nCount, sal_Bool bExpand)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL goUp(sal_Int16 nCount, sal_Bool bExpand)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);

    //XLineCursor
    virtual sal_Bool SAL_CALL isAtStartOfLine()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual sal_Bool SAL_CALL isAtEndOfLine()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL gotoEndOfLine(sal_Bool bExpand)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL gotoStartOfLine(sal_Bool bExpand)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    void    Invalidate(){m_pView = 0;}

    // ITextCursorHelper
    virtual const SwPaM*        GetPaM() const;
    virtual SwPaM*              GetPaM();
    virtual const SwDoc*        GetDoc() const;
    virtual SwDoc*              GetDoc();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
