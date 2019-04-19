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

#include <comphelper/comphelperdllapi.h>

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XInterface; }
    namespace lang {
        class XComponent; }
} } }

namespace com { namespace sun { namespace star { namespace beans { struct PropertyValue; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <class E> class Sequence; } } } }


namespace comphelper
{


    //= SynchronousDispatch

    /** a helper class for working with the dispatch interface replacing
        loadComponentFromURL
    */
    class SynchronousDispatch
    {
    public:
        static COMPHELPER_DLLPUBLIC css::uno::Reference< css::lang::XComponent > dispatch(
            const css::uno::Reference< css::uno::XInterface > &xStartPoint,
            const OUString &sURL,
            const OUString &sTarget,
            const css::uno::Sequence< css::beans::PropertyValue > &lArguments );
    };


} // namespace comphelper



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
