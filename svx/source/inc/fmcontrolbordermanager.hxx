/*************************************************************************
 *
 *  $RCSfile: fmcontrolbordermanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 15:51:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVX_SOURCE_INC_FMCONTROLBORDERMANAGER_HXX
#define SVX_SOURCE_INC_FMCONTROLBORDERMANAGER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif
/** === end UNO includes === **/

//........................................................................
namespace svxform
{
//........................................................................

    typedef sal_Int16 ControlStatus;

    #define CONTROL_STATUS_NONE         0x00
    #define CONTROL_STATUS_FOCUSED      0x01
    #define CONTROL_STATUS_MOUSE_HOVER  0x02
    #define CONTROL_STATUS_BOTH         0x03

    //====================================================================
    //= ControlBorderManager
    //====================================================================
    /** manages the dynamic border color for form controls

        Used by the <type>FmXFormController</type>, this class manages the dynamic changes in the
        border color of form controls. For this a set of events have to be forwarded to the manager
        instance, which then will switch the border color depending on the mouse and focus status
        of the controls.
    */
    class ControlBorderManager
    {
    private:
        struct ControlData
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xControl;
            sal_Int32                                                           nOriginalColor;

            ControlData() : nOriginalColor( 0x00000000 ) { }
            inline void clear()
            {
                xControl.clear();
                nOriginalColor = 0x00000000;
            }
        };

        ControlData m_aFocusControl;
        ControlData m_aMouseHoverControl;

        sal_Int32   m_nFocusColor;
        sal_Int32   m_nMouseHoveColor;
        sal_Int32   m_nCombinedColor;
        bool        m_bEnabled;

    public:
        ControlBorderManager();
        ~ControlBorderManager();

    public:
        void    focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl ) SAL_THROW(());
        void    focusLost( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl ) SAL_THROW(());
        void    mouseEntered( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl ) SAL_THROW(());
        void    mouseExited( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl ) SAL_THROW(());

        /// enables dynamic border color for the controls
        void    enableDynamicBorderColor( );
        /// disables dynamic border color for the controls
        void    disableDynamicBorderColor( );

        /** sets a color to be used for a given status
            @param _nStatus
                the status which the color should be applied for. Must not be CONTROL_STATUS_NONE
            @param _nColor
                the color to apply for the given status
        */
        void    setStatusColor( ControlStatus _nStatus, sal_Int32 _nColor );

        /** restores all colors of all controls where we possibly changed them
        */
        void    restoreAll();

    private:
        /** called when a control got one of the two possible stati (focused, and hovered with the mouse)
            @param _rxControl
                the control which gained the status
            @param _rControlData
                the control's status data, as a reference to our respective member
            @param _rOppositeStatusData
                the data for the opponent status. This is needed to properly determine the original color
        */
        void    controlStatusGained(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl,
                    ControlData& _rControlData,
                    const ControlData& _rOppositeStatusData
                ) SAL_THROW(());

        /** called when a control lost one of the two possible stati (focused, and hovered with the mouse)
            @param _rxControl
                the control which lost the status
            @param _rControlData
                the control's status data, as a reference to our respective member
        */
        void    controlStatusLost( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl, ControlData& _rControlData ) SAL_THROW(());

        /** determines whether the border of a given peer can be colored
            @param _rxPeer
                the peer to examine. Must not be <NULL/>
        */
        bool    canColorBorder( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >& _rxPeer );

        /** determines the status of the given control
        */
        ControlStatus   getControlStatus( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl ) SAL_THROW(());

        /** retrieves the color associated with a given ControlStatus
            @param _eStatus
                the status of the control. Must not be <member>ControlStatus::none</member>
        */
        sal_Int32       getControlColorByStatus( ControlStatus _eStatus );

        /** sets the border color for a given control, depending on its status
            @param _rxControl
                the control to set the border color for. Must not be <NULL/>
            @param _rxPeer
                the peer of the control, to be passed herein for optimization the caller usually needs it, anyway).
                Must not be <NULL/>
            @param _nFallbackColor
                the color to use when the control has the status CONTROL_STATUS_NONE
        */
        void            updateControlColor(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >& _rxPeer,
                            sal_Int32 _nFallbackColor
                        ) SAL_THROW(());
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_FMCONTROLBORDERMANAGER_HXX
