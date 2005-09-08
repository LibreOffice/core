/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AutoRetrievingBase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:08:00 $
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

#ifndef _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_
#include "AutoRetrievingBase.hxx"
#endif

namespace connectivity
{
    ::rtl::OUString OAutoRetrievingBase::getTransformedGeneratedStatement(const ::rtl::OUString& _sInsertStatement) const
    {
        ::rtl::OUString sStmt = _sInsertStatement;
        OSL_ENSURE( m_bAutoRetrievingEnabled,"Illegal call here. isAutoRetrievingEnabled is false!");
        sStmt = sStmt.toAsciiUpperCase();
        ::rtl::OUString sStatement;
        if ( sStmt.compareToAscii("INSERT",6) == 0 )
        {
            sStatement = m_sGeneratedValueStatement;
            static const ::rtl::OUString sColumn(RTL_CONSTASCII_USTRINGPARAM("$column"));
            static const ::rtl::OUString sTable(RTL_CONSTASCII_USTRINGPARAM("$table"));
            sal_Int32 nIndex = 0;
            nIndex = sStatement.indexOf(sColumn,nIndex);
            if ( -1 != nIndex )
            { // we need a column
            }
            nIndex = 0;
            nIndex = sStatement.indexOf(sTable,nIndex);
            if ( -1 != nIndex )
            { // we need a table name
                sal_Int32 nIntoIndex = sStmt.indexOf(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INTO ")));
                sStmt = sStmt.copy(nIntoIndex+5);
                do
                {
                    if ( sStmt.indexOf(' ') == 0 )
                        sStmt = sStmt.copy(1);
                }
                while (sStmt.indexOf(' ') == 0 );

                nIntoIndex = 0;
                ::rtl::OUString sTableName = sStmt.getToken(0,' ',nIntoIndex);
                sStatement = sStatement.replaceAt(nIndex,sTable.getLength(),sTableName);
            }
        }
        return sStatement;
    }
}

