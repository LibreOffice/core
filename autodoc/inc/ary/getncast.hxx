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



#ifndef ARY_GETNCAST_HXX
#define ARY_GETNCAST_HXX


// USED SERVICES
#include <ary/object.hxx>




namespace ary
{


template <class DEST>
inline bool
is_type(const ary::Object & i_obj)
{
    return i_obj.AryClass() == DEST::class_id;
}


template <class DEST>
inline const DEST &
ary_cast( const Object & ce)
{
    csv_assert( is_type<DEST>(ce) );
    return static_cast< const DEST& >(ce);
}

template <class DEST>
inline DEST &
ary_cast( Object & ce)
{
    csv_assert( is_type<DEST>(ce) );
    return static_cast< DEST& >(ce);
}

template <class DEST>
inline const DEST *
ary_cast( const Object * ce)
{
    if ( ce ? is_type<DEST>(*ce) : false )
        return static_cast< const DEST* >(ce);
    return 0;
}

template <class DEST>
inline DEST *
ary_cast( Object * ce)
{
    if ( ce ? is_type<DEST>(*ce) : false )
        return static_cast< DEST* >(ce);
    return 0;
}




}   // namespace ary
#endif
