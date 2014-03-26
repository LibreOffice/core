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
public :

    OptimizerDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext, com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rxFrame,
        com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > rxStatusDispatcher );
    ~OptimizerDialog();

    sal_Bool                execute();

    sal_Int16               mnCurrentStep;
    sal_Int16               mnTabIndex;
    sal_Bool                mbIsReadonly;

private :
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxContext;
    com::sun::star::uno::Reference< com::sun::star::frame::XFrame >         mxFrame;

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >       mxRoadmapControl;
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >       mxRoadmapControlModel;

    com::sun::star::uno::Reference< com::sun::star::awt::XItemListener >    mxItemListener;
    com::sun::star::uno::Reference< com::sun::star::awt::XActionListener >  mxActionListener;
    com::sun::star::uno::Reference< com::sun::star::awt::XActionListener >  mxActionListenerListBox0Pg0;
    com::sun::star::uno::Reference< com::sun::star::awt::XTextListener >    mxTextListenerFormattedField0Pg1;
    com::sun::star::uno::Reference< com::sun::star::awt::XTextListener >    mxTextListenerComboBox0Pg1;
    com::sun::star::uno::Reference< com::sun::star::awt::XSpinListener >    mxSpinListenerFormattedField0Pg1;
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >      mxStatusDispatcher;

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
    void InsertRoadmapItem( const sal_Int32 nIndex, const sal_Bool bEnabled, const OUString& rLabel, const sal_Int32 nItemID );

public :

    OptimizationStats maStats;

    void UpdateStatus( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rStatus );

    // the ConfigurationAccess is updated to actual control settings
    void UpdateConfiguration();

    void EnablePage( sal_Int16 nStep );
    void DisablePage( sal_Int16 nStep );

    void SwitchPage( sal_Int16 nNewStep );
    void UpdateControlStates( sal_Int16 nStep = -1 );

    OUString GetSelectedString( OUString const & token );
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >& GetStatusDispatcher() { return mxStatusDispatcher; };
    com::sun::star::uno::Reference< com::sun::star::frame::XFrame>& GetFrame() { return mxFrame; };
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& GetComponentContext() { return mxContext; };
};



class ItemListener : public ::cppu::WeakImplHelper1< com::sun::star::awt::XItemListener >
{
public:
    ItemListener( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class ActionListener : public ::cppu::WeakImplHelper1< com::sun::star::awt::XActionListener >
{
public:
    ActionListener( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class ActionListenerListBox0Pg0 : public ::cppu::WeakImplHelper1< com::sun::star::awt::XActionListener >
{
public:
    ActionListenerListBox0Pg0( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class TextListenerFormattedField0Pg1 : public ::cppu::WeakImplHelper1< com::sun::star::awt::XTextListener >
{
public:
    TextListenerFormattedField0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class TextListenerComboBox0Pg1 : public ::cppu::WeakImplHelper1< com::sun::star::awt::XTextListener >
{
public:
    TextListenerComboBox0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:

    OptimizerDialog& mrOptimizerDialog;
};



class SpinListenerFormattedField0Pg1 : public ::cppu::WeakImplHelper1< com::sun::star::awt::XSpinListener >
{
public:
    SpinListenerFormattedField0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL up( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL down( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL first( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL last( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:

    OptimizerDialog& mrOptimizerDialog;
};

#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_OPTIMIZERDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
