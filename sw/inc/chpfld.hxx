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
#ifndef SW_CHPFLD_HXX
#define SW_CHPFLD_HXX

#include "fldbas.hxx"


class SwFrm;
class SwCntntNode;
class SwTxtNode;


enum SwChapterFormat
{
    CF_BEGIN,
    CF_NUMBER = CF_BEGIN,       ///< only the chapter number
    CF_TITLE,                   ///< only the title
    CF_NUM_TITLE,               ///< number and title
    CF_NUMBER_NOPREPST,         ///< only chapter number without post-/prefix
    CF_NUM_NOPREPST_TITLE,      ///< chapter number without post-/prefix and title
    CF_END
};

class SwChapterFieldType : public SwFieldType
{
public:
    SwChapterFieldType();

    virtual SwFieldType*    Copy() const;

};

class SW_DLLPUBLIC SwChapterField : public SwField
{
    friend class SwChapterFieldType;
    sal_uInt8 nLevel;
    OUString sTitle;
    OUString sNumber;
    OUString sPre;
    OUString sPost;

    virtual OUString Expand() const;
    virtual SwField* Copy() const;

public:
    SwChapterField(SwChapterFieldType*, sal_uInt32 nFmt = 0);

    // #i53420#
    void ChangeExpansion( const SwFrm*,
                          const SwCntntNode*,
        sal_Bool bSrchNum = sal_False);
    void ChangeExpansion(const SwTxtNode &rNd, sal_Bool bSrchNum);

    inline sal_uInt8 GetLevel() const;
    inline void SetLevel(sal_uInt8);

    inline OUString      GetNumber() const;
    inline OUString      GetTitle() const;
    virtual bool         QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool         PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline sal_uInt8 SwChapterField::GetLevel() const   { return nLevel; }
inline void SwChapterField::SetLevel(sal_uInt8 nLev) { nLevel = nLev; }
inline OUString SwChapterField::GetNumber() const { return sNumber; }
inline OUString SwChapterField::GetTitle() const { return sTitle; }

#endif // SW_CHPFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
