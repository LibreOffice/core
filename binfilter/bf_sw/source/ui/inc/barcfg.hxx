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
#ifndef SW_BARCFG_HXX
#define SW_BARCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
namespace binfilter {

class CfgUSHORTTable;

class SwToolbarConfigItem : public ::utl::ConfigItem
{
    sal_uInt16 			aTbxIdArray[5];

    ::com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
    SwToolbarConfigItem( sal_Bool bWeb );
    ~SwToolbarConfigItem();


    void		SetTopToolbar( sal_Int32 nSelType, sal_uInt16 nBarId );
    sal_uInt16 	GetTopToolbar( sal_Int32 nSelType );	//USHRT_MAX: noch nicht eingetragen
};

} //namespace binfilter
#endif
