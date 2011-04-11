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
#include "precompiled_reportdesign.hxx"
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
