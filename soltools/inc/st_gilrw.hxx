/*************************************************************************
 *
 *  $RCSfile: st_gilrw.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-06-11 16:05:17 $
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

