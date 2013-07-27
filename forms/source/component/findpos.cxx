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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "sal/config.h"

#include "findpos.hxx"

#include "com/sun/star/uno/Sequence.hxx"
#include "comphelper/types.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <algorithm>

namespace forms_detail {

sal_Int32 findPos(
    const ::rtl::OUString& aStr,
    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rList)
{
    const ::rtl::OUString* pStrList = rList.getConstArray();
    const ::rtl::OUString* pResult = ::std::lower_bound(
        pStrList, pStrList + rList.getLength(), aStr );
    if ( ( pResult != pStrList + rList.getLength() ) && ( *pResult == aStr ) )
        return ( pResult - pStrList );

    return -1;
}

}
