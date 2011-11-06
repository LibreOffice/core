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
#include "precompiled_tools.hxx"
#include <tools/link.hxx>

/*************************************************************************
|*
|*    Link::operator==()
|*
|*    Beschreibung      LINK.SDW
|*    Ersterstellung    AM 14.02.91
|*    Letzte Aenderung  TH 07.11.95
|*
*************************************************************************/

sal_Bool Link::operator==( const Link& rLink ) const
{
    if ( pFunc == rLink.pFunc )
    {
        if ( pFunc )
        {
            if ( pInst == rLink.pInst )
                return sal_True;
            else
                return sal_False;
        }
        else
            return sal_True;
    }
    else
        return sal_False;
}
