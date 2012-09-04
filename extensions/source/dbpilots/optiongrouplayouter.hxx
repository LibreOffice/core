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

#ifndef _EXTENSIONS_DBP_OPTIONGROUPLAYOUTER_HXX_
#define _EXTENSIONS_DBP_OPTIONGROUPLAYOUTER_HXX_

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "dbptypes.hxx"

//.........................................................................
namespace dbp
{
//.........................................................................

    struct OControlWizardContext;
    struct OOptionGroupSettings;
    //=====================================================================
    //= OOptionGroupLayouter
    //=====================================================================
    class OOptionGroupLayouter
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                    mxContext;

    public:
        OOptionGroupLayouter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
            );

    public:
        void    doLayout(
            const OControlWizardContext& _rContext,
            const OOptionGroupSettings& _rSettings
        );

    protected:
        void    implAnchorShape(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxShapeProps
            );
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_OPTIONGROUPLAYOUTER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
