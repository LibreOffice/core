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

// include files of own module
#include <helper/statusindicatorfactory.hxx>

// include UNO interfaces
#include <com/sun/star/task/XStatusIndicator.hpp>

// include all others
#include <cppuhelper/implbase.hxx>
#include <unotools/weakref.hxx>

namespace framework{
class StatusIndicatorFactory;

/**
    @short          implement a status indicator object

    @descr          With this indicator you can show a message and a progress...
                    but you share the output device with other indicator objects,
                    if this instances was created by the same factory.
                    Then the last created object has full access to device.
                    All others change her internal data structure only.

                    All objects of this StatusIndicator class calls a C++ interface
                    on the StatusIndicatorFactory (where they were created).
                    The factory holds all data structures and paints the progress.

    @devstatus      ready to use
    @threadsafe     yes
*/
class StatusIndicator final : public  ::cppu::WeakImplHelper< css::task::XStatusIndicator >
{

    // member
    private:

        /** @short  weak reference to our factory
            @descr  All our interface calls will be forwarded
                    to a suitable c++ interface on this factory.
                    But we don't hold our factory alive. They
                    correspond with e.g. with a Frame service and
                    will be owned by him. If the frame will be closed
                    he close our factory too ...
         */
        unotools::WeakReference< StatusIndicatorFactory > m_xFactory;

        sal_Int32 m_nRange;
        // We want the callback percentages to increase monotonically
        int m_nLastCallbackPercent;

    // c++ interface
    public:

        /** @short  initialize new instance of this class.

            @param  pFactory
                    pointer to our factory
         */
        StatusIndicator(StatusIndicatorFactory* pFactory);

        /** @short  does nothing real...
         */
        virtual ~StatusIndicator() override;

    // uno interface
    public:

        // XStatusIndicator
        virtual void SAL_CALL start(const OUString& sText ,
                                          sal_Int32        nRange) override;

        virtual void SAL_CALL end() override;

        virtual void SAL_CALL reset() override;

        virtual void SAL_CALL setText(const OUString& sText) override;

        virtual void SAL_CALL setValue(sal_Int32 nValue) override;

}; // class StatusIndicator

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
