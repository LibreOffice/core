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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UNOTXVW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UNOTXVW_HXX
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
#include <cppuhelper/implbase.hxx>
#include <svl/itemprop.hxx>
#include "calbck.hxx"
#include "TextCursorHelper.hxx"
#include <comphelper/uno3.hxx>

#include <sfx2/objsh.hxx>

class SdrObject;
class SwView;

class SwXTextView :
    public css::view::XSelectionSupplier,
    public css::lang::XServiceInfo,
    public css::view::XFormLayerAccess,
    public css::text::XTextViewCursorSupplier,
    public css::text::XRubySelection,
    public css::view::XViewSettingsSupplier,
    public css::beans::XPropertySet,
    public css::datatransfer::XTransferableSupplier,
    public SfxBaseController
{
    ::cppu::OInterfaceContainerHelper m_SelChangedListeners;

    SwView*                     m_pView;
    const SfxItemPropertySet*   m_pPropSet;   // property map for SwXTextView properties
                                        // (not related to mxViewSettings!)

    css::uno::Reference< css::beans::XPropertySet >     mxViewSettings;
    css::uno::Reference< css::text::XTextViewCursor >   mxTextViewCursor;

    SdrObject* GetControl(
        const css::uno::Reference< css::awt::XControlModel > & Model,
        css::uno::Reference< css::awt::XControl >& xToFill  );

protected:
    virtual ~SwXTextView();
public:
    SwXTextView(SwView* pSwView);

    virtual     css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XSelectionSupplier
    virtual css::uno::Any SAL_CALL getSelection()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL select(const css::uno::Any& rInterface)
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener(const css::uno::Reference< css::view::XSelectionChangeListener > & xListener) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeSelectionChangeListener(const css::uno::Reference< css::view::XSelectionChangeListener > & xListener) throw( css::uno::RuntimeException, std::exception ) override;

    // XFormLayerAccess
    virtual css::uno::Reference< css::form::runtime::XFormController > SAL_CALL getFormController( const css::uno::Reference< css::form::XForm >& Form ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isFormDesignMode(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFormDesignMode( sal_Bool DesignMode ) throw (css::uno::RuntimeException, std::exception) override;

    // XControlAccess
    virtual css::uno::Reference< css::awt::XControl >  SAL_CALL getControl(const css::uno::Reference< css::awt::XControlModel > & Model) throw( css::container::NoSuchElementException, css::uno::RuntimeException, std::exception ) override;

    //XTextViewCursorSupplier
    virtual css::uno::Reference< css::text::XTextViewCursor >  SAL_CALL getViewCursor() throw( css::uno::RuntimeException, std::exception ) override;

    //XViewSettings
    virtual css::uno::Reference< css::beans::XPropertySet >  SAL_CALL getViewSettings() throw( css::uno::RuntimeException, std::exception ) override;

    //XRubySelection
    virtual css::uno::Sequence<
            css::uno::Sequence<
            css::beans::PropertyValue > > SAL_CALL getRubyList( sal_Bool bAutomatic )
                throw (css::uno::RuntimeException,
                       std::exception) override;

    virtual void SAL_CALL setRubyList(
        const css::uno::Sequence<
        css::uno::Sequence<
        css::beans::PropertyValue > >& RubyList, sal_Bool bAutomatic )
            throw (css::uno::RuntimeException,
                   std::exception) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    //XTransferableSupplier
    virtual css::uno::Reference< css::datatransfer::XTransferable > SAL_CALL getTransferable(  )
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL insertTransferable( const css::uno::Reference< css::datatransfer::XTransferable >& xTrans ) throw (css::datatransfer::UnsupportedFlavorException, css::uno::RuntimeException, std::exception) override;

    void                    NotifySelChanged();
    void                    NotifyDBChanged();

    SwView*                 GetView() {return m_pView;}
    void                    Invalidate();

    // temporary document used for PDF export of selections/multi-selections
    SfxObjectShellLock      BuildTmpSelectionDoc();
};

typedef cppu::WeakImplHelper<
                            css::text::XTextViewCursor,
                            css::lang::XServiceInfo,
                            css::text::XPageCursor,
                            css::view::XScreenCursor,
                            css::view::XViewCursor,
                            css::view::XLineCursor,
                            css::beans::XPropertySet,
                            css::beans::XPropertyState
                            > SwXTextViewCursor_Base;

class SwXTextViewCursor : public SwXTextViewCursor_Base,
public SwClient,
public OTextCursorHelper
{
    SwView*                         m_pView;
    const SfxItemPropertySet*       m_pPropSet;
protected:
    bool        IsTextSelection( bool bAllowTables = true ) const;
    virtual     ~SwXTextViewCursor();
public:
    SwXTextViewCursor(SwView* pVw);

    DECLARE_XINTERFACE()

    //XTextViewCursor
    virtual sal_Bool SAL_CALL isVisible() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setVisible(sal_Bool bVisible) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::awt::Point SAL_CALL getPosition() throw( css::uno::RuntimeException, std::exception ) override;

    //XTextCursor - neu
    virtual void SAL_CALL collapseToStart()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL collapseToEnd()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL isCollapsed()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand )
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand )
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL gotoStart( sal_Bool bExpand )
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand )
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL gotoRange( const css::uno::Reference< css::text::XTextRange >& xRange, sal_Bool bExpand )
        throw (css::uno::RuntimeException,
               std::exception) override;

    //XPageCursor
    virtual sal_Bool SAL_CALL jumpToFirstPage()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL jumpToLastPage()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL jumpToPage(sal_Int16 nPage) throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL jumpToNextPage() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL jumpToPreviousPage() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL jumpToEndOfPage() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL jumpToStartOfPage() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int16 SAL_CALL getPage()
        throw (css::uno::RuntimeException,
               std::exception) override;

    //XTextRange
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL  getStart()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL   getEnd()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual OUString SAL_CALL  getString()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL  setString(const OUString& aString)
        throw (css::uno::RuntimeException,
               std::exception) override;

    //XScreenCursor
    virtual sal_Bool SAL_CALL screenDown()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL screenUp()
        throw (css::uno::RuntimeException,
               std::exception) override;

    //XViewCursor
    virtual sal_Bool SAL_CALL goDown(sal_Int16 nCount, sal_Bool bExpand)
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL goUp(sal_Int16 nCount, sal_Bool bExpand)
        throw (css::uno::RuntimeException,
               std::exception) override;

    //XLineCursor
    virtual sal_Bool SAL_CALL isAtStartOfLine()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL isAtEndOfLine()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL gotoEndOfLine(sal_Bool bExpand)
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL gotoStartOfLine(sal_Bool bExpand)
        throw (css::uno::RuntimeException,
               std::exception) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
        throw (css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName )
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName )
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName )
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException,
               std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    void    Invalidate(){m_pView = 0;}

    // ITextCursorHelper
    virtual const SwPaM*        GetPaM() const override;
    virtual SwPaM*              GetPaM() override;
    virtual const SwDoc*        GetDoc() const override;
    virtual SwDoc*              GetDoc() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
