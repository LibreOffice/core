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
#include "RptPage.hxx"
#include "RptModel.hxx"
#include "Section.hxx"
#include "RptObject.hxx"
#include <svx/unoapi.hxx>
#include <svx/unoshape.hxx>
#include "ReportDrawPage.hxx"

namespace rptui
{
using namespace ::com::sun::star;
TYPEINIT1( OReportPage, SdrPage );

//----------------------------------------------------------------------------
DBG_NAME( rpt_OReportPage )
OReportPage::OReportPage( OReportModel& _rModel
                         ,const uno::Reference< report::XSection >& _xSection
                         ,FASTBOOL bMasterPage )
    :SdrPage( _rModel, bMasterPage )
    ,rModel(_rModel)
    ,m_xSection(_xSection)
     ,m_bSpecialInsertMode(false)
{
    DBG_CTOR( rpt_OReportPage,NULL);
}

//----------------------------------------------------------------------------

OReportPage::OReportPage( const OReportPage& rPage )
    :SdrPage( rPage )
    ,rModel(rPage.rModel)
     ,m_xSection(rPage.m_xSection)
     ,m_bSpecialInsertMode(rPage.m_bSpecialInsertMode)
     ,m_aTemporaryObjectList(rPage.m_aTemporaryObjectList)
{
    DBG_CTOR( rpt_OReportPage,NULL);
}

//----------------------------------------------------------------------------

OReportPage::~OReportPage()
{
    DBG_DTOR( rpt_OReportPage,NULL);
}

//----------------------------------------------------------------------------

SdrPage* OReportPage::Clone() const
{
    DBG_CHKTHIS( rpt_OReportPage,NULL);
    return new OReportPage( *this );
}

//----------------------------------------------------------------------------
sal_uLong OReportPage::getIndexOf(const uno::Reference< report::XReportComponent >& _xObject)
{
    DBG_CHKTHIS( rpt_OReportPage,NULL);
    sal_uLong nCount = GetObjCount();
    sal_uLong i = 0;
    for (; i < nCount; ++i)
    {
        OObjectBase* pObj = dynamic_cast<OObjectBase*>(GetObj(i));
        OSL_ENSURE(pObj,"Invalid object found!");
        if ( pObj && pObj->getReportComponent() == _xObject )
        {
            break;
        }
    } // for (; i < nCount; ++i)
    return i;
}
//----------------------------------------------------------------------------
void OReportPage::removeSdrObject(const uno::Reference< report::XReportComponent >& _xObject)
{
    DBG_CHKTHIS( rpt_OReportPage,NULL);
    sal_uLong nPos = getIndexOf(_xObject);
    if ( nPos < GetObjCount() )
    {
        OObjectBase* pBase = dynamic_cast<OObjectBase*>(GetObj(nPos));
        OSL_ENSURE(pBase,"Why is this not a OObjectBase?");
        if ( pBase )
            pBase->EndListening();
        /*delete */RemoveObject(nPos);
    }
}
// -----------------------------------------------------------------------------
SdrObject* OReportPage::RemoveObject(sal_uLong nObjNum)
{
    SdrObject* pObj = SdrPage::RemoveObject(nObjNum);
    if (getSpecialMode())
    {
        return pObj;
    }

    // this code is evil, but what else shall I do
    reportdesign::OSection* pSection = reportdesign::OSection::getImplementation(m_xSection);
    uno::Reference< drawing::XShape> xShape(pObj->getUnoShape(),uno::UNO_QUERY);
    pSection->notifyElementRemoved(xShape);
    if (pObj->ISA(OUnoObject))
    {
        OUnoObject* pUnoObj = dynamic_cast<OUnoObject*>(pObj);
        uno::Reference< container::XChild> xChild(pUnoObj->GetUnoControlModel(),uno::UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(NULL);
    }
    return pObj;
}
//----------------------------------------------------------------------------
//namespace
//{
//  ::rtl::OUString lcl_getControlName(const uno::Reference< lang::XServiceInfo >& _xServiceInfo)
//  {
//      if ( _xServiceInfo->supportsService( SERVICE_FIXEDTEXT ))
//          return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FixedText"));
//        if ( _xServiceInfo->supportsService( SERVICE_FORMATTEDFIELD ))
//          return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FormattedField"));
//      if ( _xServiceInfo->supportsService( SERVICE_IMAGECONTROL))
//          return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.DatabaseImageControl"));
//
//      return ::rtl::OUString();
//  }
//}
//----------------------------------------------------------------------------
void OReportPage::insertObject(const uno::Reference< report::XReportComponent >& _xObject)
{
    DBG_CHKTHIS( rpt_OReportPage,NULL);
    OSL_ENSURE(_xObject.is(),"Object is not valid to create a SdrObject!");
    if ( !_xObject.is() ) // || !m_pView )
        return;
    sal_uLong nPos = getIndexOf(_xObject);
    if ( nPos < GetObjCount() )
        return; // Object already in list

    SvxShape* pShape = SvxShape::getImplementation( _xObject );
    OObjectBase* pObject = pShape ? dynamic_cast< OObjectBase* >( pShape->GetSdrObject() ) : NULL;
    OSL_ENSURE( pObject, "OReportPage::insertObject: no implementation object found for the given shape/component!" );
    if ( pObject )
        pObject->StartListening();
}
// -----------------------------------------------------------------------------
uno::Reference< report::XSection > OReportPage::getSection() const
{
    return m_xSection;
}
// -----------------------------------------------------------------------------
uno::Reference< uno::XInterface > OReportPage::createUnoPage()
{
    return static_cast<cppu::OWeakObject*>( new reportdesign::OReportDrawPage(this,m_xSection) );
}
// -----------------------------------------------------------------------------
void OReportPage::removeTempObject(SdrObject *_pToRemoveObj)
{
    if (_pToRemoveObj)
    {
        for (sal_uLong i=0;i<GetObjCount();i++)
        {
            SdrObject *aObj = GetObj(i);
            if (aObj && aObj == _pToRemoveObj)
            {
                SdrObject* pObject = RemoveObject(i);
                (void)pObject;
                break;
                // delete pObject;
            }
        }
    }
}

void OReportPage::resetSpecialMode()
{
    const sal_Bool bChanged = rModel.IsChanged();
    ::std::vector<SdrObject*>::iterator aIter = m_aTemporaryObjectList.begin();
    ::std::vector<SdrObject*>::iterator aEnd = m_aTemporaryObjectList.end();

    for (; aIter != aEnd; ++aIter)
    {
         removeTempObject(*aIter);
    }
    m_aTemporaryObjectList.clear();
    rModel.SetChanged(bChanged);

    m_bSpecialInsertMode = false;
}
// -----------------------------------------------------------------------------
void OReportPage::NbcInsertObject(SdrObject* pObj, sal_uLong nPos, const SdrInsertReason* pReason)
{
    SdrPage::NbcInsertObject(pObj, nPos, pReason);

    OUnoObject* pUnoObj = dynamic_cast< OUnoObject* >( pObj );
    if (getSpecialMode())
    {
        m_aTemporaryObjectList.push_back(pObj);
        return;
    }

    if ( pUnoObj )
    {
        pUnoObj->CreateMediator();
        uno::Reference< container::XChild> xChild(pUnoObj->GetUnoControlModel(),uno::UNO_QUERY);
        if ( xChild.is() && !xChild->getParent().is() )
            xChild->setParent(m_xSection);
    }

    // this code is evil, but what else shall I do
    reportdesign::OSection* pSection = reportdesign::OSection::getImplementation(m_xSection);
    uno::Reference< drawing::XShape> xShape(pObj->getUnoShape(),uno::UNO_QUERY);
    pSection->notifyElementAdded(xShape);

    //// check if we are a shape
    //uno::Reference<beans::XPropertySet> xProp(xShape,uno::UNO_QUERY);
    //if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID"))) )
    //{
    //    // use MimeConfigurationHelper::GetStringClassIDRepresentation(MimeConfigurationHelper::GetSequenceClassID(SO3_SCH_OLE_EMBED_CLASSID_8))
    //    xProp->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID")),uno::makeAny(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("12dcae26-281f-416f-a234-c3086127382e"))));
    //}

    // now that the shape is inserted into its structures, we can allow the OObjectBase
    // to release the reference to it
    OObjectBase* pObjectBase = dynamic_cast< OObjectBase* >( pObj );
    OSL_ENSURE( pObjectBase, "OReportPage::NbcInsertObject: what is being inserted here?" );
    if ( pObjectBase )
        pObjectBase->releaseUnoShape();
}
//============================================================================
} // rptui
//============================================================================
