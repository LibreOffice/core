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



#ifndef SOLTOOLS_ST_GILRW_HXX
#define SOLTOOLS_ST_GILRW_HXX


#include "gilacces.hxx"
#include "st_list.hxx"

class Simstr;
class List_GenericInfo;
class GenericInfo;
class GenericInfo_Parser;

class ST_InfoListReader : public GenericInfoParseTypes,
                          private GenericInfoList_Builder
{
  public:
    // LIFECYCLE
                        ST_InfoListReader();
                        ~ST_InfoListReader();
    // OPERATIONS
    bool                LoadList(
                            List_GenericInfo &  o_rList,
                            const Simstr &      i_sFileName );
    // INFO
    E_Error             GetLastError(
                            UINT32 *            o_pErrorLine = 0 ) const;
  private:
    // Interface GenericInfoList_Builder
    virtual void        AddKey(
                            const char *        i_sKey,
                            UINT32              i_nKeyLength,
                            const char *        i_sValue,
                            UINT32              i_nValueLength,
                            const char *        i_sComment,
                            UINT32              i_nCommentLength );

    virtual void        OpenList();
    virtual void        CloseList();

    // Forbid copies:
                        ST_InfoListReader( const ST_InfoListReader & );
    ST_InfoListReader & operator=( const ST_InfoListReader & );

    // DATA
    GenericInfo_Parser *
                        dpParser;

    ST_List< List_GenericInfo * >
                        aListStack;
    GenericInfo *       pCurKey;
};

class ST_InfoListWriter : public GenericInfoParseTypes,
                          private GenericInfoList_Browser
{
  public:
    // LIFECYCLE
                        ST_InfoListWriter();
                        ~ST_InfoListWriter();
    // OPERATIONS
    bool                SaveList(
                            const Simstr &      i_sFileName,
                            List_GenericInfo &  io_rList );

    // INFO
    E_Error             GetLastError() const;

  private:
    // Interface GenericInfoList_Browser
    virtual bool        Start_CurList();
    virtual bool        NextOf_CurList();

    virtual void        Get_CurKey(
                            char *              o_rKey ) const;
    virtual void        Get_CurValue(
                            char *              o_rValue ) const;
    virtual void        Get_CurComment(
                            char *              o_rComment ) const;
    virtual bool        HasSubList_CurKey() const;

    virtual void        Push_CurList();
    virtual void        Pop_CurList();

    // Forbid copies:
                        ST_InfoListWriter( const ST_InfoListWriter & );
    ST_InfoListWriter & operator=( const ST_InfoListWriter & );

    // DATA
    GenericInfo_Parser *
                        dpParser;

    ST_List< List_GenericInfo * >
                        aListStack;
    GenericInfo *       pCurKey;
};

#endif

