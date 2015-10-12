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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_STATUSBARCOMMANDDISPATCH_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_STATUSBARCOMMANDDISPATCH_HXX

#include "CommandDispatch.hxx"
#include "ObjectIdentifier.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/util/XModifiable.hpp>

namespace chart
{

/** This is a CommandDispatch implementation for all commands the status bar offers

    This class reads the information needed from the XModel passed here.
 */

namespace impl
{
typedef ::cppu::ImplInheritanceHelper<
        CommandDispatch,
        ::com::sun::star::view::XSelectionChangeListener >
    StatusBarCommandDispatch_Base;
}

class StatusBarCommandDispatch : public impl::StatusBarCommandDispatch_Base
{
public:
    explicit StatusBarCommandDispatch(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::view::XSelectionSupplier > & xSelSupp );
    virtual ~StatusBarCommandDispatch();

    // late initialisation, especially for adding as listener
    virtual void initialize() override;

protected:
    // ____ XDispatch ____
    virtual void SAL_CALL dispatch(
        const ::com::sun::star::util::URL& URL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ WeakComponentImplHelperBase ____
    /// is called when this is disposed
    virtual void SAL_CALL disposing() override;

    // ____ XModifyListener (override from CommandDispatch) ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void fireStatusEvent(
        const OUString & rURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener ) override;

    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XModifiable > m_xModifiable;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::view::XSelectionSupplier > m_xSelectionSupplier;
    bool m_bIsModified;
    ObjectIdentifier m_aSelectedOID;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_STATUSBARCOMMANDDISPATCH_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
