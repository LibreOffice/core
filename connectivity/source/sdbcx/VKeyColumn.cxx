/*************************************************************************
 *
 *  $RCSfile: VKeyColumn.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 13:36:27 $
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

#ifndef _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_
#include "connectivity/sdbcx/VKeyColumn.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbtools
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity::dbtools;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace cppu;
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OKeyColumn::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VKeyColumnDescription");
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VKeyColumn");
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OKeyColumn::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.KeyColumnDescription");
    else
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.KeyColumn");

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeyColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;

    return sal_False;
}
// -------------------------------------------------------------------------
OKeyColumn::OKeyColumn(sal_Bool _bCase) : OColumn(_bCase)
{
    construct();
}
// -------------------------------------------------------------------------
OKeyColumn::OKeyColumn( const ::rtl::OUString&  _ReferencedColumn,
                        const ::rtl::OUString&  _Name,
                        const ::rtl::OUString&  _TypeName,
                        const ::rtl::OUString&  _DefaultValue,
                        sal_Int32               _IsNullable,
                        sal_Int32               _Precision,
                        sal_Int32               _Scale,
                        sal_Int32               _Type,
                        sal_Bool                _IsAutoIncrement,
                        sal_Bool                _IsRowVersion,
                        sal_Bool                _IsCurrency,
                        sal_Bool                _bCase
                        ) : OColumn(_Name,
                            _TypeName,
                            _DefaultValue,
                            _IsNullable,
                            _Precision,
                            _Scale,
                            _Type,
                            _IsAutoIncrement,
                            _IsRowVersion,
                            _IsCurrency,
                            _bCase)
                        ,   m_ReferencedColumn(_ReferencedColumn)
{
    construct();
}
// -------------------------------------------------------------------------
OKeyColumn::~OKeyColumn()
{
}
// -------------------------------------------------------------------------
void OKeyColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;
    registerProperty(PROPERTY_REFERENCEDCOLUMN, PROPERTY_ID_REFERENCEDCOLUMN,   nAttrib,&m_ReferencedColumn,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}



