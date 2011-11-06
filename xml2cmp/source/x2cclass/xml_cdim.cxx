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



#include "xml_cdim.hxx"

const char ComponentDescriptionImpl::C_sTagDescription[]
                                        =   "COMPONENTDESCRIPTION";
const char ComponentDescriptionImpl::C_sStatus[]
                                        =   "Status";
const char * ComponentDescriptionImpl::C_sSubTags[ComponentDescription::tag_MAX]
                                        = { "None",
                                            "Name",
                                            "Description",
                                            "ModuleName",
                                            "LoaderName",
                                            "SupportedService",
                                            "ProjectBuildDependency",
                                            "RuntimeModuleDependency",
                                            "ServiceDependency",
                                            "Language",
                                            C_sStatus,
                                            "Type"
                                          };

ComponentDescriptionImpl::ComponentDescriptionImpl()
//  :   aTags
{
    const int i_max = tag_MAX;
    aTags.reserve(i_max);

    for (int i = 0; i < i_max; ++i)
    {
        aTags.push_back( new ValueList(E_Tag(i)) );
    }  // end for
}

ComponentDescriptionImpl::~ComponentDescriptionImpl()
{
    for ( std::vector< ValueList* >::iterator aIter = aTags.begin();
          aIter != aTags.end();
          ++aIter )
    {
        delete *aIter;
    }
}

inline void
GetStatusValue( ByteString & o_sValue, const ByteString & i_sStatusTag )
{
    // o_sValue is always == "" at the beginning.

    const char * pStatusValue = strchr(i_sStatusTag.GetBuffer(), '"');
    if (pStatusValue == 0)
        return;
    pStatusValue++;
    const char * pStatusValueEnd = strrchr(pStatusValue,'"');
    if (pStatusValueEnd == 0 || pStatusValueEnd - pStatusValue < 1)
        return ;

    ByteString sValue(pStatusValue, pStatusValueEnd - pStatusValue);
    o_sValue = sValue;
}

ComponentDescriptionImpl::ValueList *
ComponentDescriptionImpl::GetBeginTag( ByteString &  o_sValue,
                                       const char *& io_pStartOfTag ) const
{
    o_sValue = "";

    const char * pCurTextEnd = strchr(io_pStartOfTag,'>');
    if ( 0 == pCurTextEnd )
        return 0;

    if ( ComponentDescriptionImpl::CheckEndOfDescription(io_pStartOfTag) )
        return 0;

    ByteString sTag(io_pStartOfTag + 1, pCurTextEnd - io_pStartOfTag - 1);
    io_pStartOfTag += sTag.Len() + 2;

    // Special case <Status ... >
    if ( strnicmp(C_sStatus, sTag.GetBuffer(), (sizeof C_sStatus) - 1 ) == 0 )
    {
        GetStatusValue(o_sValue,sTag);
        return aTags[tag_Status];
    }

    // Regular seeking for matching data list:
    for ( INT32 i = 0; i < tag_MAX; i++ )
    {
        if ( 0 == stricmp(sTag.GetBuffer(), C_sSubTags[i]) )
            return aTags[i];
    }   // end for

    return 0;
}

const std::vector< ByteString > &
ComponentDescriptionImpl::DataOf( ComponentDescriptionImpl::E_Tag i_eTag ) const
{
    if (0 < i_eTag && i_eTag < tag_MAX)
        return *aTags[i_eTag];
    else
        return ValueList::Null_();
}

ByteString
ComponentDescriptionImpl::DatumOf( ComponentDescriptionImpl::E_Tag i_eTag ) const
{
    if (0 < i_eTag && i_eTag < tag_MAX)
    {
        ValueList & rValues = *aTags[i_eTag];
        if (rValues.size() > 0)
            return rValues[0];
    }
    return "";
}

void
ComponentDescriptionImpl::ParseUntilStartOfDescription( const char * & io_pBufferPosition )
{
    for ( const char * pSearch = strchr(io_pBufferPosition,'<');
          pSearch != 0;
          pSearch = strchr(pSearch+1,'<') )
    {
        if ( pSearch != io_pBufferPosition
             && 0 == strnicmp(pSearch+1,C_sTagDescription, strlen(C_sTagDescription))
             && *(pSearch + strlen(C_sTagDescription) + 1) == '>' )
        {
            io_pBufferPosition = pSearch + strlen(C_sTagDescription) + 2;
            return;
        }
    }   // end for

    io_pBufferPosition = 0;
}

BOOL
ComponentDescriptionImpl::ValueList::MatchesEndTag( const char * i_pTextPosition ) const
{
    return strnicmp( i_pTextPosition+2, C_sSubTags[eTag], strlen(C_sSubTags[eTag]) ) == 0
           && strncmp(i_pTextPosition,"</",2) == 0
           && *(i_pTextPosition + 2 + strlen(C_sSubTags[eTag]) ) == '>';
}

INT32
ComponentDescriptionImpl::ValueList::EndTagLength() const
{
    return strlen(C_sSubTags[eTag]) + 3;
}


const ComponentDescriptionImpl::ValueList &
ComponentDescriptionImpl::ValueList::Null_()
{
    static const ValueList aNull_(ComponentDescription::tag_None);
    return aNull_;
}


