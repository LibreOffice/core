/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wrdtrans.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_transex3.hxx"

/*
#include "wrdtrans.hxx"


// NOT FULLY DECLARED SERVICES
#include <vector>
#include <vos/macros.hxx>


#include <tools/stream.hxx>
#include "wtratree.hxx"

#include <tools/string.hxx>
*/
//**************   Declaration WordTrans_ErrorList   ******************//
/*
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

*/

//**************   Implementation WordTransformer     ******************//

/*
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
*/
//**************   Implementation WordTrans_ErrorList   ******************//
/*
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
    if ( i_nNr < aErrors.size() )
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
*/
