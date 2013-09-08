/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
//  helper classes
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
    bool    bIsMoveable;
    public:
        SwOutlineContent(   const SwContentType* pCnt,
                            const String& rName,
                            sal_uInt16 nArrPos,
                            sal_uInt8 nLevel,
                            bool bMove,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos),
            nOutlinePos(nArrPos), nOutlineLevel(nLevel), bIsMoveable(bMove) {}

    sal_uInt16  GetPos(){return nOutlinePos;}
    sal_uInt8   GetOutlineLevel(){return nOutlineLevel;}
    bool    IsMoveable(){return bIsMoveable;};
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
    class ContentType contains information to one type of content.
    MemberArray is only populated if the content is requested by
    GetMember. It is reloaded after Invalidate() only if the content
    should be read again.
*/
//----------------------------------------------------------------------------

class SwContentType : public SwTypeNumber
{
    SwWrtShell*         pWrtShell;
    SwContentArr*       pMember;            // array for content
    String              sContentTypeName;   // name of content type
    String              sSingleContentTypeName; // name of content type, singular
    String              sTypeToken;         // attachment for URL
    sal_uInt16              nMemberCount;       // content count
    sal_uInt16              nContentType;       // content type's Id
    sal_uInt8               nOutlineLevel;
    bool                bDataValid :    1;  //
    bool                bEdit:          1;  // can this type be edited?
    bool                bDelete:        1;  // can this type be deleted?
protected:
        OUString            RemoveNewline(const OUString&);
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

        void                Invalidate(); // only nMemberCount is read again

        bool                IsEditable() const {return bEdit;}
        bool                IsDeletable() const {return bDelete;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
