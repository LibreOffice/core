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
#include "precompiled_framework.hxx"

//________________________________
//  my own includes

#include <jobs/jobconst.hxx>

//________________________________
//  interface includes

//________________________________
//  includes of other projects

//________________________________
//  namespace

namespace framework{

//________________________________
//  non exported const

//________________________________
//  non exported definitions

//________________________________
//  declarations

const ::rtl::OUString JobConst::ANSWER_DEACTIVATE_JOB()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("Deactivate");
    return PROP;
}

const ::rtl::OUString JobConst::ANSWER_SAVE_ARGUMENTS()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("SaveArguments");
    return PROP;
}

const ::rtl::OUString JobConst::ANSWER_SEND_DISPATCHRESULT()
{
    static const ::rtl::OUString PROP = ::rtl::OUString::createFromAscii("SendDispatchResult");
    return PROP;
}

} // namespace framework
