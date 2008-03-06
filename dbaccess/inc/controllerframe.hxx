/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controllerframe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:55:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
