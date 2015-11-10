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
    struct SW_DLLPUBLIC DocDisposingHint final : public SfxHint
    {
        DocDisposingHint() {}
        virtual ~DocDisposingHint();
    };
}

class SwUnoCrsr : public virtual SwCursor, public SwModify
{
private:
    bool m_bRemainInSection : 1;
    bool m_bSkipOverHiddenSections : 1;
    bool m_bSkipOverProtectSections : 1;

public:
    SwUnoCrsr( const SwPosition &rPos, SwPaM* pRing = nullptr );
    virtual ~SwUnoCrsr();

protected:

    virtual const SwContentFrm* DoSetBidiLevelLeftRight(
        bool & io_rbLeft, bool bVisualAllowed, bool bInsertCrsr) override;
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

    DECL_FIXEDMEMPOOL_NEWDEL( SwUnoCrsr )
};

class SwUnoTableCrsr : public virtual SwUnoCrsr, public virtual SwTableCursor
{
    // The selection has the same order as the table boxes, i.e.
    // if something is deleted from the one array at a certain position
    // it has also to be deleted from the other!
    SwCursor m_aTableSel;

    using SwTableCursor::MakeBoxSels;

public:
    SwUnoTableCrsr( const SwPosition& rPos );
    virtual ~SwUnoTableCrsr();

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
            UnoCursorPointer(std::shared_ptr<SwUnoCrsr> pCursor, bool bSectionRestricted=false)
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
                    if(typeid(rHint) == typeid(DocDisposingHint))
                        m_pCursor->Remove(this);
                    else if(m_bSectionRestricted && typeid(rHint) == typeid(LegacyModifyHint))
                    {
                        const auto pLegacyHint = static_cast<const LegacyModifyHint*>(&rHint);
                        if(pLegacyHint->m_pOld && pLegacyHint->m_pOld->Which() == RES_UNOCURSOR_LEAVES_SECTION)
                            m_pCursor->Remove(this);
                    }
                }
                if(!GetRegisteredIn())
                    m_pCursor.reset();
            };
            SwUnoCrsr& operator*() const
                { return *m_pCursor.get(); }
            SwUnoCrsr* operator->() const
                { return m_pCursor.get(); }
            UnoCursorPointer& operator=(UnoCursorPointer aOther)
            {
                if(aOther.m_pCursor)
                    aOther.m_pCursor->Add(this);
                m_pCursor = aOther.m_pCursor;
                return *this;
            }
            explicit operator bool() const
                { return static_cast<bool>(m_pCursor); }
            void reset(std::shared_ptr<SwUnoCrsr> pNew)
            {
                if(pNew)
                    pNew->Add(this);
                else if(m_pCursor)
                    m_pCursor->Remove(this);
                m_pCursor = pNew;
            }
        private:
            std::shared_ptr<SwUnoCrsr> m_pCursor;
            const bool m_bSectionRestricted;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
