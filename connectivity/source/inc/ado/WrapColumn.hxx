/*************************************************************************
 *
 *  $RCSfile: WrapColumn.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2002-01-18 16:13:03 $
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
#ifndef CONNECTIVITY_ADO_WRAP_COLUMN_HXX
#define CONNECTIVITY_ADO_WRAP_COLUMN_HXX

#ifndef _CONNECTIVITY_ADO_AOLEWRAP_HXX_
#include "ado/Aolewrap.hxx"
#endif

#include <adoint.h>

namespace connectivity
{
    namespace ado
    {
        class WpADOColumn : public WpOLEBase<_ADOColumn>
        {
        public:
            WpADOColumn(_ADOColumn* pInt=NULL)  :   WpOLEBase<_ADOColumn>(pInt){}
            WpADOColumn(const WpADOColumn& rhs){operator=(rhs);}

            void Create();

            inline WpADOColumn& operator=(const WpADOColumn& rhs)
                {WpOLEBase<_ADOColumn>::operator=(rhs); return *this;}

            ::rtl::OUString get_Name() const;
            ::rtl::OUString get_RelatedColumn() const;
            void put_Name(const ::rtl::OUString& _rName);
            void put_RelatedColumn(const ::rtl::OUString& _rName);
            DataTypeEnum get_Type() const;
            void put_Type(const DataTypeEnum& _eNum) ;
            sal_Int32 get_Precision() const;
            void put_Precision(sal_Int32 _nPre) ;
            sal_Int32 get_NumericScale() const;
            void put_NumericScale(sal_Int8 _nScale);
            SortOrderEnum get_SortOrder() const;
            void put_SortOrder(SortOrderEnum _nScale);
            ColumnAttributesEnum get_Attributes() const;
            sal_Bool put_Attributes(const ColumnAttributesEnum& _eNum);
            WpADOProperties get_Properties() const;
            void put_ParentCatalog(/* [in] */ _ADOCatalog __RPC_FAR *ppvObject);
        };
    }
}

#endif //CONNECTIVITY_ADO_WRAP_COLUMN_HXX
