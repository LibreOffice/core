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
#include "swcont.hxx"

#include <ndarr.hxx>
#include <tools/long.hxx>

class SwWrtShell;
class SwContentArr;
class SwContentType;
class SwFormatField;
class SwTextINetFormat;
class SwTOXBase;
class SwRangeRedline;
class SwTextFootnote;

//  helper classes

class SwOutlineContent final : public SwContent
{
    SwOutlineNodes::size_type nOutlinePos;
    sal_uInt8   nOutlineLevel;
    bool    bIsMoveable;
    public:
        SwOutlineContent(   const SwContentType* pCnt,
                            const OUString& rName,
                            SwOutlineNodes::size_type nArrPos,
                            sal_uInt8 nLevel,
                            bool bMove,
                            tools::Long nYPos) :
            SwContent(pCnt, rName, nYPos),
            nOutlinePos(nArrPos), nOutlineLevel(nLevel), bIsMoveable(bMove) {}

    SwOutlineNodes::size_type GetOutlinePos() const {return nOutlinePos;}
    sal_uInt8   GetOutlineLevel() const {return nOutlineLevel;}
    bool        IsMoveable() const {return bIsMoveable;};
};

class SwRegionContent final : public SwContent
{

    sal_uInt8   nRegionLevel;

    public:
        SwRegionContent(    const SwContentType* pCnt,
                            const OUString& rName,
                            sal_uInt8 nLevel,
                            tools::Long nYPos) :
            SwContent(pCnt, rName, nYPos),
                        nRegionLevel(nLevel){}
    sal_uInt8   GetRegionLevel() const {return nRegionLevel;}
};

class SwURLFieldContent final : public SwContent
{
    OUString sURL;
    const SwTextINetFormat* pINetAttr;

public:
    SwURLFieldContent(  const SwContentType* pCnt,
                            const OUString& rName,
                            const OUString& rURL,
                            const SwTextINetFormat* pAttr,
                            tools::Long nYPos )
        : SwContent( pCnt, rName, nYPos ), sURL( rURL ), pINetAttr( pAttr )
    {}

    virtual bool        IsProtect() const override;
    const OUString&     GetURL()    const   { return sURL; }
    const SwTextINetFormat* GetINetAttr() const { return pINetAttr; }
};

class SwTextFieldContent final : public SwContent
{
    const SwFormatField* m_pFormatField;
public:
    SwTextFieldContent(const SwContentType* pCnt,
                       const OUString& rName,
                       const SwFormatField* pFormatField,
                       tools::Long nYPos)
        : SwContent(pCnt, rName, nYPos),
          m_pFormatField(pFormatField)
    {}

    const SwFormatField* GetFormatField() const {return m_pFormatField;}
    virtual bool IsProtect() const override;
};

class SwTextFootnoteContent final : public SwContent
{
    const SwTextFootnote* m_pTextFootnote;
public:
    SwTextFootnoteContent(const SwContentType* pCnt,
                          const OUString& rName,
                          const SwTextFootnote* pTextFootnote,
                          tools::Long nYPos)
        : SwContent(pCnt, rName, nYPos),
          m_pTextFootnote(pTextFootnote)
    {}

    const SwTextFootnote* GetTextFootnote() const {return m_pTextFootnote;}
};

class SwPostItContent final : public SwContent
{
    const SwFormatField*     pField;
public:
    SwPostItContent( const SwContentType* pCnt,
                            const OUString& rName,
                            const SwFormatField* pFormatField,
                            tools::Long nYPos )
        : SwContent(pCnt, rName, nYPos)
        , pField(pFormatField)
    {}

    const SwFormatField* GetPostIt() const  { return pField; }
    virtual bool    IsProtect()     const override;
};

class SwGraphicContent final : public SwContent
{
    OUString      sLink;
public:
    SwGraphicContent(const SwContentType* pCnt, const OUString& rName, const OUString& rLink, tools::Long nYPos)
        : SwContent( pCnt, rName, nYPos ), sLink( rLink )
        {}
    virtual ~SwGraphicContent() override;

    const OUString&   GetLink() const {return sLink;}
};

class SwTOXBaseContent final : public SwContent
{
    const SwTOXBase* pBase;
public:
    SwTOXBaseContent(const SwContentType* pCnt, const OUString& rName, tools::Long nYPos, const SwTOXBase& rBase)
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
class SwContentType final : public SwTypeNumber
{
    SwWrtShell*         m_pWrtShell;
    std::unique_ptr<SwContentArr>
                        m_pMember;            // array for content
    OUString            m_sContentTypeName;   // name of content type
    OUString            m_sSingleContentTypeName; // name of content type, singular
    OUString            m_sTypeToken;         // attachment for URL
    size_t              m_nMemberCount;       // content count
    ContentTypeId       m_nContentType;       // content type's Id
    sal_uInt8           m_nOutlineLevel;
    bool                m_bDataValid :    1;
    bool                m_bEdit:          1;  // can this type be edited?
    bool                m_bDelete:        1;  // can this type be deleted?

        static OUString     RemoveNewline(const OUString&);
public:
        SwContentType(SwWrtShell* pParent, ContentTypeId nType, sal_uInt8 nLevel );
        virtual ~SwContentType() override;

        /** Fill the List of contents */
        void                FillMemberList(bool* pbLevelChanged = nullptr);
        size_t              GetMemberCount() const
                                {return m_nMemberCount;};
        ContentTypeId       GetType() const {return m_nContentType;}

        /** Deliver content, for that if necessary fill the list */
        const SwContent*    GetMember(size_t nIndex);
        const OUString&     GetName() const {return m_sContentTypeName;}
        const OUString&     GetSingleName() const {return m_sSingleContentTypeName;}
        const OUString&     GetTypeToken() const{return m_sTypeToken;}

        void                SetOutlineLevel(sal_uInt8 nNew)
                            {
                                m_nOutlineLevel = nNew;
                                Invalidate();
                            }

        void                Invalidate(); // only nMemberCount is read again

        bool                IsEditable() const {return m_bEdit;}
        bool                IsDeletable() const {return m_bDelete;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
