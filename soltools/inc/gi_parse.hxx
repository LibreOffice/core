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


