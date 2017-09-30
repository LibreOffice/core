/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_TSORTINDEX_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_TSORTINDEX_HXX

#include <connectivity/dbtoolsdllapi.hxx>
#include "TKeyValue.hxx"

namespace connectivity
{
    enum class OKeyType
    {
        NONE,        // do not sort
        Double,      // numeric key
        String       // String Key
    };

    enum class TAscendingOrder
    {
        ASC     = 1,            // ascending
        DESC    = -1            // otherwise
    };

    class OKeySet;
    class OKeyValue;                // simple class which holds a sal_Int32 and a std::vector<ORowSetValueDecoratorRef>

    /**
        The class OSortIndex can be used to implement a sorted index.
        This can depend on the fields which should be sorted.
    */
    class OOO_DLLPUBLIC_DBTOOLS OSortIndex
    {
    public:
        typedef std::vector< std::pair<sal_Int32,OKeyValue*> >  TIntValuePairVector;
        typedef std::vector<OKeyType>                             TKeyTypeVector;

    private:
        TIntValuePairVector             m_aKeyValues;
        TKeyTypeVector                  m_aKeyType;
        std::vector<TAscendingOrder>  m_aAscending;
        bool                        m_bFrozen;

    public:

        OSortIndex( const std::vector<OKeyType>& _aKeyType,
                    const std::vector<TAscendingOrder>& _aAscending);

        ~OSortIndex();

        static void * SAL_CALL operator new( size_t nSize )
            { return ::rtl_allocateMemory( nSize ); }
        static void * SAL_CALL operator new( size_t,void* _pHint )
            { return _pHint; }
        static void SAL_CALL operator delete( void * pMem )
            { ::rtl_freeMemory( pMem ); }
        static void SAL_CALL operator delete( void *,void* )
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
            CreateKeySet creates the keyset which values could be used to travel in your table/result
            The returned keyset is frozen.
        */
        ::rtl::Reference<OKeySet> CreateKeySet();

        const std::vector<OKeyType>& getKeyType() const { return m_aKeyType; }
        TAscendingOrder getAscending(std::vector<TAscendingOrder>::size_type _nPos) const { return m_aAscending[_nPos]; }

    };

    /**
        The class OKeySet is a refcountable vector which also has a state.
        This state gives information about if the keyset is fixed.
    */
    class OKeySet : public ORefVector<sal_Int32>
    {
        bool m_bFrozen;
    public:
        OKeySet()
            : ORefVector<sal_Int32>()
            , m_bFrozen(false){}

        bool    isFrozen() const   { return m_bFrozen; }
        void    setFrozen()        { m_bFrozen = true; }
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_TSORTINDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
