/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CONNECTIVITY_TKEYVALUE_HXX
#define CONNECTIVITY_TKEYVALUE_HXX

#include "connectivity/FValue.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OKeyValue
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
