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
    sal_Bool                bDataValid :    1;  //
    sal_Bool                bEdit:          1;  // can this type be edited?
    sal_Bool                bDelete:        1;  // can this type be deleted?
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

        void                Invalidate(); // only nMemberCount is read again

        sal_Bool                IsEditable() const {return bEdit;}
        sal_Bool                IsDeletable() const {return bDelete;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
