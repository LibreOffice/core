/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportDrawPage.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 18:58:55 $
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

#include "ReportDrawPage.hxx"
#include "RptObject.hxx"
#include "RptDef.hxx"
#include "corestrings.hrc"

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
    if ( xFactory.is() )
    {
        bool bChangeOrientation = false;
        ::rtl::OUString sServiceName;
        if ( pObj->ISA(OCustomShape) )
        {
            sServiceName = SERVICE_SHAPE;
        }
        else if ( pObj->ISA(OUnoObject) )
        {
            OUnoObject* pUnoObj = dynamic_cast<OUnoObject*>(pObj);
            switch(pUnoObj->getObjectId())
            {
                case OBJ_DLG_FORMATTEDFIELD:
                    sServiceName = SERVICE_FORMATTEDFIELD;
                    break;
                case OBJ_DLG_HFIXEDLINE:
                    sServiceName = SERVICE_FIXEDLINE;
                    bChangeOrientation = true;
                    break;
                case OBJ_DLG_VFIXEDLINE:
                    sServiceName = SERVICE_FIXEDLINE;
                    break;
                case OBJ_DLG_FIXEDTEXT:
                    sServiceName = SERVICE_FIXEDTEXT;
                    break;
                case OBJ_DLG_IMAGECONTROL:
                    sServiceName = SERVICE_IMAGECONTROL;
                    break;
                default:
                    OSL_ENSURE(0,"Illegal case value");
                    break;
            }
        }

        uno::Reference< drawing::XShape > xShape( SvxDrawPage::_CreateShape( pObj ) );

        try
        {
            uno::Sequence< uno::Any > aArgs(bChangeOrientation ? 2 : 1);
            {
                beans::NamedValue aValue;
                aValue.Name = PROPERTY_SHAPE;
                aValue.Value <<= xShape; xShape.clear();    // keep exactly *one* reference!
                aArgs[0] <<= aValue;
                if ( bChangeOrientation )
                {
                    aValue.Name = PROPERTY_ORIENTATION;
                    aValue.Value <<= sal_Int32(0);
                    aArgs[1] <<= aValue;
                }
            }
            xRet.set( xFactory->createInstanceWithArguments( sServiceName, aArgs ), uno::UNO_QUERY_THROW );
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return xRet;
}

}
