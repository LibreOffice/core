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
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextViewTextRangeSupplier.hpp>
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
#include <com/sun/star/datatransfer/XTransferableTextSupplier.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/itemprop.hxx>
#include <TextCursorHelper.hxx>
#include <comphelper/uno3.hxx>

#include <sfx2/objsh.hxx>

class SdrObject;
class SwView;

class SwXTextView final :
    public css::view::XSelectionSupplier,
    public css::lang::XServiceInfo,
    public css::view::XFormLayerAccess,
    public css::text::XTextViewCursorSupplier,
    public css::text::XTextViewTextRangeSupplier,
    public css::text::XRubySelection,
    public css::view::XViewSettingsSupplier,
    public css::beans::XPropertySet,
    public css::datatransfer::XTransferableSupplier,
    public css::datatransfer::XTransferableTextSupplier,
    public SfxBaseController
{
    ::comphelper::OInterfaceContainerHelper2 m_SelChangedListeners;

    SwView*                     m_pView;
    const SfxItemPropertySet*   m_pPropSet;   // property map for SwXTextView properties
                                        // (not related to mxViewSettings!)

    css::uno::Reference< css::beans::XPropertySet >     mxViewSettings;
    css::uno::Reference< css::text::XTextViewCursor >   mxTextViewCursor;

    SdrObject* GetControl(
        const css::uno::Reference< css::awt::XControlModel > & Model,
        css::uno::Reference< css::awt::XControl >& xToFill  );

    virtual ~SwXTextView() override;
public:
    SwXTextView(SwView* pSwView);

    virtual     css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    //XSelectionSupplier
    virtual css::uno::Any SAL_CALL getSelection() override;
    virtual sal_Bool SAL_CALL select(const css::uno::Any& rInterface) override;
    virtual void SAL_CALL addSelectionChangeListener(const css::uno::Reference< css::view::XSelectionChangeListener > & xListener) override;
    virtual void SAL_CALL removeSelectionChangeListener(const css::uno::Reference< css::view::XSelectionChangeListener > & xListener) override;

    // XFormLayerAccess
    virtual css::uno::Reference< css::form::runtime::XFormController > SAL_CALL getFormController( const css::uno::Reference< css::form::XForm >& Form ) override;
    virtual sal_Bool SAL_CALL isFormDesignMode(  ) override;
    virtual void SAL_CALL setFormDesignMode( sal_Bool DesignMode ) override;

    // XControlAccess
    virtual css::uno::Reference< css::awt::XControl >  SAL_CALL getControl(const css::uno::Reference< css::awt::XControlModel > & Model) override;

    //XTextViewCursorSupplier
    virtual css::uno::Reference< css::text::XTextViewCursor >  SAL_CALL getViewCursor() override;

    // XTextViewTextRangeSupplier
    virtual css::uno::Reference<css::text::XTextRange>
        SAL_CALL createTextRangeByPixelPosition(const css::awt::Point& rPixelPosition) override;

    //XViewSettings
    virtual css::uno::Reference< css::beans::XPropertySet >  SAL_CALL getViewSettings() override;

    //XRubySelection
    virtual css::uno::Sequence<
            css::uno::Sequence< css::beans::PropertyValue > > SAL_CALL getRubyList( sal_Bool bAutomatic ) override;

    virtual void SAL_CALL setRubyList(
        const css::uno::Sequence<
        css::uno::Sequence< css::beans::PropertyValue > >& RubyList, sal_Bool bAutomatic ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //XTransferableSupplier
    virtual css::uno::Reference< css::datatransfer::XTransferable > SAL_CALL getTransferable(  ) override;
    virtual void SAL_CALL insertTransferable( const css::uno::Reference< css::datatransfer::XTransferable >& xTrans ) override;

    // XTransferableTextSupplier
    virtual css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getTransferableForTextRange(css::uno::Reference<css::text::XTextRange> const& xTextRange) override;

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

class SwXTextViewCursor final: public SwXTextViewCursor_Base, public OTextCursorHelper
{
    SwView*                         m_pView;
    const SfxItemPropertySet*       m_pPropSet;
    bool        IsTextSelection( bool bAllowTables = true ) const;
    virtual     ~SwXTextViewCursor() override;

public:
    SwXTextViewCursor(SwView* pVw);

    DECLARE_XINTERFACE()

    //XTextViewCursor
    virtual sal_Bool SAL_CALL isVisible() override;
    virtual void SAL_CALL setVisible(sal_Bool bVisible) override;
    virtual css::awt::Point SAL_CALL getPosition() override;

    //XTextCursor - new
    virtual void SAL_CALL collapseToStart() override;
    virtual void SAL_CALL collapseToEnd() override;
    virtual sal_Bool SAL_CALL isCollapsed() override;
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand ) override;
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand ) override;
    virtual void SAL_CALL gotoStart( sal_Bool bExpand ) override;
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand ) override;
    virtual void SAL_CALL gotoRange( const css::uno::Reference< css::text::XTextRange >& xRange, sal_Bool bExpand ) override;

    //XPageCursor
    virtual sal_Bool SAL_CALL jumpToFirstPage() override;
    virtual sal_Bool SAL_CALL jumpToLastPage() override;
    virtual sal_Bool SAL_CALL jumpToPage(sal_Int16 nPage) override;
    virtual sal_Bool SAL_CALL jumpToNextPage() override;
    virtual sal_Bool SAL_CALL jumpToPreviousPage() override;
    virtual sal_Bool SAL_CALL jumpToEndOfPage() override;
    virtual sal_Bool SAL_CALL jumpToStartOfPage() override;
    virtual sal_Int16 SAL_CALL getPage() override;

    //XTextRange
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL  getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL   getEnd() override;
    virtual OUString SAL_CALL  getString() override;
    virtual void SAL_CALL  setString(const OUString& aString) override;

    //XScreenCursor
    virtual sal_Bool SAL_CALL screenDown() override;
    virtual sal_Bool SAL_CALL screenUp() override;

    //XViewCursor
    virtual sal_Bool SAL_CALL goDown(sal_Int16 nCount, sal_Bool bExpand) override;
    virtual sal_Bool SAL_CALL goUp(sal_Int16 nCount, sal_Bool bExpand) override;

    //XLineCursor
    virtual sal_Bool SAL_CALL isAtStartOfLine() override;
    virtual sal_Bool SAL_CALL isAtEndOfLine() override;
    virtual void SAL_CALL gotoEndOfLine(sal_Bool bExpand) override;
    virtual void SAL_CALL gotoStartOfLine(sal_Bool bExpand) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    void    Invalidate(){m_pView = nullptr;}

    // ITextCursorHelper
    virtual const SwPaM*        GetPaM() const override;
    virtual SwPaM*              GetPaM() override;
    virtual const SwDoc*        GetDoc() const override;
    virtual SwDoc*              GetDoc() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
