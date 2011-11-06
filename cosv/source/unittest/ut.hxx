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



#ifndef COSV_UNITTEST_UT_HXX
#define COSV_UNITTEST_UT_HXX


#define UT_CHECK( fname, cond ) \
    if ( NOT (cond) ) { std::cerr << "ftest_" << #fname << " " << #cond << endl; \
        ret = false; }

#define CUT_DECL( nsp, cname ) \
    bool classtest_##cname()
#define FUT_DECL( cname, fname ) \
    bool ftest_##fname( cname & r##cname )

#define UT_RESULT( result ) \
    if (result ) std::cout << "All unit tests passed successfully." << std::endl; \
    else std::cout << "Errors in unit tests.\n" << std::endl



CUT_DECL( csv, File );
CUT_DECL( csv, String );

#endif



