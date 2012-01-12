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
#include <ary/doc/d_oldcppdocu.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/info/all_tags.hxx>
#include <ary/info/docstore.hxx>
#include <ary/info/infodisp.hxx>
#include <docu_node_ids.hxx>




namespace ary
{
namespace doc
{

using namespace info;




unsigned char  C_ucNO_INDEX = 255;
typedef DYN StdTag * (F_CREATE)();


OldCppDocu::OldCppDocu()
    :   Node(docnt::nt_OldCppDocu),
        bIsObsolete(false),
        bIsInternal(false),
        bIsInterface(false)
{
    memset( nTags, C_ucNO_INDEX, size_t(C_eAtTag_NrOfClasses) );
}

OldCppDocu::~OldCppDocu()
{
}

void
OldCppDocu::Store2( info::DocuStore & o_rDocuStore )
{
    o_rDocuStore.Store2ConnectedDeclaration(*this);
}

AtTag *
OldCppDocu::Create_StdTag( E_AtTagId i_eId )
{
    UINT8 nIndex = static_cast<UINT8>(i_eId);
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new StdTag(i_eId);
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::CheckIn_BaseTag()
{
    UINT8 nIndex = atc_base;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new BaseTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::CheckIn_ExceptionTag()
{
    UINT8 nIndex = atc_exception;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new ExceptionTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::Create_ImplementsTag()
{
    UINT8 nIndex = atc_implements;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new ImplementsTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::Create_KeywordTag()
{
    UINT8 nIndex = atc_keyword;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new KeywordTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::CheckIn_ParameterTag()
{
    UINT8 nIndex = atc_parameter;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new ParameterTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::CheckIn_SeeTag()
{
    UINT8 nIndex = atc_see;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new SeeTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::CheckIn_TemplateTag()
{
    UINT8 nIndex = atc_template;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new TemplateTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::Create_LabelTag()
{
    UINT8 nIndex = atc_label;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new LabelTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::Create_DefaultTag()
{
    UINT8 nIndex = atid_descr;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new StdTag(atid_descr);
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}

AtTag *
OldCppDocu::Create_SinceTag()
{
    UINT8 nIndex = atc_since;
    if ( nTags[nIndex] == C_ucNO_INDEX )
    {
        AtTag * ret = new SinceTag();
        NewTag(nIndex) = ret;
        return ret;
    }
    else
    {
        return GetTag(nIndex).GetFollower();
    }
}


void
OldCppDocu::Replace_AtShort_By_AtDescr()
{
    unsigned char nPosInTags = nTags[atid_short];
    if ( nPosInTags == C_ucNO_INDEX )
        return;

    AtTag * pTag = aTags[ nPosInTags ];
    if ( pTag == 0 )    // Should be csv_assert().
        return;

    csv_assert( dynamic_cast< StdTag* >(pTag) != 0 );
    StdTag * pStdTag = static_cast< StdTag* >(pTag);

    pStdTag->ChangeId2(atid_descr);
    nTags[atid_short] = C_ucNO_INDEX;
    nTags[atid_descr] = nPosInTags;
}

void
OldCppDocu::Set_Obsolete()
{
    bIsObsolete = true;
}

void
OldCppDocu::Set_Internal()
{
    bIsInternal = true;
}

const AtTag &
OldCppDocu::Short() const
{
    static const StdTag aNull_(atid_short);

    unsigned char nPosInTags = nTags[atid_short];
    if ( nPosInTags != C_ucNO_INDEX )
    {
        AtTag * pTag = aTags[ nPosInTags ];
        if ( pTag != 0 )    // Should be csv_assert().
        {
            return *pTag;
        }
    }

    return aNull_;
}

AtTag * &
OldCppDocu::NewTag(UINT8 i_nIndex)
{
    nTags[i_nIndex] = static_cast<UINT8>(aTags.size());
    aTags.push_back(0);
    return aTags.back();
}

AtTag &
OldCppDocu::GetTag( UINT8   i_nIndex )
{
    csv_assert( i_nIndex < C_eAtTag_NrOfClasses );
    csv_assert( nTags[i_nIndex] != C_ucNO_INDEX );
    csv_assert( aTags[nTags[i_nIndex]] != 0 );
    return * aTags[nTags[i_nIndex]];
}

bool
OldCppDocu::IsInternal() const
{
    return bIsInternal;
}

bool
OldCppDocu::IsInterface() const
{
    return bIsInterface;
}

void
OldCppDocu::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor, *this);
}

}   // namespace doc
}   // namespace ary
