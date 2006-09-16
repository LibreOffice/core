/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: isofallback.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:59:30 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include "isofallback.hxx"

// -----------------------------------------------------------------------

// Return true if valid fallback found
sal_Bool GetIsoFallback( ByteString& rLanguage )
{
    rLanguage.EraseLeadingAndTrailingChars();
    if( rLanguage.Len() ){
        xub_StrLen nSepPos = rLanguage.Search( '-' );
        if ( nSepPos == STRING_NOTFOUND ){
            if ( rLanguage.Equals("en"))
            {
                // en -> ""
                rLanguage.Erase();
                return false;
            }
            else
            {
                // de -> en-US ;
                rLanguage = ByteString("en-US");
                return true;
            }
        }
        else if( !( nSepPos == 1 && ( rLanguage.GetChar(0) == 'x' || rLanguage.GetChar(0) == 'X' ) ) )
        {
            // de-CH -> de ;
            // try erase from -
            rLanguage = rLanguage.GetToken( 0, '-');
            return true;
        }
    }
    // "" -> ""; x-no-translate -> ""
    rLanguage.Erase();
    return false;
}

