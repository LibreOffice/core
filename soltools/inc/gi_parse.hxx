/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gi_parse.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:25:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


