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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <uielement/uielement.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

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
            bool bEqual = ( m_aFloatingData.m_aPos.Y() == aUIElement.m_aFloatingData.m_aPos.Y() );
            if ( bEqual )
                return ( m_aFloatingData.m_aPos.X() < aUIElement.m_aFloatingData.m_aPos.X() );
            else
                return ( m_aFloatingData.m_aPos.Y() < aUIElement.m_aFloatingData.m_aPos.Y() );
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
                    if ( !( m_aDockedData.m_aPos.Y() == aUIElement.m_aDockedData.m_aPos.Y() ) )
                        return  ( m_aDockedData.m_aPos.Y() < aUIElement.m_aDockedData.m_aPos.Y() );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.X() == aUIElement.m_aDockedData.m_aPos.X() );
                        if ( bEqual )
                        {
                            if ( m_bUserActive && !aUIElement.m_bUserActive )
                                return sal_True;
                            else if ( !m_bUserActive && aUIElement.m_bUserActive )
                                return sal_False;
                            else
                                return sal_False;
                        }
                        else
                            return ( m_aDockedData.m_aPos.X() <= aUIElement.m_aDockedData.m_aPos.X() );
                    }
                }
                else
                {
                    if ( !( m_aDockedData.m_aPos.X() == aUIElement.m_aDockedData.m_aPos.X() ) )
                        return ( m_aDockedData.m_aPos.X() < aUIElement.m_aDockedData.m_aPos.X() );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.Y() == aUIElement.m_aDockedData.m_aPos.Y() );
                        if ( bEqual )
                        {
                            if ( m_bUserActive && !aUIElement.m_bUserActive )
                                return sal_True;
                            else if ( !m_bUserActive && aUIElement.m_bUserActive )
                                return sal_False;
                            else
                                return sal_False;
                        }
                        else
                            return ( m_aDockedData.m_aPos.Y() <= aUIElement.m_aDockedData.m_aPos.Y() );
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
