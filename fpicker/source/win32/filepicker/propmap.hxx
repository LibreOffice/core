/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:35:23 $
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

#ifndef _PROPMAP_HXX_
#define _PROPMAP_HXX_

#include <hash_map>
#include <rtl/ustring.hxx>
#include <sal/types.h>

typedef ::std::hash_map< ::rtl::OUString                    ,
                         void*                              ,
                         ::rtl::OUStringHash                ,
                         ::std::equal_to< ::rtl::OUString > > TPropMapBase;

class TPropMap
{
    private:

        TPropMapBase m_aMap;

    public:

        template< class TValueType >
        void put(const ::rtl::OUString& sKey  ,
                 const TValueType&      rValue)
        {
            void* pValue = (void*)&rValue;
            m_aMap[sKey] = pValue;
        }

        template< class TValueType >
        void put_copy(const ::rtl::OUString& sKey  ,
                      const TValueType&      rValue)
        {
            TValueType* pCopy = new TValueType(rValue);
            m_aMap[sKey] = (void*)pCopy;
        }

        template< class TValueType >
        sal_Bool get(const ::rtl::OUString& sKey  ,
                           TValueType**     pValue)
        {
            TPropMapBase::iterator pIt = m_aMap.find(sKey);
            if (pIt == m_aMap.end())
                return sal_False;

            void*  pItem  = pIt->second;
                  *pValue = (TValueType*)pItem;
            return (pItem != 0);
        }

        template< class TValueType >
        sal_Bool get_copy(const ::rtl::OUString& sKey  ,
                                TValueType&      rValue)
        {
            TPropMapBase::iterator pIt = m_aMap.find(sKey);
            if (pIt == m_aMap.end())
                return sal_False;

            void* pValue = pIt->second;
            if ( ! pValue)
                return sal_False;

            rValue = *((TValueType*)pValue);
            //delete pValue;
            m_aMap.erase(pIt);
            return sal_True;
        }

        void clear()
        {
            m_aMap.clear();
        }
};

#endif
