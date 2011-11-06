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
#include <util/Decompressor.hxx>
#include <util/CompressorIterator.hxx>

using namespace xmlsearch;
using namespace xmlsearch::util;

const sal_Int32 Decompressor::BitsInByte = 8;
const sal_Int32 Decompressor::NBits = 32;

inline sal_Int32 getInteger_( const sal_Int8* v )
{
    return (((((( (v[0]&0xFF) ) << 8 )
                | (v[1]&0xFF) ) << 8 )
                | (v[2]&0xFF) ) << 8 )
                | (v[3]&0xFF);
}

sal_Int32 StreamDecompressor::getNextByte()
{
  sal_Int8 a[4];
  a[0] = a[1] = a[2] = 0;
  if( in_->readBytes( &a[3],1 ) != -1 )
    return getInteger_( a );
  else
    return -1;
}
