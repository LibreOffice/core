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


class UnoDialog
{
public:

    UnoDialog( const css::uno::Reference< css::uno::XComponentContext >& rxMSF, css::uno::Reference< css::frame::XFrame > const & rxFrame );
    ~UnoDialog();

    void execute();
    void endExecute( bool bStatus );

    css::uno::Reference< css::uno::XInterface > insertControlModel( const OUString& rServiceName, const OUString& rName,
        const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    void setVisible( const OUString& rName, bool bVisible );

    css::uno::Reference< css::awt::XButton > insertButton( const OUString& rName,
        const css::uno::Reference< css::awt::XActionListener >& xActionListener, const css::uno::Sequence< OUString >& rPropertyNames,
            const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    css::uno::Reference< css::awt::XFixedText > insertFixedText( const OUString& rName,
        const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    css::uno::Reference< css::awt::XCheckBox > insertCheckBox( const OUString& rName,
        const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    css::uno::Reference< css::awt::XControl > insertFormattedField( const OUString& rName,
        const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    css::uno::Reference< css::awt::XComboBox > insertComboBox( const OUString& rName,
        const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    css::uno::Reference< css::awt::XRadioButton > insertRadioButton( const OUString& rName,
        const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    css::uno::Reference< css::awt::XListBox > insertListBox( const OUString& rName,
        const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    css::uno::Reference< css::awt::XControl > insertImage( const OUString& rName,
        const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& rPropertyValues );

    void setControlProperty( const OUString& rControlName, const OUString& rPropertyName, const css::uno::Any& rPropertyValue );
    css::uno::Any getControlProperty( const OUString& rControlName, const OUString& rPropertyName );

    void enableControl( const OUString& rControlName );
    void disableControl( const OUString& rControlName );

    void reschedule() const { mxReschedule->reschedule(); }
    bool endStatus() const { return mbStatus; }
    css::uno::Reference<css::awt::XControl> getControl(const OUString& rControlName) const { return mxDialog->getControl(rControlName); }
    const css::uno::Reference<css::frame::XController>& controller() const { return mxController; }
    void setPropertyValues(const css::uno::Sequence<OUString>& rNameSeq, const css::uno::Sequence<css::uno::Any>& rValueSeq)
        { mxDialogModelMultiPropertySet->setPropertyValues(rNameSeq, rValueSeq); }

protected:
    css::uno::Reference< css::uno::XComponentContext >        mxContext;
    css::uno::Reference< css::frame::XController >            mxController;

private:
    css::uno::Reference< css::awt::XReschedule >              mxReschedule;
    css::uno::Reference< css::uno::XInterface >               mxDialogModel;
    css::uno::Reference< css::beans::XMultiPropertySet >      mxDialogModelMultiPropertySet;
    css::uno::Reference< css::lang::XMultiServiceFactory >    mxDialogModelMSF;
    css::uno::Reference< css::container::XNameContainer >     mxDialogModelNameContainer;
    css::uno::Reference< css::container::XNameAccess >        mxDialogModelNameAccess;

    css::uno::Reference< css::awt::XControlModel >            mxControlModel;

    css::uno::Reference< css::awt::XUnoControlDialog >        mxDialog;
    css::uno::Reference< css::awt::XControl >                 mxControl;
    bool                                                                        mbStatus;
};

#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_UNODIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
