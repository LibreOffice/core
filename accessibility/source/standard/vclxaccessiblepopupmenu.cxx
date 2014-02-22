/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <accessibility/standard/vclxaccessiblepopupmenu.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::comphelper;






VCLXAccessiblePopupMenu::VCLXAccessiblePopupMenu( Menu* pMenu )
    :OAccessibleMenuComponent( pMenu )
{
}



VCLXAccessiblePopupMenu::~VCLXAccessiblePopupMenu()
{
}



sal_Bool VCLXAccessiblePopupMenu::IsFocused()
{
    return !IsChildHighlighted();
}





OUString VCLXAccessiblePopupMenu::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessiblePopupMenu" );
}



Sequence< OUString > VCLXAccessiblePopupMenu::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessiblePopupMenu";
    return aNames;
}





sal_Int32 VCLXAccessiblePopupMenu::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}



sal_Int16 VCLXAccessiblePopupMenu::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::POPUP_MENU;
}





sal_Int32 VCLXAccessiblePopupMenu::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetStyleSettings().GetMenuColor().GetColor();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
