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

#ifndef INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_OPTIONGROUPLAYOUTER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_OPTIONGROUPLAYOUTER_HXX

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "dbptypes.hxx"


namespace dbp
{


    struct OControlWizardContext;
    struct OOptionGroupSettings;

    class OOptionGroupLayouter
    {
        css::uno::Reference< css::uno::XComponentContext >
                    mxContext;

    public:
        explicit OOptionGroupLayouter(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
            );

    public:
        void    doLayout(
            const OControlWizardContext& _rContext,
            const OOptionGroupSettings& _rSettings
        );

    protected:
        static void    implAnchorShape(
            const css::uno::Reference< css::beans::XPropertySet >& _rxShapeProps
            );
    };


}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_OPTIONGROUPLAYOUTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
