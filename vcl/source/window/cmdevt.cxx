/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmdevt.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:16:05 $
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
#include "precompiled_vcl.hxx"

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _SV_CMDEVT_HXX
#include <cmdevt.hxx>
#endif

// =======================================================================

CommandExtTextInputData::CommandExtTextInputData()
{
    mpTextAttr      = NULL;
    mnCursorPos     = 0;
    mnDeltaStart    = 0;
    mnOldTextLen    = 0;
    mnCursorFlags   = 0;
    mbOnlyCursor    = FALSE;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::CommandExtTextInputData( const XubString& rText,
                                                  const USHORT* pTextAttr,
                                                  xub_StrLen nCursorPos,
                                                  USHORT nCursorFlags,
                                                  xub_StrLen nDeltaStart,
                                                  xub_StrLen nOldTextLen,
                                                  BOOL bOnlyCursor ) :
    maText( rText )
{
    if ( pTextAttr && maText.Len() )
    {
        mpTextAttr = new USHORT[maText.Len()];
        memcpy( mpTextAttr, pTextAttr, maText.Len()*sizeof(USHORT) );
    }
    else
        mpTextAttr = NULL;
    mnCursorPos     = nCursorPos;
    mnDeltaStart    = nDeltaStart;
    mnOldTextLen    = nOldTextLen;
    mnCursorFlags   = nCursorFlags;
    mbOnlyCursor    = bOnlyCursor;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::CommandExtTextInputData( const CommandExtTextInputData& rData ) :
    maText( rData.maText )
{
    if ( rData.mpTextAttr && maText.Len() )
    {
        mpTextAttr = new USHORT[maText.Len()];
        memcpy( mpTextAttr, rData.mpTextAttr, maText.Len()*sizeof(USHORT) );
    }
    else
        mpTextAttr = NULL;
    mnCursorPos     = rData.mnCursorPos;
    mnDeltaStart    = rData.mnDeltaStart;
    mnOldTextLen    = rData.mnOldTextLen;
    mnCursorFlags   = rData.mnCursorFlags;
    mbOnlyCursor    = rData.mbOnlyCursor;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::~CommandExtTextInputData()
{
    if ( mpTextAttr )
        delete [] mpTextAttr;
}
