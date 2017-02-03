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
#ifndef INCLUDED_SW_INC_CHPFLD_HXX
#define INCLUDED_SW_INC_CHPFLD_HXX

#include "fldbas.hxx"

class SwFrame;
class SwContentNode;
class SwTextNode;
class ToxTextGeneratorTest;

enum SwChapterFormat
{
    CF_BEGIN,
    CF_NUMBER = CF_BEGIN,       ///< only the chapter number
    CF_TITLE,                   ///< only the title
    CF_NUM_TITLE,               ///< number and title
    CF_NUMBER_NOPREPST,         ///< only chapter number without post-/prefix
    CF_NUM_NOPREPST_TITLE,      ///< chapter number without post-/prefix and title
};

class SW_DLLPUBLIC SwChapterFieldType : public SwFieldType
{
public:
    SwChapterFieldType();

    virtual SwFieldType*    Copy() const override;

};

class SW_DLLPUBLIC SwChapterField : public SwField
{
    friend class SwChapterFieldType;
    friend class ToxTextGeneratorTest; // the unittest needs to mock the chapter fields.
    sal_uInt8 nLevel;
    OUString sTitle;
    OUString sNumber;
    OUString sPre;
    OUString sPost;

    virtual OUString Expand() const override;
    virtual SwField* Copy() const override;

public:
    SwChapterField(SwChapterFieldType*, sal_uInt32 nFormat = 0);

    // #i53420#
    void ChangeExpansion( const SwFrame*,
                          const SwContentNode*,
        bool bSrchNum = false);
    void ChangeExpansion(const SwTextNode &rNd, bool bSrchNum);

    inline sal_uInt8 GetLevel() const;
    inline void SetLevel(sal_uInt8);

    inline const OUString& GetNumber() const;
    inline const OUString& GetTitle() const;
    virtual bool         QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool         PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline sal_uInt8 SwChapterField::GetLevel() const   { return nLevel; }
inline void SwChapterField::SetLevel(sal_uInt8 nLev) { nLevel = nLev; }
inline const OUString& SwChapterField::GetNumber() const { return sNumber; }
inline const OUString& SwChapterField::GetTitle() const { return sTitle; }

#endif // INCLUDED_SW_INC_CHPFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
