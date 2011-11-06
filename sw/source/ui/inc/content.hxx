/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CONTENT_HXX
#define _CONTENT_HXX
#include <limits.h>     // USHRT_MAX
#include "swcont.hxx"

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

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
    sal_uInt16          nCurId;
    virtual void    Select(){nCurId = GetCurItemId();}

public:
    SwPopup() :
        PopupMenu(),
        nCurId(USHRT_MAX){}

    sal_uInt16          GetCurId() const { return nCurId; }

};

//----------------------------------------------------------------------------

class SwOutlineContent : public SwContent
{
    sal_uInt16  nOutlinePos;
    sal_uInt8   nOutlineLevel;
    sal_Bool    bIsMoveable;
    public:
        SwOutlineContent(   const SwContentType* pCnt,
                            const String& rName,
                            sal_uInt16 nArrPos,
                            sal_uInt8 nLevel,
                            sal_Bool bMove,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos),
            nOutlinePos(nArrPos), nOutlineLevel(nLevel), bIsMoveable(bMove) {}

    sal_uInt16  GetPos(){return nOutlinePos;}
    sal_uInt8   GetOutlineLevel(){return nOutlineLevel;}
    sal_Bool    IsMoveable(){return bIsMoveable;};
};

//----------------------------------------------------------------------------
class SwRegionContent : public SwContent
{

    sal_uInt8   nRegionLevel;

    public:
        SwRegionContent(    const SwContentType* pCnt,
                            const String& rName,
                            sal_uInt8 nLevel,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos),
                        nRegionLevel(nLevel){}
    sal_uInt8   GetRegionLevel() const {return nRegionLevel;}
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

    virtual sal_Bool    IsProtect() const;
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
    virtual sal_Bool    IsProtect()     const;
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
    sal_uInt16              nMemberCount;       // Inhaltsanzahl
    sal_uInt16              nContentType;       // Id der Inhaltsform
    sal_uInt8               nOutlineLevel;
    sal_Bool                bMemberFilled : 1;  // wurden die Inhalte bereits eingefuegt?
    sal_Bool                bIsInternalDrag:1;  // koennen die Inhalte verschoben werden?
    sal_Bool                bDataValid :    1;  //
    sal_Bool                bEdit:          1;  // kann diese Type bearbeitet werden ?
    sal_Bool                bDelete:        1;  // kann diese Type geloescht werden ?
protected:
        void            RemoveNewline(String&);
public:
        SwContentType(SwWrtShell* pParent, sal_uInt16 nType, sal_uInt8 nLevel );
        ~SwContentType();

        void                Init(sal_Bool* pbInvalidateWindow = 0);
        void                FillMemberList(sal_Bool* pbLevelChanged = NULL);
        sal_uInt16              GetMemberCount() const
                                {return nMemberCount;};
        sal_uInt16              GetType() const {return nContentType;}
        const SwContent*    GetMember(sal_uInt16 nIndex);
        const String&       GetName() {return sContentTypeName;}
        const String&       GetSingleName() const {return sSingleContentTypeName;}
        const String&       GetTypeToken() const{return sTypeToken;}

        void                SetOutlineLevel(sal_uInt8 nNew)
                            {
                                nOutlineLevel = nNew;
                                Invalidate();
                            }

        void                Invalidate(); // nur nMemberCount wird neu gelesen

        sal_Bool                IsEditable() const {return bEdit;}
        sal_Bool                IsDeletable() const {return bDelete;}
};

#endif

