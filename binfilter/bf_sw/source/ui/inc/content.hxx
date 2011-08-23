/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _CONTENT_HXX
#define _CONTENT_HXX

#include <limits.h>		// USHRT_MAX
#ifndef _SWCONT_HXX
#include "swcont.hxx"
#endif

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
namespace binfilter {

class SwWrtShell;
class SwContentArr;
class SwContentType;
class SwNavigationPI;
class SwFmtFld;
class SwTxtINetFmt;
class SwDocShell;
class SvDataObject;
class SwTOXBase;

//----------------------------------------------------------------------------
//	Hilfsklassen
//----------------------------------------------------------------------------

class SwPopup : public PopupMenu
{
    USHORT			nCurId;
    virtual void    Select(){nCurId = GetCurItemId();}

public:
    SwPopup() :
        PopupMenu(),
        nCurId(USHRT_MAX){}

    USHORT			GetCurId() const { return nCurId; }

};

//----------------------------------------------------------------------------

class SwOutlineContent : public SwContent
{
    USHORT	nOutlinePos;
    BYTE	nOutlineLevel;
    BOOL	bIsMoveable;
    public:
        SwOutlineContent(	const SwContentType* pCnt,
                            const String& rName,
                            USHORT nArrPos,
                            BYTE nLevel,
                            BOOL bMove,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos), nOutlineLevel(nLevel),
                        nOutlinePos(nArrPos), bIsMoveable(bMove){}

    USHORT 	GetPos(){return nOutlinePos;}
    BYTE 	GetOutlineLevel(){return nOutlineLevel;}
    BOOL	IsMoveable(){return bIsMoveable;};
};

//----------------------------------------------------------------------------
class SwRegionContent : public SwContent
{

    BYTE	nRegionLevel;

    public:
        SwRegionContent(	const SwContentType* pCnt,
                            const String& rName,
                            BYTE nLevel,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos),
                        nRegionLevel(nLevel){}
    BYTE 	GetRegionLevel() const {return nRegionLevel;}
};
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------



//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
/*
    class ContentType enthaelt Informationen zu einer Inhaltsform
    Das MemberArray wird nur gefuellt, wenn der Inhalt mit GetMember
    angefordert wird. Es wird nach Invalidate() nur dann neu geladen,
    wenn der Inhalt erneut ausgelesen werden soll.
*/
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------






} //namespace binfilter
#endif

