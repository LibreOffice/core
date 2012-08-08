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

#ifndef __FRAMEWORK_LAYOUTMANAGER_UIELEMENT_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_UIELEMENT_HXX_

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <rtl/ustring.hxx>
#include <vcl/toolbox.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

struct DockedData
{
    DockedData() : m_aPos( SAL_MAX_INT32, SAL_MAX_INT32 ),
                   m_nDockedArea( ::com::sun::star::ui::DockingArea_DOCKINGAREA_TOP ),
                   m_bLocked( false ) {}

    com::sun::star::awt::Point m_aPos;
    com::sun::star::awt::Size m_aSize;
    sal_Int16 m_nDockedArea;
    bool      m_bLocked;
};

struct FloatingData
{
    FloatingData() : m_aPos( SAL_MAX_INT32, SAL_MAX_INT32 ),
                     m_nLines( 1 ),
                     m_bIsHorizontal( true ) {}

    com::sun::star::awt::Point m_aPos;
    com::sun::star::awt::Size m_aSize;
    sal_Int16 m_nLines;
    bool      m_bIsHorizontal;
};

struct UIElement
{
    UIElement() : m_bFloating( false ),
                  m_bVisible( true ),
                  m_bUserActive( false ),
                  m_bCreateNewRowCol0( false ),
                  m_bDeactiveHide( false ),
                  m_bMasterHide( false ),
                  m_bContextSensitive( false ),
                  m_bContextActive( true ),
                  m_bNoClose( false ),
                  m_bSoftClose( false ),
                  m_bStateRead( false ),
                  m_nStyle( BUTTON_SYMBOL )
                  {}

    UIElement( const rtl::OUString& rName,
               const rtl::OUString& rType,
               const com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& rUIElement,
               bool bFloating = false
               ) : m_aType( rType ),
                   m_aName( rName ),
                   m_xUIElement( rUIElement ),
                   m_bFloating( bFloating ),
                   m_bVisible( true ),
                   m_bUserActive( false ),
                   m_bCreateNewRowCol0( false ),
                   m_bDeactiveHide( false ),
                   m_bMasterHide( false ),
                   m_bContextSensitive( false ),
                   m_bContextActive( true ),
                   m_bNoClose( false ),
                   m_bSoftClose( false ),
                   m_bStateRead( false ),
                   m_nStyle( BUTTON_SYMBOL ) {}

    bool operator< ( const UIElement& aUIElement ) const;
    UIElement& operator=( const UIElement& rUIElement );

    rtl::OUString                                                      m_aType;
    rtl::OUString                                                      m_aName;
    rtl::OUString                                                      m_aUIName;
    com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > m_xUIElement;
    bool                                                               m_bFloating,
                                                                       m_bVisible,
                                                                       m_bUserActive,
                                                                       m_bCreateNewRowCol0,
                                                                       m_bDeactiveHide,
                                                                       m_bMasterHide,
                                                                       m_bContextSensitive,
                                                                       m_bContextActive;
    bool                                                               m_bNoClose,
                                                                       m_bSoftClose,
                                                                       m_bStateRead;
    sal_Int16                                                          m_nStyle;
    DockedData                                                         m_aDockedData;
    FloatingData                                                       m_aFloatingData;
};

typedef std::vector< UIElement > UIElementVector;

} // namespace framework

#endif // __FRAMEWORK_LAYOUTMANAGER_UIELEMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
