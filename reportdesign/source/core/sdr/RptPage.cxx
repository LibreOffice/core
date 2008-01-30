/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RptPage.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 16:00:26 $
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
#ifndef _REPORT_RPTUIPAGE_HXX
#include "RptPage.hxx"
#endif

#ifndef REPORT_RPTMODEL_HXX
#include "RptModel.hxx"
#endif
#ifndef REPORTDESIGN_API_SECTION_HXX
#include "Section.hxx"
#endif
#ifndef _REPORT_RPTUIOBJ_HXX
#include "RptObject.hxx"
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef REPORTDRAWPAGE_HXX_INCLUDED
#include "ReportDrawPage.hxx"
#endif

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
ULONG OReportPage::getIndexOf(const uno::Reference< report::XReportComponent >& _xObject)
{
    DBG_CHKTHIS( rpt_OReportPage,NULL);
    ULONG nCount = GetObjCount();
    ULONG i = 0;
    if ( nCount != CONTAINER_ENTRY_NOTFOUND )
    {
        for (; i < nCount; ++i)
        {
            OObjectBase* pObj = dynamic_cast<OObjectBase*>(GetObj(i));
            OSL_ENSURE(pObj,"Invalid object found!");
            if ( pObj && pObj->getReportComponent() == _xObject )
            {
                break;
            }
        } // for (; i < nCount; ++i)
    }
    return i;
}
//----------------------------------------------------------------------------
void OReportPage::removeSdrObject(const uno::Reference< report::XReportComponent >& _xObject)
{
    DBG_CHKTHIS( rpt_OReportPage,NULL);
    ULONG nPos = getIndexOf(_xObject);
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
SdrObject* OReportPage::RemoveObject(ULONG nObjNum)
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
    ULONG nPos = getIndexOf(_xObject);
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
        for (ULONG i=0;i<GetObjCount();i++)
        {
            SdrObject *aObj = GetObj(i);
            if (aObj && aObj == _pToRemoveObj)
            {
                SdrObject* pObject = RemoveObject(i);
                (void)pObject;
                // delete pObject;
            }
        }
    }
}

void OReportPage::resetSpecialMode()
{
    ::std::vector<SdrObject*>::iterator aIter = m_aTemporaryObjectList.begin();
    ::std::vector<SdrObject*>::iterator aEnd = m_aTemporaryObjectList.end();

    for (; aIter != aEnd; ++aIter)
    {
         removeTempObject(*aIter);
    }

    m_bSpecialInsertMode = false;
}
// -----------------------------------------------------------------------------
void OReportPage::NbcInsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* pReason)
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
