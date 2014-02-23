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

#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_UNODIALOG_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_UNODIALOG_HXX

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
#include <com/sun/star/awt/XUnoControlDialog.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XReschedule.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>


// - UnoDialog -




class UnoDialog
{
public :

    UnoDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF, com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rxFrame );
    ~UnoDialog();

    void execute();
    void endExecute( sal_Bool bStatus );

    com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer > createWindowPeer( com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer > xParentPeer )
        throw ( com::sun::star::uno::Exception );

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > insertControlModel( const OUString& rServiceName, const OUString& rName,
        const com::sun::star::uno::Sequence< OUString >& rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& rPropertyValues );

    void setVisible( const OUString& rName, sal_Bool bVisible );

    com::sun::star::uno::Reference< com::sun::star::awt::XButton > insertButton( const OUString& rName,
        com::sun::star::uno::Reference< com::sun::star::awt::XActionListener > xActionListener, const com::sun::star::uno::Sequence< OUString >& rPropertyNames,
            const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XFixedText > insertFixedText( const OUString& rName,
        const com::sun::star::uno::Sequence< OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XCheckBox > insertCheckBox( const OUString& rName,
        const com::sun::star::uno::Sequence< OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XControl > insertFormattedField( const OUString& rName,
        const com::sun::star::uno::Sequence< OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XComboBox > insertComboBox( const OUString& rName,
        const com::sun::star::uno::Sequence< OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XRadioButton > insertRadioButton( const OUString& rName,
        const com::sun::star::uno::Sequence< OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XListBox > insertListBox( const OUString& rName,
        const com::sun::star::uno::Sequence< OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    com::sun::star::uno::Reference< com::sun::star::awt::XControl > insertImage( const OUString& rName,
        const com::sun::star::uno::Sequence< OUString > rPropertyNames, const com::sun::star::uno::Sequence< com::sun::star::uno::Any > rPropertyValues );

    void setControlProperty( const OUString& rControlName, const OUString& rPropertyName, const com::sun::star::uno::Any& rPropertyValue );
    com::sun::star::uno::Any getControlProperty( const OUString& rControlName, const OUString& rPropertyName );

    void enableControl( const OUString& rControlName );
    void disableControl( const OUString& rControlName );

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >        mxContext;
    com::sun::star::uno::Reference< com::sun::star::frame::XController >            mxController;
    com::sun::star::uno::Reference< com::sun::star::awt::XReschedule >              mxReschedule;

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface >               mxDialogModel;
    com::sun::star::uno::Reference< com::sun::star::beans::XMultiPropertySet >      mxDialogModelMultiPropertySet;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >           mxDialogModelPropertySet;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >    mxDialogModelMSF;
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >     mxDialogModelNameContainer;
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >        mxDialogModelNameAccess;

    com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >            mxControlModel;

    com::sun::star::uno::Reference< com::sun::star::awt::XUnoControlDialog >        mxDialog;
    com::sun::star::uno::Reference< com::sun::star::awt::XControl >                 mxControl;
    com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer >              mxWindowPeer;

    sal_Bool                                                                        mbStatus;
};

#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_UNODIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
