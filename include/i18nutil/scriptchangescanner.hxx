/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <i18nutil/i18nutildllapi.h>
#include <com/sun/star/text/ScriptHintType.hpp>
#include <rtl/ustring.hxx>
#include <optional>
#include <memory>
#include <vector>

namespace i18nutil
{
struct DirectionChange
{
    sal_Int32 m_nStartIndex = 0;
    sal_Int32 m_nEndIndex = 0;
    sal_uInt8 m_nLevel = 0;
    bool m_bHasEmbeddedStrongLTR = false;
};

struct ScriptChange
{
    sal_Int32 m_nStartIndex = 0;
    sal_Int32 m_nEndIndex = 0;
    sal_Int16 m_nScriptType = 0;
};

enum class ScriptHintType : sal_uInt16
{
    Automatic = css::text::ScriptHintType::AUTOMATIC,
    Ignore = css::text::ScriptHintType::IGNORE,
    Latin = css::text::ScriptHintType::LATIN,
    Asian = css::text::ScriptHintType::ASIAN,
    Complex = css::text::ScriptHintType::COMPLEX
};

struct ScriptHint
{
    sal_Int32 m_nStartIndex = 0;
    sal_Int32 m_nEndIndex = 0;
    ScriptHintType m_eType = ScriptHintType::Automatic;

    ScriptHint(ScriptHintType eType, sal_Int32 nStartIndex, sal_Int32 nEndIndex)
        : m_nStartIndex(nStartIndex)
        , m_nEndIndex(nEndIndex)
        , m_eType(eType)
    {
    }
};

class I18NUTIL_DLLPUBLIC ScriptHintProvider
{
private:
    std::vector<ScriptHint> m_aHints;
    ScriptHintType m_eParaHint = ScriptHintType::Automatic;

    std::vector<ScriptHint>::iterator m_pCurrIt;
    ScriptHintType m_eCurrHint = ScriptHintType::Automatic;
    sal_Int32 m_nCurrIdx = 0;

public:
    void SetParagraphLevelHint(ScriptHintType eType);
    void AddHint(ScriptHintType eType, sal_Int32 nStartIndex, sal_Int32 nEndIndex);

    void Start();
    void AdvanceTo(sal_Int32 nNextIdx);
    ScriptHintType Peek() const;
};

class I18NUTIL_DLLPUBLIC DirectionChangeScanner
{
public:
    virtual ~DirectionChangeScanner() = default;

    virtual bool AtEnd() const = 0;
    virtual void Advance() = 0;
    virtual DirectionChange Peek() const = 0;
    virtual sal_uInt8 GetLevelAt(sal_Int32 nIndex) const = 0;
    virtual void Reset() = 0;
};

class I18NUTIL_DLLPUBLIC ScriptChangeScanner
{
public:
    virtual ~ScriptChangeScanner() = default;

    virtual bool AtEnd() const = 0;
    virtual void Advance() = 0;
    virtual ScriptChange Peek() const = 0;
};

I18NUTIL_DLLPUBLIC std::unique_ptr<DirectionChangeScanner>
MakeDirectionChangeScanner(const OUString& rWord, sal_uInt8 nInitialDirection);

I18NUTIL_DLLPUBLIC std::unique_ptr<ScriptChangeScanner>
MakeScriptChangeScanner(const OUString& rWord, sal_Int16 nDefaultScriptType,
                        DirectionChangeScanner& rDirScanner, ScriptHintProvider& rHints);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
