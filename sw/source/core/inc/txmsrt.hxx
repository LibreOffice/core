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
class SwCntntNode;
class SwTxtNode;
class SwTxtTOXMark;
class SwIndex;
class SwFmtFld;
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
    const SwCntntNode* pNd;
    sal_Int32 nPos;
    bool bMainEntry;

    SwTOXSource( const SwCntntNode* pNode, sal_Int32 n, bool bMain )
        : pNd(pNode), nPos(n), bMainEntry(bMain)
    {
    }
};

typedef std::vector<SwTOXSource> SwTOXSources;

struct TextAndReading
{
    OUString sText;
    OUString sReading;

    TextAndReading() {}

    TextAndReading(OUString sTxt, OUString sRdng)
    : sText(sTxt)
    , sReading(sRdng)
    {}
};

class SwTOXInternational
{
    IndexEntrySupplierWrapper* pIndexWrapper;
    CharClass* pCharClass;
    LanguageType eLang;
    OUString sSortAlgorithm;
    sal_uInt16 nOptions;

    void Init();

public:
    SwTOXInternational( LanguageType nLang, sal_uInt16 nOptions,
                        const OUString& rSortAlgorithm );
    SwTOXInternational( const SwTOXInternational& );
    ~SwTOXInternational();

    sal_Int32 Compare( const TextAndReading& rTaR1,
                       const ::com::sun::star::lang::Locale& rLocale1,
                       const TextAndReading& rTaR2,
                       const ::com::sun::star::lang::Locale& rLocale2 ) const;

    inline bool IsEqual( const TextAndReading& rTaR1,
                         const ::com::sun::star::lang::Locale& rLocale1,
                         const TextAndReading& rTaR2,
                         const ::com::sun::star::lang::Locale& rLocale2 ) const
    {
        return 0 == Compare( rTaR1, rLocale1, rTaR2, rLocale2 );
    }

    inline bool IsLess( const TextAndReading& rTaR1,
                        const ::com::sun::star::lang::Locale& rLocale1,
                        const TextAndReading& rTaR2,
                        const ::com::sun::star::lang::Locale& rLocale2 ) const
    {
        return -1 == Compare( rTaR1, rLocale1, rTaR2, rLocale2 );
    }

    OUString GetIndexKey( const TextAndReading& rTaR,
                        const ::com::sun::star::lang::Locale& rLcl ) const;

    OUString GetFollowingText( bool bMorePages ) const;

    OUString ToUpper( const OUString& rStr, sal_Int32 nPos ) const;
    inline bool IsNumeric( const OUString& rStr ) const;
};

/*--------------------------------------------------------------------
     Beschreibung: Klassen fuer die Sortierung der Verzeichnisse
 --------------------------------------------------------------------*/

struct SwTOXSortTabBase
{
    SwTOXSources aTOXSources;
    ::com::sun::star::lang::Locale aLocale;
    const SwTxtNode* pTOXNd;
    const SwTxtTOXMark* pTxtMark;
    const SwTOXInternational* pTOXIntl;
    sal_uLong nPos;
    sal_Int32 nCntPos;
    sal_uInt16 nType;
    static sal_uInt16 nOpt;

    SwTOXSortTabBase( TOXSortType nType,
                      const SwCntntNode* pTOXSrc,
                      const SwTxtTOXMark* pTxtMark,
                      const SwTOXInternational* pIntl,
                      const ::com::sun::star::lang::Locale* pLocale = NULL );
    virtual ~SwTOXSortTabBase() {}

    sal_uInt16  GetType() const         { return nType; }
    sal_uInt16  GetOptions() const      { return nOpt; }

    virtual void    FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 nAuthField = 0) const;
    virtual sal_uInt16  GetLevel()  const = 0;
    virtual bool    operator==( const SwTOXSortTabBase& );
    virtual bool    operator<( const SwTOXSortTabBase& );

    virtual OUString  GetURL() const;

    inline TextAndReading GetTxt() const;
    inline const ::com::sun::star::lang::Locale& GetLocale() const;

private:
    mutable bool bValidTxt;
    TextAndReading m_aSort;

    virtual TextAndReading GetText_Impl() const = 0;
};

inline TextAndReading SwTOXSortTabBase::GetTxt() const
{
    if( !bValidTxt )
    {
        // 'this' is 'SwTOXSortTabBase const*', so the virtual
        // mechanism will call the derived class' GetText_Impl
        GetText_Impl();
        bValidTxt = true;
    }
    return m_aSort;
}

inline const ::com::sun::star::lang::Locale& SwTOXSortTabBase::GetLocale() const
{
    return aLocale;
}

/*--------------------------------------------------------------------
     Beschreibung: fuer Sortierung nach Text
 --------------------------------------------------------------------*/

struct SwTOXIndex : public SwTOXSortTabBase
{
    SwTOXIndex( const SwTxtNode&, const SwTxtTOXMark*, sal_uInt16 nOptions, sal_uInt8 nKeyLevel,
                const SwTOXInternational& rIntl,
                const ::com::sun::star::lang::Locale& rLocale );
    virtual ~SwTOXIndex() {}

    virtual void    FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 nAuthField = 0 ) const;
    virtual sal_uInt16  GetLevel() const;
    virtual bool    operator==( const SwTOXSortTabBase& );
    virtual bool    operator<( const SwTOXSortTabBase& );

private:
    virtual TextAndReading GetText_Impl() const;

    sal_uInt8   nKeyLevel;
};

struct SwTOXCustom : public SwTOXSortTabBase
{
    SwTOXCustom( const TextAndReading& rKey, sal_uInt16 nLevel,
                 const SwTOXInternational& rIntl,
                 const ::com::sun::star::lang::Locale& rLocale );
    virtual ~SwTOXCustom() {}

    virtual sal_uInt16 GetLevel() const;
    virtual bool   operator==( const SwTOXSortTabBase& );
    virtual bool   operator<( const SwTOXSortTabBase& );

private:
    virtual TextAndReading GetText_Impl() const;

    TextAndReading m_aKey;
    sal_uInt16  nLev;
};

/*--------------------------------------------------------------------
     Beschreibung: fuer Sortierung nach Position
 --------------------------------------------------------------------*/

struct SwTOXContent : public SwTOXSortTabBase
{
    SwTOXContent( const SwTxtNode&, const SwTxtTOXMark*,
                const SwTOXInternational& rIntl );
    virtual ~SwTOXContent() {}

    virtual void    FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 nAuthField = 0 ) const;
    virtual sal_uInt16  GetLevel() const;
private:
    virtual TextAndReading GetText_Impl() const;

};

struct SwTOXPara : public SwTOXSortTabBase
{
    SwTOXPara( const SwCntntNode&, SwTOXElement, sal_uInt16 nLevel = FORM_ALPHA_DELIMITTER, OUString sSeqName = OUString() );
    virtual ~SwTOXPara() {}

    void    SetStartIndex(sal_Int32 nSet)    { nStartIndex = nSet; }
    void    SetEndIndex(sal_Int32 nSet)      { nEndIndex = nSet; }

    virtual void    FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 nAuthField = 0 ) const;
    virtual sal_uInt16  GetLevel() const;

    virtual OUString  GetURL() const;
private:
    virtual TextAndReading GetText_Impl() const;

    SwTOXElement eType;
    sal_uInt16 m_nLevel;
    sal_Int32 nStartIndex;
    sal_Int32 nEndIndex;
    OUString m_sSequenceName;
};

struct SwTOXTable : public SwTOXSortTabBase
{
    SwTOXTable( const SwCntntNode& rNd );
    virtual ~SwTOXTable() {}

    void    SetLevel(sal_uInt16 nSet){nLevel = nSet;}

    virtual sal_uInt16  GetLevel() const;

    virtual OUString  GetURL() const;
private:
    virtual TextAndReading GetText_Impl() const;

    sal_uInt16 nLevel;
};

struct SwTOXAuthority : public SwTOXSortTabBase
{
private:
    SwFmtFld& m_rField;
    virtual void    FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 nAuthField = 0 ) const;
    virtual TextAndReading GetText_Impl() const;

public:
    SwTOXAuthority( const SwCntntNode& rNd, SwFmtFld& rField, const SwTOXInternational& rIntl );
    virtual ~SwTOXAuthority() {}

    SwFmtFld& GetFldFmt() {return m_rField;}

    virtual bool    operator==( const SwTOXSortTabBase& );
    virtual bool    operator<( const SwTOXSortTabBase& );
    virtual sal_uInt16  GetLevel() const;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_TXMSRT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
