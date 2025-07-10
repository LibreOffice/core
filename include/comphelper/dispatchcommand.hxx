/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_DISPATCHCOMMAND_HXX
#define INCLUDED_COMPHELPER_DISPATCHCOMMAND_HXX

#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::frame
{
class XDispatchResultListener;
class XDispatchProvider;
class XFrame;
class XController;
}
namespace com::sun::star::uno { template <typename > class Sequence; }

namespace comphelper
{

/** Dispatch the given UNO command in the active frame.

    @param rCommand the command to dispatch, like ".uno:Bold".

    @return true on success.
*/
COMPHELPER_DLLPUBLIC bool dispatchCommand(const OUString& rCommand,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments,
        const css::uno::Reference<css::frame::XDispatchResultListener>& xListener = {});

COMPHELPER_DLLPUBLIC bool dispatchCommand(const OUString& rCommand,
        const css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments,
        const css::uno::Reference<css::frame::XDispatchResultListener>& xListener = {});

COMPHELPER_DLLPUBLIC bool dispatchCommand(const OUString& rCommand,
        const css::uno::Reference<css::frame::XFrame>& xFrame,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments,
        const css::uno::Reference<css::frame::XDispatchResultListener>& xListener = {});

COMPHELPER_DLLPUBLIC bool dispatchCommand(const OUString& rCommand,
        const css::uno::Reference<css::frame::XController>& xController,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments,
        const css::uno::Reference<css::frame::XDispatchResultListener>& xListener = {});

}

#endif // INCLUDED_COMPHELPER_DISPATCHCOMMAND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
