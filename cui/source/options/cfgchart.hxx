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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_CFGCHART_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_CFGCHART_HXX

#include <tools/rtti.hxx>
#include <unotools/configitem.hxx>
#include <svl/poolitem.hxx>
#include <svx/xtable.hxx>

#include <vector>

class SvxChartColorTable
{
private:
    ::std::vector< XColorEntry >     m_aColorEntries;
    int                              nNextElementNumber;
    OUString                         sDefaultNamePrefix;
    OUString                         sDefaultNamePostfix;

public:
    SvxChartColorTable();
    explicit SvxChartColorTable( const SvxChartColorTable & _rSource );

    // accessors
    size_t size() const;
    const XColorEntry & operator[]( size_t _nIndex ) const;
    ColorData getColorData( size_t _nIndex ) const;

    // mutators
    void clear();
    void append( const XColorEntry & _rEntry );
    void remove( size_t _nIndex );
    void replace( size_t _nIndex, const XColorEntry & _rEntry );
    void useDefault();
    OUString getDefaultName(size_t _nIndex);

    // comparison
    bool operator==( const SvxChartColorTable & _rOther ) const;
};


// all options

class SvxChartOptions : public ::utl::ConfigItem
{
private:
    SvxChartColorTable          maDefColors;
    bool                        mbIsInitialized;

    css::uno::Sequence< OUString >
                                maPropertyNames;

    inline css::uno::Sequence< OUString > GetPropertyNames() const
        { return maPropertyNames; }
    bool RetrieveOptions();

    virtual void                ImplCommit() override;

public:
    SvxChartOptions();
    virtual ~SvxChartOptions();

    const SvxChartColorTable&   GetDefaultColors();
    void                        SetDefaultColors( const SvxChartColorTable& aCol );

    virtual void Notify( const css::uno::Sequence< OUString >& _rPropertyNames) override;
};


// items

class SvxChartColorTableItem : public SfxPoolItem
{
public:
    TYPEINFO_OVERRIDE();
    SvxChartColorTableItem( sal_uInt16 nWhich, const SvxChartColorTable& );
    SvxChartColorTableItem( const SvxChartColorTableItem& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    void                    SetOptions( SvxChartOptions* pOpts ) const;

    const SvxChartColorTable & GetColorList() const  { return m_aColorTable;}
    SvxChartColorTable &       GetColorList() { return m_aColorTable;}
    void                    ReplaceColorByIndex( size_t _nIndex, const XColorEntry & _rEntry );

private:
    SvxChartColorTable      m_aColorTable;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_CFGCHART_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
