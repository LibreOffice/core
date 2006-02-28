 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NColumns.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-02-28 10:33:06 $
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
 *    Contributor(s): Jayant Madavi_______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_EVOAB_COLUMNS_HXX_
#define _CONNECTIVITY_EVOAB_COLUMNS_HXX_

#ifndef _CONNECTIVITY_EVOAB_TABLE_HXX_
#include "NTable.hxx"
#endif

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif

namespace connectivity
{
    namespace evoab
    {
        class OEvoabColumns : public sdbcx::OCollection
        {
        protected:
            OEvoabTable*    m_pTable;

            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);

        public:
            OEvoabColumns(  OEvoabTable* _pTable,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector
                        ) : sdbcx::OCollection(*_pTable,sal_True,_rMutex,_rVector),
                            m_pTable(_pTable)
            { }
        };
    }
}

#endif // _CONNECTIVITY_EVOAB_COLUMNS_HXX_
