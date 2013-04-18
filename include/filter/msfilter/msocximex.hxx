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
#ifndef _MSOCXIMEX_HXX
#define _MSOCXIMEX_HXX

#include <sot/storage.hxx>

#include "filter/msfilter/msfilterdllapi.h"

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
    SvxMSConvertOCXControls( const  ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    virtual ~SvxMSConvertOCXControls();

    virtual sal_Bool InsertControl(
        const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent >& /*rFComp*/,
        const com::sun::star::awt::Size& /*rSize*/,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape >* /*pShape*/,
        sal_Bool /*bFloatingCtrl*/ ) {return sal_False;}

    /*begin: Backwards compatability with office 95 import, modify later*/
    const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > & GetServiceFactory();
protected:
    const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > &
        GetShapes();

    const com::sun::star::uno::Reference<
        com::sun::star::container::XIndexContainer > &  GetFormComps();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;

    // gecachte Interfaces
    com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >
        xDrawPage;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >
        xShapes;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >        xServiceFactory;

    // das einzige Formular
    com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >
        xFormComps;

    virtual const com::sun::star::uno::Reference<
        com::sun::star::drawing::XDrawPage > & GetDrawPage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
