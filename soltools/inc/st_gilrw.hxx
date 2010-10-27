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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
