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
#include <dlgedfac.hxx>
#include <strings.hxx>
#include <RptObject.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/diagnose.h>

namespace rptui
{
using namespace ::com::sun::star;


DlgEdFactory::DlgEdFactory()
{
    SdrObjFactory::InsertMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}


DlgEdFactory::~DlgEdFactory() COVERITY_NOEXCEPT_FALSE
{
    SdrObjFactory::RemoveMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}


IMPL_STATIC_LINK(
    DlgEdFactory, MakeObject, SdrObjCreatorParams, aParams, rtl::Reference<SdrObject> )
{
    rtl::Reference<SdrObject> pNewObj;

    if ( aParams.nInventor == SdrInventor::ReportDesign )
    {
        switch( aParams.nObjIdentifier )
        {
            case SdrObjKind::ReportDesignFixedText:
                    pNewObj = new OUnoObject(aParams.rSdrModel
                                                        ,u"com.sun.star.form.component.FixedText"_ustr
                                                        ,SdrObjKind::ReportDesignFixedText);
                    break;
            case SdrObjKind::ReportDesignImageControl:
                    pNewObj = new OUnoObject(aParams.rSdrModel
                                                        ,u"com.sun.star.form.component.DatabaseImageControl"_ustr
                                                        ,SdrObjKind::ReportDesignImageControl);
                    break;
            case SdrObjKind::ReportDesignFormattedField:
                    pNewObj = new OUnoObject(aParams.rSdrModel
                                                        ,u"com.sun.star.form.component.FormattedField"_ustr
                                                        ,SdrObjKind::ReportDesignFormattedField);
                    break;
            case SdrObjKind::ReportDesignVerticalFixedLine:
            case SdrObjKind::ReportDesignHorizontalFixedLine:
                {
                    rtl::Reference<OUnoObject> pObj = new OUnoObject(aParams.rSdrModel
                                                        ,u"com.sun.star.awt.UnoControlFixedLineModel"_ustr
                                                        ,aParams.nObjIdentifier);
                    pNewObj = pObj;
                    if ( aParams.nObjIdentifier == SdrObjKind::ReportDesignHorizontalFixedLine )
                    {
                        uno::Reference<beans::XPropertySet> xProp = pObj->getAwtComponent();
                        xProp->setPropertyValue( PROPERTY_ORIENTATION, uno::Any(sal_Int32(0)) );
                    }
                }
                break;
            case SdrObjKind::CustomShape:
                pNewObj = new OCustomShape(aParams.rSdrModel);
                break;
            case SdrObjKind::ReportDesignSubReport:
                pNewObj = new OOle2Obj(aParams.rSdrModel, SdrObjKind::ReportDesignSubReport);
                break;
            case SdrObjKind::OLE2:
                pNewObj = new OOle2Obj(aParams.rSdrModel, SdrObjKind::OLE2);
                break;
            default:
                OSL_FAIL("Unknown object id");
                break;
        }
    }
    return pNewObj;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
