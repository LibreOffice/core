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

/*************************************************************************
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    The structure of auto formatting should not be changed. It is used
    by various code of Writer and Calc. If a change is necessary, the
    source code of both applications must be changed!

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**************************************************************************/

#include <svl/poolitem.hxx>
#include <svx/autoformathelper.hxx>

#include "scdllapi.h"
#include "zforauto.hxx"

#include <array>
#include <memory>
#include <map>
#include <climits>

class ScDocument;

/**
A binary blob of writer-specific data. This data typically consists of types that are
unavailable to Calc (e.g. SwFmtVertOrient), or that Calc doesn't care about.

@remarks Note that in autoformat versions prior to AUTOFORMAT_DATA_ID_31005, Calc
logic handled and stored several writer-specific items (such as ScAutoFormatDataField::aAdjust).
That logic was preserved. From _31005 onward, writer-specific data should be handled by
blobs to avoid needlessly complicating the Calc logic.
*/
struct AutoFormatSwBlob
{
    std::unique_ptr<sal_uInt8[]> pData;
    std::size_t size;

    AutoFormatSwBlob() : size(0)
    {
    }
    AutoFormatSwBlob(const AutoFormatSwBlob&) = delete;
    const AutoFormatSwBlob& operator=(const AutoFormatSwBlob&) = delete;

    void Reset()
    {
        pData.reset();
        size = 0;
    }
};

/// Struct with version numbers of the Items
struct ScAfVersions : public AutoFormatVersions
{
private:
    AutoFormatSwBlob swVersions;

public:
    ScAfVersions();

    void Load( SvStream& rStream, sal_uInt16 nVer );
    void Write(SvStream& rStream, sal_uInt16 fileVersion);
};

/// Contains all items for one cell of a table autoformat.
class ScAutoFormatDataField : public AutoFormatBase
{
private:
    AutoFormatSwBlob                            m_swFields;

    // number format
    ScNumFormatAbbrev                           aNumFormat;

public:
    ScAutoFormatDataField();
    ScAutoFormatDataField( const ScAutoFormatDataField& rCopy );
    ~ScAutoFormatDataField();

    // block assignment operator
    ScAutoFormatDataField& operator=(const ScAutoFormatDataField& rRef) = delete;

    // number format
    const ScNumFormatAbbrev&    GetNumFormat() const    { return aNumFormat; }

    // number format
    void    SetNumFormat( const ScNumFormatAbbrev& rNumFormat )     { aNumFormat = rNumFormat; }

    bool    Load( SvStream& rStream, const ScAfVersions& rVersions, sal_uInt16 nVer );
    bool    Save( SvStream& rStream, sal_uInt16 fileVersion );
};

class SAL_DLLPUBLIC_RTTI ScAutoFormatData
{
private:
    OUString               aName;
    sal_uInt16                  nStrResId;
    // common flags of Calc and Writer
    bool                        bIncludeFont : 1;
    bool                        bIncludeJustify : 1;
    bool                        bIncludeFrame : 1;
    bool                        bIncludeBackground : 1;

    // Calc specific flags
    bool                        bIncludeValueFormat : 1;
    bool                        bIncludeWidthHeight : 1;

    // Writer-specific data
    AutoFormatSwBlob m_swFields;

    std::array<std::unique_ptr<ScAutoFormatDataField>,16> ppDataField;

    SAL_DLLPRIVATE ScAutoFormatDataField&       GetField( sal_uInt16 nIndex );
    SAL_DLLPRIVATE const ScAutoFormatDataField& GetField( sal_uInt16 nIndex ) const;

public:
    ScAutoFormatData();
    SC_DLLPUBLIC ScAutoFormatData( const ScAutoFormatData& rData );
    SC_DLLPUBLIC ~ScAutoFormatData();

    void            SetName( const OUString& rName )              { aName = rName; nStrResId = USHRT_MAX; }
    const OUString& GetName() const { return aName; }

    bool            GetIncludeValueFormat() const               { return bIncludeValueFormat; }
    bool            GetIncludeFont() const                      { return bIncludeFont; }
    bool            GetIncludeJustify() const                   { return bIncludeJustify; }
    bool            GetIncludeFrame() const                     { return bIncludeFrame; }
    bool            GetIncludeBackground() const                { return bIncludeBackground; }
    bool            GetIncludeWidthHeight() const               { return bIncludeWidthHeight; }

    void            SetIncludeValueFormat( bool bValueFormat )  { bIncludeValueFormat = bValueFormat; }
    void            SetIncludeFont( bool bFont )                { bIncludeFont = bFont; }
    void            SetIncludeJustify( bool bJustify )          { bIncludeJustify = bJustify; }
    void            SetIncludeFrame( bool bFrame )              { bIncludeFrame = bFrame; }
    void            SetIncludeBackground( bool bBackground )    { bIncludeBackground = bBackground; }
    void            SetIncludeWidthHeight( bool bWidthHeight )  { bIncludeWidthHeight = bWidthHeight; }

    const SfxPoolItem*          GetItem( sal_uInt16 nIndex, sal_uInt16 nWhich ) const;
    template<class T> const T*  GetItem( sal_uInt16 nIndex, TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T*>(GetItem(nIndex, sal_uInt16(nWhich)));
    }
    void                        PutItem( sal_uInt16 nIndex, const SfxPoolItem& rItem );
    void                        CopyItem( sal_uInt16 nToIndex, sal_uInt16 nFromIndex, sal_uInt16 nWhich );

    const ScNumFormatAbbrev&    GetNumFormat( sal_uInt16 nIndex ) const;

    bool                        HasSameData( sal_uInt16 nIndex1, sal_uInt16 nIndex2 ) const;

    void                        FillToItemSet( sal_uInt16 nIndex, SfxItemSet& rItemSet, const ScDocument& rDoc ) const;
    void                        GetFromItemSet( sal_uInt16 nIndex, const SfxItemSet& rItemSet, const ScNumFormatAbbrev& rNumFormat );

    bool                        Load( SvStream& rStream, const ScAfVersions& rVersions );
    bool                        Save( SvStream& rStream, sal_uInt16 fileVersion );
};

struct DefaultFirstEntry {
    bool operator() (const OUString& left, const OUString& right) const;
};

class SAL_DLLPUBLIC_RTTI ScAutoFormat
{
    typedef std::map<OUString, std::unique_ptr<ScAutoFormatData>, DefaultFirstEntry> MapType;
    MapType m_Data;
    bool mbSaveLater;
    ScAfVersions m_aVersions;

    ScAutoFormat(const ScAutoFormat&) = delete;
    const ScAutoFormat operator=(const ScAutoFormat&) = delete;

public:
    typedef MapType::const_iterator const_iterator;
    typedef MapType::iterator iterator;

    ScAutoFormat();
    void Load();
    SC_DLLPUBLIC bool Save();

    void SetSaveLater( bool bSet );
    bool IsSaveLater() const { return mbSaveLater; }

    const ScAutoFormatData* findByIndex(size_t nIndex) const;
    SC_DLLPUBLIC ScAutoFormatData* findByIndex(size_t nIndex);
    SC_DLLPUBLIC iterator find(const OUString& rName);

    SC_DLLPUBLIC iterator insert(std::unique_ptr<ScAutoFormatData> pNew);
    SC_DLLPUBLIC void erase(const iterator& it);

    SC_DLLPUBLIC size_t size() const;
    const_iterator begin() const;
    const_iterator end() const;
    SC_DLLPUBLIC iterator begin();
    SC_DLLPUBLIC iterator end();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
