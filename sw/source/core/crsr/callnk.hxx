/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: callnk.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:02:51 $
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
#ifndef _CALLNK_HXX
#define _CALLNK_HXX

#include <tools/solar.h>

class SwCrsrShell;
class SwTxtNode;

class SwCallLink
{
public:
    SwCrsrShell & rShell;
    ULONG nNode;
    xub_StrLen nCntnt;
    BYTE nNdTyp;
    long nLeftFrmPos;
    bool bHasSelection;

    SwCallLink( SwCrsrShell & rSh );
    SwCallLink( SwCrsrShell & rSh, ULONG nAktNode, xub_StrLen nAktCntnt,
                                    BYTE nAktNdTyp, long nLRPos,
                                    bool bAktSelection );
    ~SwCallLink();

    static long GetFrm( SwTxtNode& rNd, xub_StrLen nCntPos, BOOL bCalcFrm );
};



#endif  // _CALLNK_HXX
