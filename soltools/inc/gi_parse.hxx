/*************************************************************************
 *
 *  $RCSfile: gi_parse.hxx,v $
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

    void                ReadLine(
                            std::istream &      i_rSrc );
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
};


inline GenericInfo_Parser::E_Error
GenericInfo_Parser::GetLastError( UINT32 * o_pErrorLine ) const
{
    if ( o_pErrorLine != 0 )
        *o_pErrorLine = nErrorLine;
    return eErrorCode;
}


#endif


