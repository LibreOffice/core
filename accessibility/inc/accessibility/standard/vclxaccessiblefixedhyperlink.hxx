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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDHYPERLINK_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDHYPERLINK_HXX

#include <accessibility/standard/vclxaccessibletextcomponent.hxx>

//  ----------------------------------------------------
//  class VCLXAccessibleFixedHyperlink
//  ----------------------------------------------------

class VCLXAccessibleFixedHyperlink : public VCLXAccessibleTextComponent
{
protected:
    virtual ~VCLXAccessibleFixedHyperlink();

    virtual void FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );
    virtual void implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );

public:
    VCLXAccessibleFixedHyperlink( VCLXWindow* pVCLXindow );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDHYPERLINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
