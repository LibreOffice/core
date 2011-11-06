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



#ifndef _XMXTRCT_HXX
#define _XMXTRCT_HXX

#include "xmxcom.hxx"

// ----------------
// - XMLExtractor -
// ----------------

class XMLExtractor : public ::cppu::WeakImplHelper1< NMSP_IO::XXMLExtractor >
{
private:

    REF( NMSP_LANG::XMultiServiceFactory )          mxFact;

                                                    XMLExtractor();

public:

                                                    XMLExtractor( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual                                         ~XMLExtractor();

    // XXMLExtractor
    virtual REF( NMSP_IO::XInputStream ) SAL_CALL   extract( const REF( NMSP_IO::XInputStream )& rxIStm ) throw( NMSP_UNO::RuntimeException );
};

#endif
