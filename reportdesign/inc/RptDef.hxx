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

#ifndef _REPORT_RPTUIDEF_HXX
#define _REPORT_RPTUIDEF_HXX

#include "dllapi.h"
#include <tools/solar.h>
#include <svx/svdobj.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/uno3.hxx>
#include <svx/fmglob.hxx>
#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star {
    namespace report {
        class XReportComponent;
        class XReportDefinition;
    }
    namespace style {
        class XStyle;
    }
    namespace beans {
        class XPropertySet;
    }
}}}

namespace rptui
{
// not all used at the moment
#define RPT_LAYER_FRONT         0
#define RPT_LAYER_BACK          1
#define RPT_LAYER_HIDDEN        2


const sal_uInt32 ReportInventor = sal_uInt32('R')*0x00000001+
                           sal_uInt32('P')*0x00000100+
                           sal_uInt32('T')*0x00010000+
                           sal_uInt32('1')*0x01000000;


#define OBJ_DLG_FIXEDTEXT       ((sal_uInt16) OBJ_MAXI + 1)
#define OBJ_DLG_IMAGECONTROL    OBJ_FM_IMAGECONTROL
#define OBJ_DLG_FORMATTEDFIELD  ((sal_uInt16) OBJ_MAXI + 3)
#define OBJ_DLG_HFIXEDLINE      ((sal_uInt16) OBJ_MAXI + 4)
#define OBJ_DLG_VFIXEDLINE      ((sal_uInt16) OBJ_MAXI + 5)
#define OBJ_DLG_SUBREPORT       ((sal_uInt16) OBJ_MAXI + 6)

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

class AnyConverter : public ::std::binary_function< ::rtl::OUString,::com::sun::star::uno::Any,::com::sun::star::uno::Any >
{
public:
    virtual ~AnyConverter(){}
    virtual ::com::sun::star::uno::Any operator() (const ::rtl::OUString& /*_sPropertyName*/,const ::com::sun::star::uno::Any& lhs) const
    {
        return lhs;
    }
};
/** returns teh object type depending on the service name
    @param  _xComponent the report component
*/
REPORTDESIGN_DLLPUBLIC sal_uInt16 getObjectType(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent);
typedef ::std::pair< ::rtl::OUString, ::boost::shared_ptr<AnyConverter> > TPropertyConverter;
DECLARE_STL_USTRINGACCESS_MAP(TPropertyConverter , TPropertyNamePair);
/** returns the property name map for the givern property id
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
