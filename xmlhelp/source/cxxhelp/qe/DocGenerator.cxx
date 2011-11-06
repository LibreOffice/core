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
#include "precompiled_xmlhelp.hxx"
#include <qe/DocGenerator.hxx>
#include <qe/Query.hxx>


using namespace xmlsearch;
using namespace xmlsearch::qe;


const sal_Int32 NonnegativeIntegerGenerator::END = -1;


RoleFiller RoleFiller::roleFiller_;


RoleFiller::RoleFiller()
    : m_nRefcount( 0 ),
      fixedRole_( 0 ),
      filled_( 0 ),
      begin_( 0 ),
      end_( 0 ),
      parentContext_( 0 ),
      limit_( 0 ),
      next_( 0 ),
      fillers_( 0 )
{
}


RoleFiller::~RoleFiller()
{

}
