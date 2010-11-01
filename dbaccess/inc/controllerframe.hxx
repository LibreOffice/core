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

#ifndef DBACCESS_CONTROLLERFRAME_HXX
#define DBACCESS_CONTROLLERFRAME_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
/** === end UNO includes === **/

#include <memory>

//........................................................................
namespace dbaui
{
//........................................................................

    class IController;

    //====================================================================
    //= ControllerFrame
    //====================================================================
    struct ControllerFrame_Data;
    /** helper class to ancapsulate the frame which a controller is plugged into,
        doing some common actions on it.
    */
    class ControllerFrame
    {
    public:
        ControllerFrame( IController& _rController );
        ~ControllerFrame();

        /// attaches a new frame
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&
                attachFrame(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
                );

        // retrieves the current frame
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&
                getFrame() const;

        /** determines whether the frame is currently active
        */
        bool    isActive() const;

        /** notifies the instance that a certain frame action happened with our frame
        */
        void    frameAction( ::com::sun::star::frame::FrameAction _eAction );

    private:
        ::std::auto_ptr< ControllerFrame_Data > m_pData;
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_CONTROLLERFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
