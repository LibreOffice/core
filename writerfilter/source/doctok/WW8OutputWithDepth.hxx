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



#ifndef INCLUDED_WW8_OUTPUT_WITH_DEPTH
#define INCLUDED_WW8_OUTPUT_WITH_DEPTH

#include <resourcemodel/OutputWithDepth.hxx>

#include <string>

using namespace ::std;

namespace writerfilter {
namespace doctok
{
class WW8OutputWithDepth : public OutputWithDepth<string>
{
protected:
    void output(const string & output) const;
public:
    WW8OutputWithDepth();
    virtual ~WW8OutputWithDepth() { finalize(); }
};

extern WW8OutputWithDepth output;

}}

#endif // INCLUDED_WW8_OUTPUT_WITH_DEPTH
