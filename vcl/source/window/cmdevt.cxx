/*************************************************************************
 *
 *  $RCSfile: cmdevt.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: th $ $Date: 2000-11-03 09:06:12 $
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

#define _SV_CMDEVT_CXX

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _SV_CMDEVT_HXX
#include <cmdevt.hxx>
#endif

#pragma hdrstop

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
                                                  USHORT nCursorPos,
                                                  USHORT nCursorFlags,
                                                  USHORT nDeltaStart,
                                                  USHORT nOldTextLen,
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
    mnCursorFlags   = (BYTE)nCursorFlags;
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
        delete mpTextAttr;
}
