/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#include <uielement/uielement.hxx>

#include <com/sun/star/ui/DockingArea.hpp>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star;

namespace framework
{

    bool UIElement::operator< ( const ::framework::UIElement& aUIElement ) const
{
    if ( !m_xUIElement.is() && aUIElement.m_xUIElement.is() )
        return false;
    else if ( m_xUIElement.is() && !aUIElement.m_xUIElement.is() )
        return true;
    else if ( !m_bVisible && aUIElement.m_bVisible )
        return false;
    else if ( m_bVisible && !aUIElement.m_bVisible )
        return true;
    else if ( !m_bFloating && aUIElement.m_bFloating )
        return true;
    else if ( m_bFloating && !aUIElement.m_bFloating )
        return false;
    else
    {
        if ( m_bFloating )
        {
            bool bEqual = ( m_aFloatingData.m_aPos.Y == aUIElement.m_aFloatingData.m_aPos.Y );
            if ( bEqual )
                return ( m_aFloatingData.m_aPos.X < aUIElement.m_aFloatingData.m_aPos.X );
            else
                return ( m_aFloatingData.m_aPos.Y < aUIElement.m_aFloatingData.m_aPos.Y );
        }
        else
        {
            if ( m_aDockedData.m_nDockedArea < aUIElement.m_aDockedData.m_nDockedArea )
                return true;
            else if ( m_aDockedData.m_nDockedArea > aUIElement.m_aDockedData.m_nDockedArea )
                return false;
            else
            {
                if ( m_aDockedData.m_nDockedArea == ui::DockingArea_DOCKINGAREA_TOP ||
                     m_aDockedData.m_nDockedArea == ui::DockingArea_DOCKINGAREA_BOTTOM )
                {
                    if ( !( m_aDockedData.m_aPos.Y == aUIElement.m_aDockedData.m_aPos.Y ) )
                        return  ( m_aDockedData.m_aPos.Y < aUIElement.m_aDockedData.m_aPos.Y );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.X == aUIElement.m_aDockedData.m_aPos.X );
                        if ( bEqual )
                        {
                            if ( m_bUserActive && !aUIElement.m_bUserActive )
                                return sal_True;
                            else
                                return sal_False;
                        }
                        else
                            return ( m_aDockedData.m_aPos.X <= aUIElement.m_aDockedData.m_aPos.X );
                    }
                }
                else
                {
                    if ( !( m_aDockedData.m_aPos.X == aUIElement.m_aDockedData.m_aPos.X ) )
                        return ( m_aDockedData.m_aPos.X < aUIElement.m_aDockedData.m_aPos.X );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.Y == aUIElement.m_aDockedData.m_aPos.Y );
                        if ( bEqual )
                        {
                            if ( m_bUserActive && !aUIElement.m_bUserActive )
                                return sal_True;
                            else
                                return sal_False;
                        }
                        else
                            return ( m_aDockedData.m_aPos.Y <= aUIElement.m_aDockedData.m_aPos.Y );
                    }
                }
            }
        }
    }
}

UIElement& UIElement::operator= ( const UIElement& rUIElement )
{
    if (&rUIElement != this)
    {
        m_aType             = rUIElement.m_aType;
        m_aName             = rUIElement.m_aName;
        m_aUIName           = rUIElement.m_aUIName;
        m_xUIElement        = rUIElement.m_xUIElement;
        m_bFloating         = rUIElement.m_bFloating;
        m_bVisible          = rUIElement.m_bVisible;
        m_bUserActive       = rUIElement.m_bUserActive;
        m_bCreateNewRowCol0 = rUIElement.m_bCreateNewRowCol0;
        m_bDeactiveHide     = rUIElement.m_bDeactiveHide;
        m_bMasterHide       = rUIElement.m_bMasterHide;
        m_bContextSensitive = rUIElement.m_bContextSensitive;
        m_bContextActive    = rUIElement.m_bContextActive;
        m_bNoClose          = rUIElement.m_bNoClose;
        m_bSoftClose        = rUIElement.m_bSoftClose;
        m_bStateRead        = rUIElement.m_bStateRead;
        m_nStyle            = rUIElement.m_nStyle;
        m_aDockedData       = rUIElement.m_aDockedData;
        m_aFloatingData     = rUIElement.m_aFloatingData;
    }
    return *this;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
