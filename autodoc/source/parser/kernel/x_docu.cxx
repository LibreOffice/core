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
#include <x_docu.hxx>

// NOT FULLY DECLARED SERVICES



X_Docu::X_Docu( const char *        i_tag,
                const char *        i_explanation  )
    :   sTagName(i_tag),
        sExplanation(i_explanation)
{
}

X_Docu::~X_Docu()
{
}

X_Docu::E_Event
X_Docu::GetEvent() const
{
     return x_Any;
}

void
X_Docu::GetInfo( std::ostream & o_rOutputMedium ) const
{
    o_rOutputMedium
        << "Error in tag '"
        << sTagName
        << "': "
        << sExplanation
        << Endl();
}
