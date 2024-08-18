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

#include "swcrsr.hxx"
#include <svl/SfxBroadcaster.hxx>
#include <svl/lstner.hxx>
#include <utility>

namespace sw
{
    struct SW_DLLPUBLIC UnoCursorHint final : public SfxHint
    {
        UnoCursorHint() {}
        virtual ~UnoCursorHint() override;
    };
}

class SAL_DLLPUBLIC_RTTI SwUnoCursor : public virtual SwCursor
{
private:
    bool m_bRemainInSection : 1;
    bool m_bSkipOverHiddenSections : 1;
    bool m_bSkipOverProtectSections : 1;

public:
    SfxBroadcaster m_aNotifier;
    SwUnoCursor( const SwPosition &rPos );
    virtual ~SwUnoCursor() override;

protected:

    virtual const SwContentFrame* DoSetBidiLevelLeftRight(
        bool & io_rbLeft, bool bVisualAllowed, bool bInsertCursor) override;
    virtual void DoSetBidiLevelUpDown() override;

public:

    // Does a selection of content exist in table?
    // Return value indicates if the cursor remains at its old position.
    virtual bool IsSelOvr( SwCursorSelOverFlags eFlags =
                                 SwCursorSelOverFlags::CheckNodeSection |
                                 SwCursorSelOverFlags::Toggle |
                                 SwCursorSelOverFlags::ChangePos ) override;

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
};

class SwUnoTableCursor final : public virtual SwUnoCursor, public virtual SwTableCursor
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
    virtual bool IsSelOvr( SwCursorSelOverFlags eFlags =
                                 SwCursorSelOverFlags::CheckNodeSection |
                                 SwCursorSelOverFlags::Toggle |
                                 SwCursorSelOverFlags::ChangePos ) override;

    void MakeBoxSels();

          SwCursor& GetSelRing()            { return m_aTableSel; }
    const SwCursor& GetSelRing() const      { return m_aTableSel; }
};

namespace sw
{
    class UnoCursorPointer final : public SfxListener
    {
        public:
            UnoCursorPointer()
           {}
            UnoCursorPointer(std::shared_ptr<SwUnoCursor> pCursor)
                : m_pCursor(std::move(pCursor))
            {
                StartListening(m_pCursor->m_aNotifier);
            }
            UnoCursorPointer(const UnoCursorPointer& rOther)
                : SfxListener()
                , m_pCursor(rOther.m_pCursor)
            {
                if(m_pCursor)
                    StartListening(m_pCursor->m_aNotifier);
            }
            virtual ~UnoCursorPointer() override
            {
                if(m_pCursor)
                    EndListening(m_pCursor->m_aNotifier);
            }
            virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override
            {
                if(m_pCursor)
                {
                    if(typeid(rHint) == typeid(UnoCursorHint))
                        EndListening(rBC);
                }
                if(!GetBroadcasterCount())
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
                if (m_pCursor)
                {
                    EndListening(m_pCursor->m_aNotifier);
                }
                if(aOther.m_pCursor)
                    StartListening(aOther.m_pCursor->m_aNotifier);
                m_pCursor = aOther.m_pCursor;
                return *this;
            }
            explicit operator bool() const
                { return static_cast<bool>(m_pCursor); }
            void reset(const std::shared_ptr<SwUnoCursor>& pNew)
            {
                if(pNew)
                    StartListening(pNew->m_aNotifier);
                if (m_pCursor)
                    EndListening(m_pCursor->m_aNotifier);
                m_pCursor = pNew;
            }
        private:
            std::shared_ptr<SwUnoCursor> m_pCursor;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
