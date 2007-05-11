/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unodialog.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 14:01:25 $
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

#ifndef UNODIALOG_HXX
#define UNODIALOG_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTI_COMPONENT_FACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XINVOCATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XACTIONLISTENER_HPP_
#include <com/sun/star/awt/XActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLISTENER_HPP_
#include <com/sun/star/awt/XTextListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBUTTON_HPP_
#include <com/sun/star/awt/XButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCHECKBOX_HPP_
#include <com/sun/star/awt/XCheckBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCOMBOBOX_HPP_
#include <com/sun/star/awt/XComboBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XRADIOBUTTON_HPP_
#include <com/sun/star/awt/XRadioButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLISTBOX_HPP_
#include <com/sun/star/awt/XListBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFIXEDTEXT_HPP_
#include <com/sun/star/awt/XFixedText.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XRESCHEDULE_HPP_
#include <com/sun/star/awt/XReschedule.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

// -------------
// - UnoDialog -
// -------------

////////////////////////////////////////////////////////////////////////

class UnoDialog
{
public :

    UnoDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF, com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rxFrame );
    ~UnoDialog();

    void execute();
    void endExecute( sal_Bool bStatus );

    com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer > createWindowPeer( com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer > xParentPeer )
        throw ( com::sun::star::uno::Exception );

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > insertControlModel( const rtl::OUString& rServiceName, const rtl::OUString& rName,
        const com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& rPropertyValues );

    void setVisible( const rtl::OUString& rName, sal_Bool bVisible );

    sal_Bool isHighContrast();

    com::sun::star::uno::Reference< com::sun::star::awt::XButton > insertButton( const rtl::OUString& rName,
        com::sun::star::uno::Reference< com::sun::star::awt::XActionListener > xActionListener, const com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames,
            const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XFixedText > insertFixedText( const rtl::OUString& rName,
        const com::sun::star::uno::Sequence< rtl::OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XCheckBox > insertCheckBox( const rtl::OUString& rName,
        const com::sun::star::uno::Sequence< rtl::OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XControl > insertFormattedField( const rtl::OUString& rName,
        const com::sun::star::uno::Sequence< rtl::OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XComboBox > insertComboBox( const rtl::OUString& rName,
        const com::sun::star::uno::Sequence< rtl::OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XRadioButton > insertRadioButton( const rtl::OUString& rName,
        const com::sun::star::uno::Sequence< rtl::OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XListBox > insertListBox( const rtl::OUString& rName,
        const com::sun::star::uno::Sequence< rtl::OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XControl > insertImage( const rtl::OUString& rName,
        const com::sun::star::uno::Sequence< rtl::OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    void setControlProperty( const rtl::OUString& rControlName, const rtl::OUString& rPropertyName, const com::sun::star::uno::Any& rPropertyValue );
    com::sun::star::uno::Any getControlProperty( const rtl::OUString& rControlName, const rtl::OUString& rPropertyName );

    void showMessageBox( const rtl::OUString& rTitle, const rtl::OUString& rMessage, sal_Bool bErrorBox ) const;

    void enableControl( const rtl::OUString& rControlName );
    void disableControl( const rtl::OUString& rControlName );

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >        mxMSF;
    com::sun::star::uno::Reference< com::sun::star::frame::XController >            mxController;
    com::sun::star::uno::Reference< com::sun::star::awt::XReschedule >              mxReschedule;

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >               mxDialogModel;
    com::sun::star::uno::Reference< com::sun::star::beans::XMultiPropertySet >      mxDialogModelMultiPropertySet;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >           mxDialogModelPropertySet;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >    mxDialogModelMSF;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >     mxDialogModelNameContainer;
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >        mxDialogModelNameAccess;

    com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >            mxControlModel;

    com::sun::star::uno::Reference< com::sun::star::awt::XDialog >                  mxDialog;
    com::sun::star::uno::Reference< com::sun::star::awt::XControl >                 mxControl;
    com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer >              mxWindowPeer;

    com::sun::star::uno::Reference< com::sun::star::awt::XControlContainer >        mxDialogControlContainer;
    com::sun::star::uno::Reference< com::sun::star::lang::XComponent >              mxDialogComponent;
    com::sun::star::uno::Reference< com::sun::star::awt::XWindow >                  mxDialogWindow;

    sal_Bool                                                                        mbStatus;
};

#endif // UNODIALOG_HXX
