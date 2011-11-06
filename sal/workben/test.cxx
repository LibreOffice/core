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
#include "precompiled_sal.hxx"

#include <stdio.h>

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    void test_int64();
    test_int64();

    void test_profile(void);
    test_profile();

    void test_OString();
    test_OString();

/*      void test_OWString(); */
/*      test_OWString(); */

/*      void test_OStringBuffer(); */
/*      test_OStringBuffer(); */

/*      void test_OWStringBuffer(); */
/*      test_OWStringBuffer(); */

/*      void test_OString2OWStringAndViceVersa(); */
/*      test_OString2OWStringAndViceVersa(); */

    void test_uuid();
    test_uuid();

    return(0);
}


