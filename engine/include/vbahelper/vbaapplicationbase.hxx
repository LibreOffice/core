/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_VBAHELPER_VBAAPPLICATIONBASE_HXX
#define INCLUDED_VBAHELPER_VBAAPPLICATIONBASE_HXX

#include <memory>

#include <com/sun/star/uno/Reference.hxx>
#include <ooo/vba/XApplicationBase.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace com::sun::star {
    namespace frame { class XModel; }
}
namespace cpo::uno { class XComponentContext; }

typedef InheritedHelperInterfaceWeakImpl< ov::XApplicationBase > ApplicationBase_BASE;

struct VbaApplicationBase_Impl;

class VBAHELPER_DLLPUBLIC VbaApplicationBase : public ApplicationBase_BASE
{
    std::unique_ptr<VbaApplicationBase_Impl> m_pImpl;

protected:
    VbaApplicationBase( const css::uno::Reference< cpo::uno::XComponentContext >& xContext );
    virtual ~VbaApplicationBase() override;

    /// @throws cpo::uno::RuntimeException
    virtual css::frame::XModel* getCurrentDocument() = 0;
public:
    // XHelperInterface ( parent is itself )
    virtual css::uno::Reference< ov::XHelperInterface > getParent(  ) override { return this; }

    virtual bool getScreenUpdating() override;
    virtual void setScreenUpdating(bool bUpdate) override;
    virtual bool getDisplayStatusBar() override;
    virtual void setDisplayStatusBar(bool bDisplayStatusBar) override;
    virtual bool getInteractive() override;
    virtual void setInteractive( bool bInteractive ) override;
    virtual bool getVisible() override;
    virtual void setVisible( bool bVisible ) override;
    virtual OUString getCaption() override;
    virtual void setCaption( const OUString& sCaption ) override;
    virtual void OnKey( const OUString& Key, const cpo::uno::Any& Procedure ) override;
    virtual cpo::uno::Any CommandBars( const cpo::uno::Any& aIndex ) override;
    virtual OUString getVersion() override;
    virtual cpo::uno::Any getVBE() override;

    virtual cpo::uno::Any Run( const OUString& MacroName, const cpo::uno::Any& varg1, const cpo::uno::Any& varg2, const cpo::uno::Any& varg3, const cpo::uno::Any& varg4, const cpo::uno::Any& varg5, const cpo::uno::Any& varg6, const cpo::uno::Any& varg7, const cpo::uno::Any& varg8, const cpo::uno::Any& varg9, const cpo::uno::Any& varg10, const cpo::uno::Any& varg11, const cpo::uno::Any& varg12, const cpo::uno::Any& varg13, const cpo::uno::Any& varg14, const cpo::uno::Any& varg15, const cpo::uno::Any& varg16, const cpo::uno::Any& varg17, const cpo::uno::Any& varg18, const cpo::uno::Any& varg19, const cpo::uno::Any& varg20, const cpo::uno::Any& varg21, const cpo::uno::Any& varg22, const cpo::uno::Any& varg23, const cpo::uno::Any& varg24, const cpo::uno::Any& varg25, const cpo::uno::Any& varg26, const cpo::uno::Any& varg27, const cpo::uno::Any& varg28, const cpo::uno::Any& varg29, const cpo::uno::Any& varg30 ) override;
    virtual void OnTime( const cpo::uno::Any& aEarliestTime, const OUString& aFunction, const cpo::uno::Any& aLatestTime, const cpo::uno::Any& aSchedule ) override;
    virtual void Undo() override;
    virtual void Quit() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
