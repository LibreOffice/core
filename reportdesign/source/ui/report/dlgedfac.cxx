/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedfac.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:13:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"

#ifndef _REPORT_RPTUIFAC_HXX
#include "dlgedfac.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif

#ifndef _REPORT_RPTUIOBJ_HXX
#include "RptObject.hxx"
#endif

#ifndef _REPORT_RPTUIDEF_HXX
#include <RptDef.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif
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
                pObjFactory->pNewObj = new OOle2Obj(SERVICE_REPORTDEFINITION);
                break;
            case OBJ_OLE2:
                pObjFactory->pNewObj = new OOle2Obj(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.ChartDocument")));
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
