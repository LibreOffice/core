/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrapTable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:57:46 $
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
#ifndef CONNECTIVITY_ADO_WRAP_TABLE_HXX
#define CONNECTIVITY_ADO_WRAP_TABLE_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOCatalog;

        class WpADOTable : public WpOLEBase<_ADOTable>
        {
        public:
            WpADOTable(_ADOTable* pInt=NULL)    :   WpOLEBase<_ADOTable>(pInt){}
            WpADOTable(const WpADOTable& rhs){operator=(rhs);}

            inline WpADOTable& operator=(const WpADOTable& rhs)
                {WpOLEBase<_ADOTable>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const;
            void            put_Name(const ::rtl::OUString& _rName);
            ::rtl::OUString get_Type() const;
            WpADOColumns    get_Columns() const;
            WpADOIndexes    get_Indexes() const;
            WpADOKeys       get_Keys() const;
            WpADOCatalog    get_ParentCatalog() const;
            WpADOProperties get_Properties() const;
            void            putref_ParentCatalog(/* [in] */ _ADOCatalog __RPC_FAR *ppvObject);
        };


        typedef WpOLEAppendCollection<ADOTables,    _ADOTable,  WpADOTable>     WpADOTables;
    }
}

#endif //CONNECTIVITY_ADO_WRAP_TABLE_HXX
