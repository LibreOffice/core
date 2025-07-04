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
#pragma once

#include "fldbas.hxx"

class SwFrame;
class SwContentNode;
class SwTextNode;
class SwRootFrame;

enum SwChapterFormat
{
    CF_BEGIN,
    CF_NUMBER = CF_BEGIN,       ///< only the chapter number
    CF_TITLE,                   ///< only the title
    CF_NUM_TITLE,               ///< number and title
    CF_NUMBER_NOPREPST,         ///< only chapter number without post-/prefix
    CF_NUM_NOPREPST_TITLE,      ///< chapter number without post-/prefix and title
};

class SAL_DLLPUBLIC_RTTI SwChapterFieldType final : public SwFieldType
{
public:
    SwChapterFieldType();

    virtual std::unique_ptr<SwFieldType> Copy() const override;

};

class SW_DLLPUBLIC SwChapterField final : public SwField
{
    friend class SwChapterFieldType;
    friend class ToxTextGeneratorTest; // the unittest needs to mock the chapter fields.

    struct State
    {
        sal_uInt8 nLevel;
        OUString sTitle;
        OUString sNumber;
        OUString sLabelFollowedBy;
        OUString sPre;
        OUString sPost;
        State() : nLevel(0) {}
    };
    State m_State;
    State m_StateRLHidden;
    sal_uInt32 m_nFormat;

    virtual OUString ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwChapterField(SwChapterFieldType*, sal_uInt32 nFormat = 0);

    sal_uInt32 GetFormat() const { return m_nFormat; }
    void SetFormat(sal_uInt32 n) { m_nFormat = n; }

    // #i53420#
    void ChangeExpansion( const SwFrame&,
                          const SwContentNode*,
        bool bSrchNum = false);
    void ChangeExpansion(const SwTextNode &rNd, bool bSrchNum, SwRootFrame const* pLayout = nullptr);

    sal_uInt8 GetLevel(SwRootFrame const* pLayout = nullptr) const;
    void SetLevel(sal_uInt8);

    const OUString& GetNumber(SwRootFrame const* pLayout = nullptr) const;
    const OUString& GetTitle(SwRootFrame const* pLayout = nullptr) const;

    virtual bool         QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool         PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
