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

#ifndef INCLUDED_STARMATH_SOURCE_CFGITEM_HXX
#define INCLUDED_STARMATH_SOURCE_CFGITEM_HXX

#include <deque>
#include <vector>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>

#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <vcl/timer.hxx>

#include <symbol.hxx>
#include <types.hxx>
#include <memory>

class SmSym;
class SmFormat;
namespace vcl { class Font; }
struct SmCfgOther;
class SfxItemSet;

struct SmFontFormat
{
    OUString        aName;
    sal_Int16       nCharSet;
    sal_Int16       nFamily;
    sal_Int16       nPitch;
    sal_Int16       nWeight;
    sal_Int16       nItalic;

    SmFontFormat();
    explicit SmFontFormat( const vcl::Font &rFont );

    const vcl::Font GetFont() const;
    bool            operator == ( const SmFontFormat &rFntFmt ) const;
};

struct SmFntFmtListEntry
{
    OUString        aId;
    SmFontFormat    aFntFmt;

    SmFntFmtListEntry( const OUString &rId, const SmFontFormat &rFntFmt );
};

class SmFontFormatList
{
    std::deque<SmFntFmtListEntry> aEntries;
    bool                    bModified;

    SmFontFormatList(const SmFontFormatList&) = delete;
    SmFontFormatList& operator=(const SmFontFormatList&) = delete;

public:
    SmFontFormatList();

    void    Clear();
    void    AddFontFormat( const OUString &rFntFmtId, const SmFontFormat &rFntFmt );
    void    RemoveFontFormat( const OUString &rFntFmtId );

    const SmFontFormat *    GetFontFormat( const OUString &rFntFmtId ) const;
    const SmFontFormat *    GetFontFormat( size_t nPos ) const;
    const OUString          GetFontFormatId( const SmFontFormat &rFntFmt ) const;
    const OUString          GetFontFormatId( const SmFontFormat &rFntFmt, bool bAdd );
    const OUString          GetFontFormatId( size_t nPos ) const;
    const OUString          GetNewFontFormatId() const;
    size_t                  GetCount() const    { return aEntries.size(); }

    bool    IsModified() const          { return bModified; }
    void    SetModified( bool bVal )    { bModified = bVal; }
};

class SmMathConfig : public utl::ConfigItem, public SfxBroadcaster
{
    std::unique_ptr<SmFormat>         pFormat;
    std::unique_ptr<SmCfgOther>       pOther;
    std::unique_ptr<SmFontFormatList> pFontFormatList;
    std::unique_ptr<SmSymbolManager>  pSymbolMgr;
    bool                              bIsOtherModified;
    bool                              bIsFormatModified;
    SmFontPickList                    vFontPickList[7];

    SmMathConfig(const SmMathConfig&) = delete;
    SmMathConfig& operator=(const SmMathConfig&) = delete;

    void    StripFontFormatList( const std::vector< SmSym > &rSymbols );


    void    Save();

    void    ReadSymbol( SmSym &rSymbol,
                        const OUString &rSymbolName,
                        const OUString &rBaseNode ) const;
    void    ReadFontFormat( SmFontFormat &rFontFormat,
                        const OUString &rSymbolName,
                        const OUString &rBaseNode ) const;

    void            SetOtherIfNotEqual( bool &rbItem, bool bNewVal );

protected:
    void    LoadOther();
    void    SaveOther();
    void    LoadFormat();
    void    SaveFormat();
    void    LoadFontFormatList();
    void    SaveFontFormatList();

    void        SetOtherModified( bool bVal );
    inline bool IsOtherModified() const     { return bIsOtherModified; }
    void        SetFormatModified( bool bVal );
    inline bool IsFormatModified() const    { return bIsFormatModified; }
    void        SetFontFormatListModified( bool bVal );

    SmFontFormatList &          GetFontFormatList();
    const SmFontFormatList &    GetFontFormatList() const
    {
        return const_cast<SmMathConfig*>(this)->GetFontFormatList();
    }

    virtual void    ImplCommit() override;

public:
    SmMathConfig();
    virtual ~SmMathConfig();

    // utl::ConfigItem
    virtual void    Notify( const css::uno::Sequence< OUString > &rPropertyNames ) override;

    SmSymbolManager &   GetSymbolManager();
    void                GetSymbols( std::vector< SmSym > &rSymbols ) const;
    void                SetSymbols( const std::vector< SmSym > &rNewSymbols );

    const SmFormat &    GetStandardFormat() const;
    void                SetStandardFormat( const SmFormat &rFormat, bool bSaveFontFormatList = false );

    bool            IsPrintTitle() const;
    void            SetPrintTitle( bool bVal );
    bool            IsPrintFormulaText() const;
    void            SetPrintFormulaText( bool bVal );
    bool            IsPrintFrame() const;
    void            SetPrintFrame( bool bVal );
    SmPrintSize     GetPrintSize() const;
    void            SetPrintSize( SmPrintSize eSize );
    sal_uInt16      GetPrintZoomFactor() const;
    void            SetPrintZoomFactor( sal_uInt16 nVal );

    bool            IsSaveOnlyUsedSymbols() const;
    void            SetSaveOnlyUsedSymbols( bool bVal );
    bool            IsIgnoreSpacesRight() const;
    void            SetIgnoreSpacesRight( bool bVal );
    bool            IsAutoRedraw() const;
    void            SetAutoRedraw( bool bVal );
    bool            IsShowFormulaCursor() const;
    void            SetShowFormulaCursor( bool bVal );

    SmFontPickList & GetFontPickList(sal_uInt16 nIdent) { return vFontPickList[nIdent]; }

    void ItemSetToConfig(const SfxItemSet &rSet);
    void ConfigToItemSet(SfxItemSet &rSet) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
