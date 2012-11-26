/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SWITERATOR_HXX
#define _SWITERATOR_HXX

#include <calbck.hxx>
#include <tools/debug.hxx>

template< class TElementType, class TSource > class SwIterator
{
    SwClientIter aClientIter;

public:
    SwIterator( const TSource& rSrc )
    :   aClientIter(rSrc)
    {
        DBG_ASSERT( 0 != dynamic_cast< const SwClient* >(&rSrc), "Incompatible types!" );
    }

    TElementType* First()
    {
        SwClient* t = aClientIter.SwClientIter_First();
        TElementType* p = dynamic_cast< TElementType* >(t);

        while(!p && t)
        {
            t = aClientIter.SwClientIter_Next();
            p = dynamic_cast< TElementType* >(t);
        }

        return p;
    }

    TElementType* Next()
    {
        SwClient* t = aClientIter.SwClientIter_Next();
        TElementType* p = dynamic_cast< TElementType* >(t);

        while(!p && t)
        {
            t = aClientIter.SwClientIter_Next();
            p = dynamic_cast< TElementType* >(t);
        }

        return p;
    }

    static TElementType* FirstElement(const TSource& rMod)
    {
        SwClientIter aIter(rMod);
        SwClient* t = aIter.SwClientIter_First();
        TElementType* p = dynamic_cast< TElementType* >(t);

        while(!p && t)
        {
            t = aIter.SwClientIter_Next();
            p = dynamic_cast< TElementType* >(t);
        }

        return p;
    }

    bool IsChanged()
    {
        return aClientIter.IsChanged();
    }
};

#endif
// eof
