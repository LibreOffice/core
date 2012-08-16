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
#ifndef _SVX_NUMFMTSH_HXX
#define _SVX_NUMFMTSH_HXX

#include <tools/string.hxx>
#include <i18npool/lang.h>

#include "svx/svxdllapi.h"

#include <svl/zforlist.hxx>

#include <vector>
#include <map>

class Color;

enum SvxNumberValueType
{
    SVX_VALUE_TYPE_UNDEFINED = 0,
    SVX_VALUE_TYPE_NUMBER,
    SVX_VALUE_TYPE_STRING
};

// sort order of the category ListBox entries in the TabPage
#define CAT_ALL             0
#define CAT_USERDEFINED     1
#define CAT_NUMBER          2
#define CAT_PERCENT         3
#define CAT_CURRENCY        4
#define CAT_DATE            5
#define CAT_TIME            6
#define CAT_SCIENTIFIC      7
#define CAT_FRACTION        8
#define CAT_BOOLEAN         9
#define CAT_TEXT            10

#define SELPOS_NONE         -1

#define NUMBERFORMAT_ENTRY_NEW_CURRENCY     NUMBERFORMAT_ENTRY_NOT_FOUND-1


class SVX_DLLPUBLIC SvxNumberFormatShell
{
public:
                        SvxNumberFormatShell( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32         nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              const String&      rNumStr );

                        SvxNumberFormatShell( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32         nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              double             nNumVal,
                                              const String*      pNumStr = NULL );

                        ~SvxNumberFormatShell();


    static SvxNumberFormatShell*    Create( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32         nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              const String&      rNumStr );

    static SvxNumberFormatShell*    Create( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32         nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              double             nNumVal,
                                              const String*      pNumStr = NULL );


    void                GetInitSettings( sal_uInt16&           nCatLbPos,
                                         LanguageType&         rLangType,
                                         sal_uInt16&           nFmtLbSelPos,
                                         std::vector<String*>& rFmtEntries,
                                         String&               rPrevString,
                                         Color*&               rpPrevColor );

    void                CategoryChanged( sal_uInt16            nCatLbPos,
                                         short&                rFmtSelPos,
                                         std::vector<String*>& rFmtEntries );

    void                LanguageChanged( LanguageType          eLangType,
                                         short&                rFmtSelPos,
                                         std::vector<String*>& rFmtEntries );

    void                FormatChanged( sal_uInt16  nFmtLbPos,
                                       String& rPreviewStr,
                                       Color*& rpFontColor );

    bool                AddFormat( String&               rFormat,
                                   xub_StrLen&           rErrPos,
                                   sal_uInt16&           rCatLbSelPos,
                                   short&                rFmtSelPos,
                                   std::vector<String*>& rFmtEntries );

    bool                RemoveFormat( const String&         rFormat,
                                      sal_uInt16&           rCatLbSelPos,
                                      short&                rFmtSelPos,
                                      std::vector<String*>& rFmtEntries );

    void                MakeFormat( String& rFormat,
                                    bool        bThousand,
                                    bool        bNegRed,
                                    sal_uInt16  nPrecision,
                                    sal_uInt16  nLeadingZeroes,
                                    sal_uInt16  nCurrencyEntryPos);

    void                GetOptions( const String& rFormat,
                                    bool&         rThousand,
                                    bool&         rNegRed,
                                    sal_uInt16&   rPrecision,
                                    sal_uInt16&   rLeadingZeroes,
                                    sal_uInt16&   rCatLbPos );

    void                MakePreviewString( const String& rFormatStr,
                                           String&       rPreviewStr,
                                           Color*&       rpFontColor );

    void                MakePrevStringFromVal( const String& rFormatStr,
                                                String& rPreviewStr,
                                                Color*& rpFontColor,
                                                double  nValue);

    bool                IsUserDefined( const String& rFmtString );
    bool                IsTmpCurrencyFormat( const String& rFmtString );
    bool                FindEntry( const String& rFmtString, sal_uInt32* pAt = NULL );

    void                ValidateNewEntries( bool bValidate = true ) { bUndoAddList = !bValidate; }
    size_t              GetUpdateDataCount() const;
    void                GetUpdateData( sal_uInt32* pDelArray, const sal_uInt32 nSize );

    void                SetCurNumFmtKey( sal_uInt32 nNew )  { nCurFormatKey = nNew; }
    void                SetCurLanguage( LanguageType eNew ) { eCurLanguage = eNew; }
    sal_uInt32          GetCurNumFmtKey() const             { return nCurFormatKey; }
    LanguageType        GetCurLanguage() const              { return eCurLanguage; }

                        /** Returns the name of Standard, General, ... for the
                            current language. */
    String              GetStandardName() const;

    String              GetComment4Entry(short nEntry);
    short               GetCategory4Entry(short nEntry);
    bool                GetUserDefined4Entry(short nEntry);
    String              GetFormat4Entry(short nEntry);
    void                SetComment4Entry(short nEntry,String aCommentString);

    void                SetCurrencySymbol(sal_uInt16 nPos);
    sal_uInt32          GetCurrencySymbol();
    sal_uInt16          FindCurrencyFormat( const String& rFmtString );
    sal_uInt16          FindCurrencyFormat(const NfCurrencyEntry* pTmpCurrencyEntry,bool bTmpBanking);
    void                SetCurCurrencyEntry(NfCurrencyEntry*);
    short               GetListPos4Entry(sal_uInt32 nIdx);
    short               GetListPos4Entry( const String& rFmtString );

    void                GetCurrencySymbols(std::vector<rtl::OUString>& rList, sal_uInt16* pPos );
    void                GetCurrencySymbols(std::vector<rtl::OUString>& rList, bool bFlag );

    sal_uInt16          FindCurrencyTableEntry( const String& rFmtString, bool &bTestBanking );
    bool                IsInTable(sal_uInt16 nPos, bool bTmpBanking,
                            ::rtl::OUString const& rFmtString);

    void                SetUseStarFormat( bool bUse ) { bUseStarFormat = bUse; }
    bool                IsUseStarFormat( void ) { return bUseStarFormat; }
private:
    static const double     DEFAULT_NUMVALUE;

    SvNumberFormatter*      pFormatter;
    SvNumberFormatTable*    pCurFmtTable;
    SvxNumberValueType      eValType;
    String                  aValStr;
    double                  nValNum;
    bool                    bUndoAddList;
    std::vector<sal_uInt32> aAddList;
    std::vector<sal_uInt32> aDelList;
    std::vector<sal_uInt32> aCurEntryList;
    sal_uInt32              nCurFormatKey;
    short                   nCurCategory;
    LanguageType            eCurLanguage;
    std::vector<sal_uInt16> aCurCurrencyList;
    NfCurrencyEntry*        pCurCurrencyEntry;
    bool                    bBankingSymbol;
    sal_uInt16              nCurCurrencyEntryPos;
    std::vector<String*>    aCurrencyFormatList;
    bool                    bUseStarFormat;
#ifdef _SVX_NUMFMTSH_CXX
    SVX_DLLPRIVATE short FillEntryList_Impl( std::vector<String*>& rList );
    SVX_DLLPRIVATE void  FillEListWithStd_Impl( std::vector<String*>& rList,sal_uInt16 aPrivCat, short &Pos);
    SVX_DLLPRIVATE short FillEListWithFormats_Impl( std::vector<String*>& rList,short nSelPos,
                                                       NfIndexTableOffset eOffsetStart,
                                                       NfIndexTableOffset eOffsetEnd);
    SVX_DLLPRIVATE short FillEListWithDateTime_Impl( std::vector<String*>& rList,short nSelPos);
    SVX_DLLPRIVATE short FillEListWithCurrency_Impl( std::vector<String*>& rList,short nSelPos);
    SVX_DLLPRIVATE short FillEListWithSysCurrencys( std::vector<String*>& rList,short nSelPos);
    SVX_DLLPRIVATE short FillEListWithUserCurrencys( std::vector<String*>& rList,short nSelPos);
    SVX_DLLPRIVATE short FillEListWithUsD_Impl( std::vector<String*>& rList, sal_uInt16 nPrivCat, short Pos );

    SVX_DLLPRIVATE ::std::vector<sal_uInt32>::iterator GetRemoved_Impl( size_t nKey );
    SVX_DLLPRIVATE bool                                IsRemoved_Impl( size_t nKey );
    SVX_DLLPRIVATE ::std::vector<sal_uInt32>::iterator GetAdded_Impl( size_t nKey );
    SVX_DLLPRIVATE bool                                IsAdded_Impl( size_t nKey );

    SVX_DLLPRIVATE void  GetPreviewString_Impl( String& rString,
                                                Color*& rpColor );
    SVX_DLLPRIVATE void  PosToCategory_Impl( sal_uInt16 nPos, short& rCategory );
    SVX_DLLPRIVATE void  CategoryToPos_Impl( short nCategory, sal_uInt16& rPos );
#endif
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
