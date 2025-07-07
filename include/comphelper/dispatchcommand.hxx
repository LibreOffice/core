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
namespace com::sun::star::frame { class XDispatchResultListener; }
namespace com::sun::star::uno { template <typename > class Sequence; }

namespace comphelper
{

/** Dispatch the given UNO command in the active frame.

    @param rCommand the command to dispatch, like ".uno:Bold".

    @return true on success.
*/
COMPHELPER_DLLPUBLIC bool dispatchCommand(const OUString& rCommand,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments,
        const css::uno::Reference<css::frame::XDispatchResultListener>& rListener = css::uno::Reference<css::frame::XDispatchResultListener>());

COMPHELPER_DLLPUBLIC bool dispatchCommand(const OUString& rCommand,
        const css::uno::Reference<css::uno::XInterface>& xDispatchSource,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments,
        const css::uno::Reference<css::frame::XDispatchResultListener>& rListener = css::uno::Reference<css::frame::XDispatchResultListener>());

}

#endif // INCLUDED_COMPHELPER_DISPATCHCOMMAND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
