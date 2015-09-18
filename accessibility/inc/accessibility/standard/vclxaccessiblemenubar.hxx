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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUBAR_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUBAR_HXX

#include <accessibility/standard/accessiblemenucomponent.hxx>
#include <vcl/vclptr.hxx>

class VclSimpleEvent;
class VclWindowEvent;
namespace vcl { class Window; }



//  class VCLXAccessibleMenuBar


class VCLXAccessibleMenuBar :   public OAccessibleMenuComponent
{
protected:
    VclPtr<vcl::Window>     m_pWindow;

    virtual bool            IsFocused() SAL_OVERRIDE;

    DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );

    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL   disposing() SAL_OVERRIDE;

public:
    VCLXAccessibleMenuBar( Menu* pMenu );
    virtual ~VCLXAccessibleMenuBar();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleExtendedComponent
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
