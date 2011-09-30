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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <tools/debug.hxx>
#include <vcl/event.hxx>

/** inits this vcl KeyEvent with all settings from the given awt event **/
MouseEvent::MouseEvent( const ::com::sun::star::awt::MouseEvent& rEvent )
: maPos( rEvent.X, rEvent.Y )
, mnMode( 0 )
, mnClicks( static_cast< sal_uInt16 >( rEvent.ClickCount ) )
, mnCode( 0 )
{
    if( rEvent.Modifiers )
    {
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::SHIFT) != 0 )
            mnCode |= KEY_SHIFT;
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD1) != 0 )
            mnCode |= KEY_MOD1;
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD2) != 0 )
            mnCode |= KEY_MOD2;
                if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD3) != 0 )
                        mnCode |= KEY_MOD3;
    }

    if( rEvent.Buttons )
    {
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::LEFT) != 0 )
            mnCode |= MOUSE_LEFT;
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::RIGHT) != 0 )
            mnCode |= MOUSE_RIGHT;
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::MIDDLE) != 0 )
            mnCode |= MOUSE_MIDDLE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
