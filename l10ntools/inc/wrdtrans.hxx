/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wrdtrans.hxx,v $
 * $Revision: 1.4 $
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
/*

// OBSOLETE //

#ifndef TX3_WRDTRANS_HXX
#define TX3_WRDTRANS_HXX

// USED
    // Base Classes
    // Components
class WordTransTree;
class WordTrans_ErrorList;
    // Parameters
#include <tools/string.hxx>

class WordTransformer
{
  public:
    enum E_Error
    {
        OK = 0,
        ERROR_NO_WORDLIST,
        ERROR_HOTKEY,
        ERROR_OUTPUTSTRING_TOO_LONG,
        OTHER_ERROR
    };

    //  LIFECYCLE
                        WordTransformer();
                        ~WordTransformer();
    BOOL                LoadWordlist(           /// @return False, if file could not be read, or there is already a wordlist loaded.
                            const ByteString &  i_sWordlist_Filepath,
                            CharSet             i_nWorkingCharSet = RTL_TEXTENCODING_MS_1252,
                            CharSet             i_nFileCharSet  = RTL_TEXTENCODING_MS_1252 );

    // OPERATIONS
    USHORT              Transform(              /// @return The number of errors during transforming.
                            ByteString &        io_sText);

    // INQUIRY
    USHORT              NrOfErrors() const;
    E_Error             GetError(
                            USHORT              i_nNr,                   /// [0 .. NrOfErrors()-1], other values return an empty error.
                            ByteString *        o_pErrorText = 0) const; /// If o_pErrorText != 0, the String is filled with the description of the error.

  private:
    // SERVICE FUNCTION
    void                CreateError();

    // DATA
    WordTransTree *     dpTransformer;
    WordTrans_ErrorList *
                        dpErrors;
};



#endif


*/
