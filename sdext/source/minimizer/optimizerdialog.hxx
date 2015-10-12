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

#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_OPTIMIZERDIALOG_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_OPTIMIZERDIALOG_HXX
#include "optimizerdialog.hrc"
#include <vector>
#include "unodialog.hxx"
#include "optimizationstats.hxx"
#include "configurationaccess.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XSpinField.hpp>
#include <com/sun/star/awt/XSpinListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <cppuhelper/implbase.hxx>

#define MAX_STEP        4
#define OD_DIALOG_WIDTH 330
#define DIALOG_HEIGHT   210
#define BUTTON_WIDTH    50
#define BUTTON_HEIGHT   14
#define BUTTON_POS_Y    DIALOG_HEIGHT - BUTTON_HEIGHT - 6

#define PAGE_POS_X      91
#define PAGE_POS_Y      8
#define PAGE_WIDTH      OD_DIALOG_WIDTH - PAGE_POS_X


// - OPTIMIZERDIALOG -

class OptimizerDialog : public UnoDialog, public ConfigurationAccess
{
public:

    OptimizerDialog( const css::uno::Reference< css::uno::XComponentContext >& rxContext, css::uno::Reference< css::frame::XFrame >& rxFrame,
        css::uno::Reference< css::frame::XDispatch > rxStatusDispatcher );
    ~OptimizerDialog();

    bool                execute();

    sal_Int16               mnCurrentStep;
    sal_Int16               mnTabIndex;
    bool                mbIsReadonly;

private:
    css::uno::Reference< css::frame::XFrame >         mxFrame;

    css::uno::Reference< css::uno::XInterface >       mxRoadmapControl;
    css::uno::Reference< css::uno::XInterface >       mxRoadmapControlModel;

    css::uno::Reference< css::awt::XItemListener >    mxItemListener;
    css::uno::Reference< css::awt::XActionListener >  mxActionListener;
    css::uno::Reference< css::awt::XActionListener >  mxActionListenerListBox0Pg0;
    css::uno::Reference< css::awt::XTextListener >    mxTextListenerFormattedField0Pg1;
    css::uno::Reference< css::awt::XTextListener >    mxTextListenerComboBox0Pg1;
    css::uno::Reference< css::awt::XSpinListener >    mxSpinListenerFormattedField0Pg1;
    css::uno::Reference< css::frame::XDispatch >      mxStatusDispatcher;

    std::vector< std::vector< OUString > > maControlPages;

    void InitDialog();
    void InitRoadmap();
    void InitNavigationBar();
    void InitPage0();
    void InitPage1();
    void InitPage2();
    void InitPage3();
    void InitPage4();
    void UpdateControlStatesPage0();
    void UpdateControlStatesPage1();
    void UpdateControlStatesPage2();
    void UpdateControlStatesPage3();
    void UpdateControlStatesPage4();

    void ActivatePage( sal_Int16 nStep );
    void DeactivatePage( sal_Int16 nStep );
    void InsertRoadmapItem( const sal_Int32 nIndex, const bool bEnabled, const OUString& rLabel, const sal_Int32 nItemID );

public:

    OptimizationStats maStats;

    void UpdateStatus( const css::uno::Sequence< css::beans::PropertyValue >& rStatus );

    // the ConfigurationAccess is updated to actual control settings
    void UpdateConfiguration();

    void EnablePage( sal_Int16 nStep );
    void DisablePage( sal_Int16 nStep );

    void SwitchPage( sal_Int16 nNewStep );
    void UpdateControlStates( sal_Int16 nStep = -1 );

    OUString GetSelectedString( OUString const & token );
    css::uno::Reference< css::frame::XDispatch >& GetStatusDispatcher() { return mxStatusDispatcher; };
    css::uno::Reference< css::frame::XFrame>& GetFrame() { return mxFrame; };
    const css::uno::Reference< css::uno::XComponentContext >& GetComponentContext() { return UnoDialog::mxContext; };
};



class ItemListener : public ::cppu::WeakImplHelper< css::awt::XItemListener >
{
public:
    explicit ItemListener( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){}

    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception) override;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class ActionListener : public ::cppu::WeakImplHelper< css::awt::XActionListener >
{
public:
    explicit ActionListener( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){}

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception) override;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class ActionListenerListBox0Pg0 : public ::cppu::WeakImplHelper< css::awt::XActionListener >
{
public:
    explicit ActionListenerListBox0Pg0( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){}

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception) override;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class TextListenerFormattedField0Pg1 : public ::cppu::WeakImplHelper< css::awt::XTextListener >
{
public:
    explicit TextListenerFormattedField0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){}

    virtual void SAL_CALL textChanged( const css::awt::TextEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception) override;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class TextListenerComboBox0Pg1 : public ::cppu::WeakImplHelper< css::awt::XTextListener >
{
public:
    explicit TextListenerComboBox0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){}

    virtual void SAL_CALL textChanged( const css::awt::TextEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception) override;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class SpinListenerFormattedField0Pg1 : public ::cppu::WeakImplHelper< css::awt::XSpinListener >
{
public:
    explicit SpinListenerFormattedField0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){}

    virtual void SAL_CALL up( const css::awt::SpinEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL down( const css::awt::SpinEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL first( const css::awt::SpinEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL last( const css::awt::SpinEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception) override;
private:

    OptimizerDialog& mrOptimizerDialog;
};

#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_OPTIMIZERDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
