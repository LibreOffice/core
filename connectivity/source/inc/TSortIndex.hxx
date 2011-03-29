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
#ifndef CONNECTIVITY_TSORTINDEX_HXX
#define CONNECTIVITY_TSORTINDEX_HXX

#include "connectivity/dbtoolsdllapi.hxx"
#include "TKeyValue.hxx"

namespace connectivity
{
    typedef enum
    {
        SQL_ORDERBYKEY_NONE,        // do not sort
        SQL_ORDERBYKEY_DOUBLE,      // numeric key
        SQL_ORDERBYKEY_STRING       // String Key
    } OKeyType;

    typedef enum
    {
        SQL_ASC     = 1,            // ascending
        SQL_DESC    = -1            // otherwise
    } TAscendingOrder;

    class OKeySet;
    class OKeyValue;                // simple class which holds a sal_Int32 and a ::std::vector<ORowSetValueDecoratorRef>

    /**
        The class OSortIndex can be used to implement a sorted index.
        This can depend on the fields which should be sorted.
    */
    class OOO_DLLPUBLIC_DBTOOLS OSortIndex
    {
    public:
        typedef ::std::vector< ::std::pair<sal_Int32,OKeyValue*> >  TIntValuePairVector;
        typedef ::std::vector<OKeyType>                             TKeyTypeVector;

    private:
        TIntValuePairVector             m_aKeyValues;
        TKeyTypeVector                  m_aKeyType;
        ::std::vector<TAscendingOrder>  m_aAscending;
        sal_Bool                        m_bFrozen;

    public:

        OSortIndex( const ::std::vector<OKeyType>& _aKeyType,
                    const ::std::vector<TAscendingOrder>& _aAscending);

        ~OSortIndex();

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW( () )
            {  }


        /**
            AddKeyValue appends a new value.
            @param
                pKeyValue   the keyvalue to be appended
            ATTENTION: when the sortindex is already frozen the parameter will be deleted
        */
        void AddKeyValue(OKeyValue * pKeyValue);

        /**
            Freeze freezes the sortindex so that new values could only be appended by their value
        */
        void Freeze();

        /**
            CreateKeySet creates the keyset which vaalues could be used to travel in your table/result
            The returned keyset is frozen.
        */
        ::rtl::Reference<OKeySet> CreateKeySet();



        // look at the name
        sal_Bool IsFrozen() const { return m_bFrozen; }
        // returns the current size of the keyvalues
        sal_Int32 Count()   const { return m_aKeyValues.size(); }
        /** GetValue returns the value at position nPos (1..n) [sorted access].
            It only allowed to call this method after the sortindex has been frozen.
        */

        sal_Int32 GetValue(sal_Int32 nPos) const;

        inline const ::std::vector<OKeyType>& getKeyType() const { return m_aKeyType; }
        inline TAscendingOrder getAscending(::std::vector<TAscendingOrder>::size_type _nPos) const { return m_aAscending[_nPos]; }

    };

    /**
        The class OKeySet is a refcountable vector which also has a state.
        This state gives information about if the keyset is fixed.
    */
    class OOO_DLLPUBLIC_DBTOOLS OKeySet : public ORefVector<sal_Int32>
    {
        sal_Bool m_bFrozen;
    public:
        OKeySet()
            : ORefVector<sal_Int32>()
            , m_bFrozen(sal_False){}
        OKeySet(Vector::size_type _nSize)
            : ORefVector<sal_Int32>(_nSize)
            , m_bFrozen(sal_False){}

        sal_Bool    isFrozen() const                        { return m_bFrozen; }
        void        setFrozen(sal_Bool _bFrozen=sal_True)   { m_bFrozen = _bFrozen; }
    };
}
#endif // CONNECTIVITY_TSORTINDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
