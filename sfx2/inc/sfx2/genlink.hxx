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


#ifndef _SFXGENLINK_HXX
#define _SFXGENLINK_HXX


#include <tools/link.hxx>
typedef long (*CFuncPtr)(void*);

class GenLink
{
    Link        aLink;
    CFuncPtr    pFunc;

public:
    GenLink(): pFunc(0) {}
    GenLink( CFuncPtr pCFunc ): pFunc(pCFunc) {}
    GenLink( const Link& rLink ): aLink(rLink), pFunc(0) {}
    GenLink( const GenLink& rOrig ):
        aLink(rOrig.aLink), pFunc(rOrig.pFunc) {}

    GenLink& operator = ( const GenLink& rOrig )
        { pFunc = rOrig.pFunc; aLink = rOrig.aLink; return *this; }

    sal_Bool operator!() const { return !aLink && !pFunc; }
    sal_Bool IsSet() const { return aLink.IsSet() || pFunc; }

    long Call( void* pCaller )
         { return pFunc ? (*pFunc)(pCaller) : aLink.Call(pCaller); }
};

#endif
