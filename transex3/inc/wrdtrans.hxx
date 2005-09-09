/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrdtrans.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:57:11 $
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



