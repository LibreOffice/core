/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "precompiled_rptui.hxx"
#include "dlgedfac.hxx"
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
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
                OSL_ENSURE(0,"Unknown object id");
                break;
        }
    }

    return 0;
}
//----------------------------------------------------------------------------
}
