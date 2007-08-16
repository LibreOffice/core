/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optimizerdialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sj $ $Date: 2007-08-16 14:33:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OPTIMIZERDIALOG_HXX
#define OPTIMIZERDIALOG_HXX
#ifndef _OPTIMIZERDIALOG_HRC
#include "optimizerdialog.hrc"
#endif
#include <vector>
#ifndef UNODIALOG_HXX
#include "unodialog.hxx"
#endif
#ifndef _PPPOPTIMIZER_TOKEN_HXX
#include "pppoptimizertoken.hxx"
#endif
#ifndef OPTIMIZATIONSTATS_HXX
#include "optimizationstats.hxx"
#endif
#ifndef _CONFIGURATION_ACCESS_HXX_
#include "configurationaccess.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XITEMLISTENER_HPP_
#include <com/sun/star/awt/XItemListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSPINFIELD_HPP_
#include <com/sun/star/awt/XSpinField.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSPINLISTENER_HPP_
#include <com/sun/star/awt/XSpinListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XITEMEVENTBROADCASTER_HPP_
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCloseListener_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_PUSHBUTTONTYPE_HPP_
#include <com/sun/star/awt/PushButtonType.hpp>
#endif

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
class OptimizerDialog : public UnoDialog, public ConfigurationAccess
{
public :

    OptimizerDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF, com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rxFrame,
        com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > rxStatusDispatcher );
    ~OptimizerDialog();

    sal_Bool                execute();

    sal_Int16               mnCurrentStep;
    sal_Int16               mnTabIndex;
    sal_Bool                mbIsReadonly;

private :
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >mxMSF;
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

    std::vector< std::vector< rtl::OUString > > maControlPages;

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
    void InsertRoadmapItem( const sal_Int32 nIndex, const sal_Bool bEnabled, const rtl::OUString& rLabel, const sal_Int32 nItemID );

public :

    OptimizationStats maStats;

    void UpdateStatus( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rStatus );

    // the ConfigurationAccess is updated to actual control settings
    void UpdateConfiguration();

    void EnablePage( sal_Int16 nStep );
    void DisablePage( sal_Int16 nStep );

    void SwitchPage( sal_Int16 nNewStep );
    void UpdateControlStates( sal_Int16 nStep = -1 );

    rtl::OUString GetSelectedString( PPPOptimizerTokenEnum eListBox );
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >& GetStatusDispatcher() { return mxStatusDispatcher; };
    com::sun::star::uno::Reference< com::sun::star::frame::XFrame>& GetFrame() { return mxFrame; };
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& GetComponentContext() { return mxMSF; };
};

// -----------------------------------------------------------------------------

class ItemListener : public ::cppu::WeakImplHelper1< com::sun::star::awt::XItemListener >
{
public:
    ItemListener( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException);
private:

    OptimizerDialog& mrOptimizerDialog;
};

// -----------------------------------------------------------------------------

class ActionListener : public ::cppu::WeakImplHelper1< com::sun::star::awt::XActionListener >
{
public:
    ActionListener( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException);
private:

    OptimizerDialog& mrOptimizerDialog;
};

// -----------------------------------------------------------------------------

class ActionListenerListBox0Pg0 : public ::cppu::WeakImplHelper1< com::sun::star::awt::XActionListener >
{
public:
    ActionListenerListBox0Pg0( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException);
private:

    OptimizerDialog& mrOptimizerDialog;
};

// -----------------------------------------------------------------------------

class TextListenerFormattedField0Pg1 : public ::cppu::WeakImplHelper1< com::sun::star::awt::XTextListener >
{
public:
    TextListenerFormattedField0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException);
private:

    OptimizerDialog& mrOptimizerDialog;
};

// -----------------------------------------------------------------------------

class TextListenerComboBox0Pg1 : public ::cppu::WeakImplHelper1< com::sun::star::awt::XTextListener >
{
public:
    TextListenerComboBox0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException);
private:

    OptimizerDialog& mrOptimizerDialog;
};

// -----------------------------------------------------------------------------

class SpinListenerFormattedField0Pg1 : public ::cppu::WeakImplHelper1< com::sun::star::awt::XSpinListener >
{
public:
    SpinListenerFormattedField0Pg1( OptimizerDialog& rOptimizerDialog ) : mrOptimizerDialog( rOptimizerDialog ){};

    virtual void SAL_CALL up( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL down( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL first( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL last( const ::com::sun::star::awt::SpinEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException);
private:

    OptimizerDialog& mrOptimizerDialog;
};

// -----------------------------------------------------------------------------

class HelpCloseListener : public ::cppu::WeakImplHelper1< com::sun::star::util::XCloseListener >
{
public:
    HelpCloseListener( com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rXFrame ) : mrXFrame( rXFrame ){};

    virtual void SAL_CALL addCloseListener(const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL notifyClosing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL queryClosing( const com::sun::star::lang::EventObject& aEvent, sal_Bool bDeliverOwnership ) throw (com::sun::star::uno::RuntimeException, com::sun::star::util::CloseVetoException) ;
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException) ;

private:

    com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& mrXFrame;
};


#endif // OPTIMIZERDIALOG_HXX
