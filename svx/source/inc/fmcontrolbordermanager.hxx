/*************************************************************************
 *
 *  $RCSfile: fmcontrolbordermanager.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-05 12:22:58 $
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
#ifndef _COM_SUN_STAR_AWT_VISUALEFFECT_HPP_
#include <com/sun/star/awt/VisualEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif
/** === end UNO includes === **/
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#include <set>

namespace com { namespace sun { namespace star { namespace form { namespace validation {
    class XValidatableFormComponent;
} } } } }

//........................................................................
namespace svxform
{
//........................................................................

    typedef sal_Int16 ControlStatus;

    #define CONTROL_STATUS_NONE         0x00
    #define CONTROL_STATUS_FOCUSED      0x01
    #define CONTROL_STATUS_MOUSE_HOVER  0x02
    #define CONTROL_STATUS_INVALID      0x04

    //====================================================================
    //= BorderDescriptor
    //====================================================================
    struct BorderDescriptor
    {
        sal_Int16   nBorderType;
        sal_Int32   nBorderColor;

        BorderDescriptor()
            :nBorderType( ::com::sun::star::awt::VisualEffect::FLAT )
            ,nBorderColor( 0x00000000 )
        {
        }
        inline void clear()
        {
            nBorderType = ::com::sun::star::awt::VisualEffect::FLAT;
            nBorderColor = 0x00000000;
        }
    };

    //====================================================================
    //= UnderlineDescriptor
    //====================================================================
    struct UnderlineDescriptor
    {
        sal_Int16 nUnderlineType;
        sal_Int32 nUnderlineColor;

        UnderlineDescriptor()
            :nUnderlineType( ::com::sun::star::awt::FontUnderline::NONE )
            ,nUnderlineColor( 0x00000000 )
        {
        }

        UnderlineDescriptor( sal_Int16 _nUnderlineType, sal_Int32 _nUnderlineColor )
            :nUnderlineType( _nUnderlineType )
            ,nUnderlineColor( _nUnderlineColor )
        {
        }

        inline void clear()
        {
            nUnderlineType = ::com::sun::star::awt::FontUnderline::NONE;
            nUnderlineColor = 0x00000000;
        }
    };

    //====================================================================
    //= ControlData
    //====================================================================
    struct ControlData : public BorderDescriptor, UnderlineDescriptor
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xControl;
        ::rtl::OUString                                                     sOriginalHelpText;

        ControlData() : BorderDescriptor() { }
        ControlData( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl )
            :xControl( _rxControl )
        {
        }
        void clear()
        {
            BorderDescriptor::clear();
            UnderlineDescriptor::clear();
            xControl.clear();
            sOriginalHelpText = ::rtl::OUString();
        }
    };

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
        struct ControlDataCompare : public ::std::binary_function< ControlData, ControlData, bool >
        {
           bool operator()( const ControlData& _rLHS, const ControlData& _rRHS ) const
           {
               return _rLHS.xControl.get() < _rRHS.xControl.get();
           }
        };

        typedef ::std::set< ControlData, ControlDataCompare > ControlBag;
        typedef ::com::sun::star::awt::XVclWindowPeer                                       WindowPeer;
        typedef ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >   WindowPeerRef;
        typedef ::std::set< WindowPeerRef, ::comphelper::OInterfaceCompare< WindowPeer > >  PeerBag;

        PeerBag     m_aColorableControls;
        PeerBag     m_aNonColorableControls;

        ControlData m_aFocusControl;
        ControlData m_aMouseHoverControl;
        ControlBag  m_aInvalidControls;

        // ----------------
        // attributes
        sal_Int32   m_nFocusColor;
        sal_Int32   m_nMouseHoveColor;
        sal_Int32   m_nInvalidColor;
        bool        m_bDynamicBorderColors;

    public:
        ControlBorderManager();
        ~ControlBorderManager();

    public:
        void    focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl ) SAL_THROW(());
        void    focusLost( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl ) SAL_THROW(());
        void    mouseEntered( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl ) SAL_THROW(());
        void    mouseExited( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl ) SAL_THROW(());

        void    validityChanged(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::form::validation::XValidatableFormComponent >& _rxValidatable
                ) SAL_THROW(());

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
        */
        void    controlStatusGained(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxControl,
                    ControlData& _rControlData
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
            @param _rFallback
                the color/type to use when the control has the status CONTROL_STATUS_NONE
        */
        void            updateBorderStyle(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >& _rxPeer,
                            const BorderDescriptor& _rFallback
                        ) SAL_THROW(());

        /** determines the to-be-remembered original border color and type for a control

            The method also takes into account that the control may currently have an overwritten
            border style

            @param _rxControl
                the control to examine. Must not be <NULL/>, and have a non-<NULL/> peer
        */
        void determineOriginalBorderStyle(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl,
                    BorderDescriptor& _rData
                ) const;
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_FMCONTROLBORDERMANAGER_HXX
