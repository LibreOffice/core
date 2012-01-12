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
#include "cmd_sincedata.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/file.hxx>
#include <cosv/tpl/tpltools.hxx>
#include "adc_cmds.hxx"



namespace autodoc
{
namespace command
{

SinceTagTransformationData::SinceTagTransformationData()
    :   aTransformationTable()
{
}

SinceTagTransformationData::~SinceTagTransformationData()
{
}

bool
SinceTagTransformationData::DoesTransform() const
{
    return NOT aTransformationTable.empty();
}

const String &
SinceTagTransformationData::DisplayOf( const String & i_versionNumber ) const
{
    if (DoesTransform())
    {
        StreamLock
            sl(200);
        sl() << i_versionNumber;
        sl().strip_frontback_whitespace();
        String
            sVersionNumber(sl().c_str());

        const String *
            ret = csv::find_in_map(aTransformationTable, sVersionNumber);
        return ret != 0
                ?   *ret
                :   String::Null_();
    }
    else
    {
        return i_versionNumber;
    }
}

void
SinceTagTransformationData::do_Init( opt_iter &  it,
                                     opt_iter    itEnd )
{
    ++it;   // Cur is since-file path.

    CHECKOPT(   it != itEnd ,
              "file path",
              C_opt_SinceFile );

    csv::File           aSinceFile(*it);
    csv::OpenCloseGuard aSinceFileGuard(aSinceFile);
    StreamStr           sLine(200);

    if (aSinceFileGuard)
    {
        for ( sLine.operator_read_line(aSinceFile);
              NOT sLine.empty();
              sLine.operator_read_line(aSinceFile) )
        {

            if (*sLine.begin() != '"')
                continue;

            const char * pVersion = sLine.c_str() + 1;
            const char * pVersionEnd = strchr(pVersion, '"');
            if (pVersionEnd == 0)
                continue;
            const char * pDisplay = strchr(pVersionEnd+1, '"');
            if (pDisplay == 0)
                continue;
            ++pDisplay;
            const char * pDisplayEnd = strchr(pDisplay, '"');
            if (pDisplayEnd == 0)
                continue;

            aTransformationTable[ String(pVersion,pVersionEnd) ]
                                        = String(pDisplay,pDisplayEnd);
            sLine.clear();
        }   // end for
    }   // end if

    ++it;   // Cur is next option.
}

}   // namespace command
}   // namespace autodoc
