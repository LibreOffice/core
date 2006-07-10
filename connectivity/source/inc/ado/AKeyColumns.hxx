/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AKeyColumns.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:34:25 $
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

#ifndef _CONNECTIVITY_ADO_KEYCOLUMNS_HXX_
#define _CONNECTIVITY_ADO_KEYCOLUMNS_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif

namespace connectivity
{
    namespace ado
    {
        class OKeyColumns : public OColumns
        {
        protected:
            virtual ::com::sun::star::uno::Reference< starcontainer::XNamed > createObject(const ::rtl::OUString& _rName);
            virtual ::com::sun::star::uno::Reference< starbeans::XPropertySet > createDescriptor();
        public:
            OKeyColumns(    ::cppu::OWeakObject& _rParent,
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
#endif // _CONNECTIVITY_ADO_KEYCOLUMNS_HXX_

