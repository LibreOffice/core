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


#ifndef _SVX_STRARRAY_HXX
#define _SVX_STRARRAY_HXX

// include ---------------------------------------------------------------

#ifndef _RESARY_HXX //autogen
#include <tools/resary.hxx>
#endif
#include "svx/svxdllapi.h"

//  class SvxStringArray -------------------------------------------------

class SVX_DLLPUBLIC SvxStringArray : public ResStringArray
{
public:
    SvxStringArray( sal_uInt32 nResId );
    SvxStringArray( const ResId& rResId );
    ~SvxStringArray();

    const String&   GetStringByPos( sal_uInt32 nPos ) const;
    const String&   GetStringByType( long nType ) const;
    long            GetValueByStr( const String& rStr ) const;
};


#endif

