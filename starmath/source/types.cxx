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
#include "precompiled_starmath.hxx"


#include <types.hxx>


sal_Unicode ConvertMathPrivateUseAreaToUnicode( sal_Unicode cChar )
{
    sal_Unicode cRes = cChar;
    if (IsInPrivateUseArea( cChar ))
    {
        DBG_ASSERT( 0, "Error: private use area characters should no longer be in use!" );
        cRes = (sal_Unicode) '@'; // just some character that should easily be notice as odd in the context
    }
    return cRes;
}


sal_Unicode ConvertMathToMathML( sal_Unicode cChar )
{
    sal_Unicode cRes = ConvertMathPrivateUseAreaToUnicode( cChar );
    return cRes;
}

