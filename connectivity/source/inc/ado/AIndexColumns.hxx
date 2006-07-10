/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AIndexColumns.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:34:03 $
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

#ifndef _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_
#define _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_

#ifndef _CONNECTIVITY_ADABAS_COLUMNS_HXX_
#include "adabas/BColumns.hxx"
#endif

namespace connectivity
{
    namespace adabas
    {
        class OIndexColumns : public OColumns
        {
        protected:
            virtual ::com::sun::star::uno::Reference< starcontainer::XNamed > createObject(const ::rtl::OUString& _rName);
            virtual ::com::sun::star::uno::Reference< starbeans::XPropertySet > createDescriptor();
        public:
            OIndexColumns(  ::cppu::OWeakObject& _rParent,
                            ::osl::Mutex& _rMutex,
                            const TStringVector &_rVector,
                            OTable* _pTable,
                            connectivity::sdbcx::IRefreshableColumns*   _pParent)
                        : OColumns(_rParent,_rMutex,_rVector,_pTable,_pParent)
            {
            }
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_

