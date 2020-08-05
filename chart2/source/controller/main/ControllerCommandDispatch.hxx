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
#pragma once

#include "CommandDispatch.hxx"
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <memory>

namespace com::sun::star::view { class XSelectionSupplier; }

namespace chart
{

class ChartController;
class CommandDispatchContainer;

namespace impl
{
struct ModelState;
struct ControllerState;

// #i63017# : need to implement the XSelectionChangeListener in order
// to update the ControllerState when the selection changes.
typedef ::cppu::ImplInheritanceHelper<
        CommandDispatch,
        css::view::XSelectionChangeListener >
    ControllerCommandDispatch_Base;
}

/** This class is a CommandDispatch that is responsible for all commands that
    the ChartController supports.

    This class determines which commands are currently available (via the model
    state) and if an available command is called forwards it to the
    ChartController.
 */
class ControllerCommandDispatch : public impl::ControllerCommandDispatch_Base
{
public:
    explicit ControllerCommandDispatch(
        const css::uno::Reference< css::uno::XComponentContext > & xContext,
        ChartController* pController, CommandDispatchContainer* pContainer );
    virtual ~ControllerCommandDispatch() override;

    // late initialisation, especially for adding as listener
    virtual void initialize() override;

    bool commandAvailable( const OUString & rCommand );

protected:
    // ____ XDispatch ____
    virtual void SAL_CALL dispatch(
        const css::util::URL& URL,
        const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;

    // ____ WeakComponentImplHelperBase ____
    /// is called when this is disposed
    virtual void SAL_CALL disposing() override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    virtual void fireStatusEvent(
        const OUString & rURL,
        const css::uno::Reference< css::frame::XStatusListener > & xSingleListener ) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const css::lang::EventObject& aEvent ) override;

private:
    void fireStatusEventForURLImpl(
        const OUString & rURL,
        const css::uno::Reference< css::frame::XStatusListener > & xSingleListener );

    void updateCommandAvailability();

    bool isShapeControllerCommandAvailable( const OUString& rCommand );

    rtl::Reference<ChartController> m_xChartController;
    css::uno::Reference< css::view::XSelectionSupplier > m_xSelectionSupplier;
    css::uno::Reference< css::frame::XDispatch > m_xDispatch;

    std::unique_ptr< impl::ModelState > m_apModelState;
    std::unique_ptr< impl::ControllerState > m_apControllerState;

    mutable std::map< OUString, bool > m_aCommandAvailability;
    mutable std::map< OUString, css::uno::Any > m_aCommandArguments;

    CommandDispatchContainer* m_pDispatchContainer;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
