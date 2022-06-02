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

#pragma once

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <rtl/ustring.hxx>
#include <utility>
#include <vcl/toolbox.hxx>

namespace framework
{

struct DockedData
{
    DockedData() : m_aPos( SAL_MAX_INT32, SAL_MAX_INT32 ),
                   m_nDockedArea( css::ui::DockingArea_DOCKINGAREA_TOP ),
                   m_bLocked( false ) {}

    css::awt::Point      m_aPos;
    css::ui::DockingArea m_nDockedArea;
    bool                 m_bLocked;
};

struct FloatingData
{
    FloatingData() : m_aPos( SAL_MAX_INT32, SAL_MAX_INT32 ),
                     m_nLines( 1 ),
                     m_bIsHorizontal( true ) {}

    css::awt::Point  m_aPos;
    css::awt::Size   m_aSize;
    ToolBox::ImplToolItems::size_type m_nLines;
    bool             m_bIsHorizontal;
};

struct UIElement
{
    UIElement() : m_bFloating( false ),
                  m_bVisible( true ),
                  m_bUserActive( false ),
                  m_bMasterHide( false ),
                  m_bContextSensitive( false ),
                  m_bNoClose( false ),
                  m_bStateRead( false ),
                  m_nStyle( ButtonType::SYMBOLONLY )
                  {}

    UIElement( OUString aName,
               OUString aType,
               css::uno::Reference< css::ui::XUIElement > xUIElement
               ) : m_aType(std::move( aType )),
                   m_aName(std::move( aName )),
                   m_xUIElement(std::move( xUIElement )),
                   m_bFloating( false ),
                   m_bVisible( true ),
                   m_bUserActive( false ),
                   m_bMasterHide( false ),
                   m_bContextSensitive( false ),
                   m_bNoClose( false ),
                   m_bStateRead( false ),
                   m_nStyle( ButtonType::SYMBOLONLY ) {}

    bool operator< ( const UIElement& aUIElement ) const;

    OUString                                                      m_aType;
    OUString                                                      m_aName;
    OUString                                                      m_aUIName;
    css::uno::Reference< css::ui::XUIElement >                    m_xUIElement;
    bool                                                               m_bFloating,
                                                                       m_bVisible,
                                                                       m_bUserActive,
                                                                       m_bMasterHide,
                                                                       m_bContextSensitive;
    bool                                                               m_bNoClose,
                                                                       m_bStateRead;
    ButtonType                                                         m_nStyle;
    DockedData                                                         m_aDockedData;
    FloatingData                                                       m_aFloatingData;
};

typedef std::vector< UIElement > UIElementVector;

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
