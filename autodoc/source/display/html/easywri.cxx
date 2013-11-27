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

#include <precomp.h>
#include "easywri.hxx"


// NOT FULLY DEFINED SERVICES


using namespace csi::html;


EasyWriter::EasyWriter()
{
}

EasyWriter::~EasyWriter()
{
}

void
EasyWriter::Open_OutputNode( csi::xml::Element & io_rDestination )
{
    aCurDestination.push(&io_rDestination);
}

void
EasyWriter::Finish_OutputNode()
{
    csv_assert( NOT aCurDestination.empty() );
    aCurDestination.pop();
}

csi::xml::Element &
EasyWriter::Out()
{
    csv_assert( ! aCurDestination.empty() );
    return *aCurDestination.top();
}

