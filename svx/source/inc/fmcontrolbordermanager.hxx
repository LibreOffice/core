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

#ifndef INCLUDED_SVX_SOURCE_INC_FMCONTROLBORDERMANAGER_HXX
#define INCLUDED_SVX_SOURCE_INC_FMCONTROLBORDERMANAGER_HXX

#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <comphelper/stl_types.hxx>

#include <set>

namespace com { namespace sun { namespace star { namespace form { namespace validation {
    class XValidatableFormComponent;
} } } } }


namespace svxform
{


    typedef sal_Int16 ControlStatus;

    #define CONTROL_STATUS_NONE         0x00
    #define CONTROL_STATUS_FOCUSED      0x01
    #define CONTROL_STATUS_MOUSE_HOVER  0x02
    #define CONTROL_STATUS_INVALID      0x04

    struct BorderDescriptor
    {
        sal_Int16   nBorderType;
        sal_Int32   nBorderColor;

        BorderDescriptor()
            :nBorderType( css::awt::VisualEffect::FLAT )
            ,nBorderColor( 0x00000000 )
        {
        }
    };

    struct UnderlineDescriptor
    {
        sal_Int16 nUnderlineType;
        sal_Int32 nUnderlineColor;

        UnderlineDescriptor()
            :nUnderlineType( css::awt::FontUnderline::NONE )
            ,nUnderlineColor( 0x00000000 )
        {
        }

        UnderlineDescriptor( sal_Int16 _nUnderlineType, sal_Int32 _nUnderlineColor )
            :nUnderlineType( _nUnderlineType )
            ,nUnderlineColor( _nUnderlineColor )
        {
        }
    };

    struct ControlData : public BorderDescriptor, UnderlineDescriptor
    {
        css::uno::Reference< css::awt::XControl > xControl;
        OUString                                                     sOriginalHelpText;

        ControlData() : BorderDescriptor() { }
        ControlData( const css::uno::Reference< css::awt::XControl >& _rxControl )
            :xControl( _rxControl )
        {
        }
    };


    //= ControlBorderManager

    /** manages the dynamic border color for form controls

        Used by the <type>FormController</type>, this class manages the dynamic changes in the
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
        typedef ::std::set< css::uno::Reference< css::awt::XVclWindowPeer >,
                            ::comphelper::OInterfaceCompare< css::awt::XVclWindowPeer > >  PeerBag;

        PeerBag     m_aColorableControls;
        PeerBag     m_aNonColorableControls;

        ControlData m_aFocusControl;
        ControlData m_aMouseHoverControl;
        ControlBag  m_aInvalidControls;


        // attributes
        sal_Int32   m_nFocusColor;
        sal_Int32   m_nMouseHoveColor;
        sal_Int32   m_nInvalidColor;
        bool        m_bDynamicBorderColors;

    public:
        ControlBorderManager();
        ~ControlBorderManager();

    public:
        void    focusGained( const css::uno::Reference< css::uno::XInterface >& _rxControl );
        void    focusLost( const css::uno::Reference< css::uno::XInterface >& _rxControl );
        void    mouseEntered( const css::uno::Reference< css::uno::XInterface >& _rxControl );
        void    mouseExited( const css::uno::Reference< css::uno::XInterface >& _rxControl );

        void    validityChanged(
                    const css::uno::Reference< css::awt::XControl >& _rxControl,
                    const css::uno::Reference< css::form::validation::XValidatableFormComponent >& _rxValidatable
                );

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
        /** called when a control got one of the two possible statuses (focused, and hovered with the mouse)
            @param _rxControl
                the control which gained the status
            @param _rControlData
                the control's status data, as a reference to our respective member
        */
        void    controlStatusGained(
                    const css::uno::Reference< css::uno::XInterface >& _rxControl,
                    ControlData& _rControlData
                );

        /** called when a control lost one of the two possible statuses (focused, and hovered with the mouse)
            @param _rxControl
                the control which lost the status
            @param _rControlData
                the control's status data, as a reference to our respective member
        */
        void    controlStatusLost( const css::uno::Reference< css::uno::XInterface >& _rxControl, ControlData& _rControlData );

        /** determines whether the border of a given peer can be colored
            @param _rxPeer
                the peer to examine. Must not be <NULL/>
        */
        bool    canColorBorder( const css::uno::Reference< css::awt::XVclWindowPeer >& _rxPeer );

        /** determines the status of the given control
        */
        ControlStatus   getControlStatus( const css::uno::Reference< css::awt::XControl >& _rxControl );

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
                            const css::uno::Reference< css::awt::XControl >& _rxControl,
                            const css::uno::Reference< css::awt::XVclWindowPeer >& _rxPeer,
                            const BorderDescriptor& _rFallback
                        );

        /** determines the to-be-remembered original border color and type for a control

            The method also takes into account that the control may currently have an overwritten
            border style

            @param _rxControl
                the control to examine. Must not be <NULL/>, and have a non-<NULL/> peer
        */
        void determineOriginalBorderStyle(
                    const css::uno::Reference< css::awt::XControl >& _rxControl,
                    BorderDescriptor& _rData
                ) const;
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_FMCONTROLBORDERMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
