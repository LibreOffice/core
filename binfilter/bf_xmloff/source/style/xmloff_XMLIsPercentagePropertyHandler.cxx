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

#ifndef _TOOLS_DEBUG_HXX 
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif


#ifndef _XMLOFF_XMLISPERCENTAGEPROPERTYHANDLER_HXX
#include "XMLIsPercentagePropertyHandler.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

XMLIsPercentagePropertyHandler::~XMLIsPercentagePropertyHandler()
{
}

sal_Bool XMLIsPercentagePropertyHandler::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    rValue <<= (sal_Bool)(rStrImpValue.indexOf( sal_Unicode('%') ) != -1);
    return sal_True;
}

sal_Bool XMLIsPercentagePropertyHandler::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ERROR( "XMLIsPercentagePropertyHandler is not for export!" ); 
    return sal_False;
}

}//end of namespace binfilter
