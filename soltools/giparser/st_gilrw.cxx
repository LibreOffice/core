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
#include "precompiled_soltools.hxx"


#include <st_gilrw.hxx>

#include <gen_info.hxx>
#include <gi_list.hxx>
#include <gi_parse.hxx>
#include <simstr.hxx>
#include <st_list.hxx>


using namespace std;


ST_InfoListReader::ST_InfoListReader()
{
    dpParser = new GenericInfo_Parser;
}

ST_InfoListReader::~ST_InfoListReader()
{
    delete dpParser;
}


bool
ST_InfoListReader::LoadList( List_GenericInfo &  o_rList,
                             const Simstr &      i_sFileName )
{
    aListStack.push_back(&o_rList);
    return dpParser->LoadList(*this, i_sFileName);
}

ST_InfoListReader::E_Error
ST_InfoListReader::GetLastError( UINT32 * o_pErrorLine ) const
{
    return dpParser->GetLastError(o_pErrorLine);
}

void
ST_InfoListReader::AddKey( const char *        i_sKey,
                           UINT32              i_nKeyLength,
                           const char *        i_sValue,
                           UINT32              i_nValueLength,
                           const char *        i_sComment,
                           UINT32              i_nCommentLength )
{
    Simstr sKey(i_sKey, i_nKeyLength);
    Simstr sValue(i_sValue, i_nValueLength);
    Simstr sComment(i_sComment, i_nCommentLength);

    pCurKey = new GenericInfo(sKey, sValue, sComment);
    aListStack.back()->InsertInfo( pCurKey );
}

void
ST_InfoListReader::OpenList()
{
    if ( pCurKey == 0 )
    {
         cerr << "error: '{' without key found." << endl;
        exit(1);
    }

    aListStack.push_back( & pCurKey->SubList() );
}

void
ST_InfoListReader::CloseList()
{
    if ( aListStack.size() == 0 )
    {
         cerr << "error: '}' without corresponding '}' found." << endl;
        exit(1);
    }

    aListStack.pop_back();
}


#if 0
ST_InfoListWriter::ST_InfoListWriter()
{

}

ST_InfoListWriter::~ST_InfoListWriter()

bool
ST_InfoListWriter::SaveList( const Simstr &      i_sFileName,
                             List_GenericInfo &  io_rList )
{

}

E_Error
ST_InfoListWriter::GetLastError() const
{
    return dpParser->GetLastError(o_pErrorLine);
}

bool
ST_InfoListWriter::Start_CurList()
{

}

bool
ST_InfoListWriter::NextOf_CurList()
{

}

void
ST_InfoListWriter::Get_CurKey( char * o_rKey ) const
{

}

void
ST_InfoListWriter::Get_CurValue( char * o_rValue ) const
{

}

void
ST_InfoListWriter::Get_CurComment( char * o_rComment ) const
{

}

bool
ST_InfoListWriter::HasSubList_CurKey() const
{

}

void
ST_InfoListWriter::Push_CurList()
{

}

void
ST_InfoListWriter::Pop_CurList()
{

}
#endif


