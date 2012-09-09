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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUBAR_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUBAR_HXX

#include <accessibility/standard/accessiblemenucomponent.hxx>

class VclSimpleEvent;
class VclWindowEvent;
class Window;


//  ----------------------------------------------------
//  class VCLXAccessibleMenuBar
//  ----------------------------------------------------

class VCLXAccessibleMenuBar :   public OAccessibleMenuComponent
{
protected:
    Window*                 m_pWindow;

    virtual sal_Bool        IsFocused();

    DECL_LINK( WindowEventListener, VclSimpleEvent* );

    virtual void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL   disposing();

public:
    VCLXAccessibleMenuBar( Menu* pMenu );
    virtual ~VCLXAccessibleMenuBar();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleExtendedComponent
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
