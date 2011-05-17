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

#ifndef UNODIALOG_HXX
#define UNODIALOG_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XTextListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XReschedule.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>

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

    sal_Int32 getMapsFromPixels( sal_Int32 nPixels ) const;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
