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

#ifndef __FRAMEWORK_LAYOUTMANAGER_UIELEMENT_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_UIELEMENT_HXX_

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <rtl/ustring.hxx>
#include <vcl/toolbox.hxx>

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

    UIElement( const OUString& rName,
               const OUString& rType,
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

    OUString                                                      m_aType;
    OUString                                                      m_aName;
    OUString                                                      m_aUIName;
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
