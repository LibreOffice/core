/*************************************************************************
 *
 *  $RCSfile: FieldDescriptions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:54:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

//========================================================================
// class OFieldDescription
//========================================================================
DBG_NAME(OFieldDescription);
//------------------------------------------------------------------------------
OFieldDescription::OFieldDescription() :
    m_bIsPrimaryKey(sal_False)
    ,m_nFormatKey(0)
    ,m_bIsAutoIncrement(sal_False)
    ,m_eHorJustify(SVX_HOR_JUSTIFY_STANDARD)
    ,m_nScale(0)
    ,m_nPrecision(0)
    ,m_pType(NULL)
    ,m_nIsNullable(ColumnValue::NULLABLE)
    ,m_nType(DataType::VARCHAR)
{
    DBG_CTOR(OFieldDescription,NULL);
}

//------------------------------------------------------------------------------
OFieldDescription::OFieldDescription( const OFieldDescription& rDescr ) :
     m_sName(rDescr.m_sName)
    ,m_sTypeName(rDescr.m_sTypeName)
    ,m_sDescription(rDescr.m_sDescription)
    ,m_sDefaultValue(rDescr.m_sDefaultValue)
    ,m_pType(rDescr.m_pType)
    ,m_nPrecision(rDescr.m_nPrecision)
    ,m_nScale(rDescr.m_nScale)
    ,m_nIsNullable(rDescr.m_nIsNullable)
    ,m_nFormatKey(rDescr.m_nFormatKey)
    ,m_eHorJustify(rDescr.m_eHorJustify)
    ,m_bIsAutoIncrement(rDescr.m_bIsAutoIncrement)
    ,m_bIsPrimaryKey(rDescr.m_bIsPrimaryKey)
    ,m_nType(DataType::VARCHAR)
{
    DBG_CTOR(OFieldDescription,NULL);
}

//------------------------------------------------------------------------------
OFieldDescription::~OFieldDescription()
{
    DBG_DTOR(OFieldDescription,NULL);
}
//------------------------------------------------------------------------------
OFieldDescription::OFieldDescription(const Reference< XPropertySet >& xAffectedCol)
    :m_bIsPrimaryKey(sal_False)
    ,m_nFormatKey(0)
    ,m_bIsAutoIncrement(sal_False)
    ,m_eHorJustify(SVX_HOR_JUSTIFY_STANDARD)
    ,m_nScale(0)
    ,m_nPrecision(0)
    ,m_pType(NULL)
    ,m_nIsNullable(ColumnValue::NULLABLE)
    ,m_nType(DataType::VARCHAR)
{
    DBG_CTOR(OFieldDescription,NULL);
    OSL_ENSURE(xAffectedCol.is(),"PropetySet can notbe null!");
    Reference<XPropertySetInfo> xPropSetInfo = xAffectedCol->getPropertySetInfo();
    if(xPropSetInfo->hasPropertyByName(PROPERTY_NAME))
        SetName(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_NAME)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_DESCRIPTION))
        SetDescription(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_DESCRIPTION)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_DEFAULTVALUE))
        SetDefaultValue(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_DEFAULTVALUE)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_TYPE))
        SetTypeValue(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_TYPE)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_PRECISION))
        SetPrecision(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_PRECISION)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_SCALE))
        SetScale(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_SCALE)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_ISNULLABLE))
        SetIsNullable(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_ISNULLABLE)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_FORMATKEY))
        SetFormatKey(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_FORMATKEY)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_ALIGN))
        SetHorJustify((SvxCellHorJustify)::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_ALIGN)));
    if(xPropSetInfo->hasPropertyByName(PROPERTY_ISAUTOINCREMENT))
        SetAutoIncrement(::cppu::any2bool(xAffectedCol->getPropertyValue(PROPERTY_ISAUTOINCREMENT)));
}
// -----------------------------------------------------------------------------




