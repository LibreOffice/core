/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
