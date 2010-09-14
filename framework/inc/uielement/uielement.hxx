/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layoutmanager.hxx,v $
 * $Revision: 1.34 $
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

#ifndef __FRAMEWORK_LAYOUTMANAGER_UIELEMENT_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_UIELEMENT_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/DockingArea.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <rtl/ustring.hxx>
#include <vcl/toolbox.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

struct DockedData
{
    DockedData() : m_aPos( LONG_MAX, LONG_MAX ),
                   m_nDockedArea( ::com::sun::star::ui::DockingArea_DOCKINGAREA_TOP ),
                   m_bLocked( sal_False ) {}

    Point       m_aPos;
    Size        m_aSize;
    sal_Int16   m_nDockedArea;
    sal_Bool    m_bLocked;
};

struct FloatingData
{
    FloatingData() : m_aPos( LONG_MAX, LONG_MAX ),
                     m_nLines( 1 ),
                     m_bIsHorizontal( sal_True ) {}

    Point       m_aPos;
    Size        m_aSize;
    sal_Int16   m_nLines;
    sal_Bool    m_bIsHorizontal;
};

struct UIElement
{
    UIElement() : m_bFloating( sal_False ),
                  m_bVisible( sal_True ),
                  m_bUserActive( sal_False ),
                  m_bCreateNewRowCol0( sal_False ),
                  m_bDeactiveHide( sal_False ),
                  m_bMasterHide( sal_False ),
                  m_bContextSensitive( sal_False ),
                  m_bContextActive( sal_True ),
                  m_bNoClose( sal_False ),
                  m_bSoftClose( sal_False ),
                  m_bStateRead( sal_False ),
                  m_nStyle( BUTTON_SYMBOL )
                  {}

    UIElement( const rtl::OUString& rName,
               const rtl::OUString& rType,
               const com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& rUIElement,
               sal_Bool bFloating = sal_False
               ) : m_aType( rType ),
                   m_aName( rName ),
                   m_xUIElement( rUIElement ),
                   m_bFloating( bFloating ),
                   m_bVisible( sal_True ),
                   m_bUserActive( sal_False ),
                   m_bCreateNewRowCol0( sal_False ),
                   m_bDeactiveHide( sal_False ),
                   m_bMasterHide( sal_False ),
                   m_bContextSensitive( sal_False ),
                   m_bContextActive( sal_True ),
                   m_bNoClose( sal_False ),
                   m_bSoftClose( sal_False ),
                   m_bStateRead( sal_False ),
                   m_nStyle( BUTTON_SYMBOL ) {}

    bool operator< ( const UIElement& aUIElement ) const;
    UIElement& operator=( const UIElement& rUIElement );

    rtl::OUString                                                            m_aType;
    rtl::OUString                                                            m_aName;
    rtl::OUString                                                            m_aUIName;
    com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >       m_xUIElement;
    sal_Bool                                                                 m_bFloating,
                                                                             m_bVisible,
                                                                             m_bUserActive,
                                                                             m_bCreateNewRowCol0,
                                                                             m_bDeactiveHide,
                                                                             m_bMasterHide,
                                                                             m_bContextSensitive,
                                                                             m_bContextActive;
    sal_Bool                                                                 m_bNoClose,
                                                                             m_bSoftClose,
                                                                             m_bStateRead;
    sal_Int16                                                                m_nStyle;
    DockedData                                                               m_aDockedData;
    FloatingData                                                             m_aFloatingData;
};

typedef std::vector< UIElement > UIElementVector;

} // namespace framework

#endif // __FRAMEWORK_LAYOUTMANAGER_UIELEMENT_HXX_
