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
#ifndef INCLUDED_SW_INC_UNOCRSR_HXX
#define INCLUDED_SW_INC_UNOCRSR_HXX

#include <swcrsr.hxx>
#include <calbck.hxx>

namespace sw
{
    enum class UnoCursorHintType
    {
        DOC_DISPOSING,
        LEAVES_SECTION
    };

    struct SW_DLLPUBLIC UnoCursorHint final : public SfxHint
    {
        UnoCursorHintType m_eType;
        UnoCursorHint(UnoCursorHintType eType)
                : m_eType(eType) {};
        virtual ~UnoCursorHint() override;
    };
}

class SwUnoCursor : public virtual SwCursor, public SwModify
{
private:
    bool m_bRemainInSection : 1;
    bool m_bSkipOverHiddenSections : 1;
    bool m_bSkipOverProtectSections : 1;

public:
    SwUnoCursor( const SwPosition &rPos );
    virtual ~SwUnoCursor() override;

protected:

    virtual const SwContentFrame* DoSetBidiLevelLeftRight(
        bool & io_rbLeft, bool bVisualAllowed, bool bInsertCursor) override;
    virtual void DoSetBidiLevelUpDown() override;

public:

    // Does a selection of content exist in table?
    // Return value indicates if the cursor remains at its old position.
    virtual bool IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS )) override;

    virtual bool IsReadOnlyAvailable() const override;

    bool IsRemainInSection() const          { return m_bRemainInSection; }
    void SetRemainInSection( bool bFlag )   { m_bRemainInSection = bFlag; }

    virtual bool IsSkipOverProtectSections() const override
                                    { return m_bSkipOverProtectSections; }
    void SetSkipOverProtectSections( bool bFlag )
                                    { m_bSkipOverProtectSections = bFlag; }

    virtual bool IsSkipOverHiddenSections() const override
                                    { return m_bSkipOverHiddenSections; }
    void SetSkipOverHiddenSections( bool bFlag )
                                    { m_bSkipOverHiddenSections = bFlag; }

    DECL_FIXEDMEMPOOL_NEWDEL( SwUnoCursor )
};

class SwUnoTableCursor : public virtual SwUnoCursor, public virtual SwTableCursor
{
    // The selection has the same order as the table boxes, i.e.
    // if something is deleted from the one array at a certain position
    // it has also to be deleted from the other!
    SwCursor m_aTableSel;

    using SwTableCursor::MakeBoxSels;

public:
    SwUnoTableCursor( const SwPosition& rPos );
    virtual ~SwUnoTableCursor() override;

    // Does a selection of content exist in table?
    // Return value indicates if the cursor remains at its old position.
    virtual bool IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS )) override;

    void MakeBoxSels();

          SwCursor& GetSelRing()            { return m_aTableSel; }
    const SwCursor& GetSelRing() const      { return m_aTableSel; }
};

namespace sw
{
    class UnoCursorPointer : public SwClient
    {
        public:
            UnoCursorPointer()
                : m_pCursor(nullptr), m_bSectionRestricted(false)
            {}
            UnoCursorPointer(std::shared_ptr<SwUnoCursor> pCursor, bool bSectionRestricted=false)
                : m_pCursor(pCursor), m_bSectionRestricted(bSectionRestricted)
            {
                m_pCursor->Add(this);
            }
            UnoCursorPointer(const UnoCursorPointer& rOther)
                : SwClient(nullptr)
                , m_pCursor(rOther.m_pCursor)
                , m_bSectionRestricted(rOther.m_bSectionRestricted)
            {
                if(m_pCursor)
                    m_pCursor->Add(this);
            }
            virtual ~UnoCursorPointer() override
            {
                if(m_pCursor)
                    m_pCursor->Remove(this);
            }
            virtual void SwClientNotify(const SwModify& rModify, const SfxHint& rHint) override
            {
                SwClient::SwClientNotify(rModify, rHint);
                if(m_pCursor)
                {
                    if(typeid(rHint) == typeid(UnoCursorHint))
                        m_pCursor->Remove(this);
                }
                if(!GetRegisteredIn())
                    m_pCursor.reset();
            };
            SwUnoCursor* get() const
                { return m_pCursor.get(); }
            SwUnoCursor* operator->() const
                { return get(); }
            SwUnoCursor& operator*() const
                { return *get(); }
            UnoCursorPointer& operator=(UnoCursorPointer aOther)
            {
                if(aOther.m_pCursor)
                    aOther.m_pCursor->Add(this);
                m_pCursor = aOther.m_pCursor;
                return *this;
            }
            explicit operator bool() const
                { return static_cast<bool>(m_pCursor); }
            void reset(std::shared_ptr<SwUnoCursor> pNew)
            {
                if(pNew)
                    pNew->Add(this);
                else if(m_pCursor)
                    m_pCursor->Remove(this);
                m_pCursor = pNew;
            }
        private:
            std::shared_ptr<SwUnoCursor> m_pCursor;
            const bool m_bSectionRestricted;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
