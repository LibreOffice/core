#ifndef _REPORT_RPTUIDEF_HXX
#define _REPORT_RPTUIDEF_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RptDef.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:17:41 $
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


#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#include <svx/fmglob.hxx>
#include "dllapi.h"

FORWARD_DECLARE_INTERFACE(report,XReportComponent)
FORWARD_DECLARE_INTERFACE(report,XReportDefinition)
FORWARD_DECLARE_INTERFACE(style,XStyle)
FORWARD_DECLARE_INTERFACE(beans,XPropertySet)

namespace rptui
{

const UINT32 ReportInventor = UINT32('R')*0x00000001+
                           UINT32('P')*0x00000100+
                           UINT32('T')*0x00010000+
                           UINT32('1')*0x01000000;


#define OBJ_DLG_FIXEDTEXT       ((UINT16) OBJ_MAXI + 1)
#define OBJ_DLG_IMAGECONTROL    OBJ_FM_IMAGECONTROL
#define OBJ_DLG_FORMATTEDFIELD  ((UINT16) OBJ_MAXI + 3)
#define OBJ_DLG_HFIXEDLINE      ((UINT16) OBJ_MAXI + 4)
#define OBJ_DLG_VFIXEDLINE      ((UINT16) OBJ_MAXI + 5)
#define OBJ_DLG_SUBREPORT       ((UINT16) OBJ_MAXI + 6)

// allows the alignment and resizing of controls
namespace ControlModification
{
    static const ::sal_Int32 NONE               = (sal_Int32)0;
    static const ::sal_Int32 LEFT               = (sal_Int32)1;
    static const ::sal_Int32 RIGHT              = (sal_Int32)2;
    static const ::sal_Int32 TOP                = (sal_Int32)3;
    static const ::sal_Int32 BOTTOM             = (sal_Int32)4;
    static const ::sal_Int32 CENTER_HORIZONTAL  = (sal_Int32)5;
    static const ::sal_Int32 CENTER_VERTICAL    = (sal_Int32)6;
    static const ::sal_Int32 WIDTH_SMALLEST     = (sal_Int32)7;
    static const ::sal_Int32 HEIGHT_SMALLEST    = (sal_Int32)8;
    static const ::sal_Int32 WIDTH_GREATEST     = (sal_Int32)9;
    static const ::sal_Int32 HEIGHT_GREATEST    = (sal_Int32)10;
}

/** returns teh object type depending on the service name
    @param  _xComponent the report component
*/
REPORTDESIGN_DLLPUBLIC sal_uInt16 getObjectType(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent);

DECLARE_STL_USTRINGACCESS_MAP(::rtl::OUString , TPropertyNamePair);
/** returns teh property name map for the givern property id
    @param  _nObjectId  the object id
*/
REPORTDESIGN_DLLPUBLIC const TPropertyNamePair& getPropertyNameMap(sal_uInt16 _nObjectId);
REPORTDESIGN_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle> getUsedStyle(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xReport);

// -----------------------------------------------------------------------------
template < typename T> T getStyleProperty(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xReport,const ::rtl::OUString& _sPropertyName)
{
    T nReturn = T();
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xProp(getUsedStyle(_xReport),::com::sun::star::uno::UNO_QUERY_THROW);
    xProp->getPropertyValue(_sPropertyName) >>= nReturn;
    return nReturn;
}
// -----------------------------------------------------------------------------
template<typename T> void setStyleProperty(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xReport,const ::rtl::OUString& _sPropertyName,const T& _aValue)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xProp(getUsedStyle(_xReport),::com::sun::star::uno::UNO_QUERY);
    if ( xProp.is() )
        xProp->setPropertyValue(_sPropertyName,::com::sun::star::uno::makeAny(_aValue));
}
}

#endif // _REPORT_RPTUIDEF_HXX
