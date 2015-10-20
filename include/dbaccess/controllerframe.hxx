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

#ifndef INCLUDED_DBACCESS_CONTROLLERFRAME_HXX
#define INCLUDED_DBACCESS_CONTROLLERFRAME_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameAction.hpp>

#include <memory>


namespace dbaui
{


    class IController;


    //= ControllerFrame

    struct ControllerFrame_Data;
    /** helper class to encapsulate the frame which a controller is plugged into,
        doing some common actions on it.
    */
    class ControllerFrame
    {
    public:
        ControllerFrame( IController& _rController );
        ~ControllerFrame();

        /// attaches a new frame
        const css::uno::Reference< css::frame::XFrame >&
                attachFrame(
                    const css::uno::Reference< css::frame::XFrame >& _rxFrame
                );

        // retrieves the current frame
        const css::uno::Reference< css::frame::XFrame >&
                getFrame() const;

        /** determines whether the frame is currently active
        */
        bool    isActive() const;

        /** notifies the instance that a certain frame action happened with our frame
        */
        void    frameAction( css::frame::FrameAction _eAction );

    private:
        ::std::unique_ptr< ControllerFrame_Data > m_pData;
    };


} // namespace dbaui


#endif // INCLUDED_DBACCESS_CONTROLLERFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
