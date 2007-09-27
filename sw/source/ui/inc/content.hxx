/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: content.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:56:05 $
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
#ifndef _CONTENT_HXX
#define _CONTENT_HXX
#include <limits.h>     // USHRT_MAX
#ifndef _SWCONT_HXX
#include "swcont.hxx"
#endif

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

class SwWrtShell;
class SwContentArr;
class SwContentType;
class SwFmtFld;
class SwTxtINetFmt;
class SwTOXBase;

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
public:
    SwPostItContent( const SwContentType* pCnt,
                            const String& rName,
                            const SwFmtFld* pField,
                            long nYPos )
        : SwContent( pCnt, rName, nYPos ), pFld( pField )
    {}

    const SwFmtFld* GetPostIt()     { return pFld; }
    virtual BOOL    IsProtect() const;
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

