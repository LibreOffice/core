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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHIC_GRAPHICEXPORTDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHIC_GRAPHICEXPORTDIALOG_HXX

#include <tools/fldunit.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/document/XExporter.hpp>

using namespace css;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;

class GraphicExportDialog : public cppu::WeakImplHelper
<
    document::XExporter,
    ui::dialogs::XExecutableDialog,
    beans::XPropertyAccess,
    lang::XInitialization
>
{
    Sequence<PropertyValue>         maMediaDescriptor;
    Sequence<PropertyValue>         maFilterDataSequence;
    Reference<XComponent>           mxSourceDocument;
    Reference<XComponentContext>    mxContext;

    OUString   maDialogTitle;
    FieldUnit  meFieldUnit;
    bool       mbExportSelection;

public:

    explicit GraphicExportDialog( const Reference<XComponentContext>& rxContext );
    virtual ~GraphicExportDialog();

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence<Any>& aArguments ) throw (Exception, RuntimeException, std::exception ) override;

    // XPropertyAccess
    virtual Sequence<PropertyValue> SAL_CALL getPropertyValues() throw ( RuntimeException, std::exception ) override;
    virtual void SAL_CALL setPropertyValues( const Sequence<PropertyValue>& aProps )
        throw ( UnknownPropertyException, PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException,
                RuntimeException, std::exception ) override;

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute() throw ( RuntimeException, std::exception ) override;
    virtual void SAL_CALL setTitle( const OUString& aTitle ) throw ( RuntimeException, std::exception ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference<lang::XComponent>& xDocument ) throw ( lang::IllegalArgumentException, RuntimeException, std::exception ) override;
};


#endif // _GRAPHICEXPORTDIALOGUNO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
