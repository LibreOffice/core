/*************************************************************************
 *
 *  $RCSfile: wrdtrans.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nf $ $Date: 2001-06-26 12:56:08 $
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



