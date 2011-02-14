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

#include "ReportDrawPage.hxx"
#include "RptObject.hxx"
#include "RptModel.hxx"
#include "RptDef.hxx"
#include "corestrings.hrc"
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/documentconstants.hxx>

#include <svx/svdmodel.hxx>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <tools/diagnose_ex.h>
#include <svx/unoshape.hxx>

namespace reportdesign
{
    using namespace ::com::sun::star;
    using namespace rptui;

OReportDrawPage::OReportDrawPage(SdrPage* _pPage
                                 ,const uno::Reference< report::XSection >& _xSection)
: SvxDrawPage(_pPage)
,m_xSection(_xSection)
{
}

SdrObject* OReportDrawPage::_CreateSdrObject( const uno::Reference< drawing::XShape > & xDescr ) throw ()
{
    uno::Reference< report::XReportComponent> xReportComponent(xDescr,uno::UNO_QUERY);
    if ( xReportComponent.is() )
        return OObjectBase::createObject(xReportComponent);
    return SvxDrawPage::_CreateSdrObject( xDescr );
}

uno::Reference< drawing::XShape >  OReportDrawPage::_CreateShape( SdrObject *pObj ) const throw ()
{
    OObjectBase* pBaseObj = dynamic_cast<OObjectBase*>(pObj);
    if ( !pBaseObj )
        return SvxDrawPage::_CreateShape( pObj );

    uno::Reference< report::XSection> xSection = m_xSection;
    uno::Reference< lang::XMultiServiceFactory> xFactory;
    if ( xSection.is() )
        xFactory.set(xSection->getReportDefinition(),uno::UNO_QUERY);
    uno::Reference< drawing::XShape > xRet;
    uno::Reference< drawing::XShape > xShape;
    if ( xFactory.is() )
    {
        bool bChangeOrientation = false;
        ::rtl::OUString sServiceName = pBaseObj->getServiceName();
        OSL_ENSURE(sServiceName.getLength(),"No Service Name given!");

        if ( pObj->ISA(OUnoObject) )
        {
            OUnoObject* pUnoObj = dynamic_cast<OUnoObject*>(pObj);
            if ( pUnoObj->GetObjIdentifier() == OBJ_DLG_FIXEDTEXT )
            {
                uno::Reference<beans::XPropertySet> xControlModel(pUnoObj->GetUnoControlModel(),uno::UNO_QUERY);
                if ( xControlModel.is() )
                    xControlModel->setPropertyValue( PROPERTY_MULTILINE,uno::makeAny(sal_True));
            }
            else
                bChangeOrientation = pUnoObj->GetObjIdentifier() == OBJ_DLG_HFIXEDLINE;
            SvxShapeControl* pShape = new SvxShapeControl( pObj );
            xShape.set(*pShape,uno::UNO_QUERY);
            pShape->setShapeKind(pObj->GetObjIdentifier());
        }
        else if ( pObj->ISA(OCustomShape) )
        {
            SvxCustomShape* pShape = new SvxCustomShape( pObj );
            uno::Reference < drawing::XEnhancedCustomShapeDefaulter > xShape2 = pShape;
            xShape.set(xShape2,uno::UNO_QUERY);
            pShape->setShapeKind(pObj->GetObjIdentifier());
        }
        else if ( pObj->ISA(SdrOle2Obj) )
        {
            SdrOle2Obj* pOle2Obj = dynamic_cast<SdrOle2Obj*>(pObj);
            if ( !pOle2Obj->GetObjRef().is() )
            {
                sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
                uno::Reference < embed::XEmbeddedObject > xObj;
                ::rtl::OUString sName;
                xObj = pObj->GetModel()->GetPersist()->getEmbeddedObjectContainer().CreateEmbeddedObject(
                    ::comphelper::MimeConfigurationHelper::GetSequenceClassIDRepresentation(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("80243D39-6741-46C5-926E-069164FF87BB"))), sName );
                OSL_ENSURE(xObj.is(),"Embedded Object could not be created!");

                /**************************************************
                * Das leere OLE-Objekt bekommt ein neues IPObj
                **************************************************/
                pObj->SetEmptyPresObj(sal_False);
                pOle2Obj->SetOutlinerParaObject(NULL);
                pOle2Obj->SetObjRef(xObj);
                pOle2Obj->SetPersistName(sName);
                pOle2Obj->SetName(sName);
                pOle2Obj->SetAspect(nAspect);
                Rectangle aRect = pOle2Obj->GetLogicRect();

                Size aTmp = aRect.GetSize();
                awt::Size aSz( aTmp.Width(), aTmp.Height() );
                xObj->setVisualAreaSize( nAspect, aSz );
            }
            SvxOle2Shape* pShape = new SvxOle2Shape( pObj );
            xShape.set(*pShape,uno::UNO_QUERY);
            pShape->setShapeKind(pObj->GetObjIdentifier());
            //xShape = new SvxOle2Shape( pOle2Obj );
        }

        if ( !xShape.is() )
            xShape.set( SvxDrawPage::_CreateShape( pObj ) );

        try
        {
            OReportModel* pRptModel = static_cast<OReportModel*>(pObj->GetModel());
            xRet.set( pRptModel->createShape(sServiceName,xShape,bChangeOrientation ? 0 : 1), uno::UNO_QUERY_THROW );
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return xRet;
}

}
