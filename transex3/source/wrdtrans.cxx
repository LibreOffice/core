/*************************************************************************
 *
 *  $RCSfile: wrdtrans.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:26 $
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


#include "wrdtrans.hxx"


// NOT FULLY DECLARED SERVICES
    // STL einbinden
#if STLPORT_VERSION < 321
#include <tools/presys.h>
#include <vector.h>
#include <tools/postsys.h>
#else
#include <vector>
#endif  // STLPORT_VERSION < 321
#include <vos/macros.hxx>


#include <tools/stream.hxx>
#include "wtratree.hxx"

#include <tools/string.hxx>

//**************   Declaration WordTrans_ErrorList   ******************//
typedef NAMESPACE_STD(vector)<ByteString> Stl_ByteStringList;

class WordTrans_ErrorList
{
  public:
    // OPERATIONS
    void                AddError(
                            WordTransformer::E_Error
                                                i_eType,
                            const char *        i_sErrorDescription );
    void                Clear();                /// Empties the list.

    // INQUIRY
    USHORT              NrOfErrors() const;
    WordTransformer::E_Error
                        GetError(
                            USHORT              i_nNr,                   /// [0 .. NrOfErrors()-1], other values return an empty error.
                            ByteString *        o_pErrorText ) const;    /// If o_pErrorText != 0, the String is filled with the description of the error.
  private:
    // DATA
    Stl_ByteStringList      aErrors;
};



//**************   Implementation WordTransformer     ******************//


WordTransformer::WordTransformer()
    :   dpTransformer(0),
        dpErrors(new WordTrans_ErrorList)
{
}

WordTransformer::~WordTransformer()
{
    if (dpTransformer != 0)
        delete dpTransformer;
    delete dpErrors;
}

BOOL
WordTransformer::LoadWordlist( const ByteString &  i_sWordlist_Filepath,
                               CharSet         i_nWorkingCharSet,
                               CharSet         i_nFileCharSet )
{
    if (dpTransformer != 0)
        return FALSE;

    SvFileStream    aFile(String(i_sWordlist_Filepath,RTL_TEXTENCODING_ASCII_US),STREAM_STD_READ);
    if (! aFile.IsOpen())
        return FALSE;
    aFile.SetStreamCharSet( i_nFileCharSet ) ;
//  aFile.SetTargetCharSet( i_nWorkingCharSet );

    dpTransformer = new WordTransTree;

    ByteString sTrans;
    while ( aFile.ReadLine(sTrans) )
    {
        dpTransformer->AddWordPair(sTrans.GetToken(0,';'),sTrans.GetToken(1,';'));
    }

    aFile.Close();
    return TRUE;
}

USHORT
WordTransformer::Transform(ByteString & io_sText)
{
    // Initialization and precondition testing:
    dpErrors->Clear();
    if (dpTransformer == 0)
    {
        dpErrors->AddError(ERROR_NO_WORDLIST,"Error: No wordlist was loaded.");
        return dpErrors->NrOfErrors();
    }
    else if (io_sText.Len() > 63 * 1024)
    {
        dpErrors->AddError(ERROR_OUTPUTSTRING_TOO_LONG,"Error: Inputstring was too long (bigger than 63 KB).");
        return dpErrors->NrOfErrors();
    }
    else if (io_sText.Len() == 0)
    {
        return 0;
    }

    // Transform:
    dpTransformer->InitTransformation(
                            io_sText.GetBuffer(),
                            io_sText.Len() );

    for ( ; !dpTransformer->TextEndReached(); )
    {
        if (dpTransformer->TransformNextToken() != WordTransTree::OK)
        {
            CreateError();
        }
    }
    io_sText = dpTransformer->Output();
    return dpErrors->NrOfErrors();
}

USHORT
WordTransformer::NrOfErrors() const
{
    return dpErrors->NrOfErrors();
}

WordTransformer::E_Error
WordTransformer::GetError( USHORT   i_nNr,
                           ByteString * o_pErrorText) const
{
    return dpErrors->GetError(i_nNr,o_pErrorText);
}

void
WordTransformer::CreateError()
{
    ByteString sErr;

    switch (dpTransformer->CurResult())
    {
        case WordTransTree::OK:
            break;
        case WordTransTree::HOTKEY_LOST:
                sErr = ByteString("Error: By replacement of string ");
                sErr += dpTransformer->CurReplacedString();
                sErr += " by ";
                sErr += dpTransformer->CurReplacingString();
                sErr += "the hotkey at char '";
                sErr += dpTransformer->CurHotkey();
                sErr += "' was lost.";
            dpErrors->AddError( ERROR_HOTKEY,sErr.GetBufferAccess());
            sErr.ReleaseBufferAccess();
            break;
        case WordTransTree::OUTPUT_OVERFLOW:
            dpErrors->AddError(ERROR_OUTPUTSTRING_TOO_LONG,"Error: Output buffer overflow.");
            break;
        default:
            dpErrors->AddError(OTHER_ERROR,"Error: Unknown error.");
    }
}

//**************   Implementation WordTrans_ErrorList   ******************//

void
WordTrans_ErrorList::AddError( WordTransformer::E_Error i_eType,
                               const char *             i_sErrorDescription )
{
    ByteString sErrorType = "xxx";
    char * pErrorChars = sErrorType.GetBufferAccess();
    pErrorChars[0] = char(i_eType / 100 + '0');
    pErrorChars[1] = char( (i_eType % 100) / 10 + '0');
    pErrorChars[2] = char(i_eType % 10 + '0');
    sErrorType += i_sErrorDescription;

    aErrors.push_back(sErrorType);
    sErrorType.ReleaseBufferAccess();
}

void
WordTrans_ErrorList::Clear()
{
     aErrors.erase(aErrors.begin(),aErrors.end());
}

USHORT
WordTrans_ErrorList::NrOfErrors() const
{
    return aErrors.size();
}

WordTransformer::E_Error
WordTrans_ErrorList::GetError( USHORT       i_nNr,
                               ByteString * o_pErrorText ) const
{
    if (0 <= i_nNr && i_nNr < aErrors.size() )
    {
        const ByteString & rError = aErrors[i_nNr];
        const char * pErrorChars = rError.GetBuffer();

        USHORT nError = USHORT( (pErrorChars[0] - '0') ) * 100
                    +   (pErrorChars[1] - '0') * 10
                    +   pErrorChars[2] - '0';

        if (o_pErrorText != 0)
            *o_pErrorText = pErrorChars+3;

        return WordTransformer::E_Error(nError);
    }
    else
    {
        if (o_pErrorText != 0)
            *o_pErrorText = "";
        return WordTransformer::OK;
    }
}

