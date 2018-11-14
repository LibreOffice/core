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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CONTENT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CONTENT_HXX
#include <memory>
#include <limits.h>
#include "swcont.hxx"

#include <vcl/menu.hxx>

class SwWrtShell;
class SwContentArr;
class SwContentType;
class SwFormatField;
class SwTextINetFormat;
class SwTOXBase;
class SwRangeRedline;

//  helper classes

class SwOutlineContent : public SwContent
{
    SwOutlineNodes::size_type const nOutlinePos;
    sal_uInt8 const   nOutlineLevel;
    bool const    bIsMoveable;
    public:
        SwOutlineContent(   const SwContentType* pCnt,
                            const OUString& rName,
                            SwOutlineNodes::size_type nArrPos,
                            sal_uInt8 nLevel,
                            bool bMove,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos),
            nOutlinePos(nArrPos), nOutlineLevel(nLevel), bIsMoveable(bMove) {}

    SwOutlineNodes::size_type GetOutlinePos() const {return nOutlinePos;}
    sal_uInt8   GetOutlineLevel() const {return nOutlineLevel;}
    bool        IsMoveable() const {return bIsMoveable;};
};

class SwRegionContent : public SwContent
{

    sal_uInt8 const   nRegionLevel;

    public:
        SwRegionContent(    const SwContentType* pCnt,
                            const OUString& rName,
                            sal_uInt8 nLevel,
                            long nYPos) :
            SwContent(pCnt, rName, nYPos),
                        nRegionLevel(nLevel){}
    sal_uInt8   GetRegionLevel() const {return nRegionLevel;}
};

class SwURLFieldContent : public SwContent
{
    OUString const sURL;
    const SwTextINetFormat* pINetAttr;

public:
    SwURLFieldContent(  const SwContentType* pCnt,
                            const OUString& rName,
                            const OUString& rURL,
                            const SwTextINetFormat* pAttr,
                            long nYPos )
        : SwContent( pCnt, rName, nYPos ), sURL( rURL ), pINetAttr( pAttr )
    {}

    virtual bool        IsProtect() const override;
    const OUString&     GetURL()    const   { return sURL; }
    const SwTextINetFormat* GetINetAttr()       { return pINetAttr; }
};

class SwPostItContent : public SwContent
{
    const SwFormatField*     pField;
public:
    SwPostItContent( const SwContentType* pCnt,
                            const OUString& rName,
                            const SwFormatField* pFormatField,
                            long nYPos )
        : SwContent(pCnt, rName, nYPos)
        , pField(pFormatField)
    {}

    const SwFormatField* GetPostIt() const  { return pField; }
    virtual bool    IsProtect()     const override;
};

class SwGraphicContent : public SwContent
{
    OUString const      sLink;
public:
    SwGraphicContent(const SwContentType* pCnt, const OUString& rName, const OUString& rLink, long nYPos)
        : SwContent( pCnt, rName, nYPos ), sLink( rLink )
        {}
    virtual ~SwGraphicContent() override;

    const OUString&   GetLink() const {return sLink;}
};

class SwTOXBaseContent : public SwContent
{
    const SwTOXBase* pBase;
public:
    SwTOXBaseContent(const SwContentType* pCnt, const OUString& rName, long nYPos, const SwTOXBase& rBase)
        : SwContent( pCnt, rName, nYPos ), pBase(&rBase)
        {}
    virtual ~SwTOXBaseContent() override;

    const SwTOXBase* GetTOXBase() const {return pBase;}
};

/**
 * Content type, knows it's contents and the WrtShell.
 *
 * The class ContentType contains information to one type of content.
 * MemberArray is only populated if the content is requested by
 * GetMember. It is reloaded after Invalidate() only if the content
 * should be read again.
*/
class SwContentType : public SwTypeNumber
{
    SwWrtShell*         pWrtShell;
    std::unique_ptr<SwContentArr>
                        pMember;            // array for content
    OUString const      sContentTypeName;   // name of content type
    OUString const      sSingleContentTypeName; // name of content type, singular
    OUString            sTypeToken;         // attachment for URL
    size_t              nMemberCount;       // content count
    ContentTypeId const nContentType;       // content type's Id
    sal_uInt8           nOutlineLevel;
    bool                bDataValid :    1;
    bool                bEdit:          1;  // can this type be edited?
    bool                bDelete:        1;  // can this type be deleted?
protected:
        static OUString     RemoveNewline(const OUString&);
public:
        SwContentType(SwWrtShell* pParent, ContentTypeId nType, sal_uInt8 nLevel );
        virtual ~SwContentType() override;

        void                Init(bool* pbInvalidateWindow = nullptr);

        /** Fill the List of contents */
        void                FillMemberList(bool* pbLevelChanged = nullptr);
        size_t              GetMemberCount() const
                                {return nMemberCount;};
        ContentTypeId       GetType() const {return nContentType;}

        /** Deliver content, for that if necessary fill the list */
        const SwContent*    GetMember(size_t nIndex);
        const OUString&     GetName() {return sContentTypeName;}
        const OUString&     GetSingleName() const {return sSingleContentTypeName;}
        const OUString&     GetTypeToken() const{return sTypeToken;}

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
