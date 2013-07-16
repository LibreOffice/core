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
#ifndef SW_TXMSRT_HXX
#define SW_TXMSRT_HXX

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
    xub_StrLen nPos;
    sal_Bool bMainEntry;

    SwTOXSource( const SwCntntNode* pNode, xub_StrLen n, sal_Bool bMain )
        : pNd(pNode), nPos(n), bMainEntry(bMain)
    {
    }
};

typedef std::vector<SwTOXSource> SwTOXSources;

class SwTOXInternational
{
    IndexEntrySupplierWrapper* pIndexWrapper;
    CharClass* pCharClass;
    LanguageType eLang;
    String sSortAlgorithm;
    sal_uInt16 nOptions;

    void Init();

public:
    SwTOXInternational( LanguageType nLang, sal_uInt16 nOptions,
                        const String& rSortAlgorithm );
    SwTOXInternational( const SwTOXInternational& );
    ~SwTOXInternational();

    sal_Int32 Compare( const String& rTxt1, const String& rTxtReading1,
                       const ::com::sun::star::lang::Locale& rLocale1,
                       const String& rTxt2, const String& rTxtReading2,
                       const ::com::sun::star::lang::Locale& rLocale2 ) const;

    inline bool IsEqual( const String& rTxt1, const String& rTxtReading1,
                         const ::com::sun::star::lang::Locale& rLocale1,
                         const String& rTxt2, const String& rTxtReading2,
                         const ::com::sun::star::lang::Locale& rLocale2 ) const
    {
        return 0 == Compare( rTxt1, rTxtReading1, rLocale1,
                             rTxt2, rTxtReading2, rLocale2 );
    }

    inline bool IsLess( const String& rTxt1, const String& rTxtReading1,
                        const ::com::sun::star::lang::Locale& rLocale1,
                        const String& rTxt2, const String& rTxtReading2,
                        const ::com::sun::star::lang::Locale& rLocale2 ) const
    {
        return -1 == Compare( rTxt1, rTxtReading1, rLocale1,
                              rTxt2, rTxtReading2, rLocale2 );
    }

    String GetIndexKey( const String& rTxt, const String& rTxtReading,
                        const ::com::sun::star::lang::Locale& rLcl ) const;

    String GetFollowingText( sal_Bool bMorePages ) const;

    String ToUpper( const String& rStr, xub_StrLen nPos ) const;
    inline sal_Bool IsNumeric( const String& rStr ) const;
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
    xub_StrLen nCntPos;
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

    virtual String  GetURL() const;

    inline void GetTxt( String&, String& ) const;
    inline const ::com::sun::star::lang::Locale& GetLocale() const;

private:
    sal_Bool bValidTxt;
    String sSortTxt;
    String sSortTxtReading;

    virtual void GetText_Impl( String&, String& ) const = 0;
};

inline void SwTOXSortTabBase::GetTxt( String& rSortTxt,
                                      String& rSortTxtReading ) const
{
    if( !bValidTxt )
    {
        SwTOXSortTabBase* pThis = (SwTOXSortTabBase*)this;
        pThis->GetText_Impl( pThis->sSortTxt, pThis->sSortTxtReading );
        pThis->bValidTxt = sal_True;
    }

    rSortTxt = sSortTxt;
    rSortTxtReading = sSortTxtReading;
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
    virtual void GetText_Impl( String&, String& ) const;

    sal_uInt8   nKeyLevel;
};

struct SwTOXCustom : public SwTOXSortTabBase
{
    SwTOXCustom( const String& rKey, const String& rReading, sal_uInt16 nLevel,
                 const SwTOXInternational& rIntl,
                 const ::com::sun::star::lang::Locale& rLocale );
    virtual ~SwTOXCustom() {}

    virtual sal_uInt16 GetLevel() const;
    virtual bool   operator==( const SwTOXSortTabBase& );
    virtual bool   operator<( const SwTOXSortTabBase& );

private:
    virtual void GetText_Impl( String&, String& ) const;

    String  aKey;
    String  sReading;
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
    virtual void GetText_Impl( String&, String& ) const;

};

struct SwTOXPara : public SwTOXSortTabBase
{
    SwTOXPara( const SwCntntNode&, SwTOXElement, sal_uInt16 nLevel = FORM_ALPHA_DELIMITTER, OUString sSeqName = OUString() );
    virtual ~SwTOXPara() {}

    void    SetStartIndex( xub_StrLen nSet)     { nStartIndex = nSet;}
    void    SetEndIndex( xub_StrLen nSet )      { nEndIndex = nSet;}

    virtual void    FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 nAuthField = 0 ) const;
    virtual sal_uInt16  GetLevel() const;

    virtual String  GetURL() const;
private:
    virtual void GetText_Impl( String&, String& ) const;

    SwTOXElement eType;
    sal_uInt16 m_nLevel;
    xub_StrLen nStartIndex;
    xub_StrLen nEndIndex;
    OUString m_sSequenceName;
};

struct SwTOXTable : public SwTOXSortTabBase
{
    SwTOXTable( const SwCntntNode& rNd );
    virtual ~SwTOXTable() {}

    void    SetLevel(sal_uInt16 nSet){nLevel = nSet;}

    virtual sal_uInt16  GetLevel() const;

    virtual String  GetURL() const;
private:
    virtual void GetText_Impl( String&, String& ) const;

    sal_uInt16 nLevel;
};

struct SwTOXAuthority : public SwTOXSortTabBase
{
private:
    SwFmtFld& m_rField;
    virtual void    FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 nAuthField = 0 ) const;
    virtual void GetText_Impl( String&, String& ) const;

public:
    SwTOXAuthority( const SwCntntNode& rNd, SwFmtFld& rField, const SwTOXInternational& rIntl );
    virtual ~SwTOXAuthority() {}

    SwFmtFld& GetFldFmt() {return m_rField;}

    virtual bool    operator==( const SwTOXSortTabBase& );
    virtual bool    operator<( const SwTOXSortTabBase& );
    virtual sal_uInt16  GetLevel() const;
};


#endif // SW_TXMSRT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
