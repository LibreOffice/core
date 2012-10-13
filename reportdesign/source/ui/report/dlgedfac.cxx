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
#include "dlgedfac.hxx"
#include "uistrings.hrc"
#include "RptObject.hxx"
#include <RptDef.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <svx/svdoole2.hxx>
namespace rptui
{
using namespace ::com::sun::star;

//----------------------------------------------------------------------------

DlgEdFactory::DlgEdFactory()
{
    SdrObjFactory::InsertMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}

//----------------------------------------------------------------------------

DlgEdFactory::~DlgEdFactory()
{
    SdrObjFactory::RemoveMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}

//----------------------------------------------------------------------------

IMPL_LINK( DlgEdFactory, MakeObject, SdrObjFactory *, pObjFactory )
{
    if ( pObjFactory->nInventor == ReportInventor )
    {
        switch( pObjFactory->nIdentifier )
        {
            case OBJ_DLG_FIXEDTEXT:
                    pObjFactory->pNewObj = new OUnoObject( SERVICE_FIXEDTEXT
                                                        ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FixedText"))
                                                        ,OBJ_DLG_FIXEDTEXT);
                    break;
            case OBJ_DLG_IMAGECONTROL:
                    pObjFactory->pNewObj = new OUnoObject( SERVICE_IMAGECONTROL
                                                        ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.DatabaseImageControl"))
                                                        ,OBJ_DLG_IMAGECONTROL);
                    break;
            case OBJ_DLG_FORMATTEDFIELD:
                    pObjFactory->pNewObj = new OUnoObject( SERVICE_FORMATTEDFIELD
                                                        ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FormattedField"))
                                                        ,OBJ_DLG_FORMATTEDFIELD);
                    break;
            case OBJ_DLG_VFIXEDLINE:
            case OBJ_DLG_HFIXEDLINE:
                {
                    OUnoObject* pObj = new OUnoObject( SERVICE_FIXEDLINE
                                                        ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedLineModel"))
                                                        ,pObjFactory->nIdentifier);
                    pObjFactory->pNewObj = pObj;
                    if ( pObjFactory->nIdentifier == OBJ_DLG_HFIXEDLINE )
                    {
                        uno::Reference<beans::XPropertySet> xProp = pObj->getAwtComponent();
                        xProp->setPropertyValue( PROPERTY_ORIENTATION, uno::makeAny(sal_Int32(0)) );
                    }
                }
                break;
            case OBJ_CUSTOMSHAPE:
                pObjFactory->pNewObj = new OCustomShape(SERVICE_SHAPE);
                break;
            case OBJ_DLG_SUBREPORT:
                pObjFactory->pNewObj = new OOle2Obj(SERVICE_REPORTDEFINITION,OBJ_DLG_SUBREPORT);
                break;
            case OBJ_OLE2:
                pObjFactory->pNewObj = new OOle2Obj(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.ChartDocument")),OBJ_OLE2);
                break;
            default:
                OSL_FAIL("Unknown object id");
                break;
        }
    }

    return 0;
}
//----------------------------------------------------------------------------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
