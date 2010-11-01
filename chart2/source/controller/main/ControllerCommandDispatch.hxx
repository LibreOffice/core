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
#ifndef CHART2_CONTROLLERCOMMANDDISPATCH_HXX
#define CHART2_CONTROLLERCOMMANDDISPATCH_HXX

#include "CommandDispatch.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cppuhelper/implbase1.hxx>

#include <memory>

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
typedef ::cppu::ImplInheritanceHelper1<
        CommandDispatch,
        ::com::sun::star::view::XSelectionChangeListener >
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
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        ChartController* pController, CommandDispatchContainer* pContainer );
    virtual ~ControllerCommandDispatch();

    // late initialisation, especially for adding as listener
    virtual void initialize();

protected:
    // ____ XDispatch ____
    virtual void SAL_CALL dispatch(
        const ::com::sun::star::util::URL& URL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ WeakComponentImplHelperBase ____
    /// is called when this is disposed
    virtual void SAL_CALL disposing();

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void fireStatusEvent(
        const ::rtl::OUString & rURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener );

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

private:
    void fireStatusEventForURLImpl(
        const ::rtl::OUString & rURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener );

    bool commandAvailable( const ::rtl::OUString & rCommand );
    void updateCommandAvailability();

    bool isShapeControllerCommandAvailable( const ::rtl::OUString& rCommand );

    ChartController* m_pChartController;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController > m_xController;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::view::XSelectionSupplier > m_xSelectionSupplier;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XDispatch > m_xDispatch;

    ::std::auto_ptr< impl::ModelState > m_apModelState;
    ::std::auto_ptr< impl::ControllerState > m_apControllerState;

    mutable ::std::map< ::rtl::OUString, bool > m_aCommandAvailability;
    mutable ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any > m_aCommandArguments;

    CommandDispatchContainer* m_pDispatchContainer;
};

} //  namespace chart

// CHART2_CONTROLLERCOMMANDDISPATCH_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
