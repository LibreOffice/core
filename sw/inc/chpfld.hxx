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
    String sTitle, sNumber, sPre, sPost;

    virtual String   Expand() const;
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

    inline const String& GetNumber() const;
    inline const String& GetTitle() const;
    virtual bool         QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool         PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline sal_uInt8 SwChapterField::GetLevel() const   { return nLevel; }
inline void SwChapterField::SetLevel(sal_uInt8 nLev) { nLevel = nLev; }
inline const String& SwChapterField::GetNumber() const { return sNumber; }
inline const String& SwChapterField::GetTitle() const { return sTitle; }

#endif // SW_CHPFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
