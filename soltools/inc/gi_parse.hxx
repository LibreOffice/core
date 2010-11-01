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
#ifndef SOLTOOLS_GI_PARSE_HXX
#define SOLTOOLS_GI_PARSE_HXX

#include "simstr.hxx"
#include "gilacces.hxx"
#include <fstream>

class GenericInfoList_Builder;
class GenericInfoList_Browser;

/** Reads generic information files into a simple structure in memory.

Information files used by this parser have the following format:

key [value]
{
    key [value]
    key [value]
    {
        key [value]
        ...
        ...
    }
}
key [value]
...
...

*/


class GenericInfo_Parser : public GenericInfoParseTypes
{
  public:
    typedef unsigned long   UINT32;
    typedef short           INT16;

                        GenericInfo_Parser();
                        ~GenericInfo_Parser();

    /** reads a information file and stores the data in a
        List_GenericInfo
    */
    bool                LoadList(
                            GenericInfoList_Builder &
                                                o_rResult,
                            const Simstr &      i_sSourceFileName );

    /** save the InformationList to rSourceFile
        returns false on error
    */
    bool                SaveList(
                            const Simstr &      i_rOutputFile,
                            GenericInfoList_Browser &
                                                io_rListBrowser );

    E_Error             GetLastError(
                            UINT32 *            o_pErrorLine = 0 ) const;

  private:
    enum E_LineType
    {
        lt_empty = 0,
        lt_key,
        lt_open_list,
        lt_close_list,
        lt_comment
    };

    void                SetError(
                            E_Error             i_eError );
    void                ResetState(
                            GenericInfoList_Builder &
                                                io_rResult );
    void                ResetState(
                            GenericInfoList_Browser &
                                                io_rSrc );

    void                ReadLine();
    bool                InterpretLine();
    E_LineType          ClassifyLine();

    void                ReadKey();
    void                PushLevel_Read();           /// When list is opened by '{':
    void                PopLevel_Read();            /// When list is closed by '}':
    void                AddCurLine2CurComment();

    void                WriteList(
                            std::ostream &      o_rFile );

    void                PushLevel_Write();          /// When SubList is pushed in pResource
    void                PopLevel_Write();           /// When SubList is popped in pResource

    void                WriteComment(
                            std::ostream &      o_rFile,
                            const char *        i_sStr );
    void                WriteKey(
                            std::ostream &      o_rFile,
                            const char *        i_sStr );
    void                WriteValue(
                            std::ostream &      o_rFile,
                            const char *        i_sStr );
    void                WriteIndentation(
                            std::ostream &      o_rFile );

    // DATA
    const char *        sCurParsePosition;

    UINT32              nCurLine;
    INT16               nLevel;
    bool                bGoon;

    Simstr              sCurComment;

    E_Error             eErrorCode;
    UINT32              nErrorLine;

    GenericInfoList_Builder *
                        pResult;
    GenericInfoList_Browser *
                        pResource;

    char *              dpBuffer;
    char *              sFilePtr;
};


inline GenericInfo_Parser::E_Error
GenericInfo_Parser::GetLastError( UINT32 * o_pErrorLine ) const
{
    if ( o_pErrorLine != 0 )
        *o_pErrorLine = nErrorLine;
    return eErrorCode;
}


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
