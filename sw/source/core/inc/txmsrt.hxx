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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TXMSRT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TXMSRT_HXX

#include <i18nlangtag/lang.h>
#include <tox.hxx>

#include <com/sun/star/lang/Locale.hpp>

class CharClass;
class SwContentNode;
class SwTextNode;
class SwTextTOXMark;
class SwIndex;
class SwFormatField;
class SwRootFrame;
class IndexEntrySupplierWrapper;

enum TOXSortType
{
    TOX_SORT_INDEX,
    TOX_SORT_CUSTOM,
    TOX_SORT_CONTENT,
    TOX_SORT_PARA,
    TOX_SORT_TABLE,
    TOX_SORT_AUTHORITY
};

struct SwTOXSource
{
    const SwContentNode* pNd;
    sal_Int32 nPos;
    bool bMainEntry;

    SwTOXSource( const SwContentNode* pNode, sal_Int32 n, bool bMain )
        : pNd(pNode), nPos(n), bMainEntry(bMain)
    {
    }
};

struct TextAndReading
{
    OUString sText;
    OUString sReading;

    TextAndReading() {}

    TextAndReading(const OUString& rText, const OUString& rReading)
    : sText(rText)
    , sReading(rReading)
    {}
};

class SwTOXInternational
{
    std::unique_ptr<IndexEntrySupplierWrapper> m_pIndexWrapper;
    std::unique_ptr<CharClass>                 m_pCharClass;
    LanguageType               m_eLang;
    OUString                   m_sSortAlgorithm;
    SwTOIOptions               m_nOptions;

    void Init();

public:
    SwTOXInternational( LanguageType nLang, SwTOIOptions nOptions,
                        const OUString& rSortAlgorithm );
    SwTOXInternational( const SwTOXInternational& );
    ~SwTOXInternational();

    sal_Int32 Compare( const TextAndReading& rTaR1,
                       const css::lang::Locale& rLocale1,
                       const TextAndReading& rTaR2,
                       const css::lang::Locale& rLocale2 ) const;

    bool IsEqual( const TextAndReading& rTaR1,
                         const css::lang::Locale& rLocale1,
                         const TextAndReading& rTaR2,
                         const css::lang::Locale& rLocale2 ) const
    {
        return 0 == Compare( rTaR1, rLocale1, rTaR2, rLocale2 );
    }

    bool IsLess( const TextAndReading& rTaR1,
                        const css::lang::Locale& rLocale1,
                        const TextAndReading& rTaR2,
                        const css::lang::Locale& rLocale2 ) const
    {
        return -1 == Compare( rTaR1, rLocale1, rTaR2, rLocale2 );
    }

    OUString GetIndexKey( const TextAndReading& rTaR,
                        const css::lang::Locale& rLcl ) const;

    OUString GetFollowingText( bool bMorePages ) const;

    OUString ToUpper( const OUString& rStr, sal_Int32 nPos ) const;
    inline bool IsNumeric( const OUString& rStr ) const;
};

/**
 * Class for sorting directories
 */
struct SwTOXSortTabBase
{
    bool operator==(const SwTOXSortTabBase&) const = delete;
    bool operator<(const SwTOXSortTabBase&) const = delete;

    std::vector<SwTOXSource>       aTOXSources;
    css::lang::Locale aLocale;
    const SwTextNode* pTOXNd;
    const SwTextTOXMark* pTextMark;
    const SwTOXInternational* pTOXIntl;
    sal_uLong nPos;
    sal_Int32 nCntPos;
    sal_uInt16 nType;
    static SwTOIOptions nOpt;

    SwTOXSortTabBase( TOXSortType nType,
                      const SwContentNode* pTOXSrc,
                      const SwTextTOXMark* pTextMark,
                      const SwTOXInternational* pIntl,
                      const css::lang::Locale* pLocale = nullptr );
    virtual ~SwTOXSortTabBase() {}

    sal_uInt16  GetType() const         { return nType; }
    static SwTOIOptions  GetOptions()   { return nOpt; }

    virtual void    FillText(SwTextNode& rNd, const SwIndex& rInsPos,
            sal_uInt16 nAuthField, SwRootFrame const* pLayout) const;
    virtual sal_uInt16  GetLevel()  const = 0;
    virtual bool    equivalent( const SwTOXSortTabBase& );
    virtual bool    sort_lt( const SwTOXSortTabBase& );

    virtual OUString  GetURL() const;

    virtual bool IsFullPara() const;

    // must be called
    inline void InitText(SwRootFrame const*const pLayout);
    inline TextAndReading const & GetText() const;
    inline const css::lang::Locale& GetLocale() const;

private:
    bool m_bValidText;
    TextAndReading m_aSort;

    virtual TextAndReading GetText_Impl(SwRootFrame const* pLayout) const = 0;
};

inline void SwTOXSortTabBase::InitText(SwRootFrame const*const pLayout)
{
        // 'this' is 'SwTOXSortTabBase const*', so the virtual
        // mechanism will call the derived class' GetText_Impl
    assert(!m_bValidText);
    m_aSort = GetText_Impl(pLayout);
    m_bValidText = true;
}

inline TextAndReading const & SwTOXSortTabBase::GetText() const
{
    assert(m_bValidText);
    return m_aSort;
}

inline const css::lang::Locale& SwTOXSortTabBase::GetLocale() const
{
    return aLocale;
}

/**
 * For sorting by text
 */
struct SwTOXIndex final : public SwTOXSortTabBase
{
    SwTOXIndex( const SwTextNode&, const SwTextTOXMark*, SwTOIOptions nOptions, sal_uInt8 nKeyLevel,
                const SwTOXInternational& rIntl,
                const css::lang::Locale& rLocale );

    virtual void    FillText(SwTextNode& rNd, const SwIndex& rInsPos,
            sal_uInt16 nAuthField, SwRootFrame const* pLayout) const override;
    virtual sal_uInt16  GetLevel() const override;
    virtual bool    equivalent( const SwTOXSortTabBase& ) override;
    virtual bool    sort_lt( const SwTOXSortTabBase& ) override;

private:
    virtual TextAndReading GetText_Impl(SwRootFrame const* pLayout) const override;

    sal_uInt8   nKeyLevel;
};

struct SwTOXCustom final : public SwTOXSortTabBase
{
    SwTOXCustom( const TextAndReading& rKey, sal_uInt16 nLevel,
                 const SwTOXInternational& rIntl,
                 const css::lang::Locale& rLocale );

    virtual sal_uInt16 GetLevel() const override;
    virtual bool   equivalent( const SwTOXSortTabBase& ) override;
    virtual bool   sort_lt( const SwTOXSortTabBase& ) override;

private:
    virtual TextAndReading GetText_Impl(SwRootFrame const* pLayout) const override;

    TextAndReading m_aKey;
    sal_uInt16  nLev;
};

/**
 * For sorting by position
 */
struct SwTOXContent final : public SwTOXSortTabBase
{
    SwTOXContent( const SwTextNode&, const SwTextTOXMark*,
                const SwTOXInternational& rIntl );

    virtual void    FillText(SwTextNode& rNd, const SwIndex& rInsPos,
            sal_uInt16 nAuthField, SwRootFrame const* pLayout) const override;
    virtual sal_uInt16  GetLevel() const override;
private:
    virtual TextAndReading GetText_Impl(SwRootFrame const* pLayout) const override;

};

struct SwTOXPara final : public SwTOXSortTabBase
{
    SwTOXPara(SwContentNode&, SwTOXElement,
              sal_uInt16 nLevel = FORM_ALPHA_DELIMITER,
              const OUString& sSeqName = OUString());

    void    SetStartIndex(sal_Int32 nSet)    { nStartIndex = nSet; }
    void    SetEndIndex(sal_Int32 nSet)      { nEndIndex = nSet; }

    virtual void    FillText(SwTextNode& rNd, const SwIndex& rInsPos,
            sal_uInt16 nAuthField, SwRootFrame const* pLayout) const override;
    virtual sal_uInt16  GetLevel() const override;

    virtual OUString  GetURL() const override;
    virtual bool IsFullPara() const override;
private:
    virtual TextAndReading GetText_Impl(SwRootFrame const* pLayout) const override;

    SwTOXElement eType;
    sal_uInt16 m_nLevel;
    sal_Int32 nStartIndex;
    sal_Int32 nEndIndex;
    OUString m_sSequenceName;
};

struct SwTOXTable final : public SwTOXSortTabBase
{
    SwTOXTable( const SwContentNode& rNd );

    void    SetLevel(sal_uInt16 nSet){nLevel = nSet;}

    virtual sal_uInt16  GetLevel() const override;

    virtual OUString  GetURL() const override;
private:
    virtual TextAndReading GetText_Impl(SwRootFrame const* pLayout) const override;

    sal_uInt16 nLevel;
};

struct SwTOXAuthority final : public SwTOXSortTabBase
{
private:
    SwFormatField& m_rField;
    virtual void    FillText(SwTextNode& rNd, const SwIndex& rInsPos,
            sal_uInt16 nAuthField, SwRootFrame const* pLayout) const override;
    virtual TextAndReading GetText_Impl(SwRootFrame const* pLayout) const override;

public:
    SwTOXAuthority( const SwContentNode& rNd, SwFormatField& rField, const SwTOXInternational& rIntl );

    SwFormatField& GetFieldFormat() {return m_rField;}

    virtual bool    equivalent( const SwTOXSortTabBase& ) override;
    virtual bool    sort_lt( const SwTOXSortTabBase& ) override;
    virtual sal_uInt16  GetLevel() const override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_TXMSRT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
