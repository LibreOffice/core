/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <limits.h>     // USHRT_MAX
#include "swcont.hxx"

#include <vcl/menu.hxx>

class SwWrtShell;
class SwContentArr;
class SwContentType;
class SwFmtFld;
class SwTxtINetFmt;
class SwTOXBase;
class SwRedline;

//----------------------------------------------------------------------------
//  Hilfsklassen
//----------------------------------------------------------------------------

class SwPopup : public PopupMenu
{
    USHORT          nCurId;
    virtual void    Select(){nCurId = GetCurItemId();}

public:
    SwPopup() :
        PopupMenu(),
        nCurId(USHRT_MAX){}

    USHORT          GetCurId() const { return nCurId; }

};

//----------------------------------------------------------------------------

class SwOutlineContent : public SwContent
{
    USHORT  nOutlinePos;
    BYTE    nOutlineLevel;
    BOOL    bIsMoveable;
    public:
        SwOutlineContent(   const SwContentType* pCnt,
                            const String& rName,
                            USHORT nArrPos,
                            BYTE nLevel,
                            BOOL bMove,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos),
            nOutlinePos(nArrPos), nOutlineLevel(nLevel), bIsMoveable(bMove) {}

    USHORT  GetPos(){return nOutlinePos;}
    BYTE    GetOutlineLevel(){return nOutlineLevel;}
    BOOL    IsMoveable(){return bIsMoveable;};
};

//----------------------------------------------------------------------------
class SwRegionContent : public SwContent
{

    BYTE    nRegionLevel;

    public:
        SwRegionContent(    const SwContentType* pCnt,
                            const String& rName,
                            BYTE nLevel,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos),
                        nRegionLevel(nLevel){}
    BYTE    GetRegionLevel() const {return nRegionLevel;}
};
//----------------------------------------------------------------------------

class SwURLFieldContent : public SwContent
{
    String sURL;
    const SwTxtINetFmt* pINetAttr;

public:
    SwURLFieldContent(  const SwContentType* pCnt,
                            const String& rName,
                            const String& rURL,
                            const SwTxtINetFmt* pAttr,
                            long nYPos )
        : SwContent( pCnt, rName, nYPos ), sURL( rURL ), pINetAttr( pAttr )
    {}

    virtual BOOL    IsProtect() const;
    const String&   GetURL()                { return sURL; }
    const SwTxtINetFmt* GetINetAttr()       { return pINetAttr; }
};

//----------------------------------------------------------------------------
class SwPostItContent : public SwContent
{
    const SwFmtFld*     pFld;
    SwRedline*          pRedline;
    bool                mbPostIt;
public:
    SwPostItContent( const SwContentType* pCnt,
                            const String& rName,
                            const SwFmtFld* pField,
                            long nYPos )
        : SwContent( pCnt, rName, nYPos ), pFld( pField ),mbPostIt(true)
    {}
    SwPostItContent( const SwContentType* pCnt,
                            const String& rName,
                            SwRedline* pRed,
                            long nYPos )
        : SwContent( pCnt, rName, nYPos ), pRedline( pRed ),mbPostIt(false)
    {}

    const SwFmtFld* GetPostIt()     { return pFld; }
    SwRedline* GetRedline() { return pRedline; }
    virtual BOOL    IsProtect()     const;
    bool            IsPostIt()      {return mbPostIt; }
};

//----------------------------------------------------------------------------

class SwGraphicContent : public SwContent
{
    String      sLink;
public:
    SwGraphicContent(const SwContentType* pCnt, const String& rName, const String& rLink, long nYPos)
        : SwContent( pCnt, rName, nYPos ), sLink( rLink )
        {}
    virtual ~SwGraphicContent();

    const String&   GetLink() const {return sLink;}
};

//----------------------------------------------------------------------------
class SwTOXBaseContent : public SwContent
{
    const SwTOXBase* pBase;
public:
    SwTOXBaseContent(const SwContentType* pCnt, const String& rName, long nYPos, const SwTOXBase& rBase)
        : SwContent( pCnt, rName, nYPos ), pBase(&rBase)
        {}
    virtual ~SwTOXBaseContent();

    const SwTOXBase* GetTOXBase() const {return pBase;}
};
/*
    class ContentType enthaelt Informationen zu einer Inhaltsform
    Das MemberArray wird nur gefuellt, wenn der Inhalt mit GetMember
    angefordert wird. Es wird nach Invalidate() nur dann neu geladen,
    wenn der Inhalt erneut ausgelesen werden soll.
*/
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class SwContentType : public SwTypeNumber
{
    SwWrtShell*         pWrtShell;
    SwContentArr*       pMember;            // Array fuer Inhalte
    String              sContentTypeName;   // Name der Inhaltsform
    String              sSingleContentTypeName; // Name der Inhaltsform, Singular
    String              sTypeToken;         // Anhaengsel fuer URL
    USHORT              nMemberCount;       // Inhaltsanzahl
    USHORT              nContentType;       // Id der Inhaltsform
    BYTE                nOutlineLevel;
    BOOL                bMemberFilled : 1;  // wurden die Inhalte bereits eingefuegt?
    BOOL                bIsInternalDrag:1;  // koennen die Inhalte verschoben werden?
    BOOL                bDataValid :    1;  //
    BOOL                bEdit:          1;  // kann diese Type bearbeitet werden ?
    BOOL                bDelete:        1;  // kann diese Type geloescht werden ?
protected:
        void            RemoveNewline(String&);
public:
        SwContentType(SwWrtShell* pParent, USHORT nType, BYTE nLevel );
        ~SwContentType();

        void                Init(BOOL* pbInvalidateWindow = 0);
        void                FillMemberList(BOOL* pbLevelChanged = NULL);
        USHORT              GetMemberCount() const
                                {return nMemberCount;};
        USHORT              GetType() const {return nContentType;}
        const SwContent*    GetMember(USHORT nIndex);
        const String&       GetName() {return sContentTypeName;}
        const String&       GetSingleName() const {return sSingleContentTypeName;}
        const String&       GetTypeToken() const{return sTypeToken;}

        void                SetOutlineLevel(BYTE nNew)
                            {
                                nOutlineLevel = nNew;
                                Invalidate();
                            }

        void                Invalidate(); // nur nMemberCount wird neu gelesen

        BOOL                IsEditable() const {return bEdit;}
        BOOL                IsDeletable() const {return bDelete;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
