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

#ifndef CSV_TPLTOOLS_HXX
#define CSV_TPLTOOLS_HXX

#include <vector>
#include <map>




namespace csv
{


template <class COLLECTION>
inline void         erase_container(
                        COLLECTION &        o_rCollection );

/// Version for other containers than std::map, with non-pair value_type.
template <class COLLECTION>
void                erase_container_of_heap_ptrs(
                        COLLECTION &        o_rCollection );


template <class KEY, class VAL>
const VAL *         find_in_map(            /// Usable for all kinds of values
                        const std::map< KEY, VAL > &
                                            i_rMap,
                        const KEY &         i_rKey );


/** @return the value in the map, if it is in there, else 0.
    @precond VAL has to be convertible to "0".
*/
template <class KEY, class VAL>
VAL                 value_from_map(
                        const std::map< KEY, VAL > &
                                            i_rMap,
                        const KEY &         i_rKey );

/** @return the value in the map, if it is in there, else i_notFound.
*/
template <class KEY, class VAL>
VAL                 value_from_map(
                        const std::map< KEY, VAL > &
                                            i_rMap,
                        const KEY &         i_rKey,
                        VAL                 i_notFound );

template <class COLLECTION, class VALUE>
bool                contains(
                        const COLLECTION &  i_collection,
                        const VALUE &       i_value );




// IMPLEMENTATION
template <class COLLECTION>
inline void
erase_container( COLLECTION & o_rCollection )
{
    o_rCollection.erase( o_rCollection.begin(),
                         o_rCollection.end() );
}

template <class COLLECTION>
void
erase_container_of_heap_ptrs( COLLECTION & o_rCollection )
{
    typename COLLECTION::iterator itEnd = o_rCollection.end();
    for ( typename COLLECTION::iterator it = o_rCollection.begin();
          it != itEnd;
          ++it )
    {
        delete *it;
    }

    o_rCollection.erase( o_rCollection.begin(),
                         o_rCollection.end() );
}

template <class KEY, class VAL>
const VAL *
find_in_map( const std::map< KEY, VAL > & i_rMap,
             const KEY &                  i_rKey )
{
    typename std::map< KEY, VAL >::const_iterator
            ret =  i_rMap.find(i_rKey);
    return ret != i_rMap.end()
                ?   & (*ret).second
                :   (const VAL*)0;
}

template <class KEY, class VAL>
VAL
value_from_map( const std::map< KEY, VAL > & i_rMap,
                const KEY &                  i_rKey )
{
    typename std::map< KEY, VAL >::const_iterator
            ret =  i_rMap.find(i_rKey);
    return ret != i_rMap.end()
                ?   (*ret).second
                :   VAL(0);
}

template <class KEY, class VAL>
VAL
value_from_map( const std::map< KEY, VAL > & i_rMap,
                const KEY &                  i_rKey,
                VAL                          i_notFound )
{
    typename std::map< KEY, VAL >::const_iterator
            ret =  i_rMap.find(i_rKey);
    return ret != i_rMap.end()
                ?   (*ret).second
                :   i_notFound;
}

template <class COLLECTION, class VALUE>
bool
contains( const COLLECTION &  i_collection,
          const VALUE &       i_value )
{
    return std::find(i_collection.begin(), i_collection.end(), i_value)
           !=
           i_collection.end();
}




}   // namespace csv
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
