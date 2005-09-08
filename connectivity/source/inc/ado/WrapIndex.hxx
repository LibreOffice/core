/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrapIndex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:57:12 $
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
#ifndef CONNECTIVITY_ADO_WRAP_INDEX_HXX
#define CONNECTIVITY_ADO_WRAP_INDEX_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOIndex : public WpOLEBase<_ADOIndex>
        {
        public:
            WpADOIndex(_ADOIndex* pInt=NULL)    :   WpOLEBase<_ADOIndex>(pInt){}
            WpADOIndex(const WpADOIndex& rhs){operator=(rhs);}

            inline WpADOIndex& operator=(const WpADOIndex& rhs)
                {WpOLEBase<_ADOIndex>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const;
            void            put_Name(const ::rtl::OUString& _rName);
            sal_Bool        get_Clustered() const;
            void            put_Clustered(sal_Bool _b);
            sal_Bool        get_Unique() const;
            void            put_Unique(sal_Bool _b);
            sal_Bool        get_PrimaryKey() const;
            void            put_PrimaryKey(sal_Bool _b);
            WpADOColumns    get_Columns() const;
        };
    }
}

#endif //CONNECTIVITY_ADO_WRAP_INDEX_HXX
