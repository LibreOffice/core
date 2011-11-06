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



#ifndef SC_STRLOAD_HXX
#define SC_STRLOAD_HXX

#include "scresid.hxx"
#include <tools/rc.hxx>

class ScRscStrLoader : public Resource
{
public:
    ScRscStrLoader( sal_uInt16 nRsc, sal_uInt16 nStrId ) :
        Resource( ScResId( nRsc ) ), theStr( ScResId( nStrId ) )
    {
        FreeResource();
    }

    const String& GetString() const { return theStr; }

private:
    String theStr;
};


#endif // SC_STRLOAD_HXX

