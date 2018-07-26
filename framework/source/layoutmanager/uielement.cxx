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

#include <uielement/uielement.hxx>

#include <com/sun/star/ui/DockingArea.hpp>

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
                    if (  m_aDockedData.m_aPos.Y != aUIElement.m_aDockedData.m_aPos.Y )
                        return  ( m_aDockedData.m_aPos.Y < aUIElement.m_aDockedData.m_aPos.Y );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.X == aUIElement.m_aDockedData.m_aPos.X );
                        if ( bEqual )
                        {
                            return m_bUserActive && !aUIElement.m_bUserActive;
                        }
                        else
                            return ( m_aDockedData.m_aPos.X <= aUIElement.m_aDockedData.m_aPos.X );
                    }
                }
                else
                {
                    if ( m_aDockedData.m_aPos.X != aUIElement.m_aDockedData.m_aPos.X )
                        return ( m_aDockedData.m_aPos.X < aUIElement.m_aDockedData.m_aPos.X );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.Y == aUIElement.m_aDockedData.m_aPos.Y );
                        if ( bEqual )
                        {
                            return m_bUserActive && !aUIElement.m_bUserActive;
                        }
                        else
                            return ( m_aDockedData.m_aPos.Y <= aUIElement.m_aDockedData.m_aPos.Y );
                    }
                }
            }
        }
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
