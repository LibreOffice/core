/*************************************************************************
 *
 *  $RCSfile: st_gilrw.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-06-11 16:04:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


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


