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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _MATH_CFGITEM_HXX_
#define _MATH_CFGITEM_HXX_

#include <deque>
#include <vector>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>

#include <tools/solar.h>
#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <vcl/timer.hxx>

#include <symbol.hxx>
#include <types.hxx>

using namespace com::sun::star;

class SmSym;
class SmFormat;
class Font;
struct SmCfgOther;

/////////////////////////////////////////////////////////////////


struct SmFontFormat
{
    String      aName;
    sal_Int16       nCharSet;
    sal_Int16       nFamily;
    sal_Int16       nPitch;
    sal_Int16       nWeight;
    sal_Int16       nItalic;

    SmFontFormat();
    SmFontFormat( const Font &rFont );

    const Font      GetFont() const;
    bool            operator == ( const SmFontFormat &rFntFmt ) const;
};


struct SmFntFmtListEntry
{
    String          aId;
    SmFontFormat    aFntFmt;

    SmFntFmtListEntry( const String &rId, const SmFontFormat &rFntFmt );
};

class SmFontFormatList
{
    std::deque<SmFntFmtListEntry> aEntries;
    bool                    bModified;

    // disallow copy-constructor and assignment-operator for now
    SmFontFormatList( const SmFontFormatList & );
    SmFontFormatList & operator = ( const SmFontFormatList & );

public:
    SmFontFormatList();

    void    Clear();
    void    AddFontFormat( const String &rFntFmtId, const SmFontFormat &rFntFmt );
    void    RemoveFontFormat( const String &rFntFmtId );

    const SmFontFormat *    GetFontFormat( const String &rFntFmtId ) const;
    const SmFontFormat *    GetFontFormat( size_t nPos ) const;
    const String            GetFontFormatId( const SmFontFormat &rFntFmt ) const;
    const String            GetFontFormatId( const SmFontFormat &rFntFmt, bool bAdd );
    const String            GetFontFormatId( size_t nPos ) const;
    const String            GetNewFontFormatId() const;
    size_t                  GetCount() const    { return aEntries.size(); }

    bool    IsModified() const          { return bModified; }
    void    SetModified( bool bVal )    { bModified = bVal; }
};


/////////////////////////////////////////////////////////////////

class SmMathConfig : public utl::ConfigItem
{
    SmFormat *          pFormat;
    SmCfgOther *        pOther;
    SmFontFormatList *  pFontFormatList;
    SmSymbolManager *   pSymbolMgr;
    bool                bIsOtherModified;
    bool                bIsFormatModified;

    // disallow copy-constructor and assignment-operator for now
    SmMathConfig( const SmMathConfig & );
    SmMathConfig & operator = ( const SmMathConfig & );


    void    StripFontFormatList( const std::vector< SmSym > &rSymbols );


    void    Save();

    void    ReadSymbol( SmSym &rSymbol,
                        const rtl::OUString &rSymbolName,
                        const rtl::OUString &rBaseNode ) const;
    void    ReadFontFormat( SmFontFormat &rFontFormat,
                        const rtl::OUString &rSymbolName,
                        const rtl::OUString &rBaseNode ) const;

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
    inline bool IsFontFormatListModified() const    { return pFontFormatList ? pFontFormatList->IsModified(): false; }

    SmFontFormatList &          GetFontFormatList();
    const SmFontFormatList &    GetFontFormatList() const
    {
        return ((SmMathConfig *) this)->GetFontFormatList();
    }

public:
    SmMathConfig();
    virtual ~SmMathConfig();

    // utl::ConfigItem
    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString > &rPropertyNames );
    virtual void    Commit();

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
    sal_uInt16          GetPrintZoomFactor() const;
    void            SetPrintZoomFactor( sal_uInt16 nVal );

    bool            IsSaveOnlyUsedSymbols() const;
    void            SetSaveOnlyUsedSymbols( bool bVal );
    bool            IsIgnoreSpacesRight() const;
    void            SetIgnoreSpacesRight( bool bVal );
    bool            IsAutoRedraw() const;
    void            SetAutoRedraw( bool bVal );
    bool            IsShowFormulaCursor() const;
    void            SetShowFormulaCursor( bool bVal );
    void            SetAutoRedraw( sal_Bool bVal );
};

/////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
