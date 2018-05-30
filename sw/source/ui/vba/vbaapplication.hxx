/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAAPPLICATION_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAAPPLICATION_HXX

#include <vector>

#include <ooo/vba/XSink.hpp>
#include <ooo/vba/word/XApplication.hpp>
#include <ooo/vba/word/XDocument.hpp>
#include <ooo/vba/word/XWindow.hpp>
#include <ooo/vba/word/XSystem.hpp>
#include <ooo/vba/word/XOptions.hpp>
#include <ooo/vba/word/XSelection.hpp>
#include <ooo/vba/word/XAddins.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbaapplicationbase.hxx>
#include <cppuhelper/implbase.hxx>

typedef cppu::ImplInheritanceHelper< VbaApplicationBase, ooo::vba::word::XApplication > SwVbaApplication_BASE;

class SwVbaApplication : public SwVbaApplication_BASE
{
    // FIXME: We allow just one sink at a time
    std::vector<css::uno::Reference< ooo::vba::XSink >> mvSinks;

public:
    explicit SwVbaApplication( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~SwVbaApplication() override;

    sal_uInt32 AddSink( const css::uno::Reference< ooo::vba::XSink >& xSink );
    void RemoveSink( sal_uInt32 nNumber );

    // XApplication
    virtual OUString SAL_CALL getName() override;
    virtual css::uno::Reference< ooo::vba::word::XSystem > SAL_CALL getSystem() override;
    virtual css::uno::Reference< ov::word::XDocument > SAL_CALL getActiveDocument() override;
    virtual css::uno::Reference< ov::word::XWindow > SAL_CALL getActiveWindow() override;
    virtual css::uno::Reference< ooo::vba::word::XOptions > SAL_CALL getOptions() override;
    virtual css::uno::Reference< ooo::vba::word::XSelection > SAL_CALL getSelection() override;
    virtual css::uno::Any SAL_CALL CommandBars( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Documents( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Addins( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Dialogs( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL ListGalleries( const css::uno::Any& aIndex ) override;
    virtual sal_Bool SAL_CALL getDisplayAutoCompleteTips() override;
    virtual void SAL_CALL setDisplayAutoCompleteTips( sal_Bool _displayAutoCompleteTips ) override;
    virtual sal_Int32 SAL_CALL getEnableCancelKey() override;
    virtual void SAL_CALL setEnableCancelKey( sal_Int32 _enableCancelKey ) override;
    virtual float SAL_CALL CentimetersToPoints( float Centimeters ) override;
    virtual void SAL_CALL ShowMe() override;

    // XInterfaceWithIID
    virtual OUString SAL_CALL getIID() override;

    // XConnectable
    virtual OUString SAL_CALL GetIIDForClassItselfNotCoclass() override;
    virtual ov::TypeAndIID SAL_CALL GetConnectionPoint() override;
    virtual css::uno::Reference<ov::XConnectionPoint> SAL_CALL FindConnectionPoint() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
protected:
    virtual css::uno::Reference< css::frame::XModel > getCurrentDocument() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAAPPLICATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
