/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TKeyValue.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:57:52 $
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
#ifndef CONNECTIVITY_TKEYVALUE_HXX
#define CONNECTIVITY_TKEYVALUE_HXX

#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include "connectivity/FValue.hxx"
#endif

namespace connectivity
{
    class OKeyValue
    {
    private:
        ::std::vector<ORowSetValueDecoratorRef> m_aKeys;
        sal_Int32 m_nValue;

    protected:
        OKeyValue();
        OKeyValue(sal_Int32 nVal);
    public:

        ~OKeyValue();

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW( () )
            {  }

        static OKeyValue* createKeyValue(sal_Int32 nVal);
        //  static OKeyValue* createEmptyKeyValue();

        inline void pushKey(const ORowSetValueDecoratorRef& _aValueRef)
        {
            m_aKeys.push_back(_aValueRef);
        }
        inline void setValue(sal_Int32 nVal) { m_nValue = nVal; }

        ::rtl::OUString getKeyString(::std::vector<ORowSetValueDecoratorRef>::size_type i) const
        {
            OSL_ENSURE(m_aKeys.size() > i,"Wrong index for KEyValue");
            return m_aKeys[i]->getValue();
        }
        double          getKeyDouble(::std::vector<ORowSetValueDecoratorRef>::size_type i) const
        {
            OSL_ENSURE(m_aKeys.size() > i,"Wrong index for KEyValue");
            return m_aKeys[i]->getValue();
        }

        inline sal_Int32 getValue() const { return m_nValue; }
    };
}

#endif // CONNECTIVITY_TKEYVALUE_HXX

