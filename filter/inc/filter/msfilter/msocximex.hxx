/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _MSOCXIMEX_HXX
#define _MSOCXIMEX_HXX

#include <sot/storage.hxx>
#include <tools/debug.hxx>

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
