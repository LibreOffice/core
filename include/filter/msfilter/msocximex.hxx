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
#ifndef INCLUDED_FILTER_MSFILTER_MSOCXIMEX_HXX
#define INCLUDED_FILTER_MSFILTER_MSOCXIMEX_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <filter/msfilter/msfilterdllapi.h>

namespace com{namespace sun{namespace star{
        namespace drawing{
                class XDrawPage;
                class XShape;
                class XShapes;
        }
        namespace form{
                class XFormComponent;
        }
        namespace lang{
                class XMultiServiceFactory;
        }
        namespace container{
                class XIndexContainer;
                class XNameContainer;
        }
        namespace beans{
                class XPropertySet;
        }
        namespace text{
                class XText;
        }
        namespace awt{
                struct Size;
                class XControlModel;
        }
        namespace uno{
                class XComponentContext;
        }
        namespace frame{
                class XModel;
        }

}}}

class SfxObjectShell;

class MSFILTER_DLLPUBLIC SvxMSConvertOCXControls
{
public:
    SvxMSConvertOCXControls( const  css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~SvxMSConvertOCXControls();

    virtual bool InsertControl(
        const css::uno::Reference<
        css::form::XFormComponent >& /*rFComp*/,
        const css::awt::Size& /*rSize*/,
        css::uno::Reference<
        css::drawing::XShape >* /*pShape*/,
        bool /*bFloatingCtrl*/ ) {return false;}

    /*begin: Backwards compatibility with office 95 import, modify later*/
    const css::uno::Reference< css::lang::XMultiServiceFactory > & GetServiceFactory();
protected:
    const css::uno::Reference< css::drawing::XShapes > &  GetShapes();

    const css::uno::Reference< css::container::XIndexContainer > &  GetFormComps();

    css::uno::Reference< css::frame::XModel >               mxModel;

    // gecachte Interfaces
    css::uno::Reference< css::drawing::XDrawPage >          xDrawPage;
    css::uno::Reference< css::drawing::XShapes >            xShapes;
    css::uno::Reference< css::lang::XMultiServiceFactory >  xServiceFactory;

    // das einzige Formular
    css::uno::Reference< css::container::XIndexContainer >  xFormComps;

    virtual const css::uno::Reference< css::drawing::XDrawPage > & GetDrawPage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
