/*************************************************************************
 *
 *  $RCSfile: PColumn.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-30 07:47:10 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/PColumn.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbtools
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity;
using namespace dbtools;
using namespace connectivity::parse;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
// -------------------------------------------------------------------------
OParseColumn::OParseColumn(const Reference<XPropertySet>& _xColumn,sal_Bool     _bCase)
    : connectivity::sdbcx::OColumn( getString(_xColumn->getPropertyValue(PROPERTY_NAME))
                                ,   getString(_xColumn->getPropertyValue(PROPERTY_TYPENAME))
                                ,   getString(_xColumn->getPropertyValue(PROPERTY_DEFAULTVALUE))
                                ,   getINT32(_xColumn->getPropertyValue(PROPERTY_ISNULLABLE))
                                ,   getINT32(_xColumn->getPropertyValue(PROPERTY_PRECISION))
                                ,   getINT32(_xColumn->getPropertyValue(PROPERTY_SCALE))
                                ,   getINT32(_xColumn->getPropertyValue(PROPERTY_TYPE))
                                ,   getBOOL(_xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT))
                                ,   sal_False
                                ,   getBOOL(_xColumn->getPropertyValue(PROPERTY_ISCURRENCY))
                                ,   _bCase
                                )
    , m_bFunction(sal_False)
    , m_bDbasePrecisionChanged(sal_False)
{
    construct();
}
// -------------------------------------------------------------------------
OParseColumn::OParseColumn( const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _TypeName,
                    const ::rtl::OUString& _DefaultValue,
                    sal_Int32       _IsNullable,
                    sal_Int32       _Precision,
                    sal_Int32       _Scale,
                    sal_Int32       _Type,
                    sal_Bool        _IsAutoIncrement,
                    sal_Bool        _IsCurrency,
                    sal_Bool        _bCase
                ) : connectivity::sdbcx::OColumn(_Name,
                                  _TypeName,
                                  _DefaultValue,
                                  _IsNullable,
                                  _Precision,
                                  _Scale,
                                  _Type,
                                  _IsAutoIncrement,
                                  sal_False,
                                  _IsCurrency,
                                  _bCase)
    , m_bFunction(sal_False)
    , m_bDbasePrecisionChanged(sal_False)
{
    construct();
}
// -------------------------------------------------------------------------
OParseColumn::~OParseColumn()
{
}
// -------------------------------------------------------------------------
void OParseColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(PROPERTY_FUNCTION,             PROPERTY_ID_FUNCTION,           nAttrib,&m_bFunction,       ::getCppuType(reinterpret_cast< sal_Bool*>(NULL)));
    registerProperty(PROPERTY_TABLENAME,            PROPERTY_ID_TABLENAME,          nAttrib,&m_aTableName,      ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_REALNAME,             PROPERTY_ID_REALNAME,           nAttrib,&m_aRealName,       ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_DBASEPRECISIONCHANGED,    PROPERTY_ID_DBASEPRECISIONCHANGED,          nAttrib,&m_bDbasePrecisionChanged,      ::getCppuType(reinterpret_cast<sal_Bool*>(NULL)));

}

