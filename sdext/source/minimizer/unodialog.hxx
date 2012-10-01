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
