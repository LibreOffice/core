/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
