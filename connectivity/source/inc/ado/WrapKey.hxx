/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrapKey.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:57:26 $
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
#ifndef CONNECTIVITY_ADO_WRAP_KEY_HXX
#define CONNECTIVITY_ADO_WRAP_KEY_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOKey : public WpOLEBase<ADOKey>
        {
        public:
            WpADOKey(ADOKey* pInt=NULL) :   WpOLEBase<ADOKey>(pInt){}
            WpADOKey(const WpADOKey& rhs){operator=(rhs);}

            inline WpADOKey& operator=(const WpADOKey& rhs)
                {WpOLEBase<ADOKey>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const;
            void put_Name(const ::rtl::OUString& _rName);
            KeyTypeEnum get_Type() const;
            void put_Type(const KeyTypeEnum& _eNum) ;
            ::rtl::OUString get_RelatedTable() const;
            void put_RelatedTable(const ::rtl::OUString& _rName);
            RuleEnum get_DeleteRule() const;
            void put_DeleteRule(const RuleEnum& _eNum) ;
            RuleEnum get_UpdateRule() const;
            void put_UpdateRule(const RuleEnum& _eNum) ;
            WpADOColumns get_Columns() const;
        };
    }
}

#endif //CONNECTIVITY_ADO_WRAP_KEY_HXX
