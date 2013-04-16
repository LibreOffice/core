/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OPTIMIZERDIALOG_HXX
#define OPTIMIZERDIALOG_HXX
#ifndef _OPTIMIZERDIALOG_HRC
#include "optimizerdialog.hrc"
#endif
#include <vector>
#include "unodialog.hxx"
#include "pppoptimizertoken.hxx"
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
#ifndef _COM_SUN_STAR_UTIL_XCloseListener_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase1.hxx>

#define MAX_STEP        4
#define OD_DIALOG_WIDTH 330
#define DIALOG_HEIGHT   210
#define BUTTON_WIDTH    50
#define BUTTON_HEIGHT   14
#define BUTTON_POS_Y    DIALOG_HEIGHT - BUTTON_HEIGHT - 6

#define PAGE_POS_X      91
#define PAGE_POS_Y      8
#define PAGE_WIDTH      OD_DIALOG_WIDTH - PAGE_POS_X

// -------------------
// - OPTIMIZERDIALOG -
// -------------------

typedef ::cppu::WeakImplHelper5<
    com::sun::star::frame::XStatusListener,
    com::sun::star::awt::XSpinListener,
    com::sun::star::awt::XItemListener,
    com::sun::star::awt::XActionListener,
    com::sun::star::awt::XTextListener > OptimizerDialog_Base;

class OptimizerDialog : public UnoDialog, public ConfigurationAccess, public OptimizerDialog_Base
{
public :

    OptimizerDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                     const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& xFrame,
                     const com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer >& rxParent );
    ~OptimizerDialog();

    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& aState ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aSource ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL up( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL down( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL first( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL last( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& Event ) throw ( com::sun::star::uno::RuntimeException );

    sal_Bool                execute();

private :
    com::sun::star::uno::Reference< com::sun::star::frame::XFrame >         mxFrame;
    sal_Int16               mnCurrentStep;
    sal_Int16               mnTabIndex;
    sal_Bool                mbIsReadonly;
    com::sun::star::uno::Reference< com::sun::star::frame::XModel >         mxModel;
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >       mxRoadmapControl;
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >       mxRoadmapControlModel;

    std::vector< std::vector< rtl::OUString > > maControlPages;
    OptimizationStats maStats;

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
    void EnablePage( sal_Int16 nStep );
    void DisablePage( sal_Int16 nStep );
    void SwitchPage( sal_Int16 nNewStep );

    // the ConfigurationAccess is updated to actual control settings
    void UpdateConfiguration();
    void UpdateStatus( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rStatus );
    void UpdateControlStates( sal_Int16 nStep = -1 );

    rtl::OUString GetSelectedString( PPPOptimizerTokenEnum eListBox );
    void ImplSetBold( const rtl::OUString& rControl );
    void InsertRoadmapItem( const sal_Int32 nIndex,
                            const sal_Bool bEnabled,
                            const rtl::OUString& rLabel,
                            const sal_Int32 nItemID );
    rtl::OUString ImplInsertSeparator( const rtl::OUString& rControlName,
                                       sal_Int32 nOrientation,
                                       sal_Int32 nPosX,
                                       sal_Int32 nPosY,
                                       sal_Int32 nWidth,
                                       sal_Int32 nHeight );
    rtl::OUString ImplInsertButton( const rtl::OUString& rControlName,
                                    const rtl::OUString& rHelpURL,
                                    sal_Int32 nXPos,
                                    sal_Int32 nYPos,
                                    sal_Int32 nWidth,
                                    sal_Int32 nHeight,
                                    sal_Int16 nTabIndex,
                                    sal_Bool bEnabled,
                                    sal_Int32 nResID,
                                    sal_Int16 nPushButtonType );
    rtl::OUString ImplInsertFixedText( const rtl::OUString& rControlName,
                                       const rtl::OUString& rLabel,
                                       sal_Int32 nXPos,
                                       sal_Int32 nYPos,
                                       sal_Int32 nWidth,
                                       sal_Int32 nHeight,
                                       sal_Bool bMultiLine,
                                       sal_Bool bBold,
                                       sal_Int16 nTabIndex );
    rtl::OUString ImplInsertCheckBox( const rtl::OUString& rControlName,
                                      const rtl::OUString& rLabel,
                                      const rtl::OUString& rHelpURL,
                                      sal_Int32 nXPos,
                                      sal_Int32 nYPos,
                                      sal_Int32 nWidth,
                                      sal_Int32 nHeight,
                                      sal_Int16 nTabIndex );
    rtl::OUString ImplInsertFormattedField( const rtl::OUString& rControlName,
                                            const rtl::OUString& rHelpURL,
                                            sal_Int32 nXPos,
                                            sal_Int32 nYPos,
                                            sal_Int32 nWidth,
                                            double fEffectiveMin,
                                            double fEffectiveMax,
                                            sal_Int16 nTabIndex );
    rtl::OUString ImplInsertComboBox( const rtl::OUString& rControlName,
                                      const rtl::OUString& rHelpURL,
                                      const sal_Bool bEnabled,
                                      const com::sun::star::uno::Sequence< rtl::OUString >& rItemList,
                                      sal_Int32 nXPos,
                                      sal_Int32 nYPos,
                                      sal_Int32 nWidth,
                                      sal_Int32 nHeight,
                                      sal_Int16 nTabIndex,
                                      bool bListen = true );
    rtl::OUString ImplInsertRadioButton( const rtl::OUString& rControlName,
                                         const rtl::OUString& rLabel,
                                         const rtl::OUString& rHelpURL,
                                         sal_Int32 nXPos,
                                         sal_Int32 nYPos,
                                         sal_Int32 nWidth,
                                         sal_Int32 nHeight,
                                         sal_Bool bMultiLine,
                                         sal_Int16 nTabIndex );
    rtl::OUString ImplInsertListBox( const rtl::OUString& rControlName,
                                     const rtl::OUString& rHelpURL,
                                     const sal_Bool bEnabled,
                                     const com::sun::star::uno::Sequence< rtl::OUString >& rItemList,
                                     sal_Int32 nXPos,
                                     sal_Int32 nYPos,
                                     sal_Int32 nWidth,
                                     sal_Int32 nHeight,
                                     sal_Int16 nTabIndex );
};

#endif // OPTIMIZERDIALOG_HXX
