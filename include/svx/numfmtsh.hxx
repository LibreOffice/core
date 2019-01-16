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
#ifndef INCLUDED_SVX_NUMFMTSH_HXX
#define INCLUDED_SVX_NUMFMTSH_HXX

#include <i18nlangtag/lang.h>

#include <svx/svxdllapi.h>

#include <svl/zforlist.hxx>

#include <vector>
#include <map>

class Color;

enum class SvxNumberValueType
{
    Undefined,
    Number,
    String
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
                                              const OUString&    rNumStr );

                        SvxNumberFormatShell( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32         nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              double             nNumVal,
                                              const OUString*    pNumStr );

                        ~SvxNumberFormatShell();


    static SvxNumberFormatShell*    Create( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32         nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              const OUString&    rNumStr );

    static SvxNumberFormatShell*    Create( SvNumberFormatter* pNumFormatter,
                                              sal_uInt32         nFormatKey,
                                              SvxNumberValueType eNumValType,
                                              double             nNumVal,
                                              const OUString*    pNumStr );


    void                GetInitSettings( sal_uInt16&           nCatLbPos,
                                         LanguageType&         rLangType,
                                         sal_uInt16&           nFmtLbSelPos,
                                         std::vector<OUString>& rFmtEntries,
                                         OUString&             rPrevString,
                                         Color*&               rpPrevColor );

    void                CategoryChanged( sal_uInt16            nCatLbPos,
                                         short&                rFmtSelPos,
                                         std::vector<OUString>& rFmtEntries );

    void                LanguageChanged( LanguageType          eLangType,
                                         short&                rFmtSelPos,
                                         std::vector<OUString>& rFmtEntries );

    void                FormatChanged( sal_uInt16  nFmtLbPos,
                                       OUString&   rPreviewStr,
                                       Color*&     rpFontColor );

    bool                AddFormat( OUString&             rFormat,
                                   sal_Int32&            rErrPos,
                                   sal_uInt16&           rCatLbSelPos,
                                   short&                rFmtSelPos,
                                   std::vector<OUString>& rFmtEntries );

    void                RemoveFormat( const OUString&       rFormat,
                                      sal_uInt16&           rCatLbSelPos,
                                      short&                rFmtSelPos,
                                      std::vector<OUString>& rFmtEntries );

    void                MakeFormat( OUString&   rFormat,
                                    bool        bThousand,
                                    bool        bNegRed,
                                    sal_uInt16  nPrecision,
                                    sal_uInt16  nLeadingZeroes,
                                    sal_uInt16  nCurrencyEntryPos);

    void                GetOptions( const OUString& rFormat,
                                    bool&           rThousand,
                                    bool&           rNegRed,
                                    sal_uInt16&     rPrecision,
                                    sal_uInt16&     rLeadingZeroes,
                                    sal_uInt16&     rCatLbPos );

    sal_uInt16          GetFormatIntegerDigits( const OUString&  rFormat ) const;

    void                MakePreviewString( const OUString& rFormatStr,
                                           OUString&       rPreviewStr,
                                           Color*&         rpFontColor );

    void                MakePrevStringFromVal( const OUString& rFormatStr,
                                               OUString& rPreviewStr,
                                               Color*& rpFontColor,
                                               double  nValue);

    bool                IsUserDefined( const OUString& rFmtString );
    bool                IsTmpCurrencyFormat( const OUString& rFmtString );
    bool                FindEntry( const OUString& rFmtString, sal_uInt32* pAt = nullptr );

    void                ValidateNewEntries() { bUndoAddList = false; }
    std::vector<sal_uInt32> const & GetUpdateData() const;

    void                SetCurNumFmtKey( sal_uInt32 nNew )  { nCurFormatKey = nNew; }
    sal_uInt32          GetCurNumFmtKey() const             { return nCurFormatKey; }
    LanguageType        GetCurLanguage() const              { return eCurLanguage; }

                        /** Returns the name of Standard, General, ... for the
                            current language. */
    OUString            GetStandardName() const;

    OUString            GetComment4Entry(short nEntry);
    short               GetCategory4Entry(short nEntry) const;
    bool                GetUserDefined4Entry(short nEntry);
    OUString            GetFormat4Entry(short nEntry);
    void                SetComment4Entry(short nEntry, const OUString& aCommentString);

    void                SetCurrencySymbol(sal_uInt32 nPos);
    sal_uInt32          GetCurrencySymbol() { return nCurCurrencyEntryPos;}
    sal_uInt16          FindCurrencyFormat( const OUString& rFmtString );
    sal_uInt16          FindCurrencyFormat(const NfCurrencyEntry* pTmpCurrencyEntry,bool bTmpBanking);
    void                SetCurCurrencyEntry(NfCurrencyEntry*);
    short               GetListPos4Entry( sal_uInt32 nIdx, const OUString& rFmtString );

    void                GetCurrencySymbols(std::vector<OUString>& rList, sal_uInt16* pPos );

    sal_uInt16          FindCurrencyTableEntry( const OUString& rFmtString, bool &bTestBanking );
    bool                IsInTable(sal_uInt16 nPos, bool bTmpBanking,
                            OUString const& rFmtString);

    void                SetUseStarFormat( bool bUse ) { bUseStarFormat = bUse; }
private:
    SvNumberFormatter*      pFormatter;
    SvNumberFormatTable*    pCurFmtTable;
    SvxNumberValueType const eValType;
    OUString                aValStr;
    double                  nValNum;
    bool                    bUndoAddList;
    std::vector<sal_uInt32> aAddList;
    std::vector<sal_uInt32> aDelList;
    std::vector<sal_uInt32> aCurEntryList;
    sal_uInt32              nCurFormatKey;
    SvNumFormatType         nCurCategory;
    LanguageType            eCurLanguage;
    std::vector<sal_uInt16> aCurCurrencyList;
    NfCurrencyEntry*        pCurCurrencyEntry;
    bool                    bBankingSymbol;
    sal_uInt32              nCurCurrencyEntryPos;
    std::vector<OUString>   aCurrencyFormatList;
    bool                    bUseStarFormat;
    bool                    bIsDefaultValNum;

    SVX_DLLPRIVATE short FillEntryList_Impl( std::vector<OUString>& rList );
    SVX_DLLPRIVATE void  FillEListWithStd_Impl( std::vector<OUString>& rList, SvNumFormatType eCategory, short &Pos,
                                                bool bSuppressDuplicates = false );
    SVX_DLLPRIVATE short FillEListWithFormats_Impl( std::vector<OUString>& rList,short nSelPos,
                                                       NfIndexTableOffset eOffsetStart,
                                                       NfIndexTableOffset eOffsetEnd,
                                                       bool bSuppressDuplicates );
    SVX_DLLPRIVATE short FillEListWithDateTime_Impl( std::vector<OUString>& rList,short nSelPos,
                                                     bool bSuppressDuplicates );
    SVX_DLLPRIVATE short FillEListWithCurrency_Impl( std::vector<OUString>& rList,short nSelPos);
    SVX_DLLPRIVATE short FillEListWithSysCurrencys( std::vector<OUString>& rList,short nSelPos);
    SVX_DLLPRIVATE short FillEListWithUserCurrencys( std::vector<OUString>& rList,short nSelPos);
    SVX_DLLPRIVATE short FillEListWithUsD_Impl( std::vector<OUString>& rList, SvNumFormatType eCategory, short Pos );

    SVX_DLLPRIVATE bool IsEssentialFormat_Impl( SvNumFormatType eType, sal_uInt32 nKey );

    SVX_DLLPRIVATE ::std::vector<sal_uInt32>::iterator GetRemoved_Impl( size_t nKey );
    SVX_DLLPRIVATE bool                                IsRemoved_Impl( size_t nKey );
    SVX_DLLPRIVATE ::std::vector<sal_uInt32>::iterator GetAdded_Impl( size_t nKey );

    SVX_DLLPRIVATE void  GetPreviewString_Impl( OUString& rString,
                                                Color*& rpColor );
    SVX_DLLPRIVATE static void PosToCategory_Impl(sal_uInt16 nPos, SvNumFormatType& rCategory);
    SVX_DLLPRIVATE static void CategoryToPos_Impl(SvNumFormatType nCategory, sal_uInt16& rPos);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
