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

#ifndef SC_VALIDAT_HXX
#define SC_VALIDAT_HXX

#include "conditio.hxx"
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include "scdllapi.h"

namespace ValidListType = ::com::sun::star::sheet::TableValidationVisibility;

namespace sc {

struct RefUpdateContext;

}

class ScPatternAttr;
class ScTokenArray;
class ScTypedStrData;

enum ScValidationMode
{
    SC_VALID_ANY,
    SC_VALID_WHOLE,
    SC_VALID_DECIMAL,
    SC_VALID_DATE,
    SC_VALID_TIME,
    SC_VALID_TEXTLEN,
    SC_VALID_LIST,
    SC_VALID_CUSTOM
};

enum ScValidErrorStyle
{
    SC_VALERR_STOP,
    SC_VALERR_WARNING,
    SC_VALERR_INFO,
    SC_VALERR_MACRO
};

//
// Entry for validation (only one condition exists)
//

class SC_DLLPUBLIC ScValidationData : public ScConditionEntry
{
    sal_uInt32          nKey;               // index in attributes

    ScValidationMode    eDataMode;
    sal_Bool                bShowInput;
    sal_Bool                bShowError;
    ScValidErrorStyle   eErrorStyle;
    sal_Int16           mnListType;         // selection list type: none, unsorted, sorted.
    String              aInputTitle;
    String              aInputMessage;
    String              aErrorTitle;
    String              aErrorMessage;

    sal_Bool                bIsUsed;            // temporary during saving

    sal_Bool            DoMacro( const ScAddress& rPos, const String& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const;

    sal_Bool            DoScript( const ScAddress& rPos, const String& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const;

    using ScConditionEntry::operator==;

public:
            ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rExprNmsp1 = EMPTY_STRING, const String& rExprNmsp2 = EMPTY_STRING,
                                formula::FormulaGrammar::Grammar eGrammar1 = formula::FormulaGrammar::GRAM_DEFAULT,
                                formula::FormulaGrammar::Grammar eGrammar2 = formula::FormulaGrammar::GRAM_DEFAULT );
            ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos );
            ScValidationData( const ScValidationData& r );
            ScValidationData( ScDocument* pDocument, const ScValidationData& r );
    virtual ~ScValidationData();

    ScValidationData* Clone() const     // real copy
                    { return new ScValidationData( GetDocument(), *this ); }
    ScValidationData* Clone(ScDocument* pNew) const
                    { return new ScValidationData( pNew, *this ); }

    void            ResetInput();
    void            ResetError();
    void            SetInput( const String& rTitle, const String& rMsg );
    void            SetError( const String& rTitle, const String& rMsg,
                                ScValidErrorStyle eStyle );

    sal_Bool            GetInput( String& rTitle, String& rMsg ) const
                        { rTitle = aInputTitle; rMsg = aInputMessage; return bShowInput; }
    sal_Bool            GetErrMsg( String& rTitle, String& rMsg, ScValidErrorStyle& rStyle ) const;

    sal_Bool            HasErrMsg() const       { return bShowError; }

    ScValidationMode GetDataMode() const    { return eDataMode; }

    inline sal_Int16 GetListType() const                { return mnListType; }
    inline void     SetListType( sal_Int16 nListType )  { mnListType = nListType; }

    /** Returns true, if the validation cell will show a selection list.
        @descr  Use this instead of GetListType() which returns the raw property
        regardless of the validation type. */
    bool            HasSelectionList() const;
    /** Tries to fill the passed collection with list validation entries.
        @descr  Fills the list only, if this is a list validation and IsShowList() is enabled.
        @param rStrings  (out-param) The string list to fill with list validation entires.
        @return  true = rStrings has been filled with at least one entry. */
    bool FillSelectionList(std::vector<ScTypedStrData>& rStrings, const ScAddress& rPos) const;

    //  with string: during input, with cell: for detective / RC_FORCED
    bool IsDataValid(
        const OUString& rTest, const ScPatternAttr& rPattern, const ScAddress& rPos ) const;

    bool IsDataValid( ScRefCellValue& rCell, const ScAddress& rPos ) const;

                    // TRUE -> break
    sal_Bool            DoError( Window* pParent, const String& rInput, const ScAddress& rPos ) const;
    void            DoCalcError( ScFormulaCell* pCell ) const;

    sal_Bool            IsEmpty() const;
    sal_uInt32      GetKey() const          { return nKey; }
    void            SetKey(sal_uInt32 nNew) { nKey = nNew; }    // only if not inserted!

    void            SetUsed(sal_Bool bSet)      { bIsUsed = bSet; }
    sal_Bool            IsUsed() const          { return bIsUsed; }

    sal_Bool            EqualEntries( const ScValidationData& r ) const;    // for undo

    //  sort (using std::set) by index
    //  operator== only for sorting
    bool operator ==( const ScValidationData& r ) const { return nKey == r.nKey; }
    bool operator < ( const ScValidationData& r ) const { return nKey <  r.nKey; }

private:
    /** Tries to fill the passed collection with list validation entries.
        @descr  Fills the list only if it is non-NULL,
        @param pStrings  (out-param) Optionally NULL, string list to fill with list validation entires.
        @param pCell     can be NULL if it is not necessary to which element in the list is selected.
        @param rPos      the base address for relative references.
        @param rTokArr   Formula token array.
        @param rMatch    (out-param) the index of the first item that matched, -1 if nothing matched.
        @return  true = Cell range found, rRange is valid, or an error entry stuffed into the list if pCell==NULL. */
    bool GetSelectionFromFormula(
        std::vector<ScTypedStrData>* pStrings, ScRefCellValue& rCell, const ScAddress& rPos,
        const ScTokenArray& rTokArr, int& rMatch) const;

    /** Tests, if pCell is equal to what the passed token array represents. */
    bool IsEqualToTokenArray( ScRefCellValue& rCell, const ScAddress& rPos, const ScTokenArray& rTokArr ) const;

    /** Tests, if contents of pCell occur in cell range referenced by own formula, or in a string list. */
    bool IsListValid( ScRefCellValue& rCell, const ScAddress& rPos ) const;
};

//
//  list of contitions:
//

struct CompareScValidationDataPtr
{
  bool operator()( ScValidationData* const& lhs, ScValidationData* const& rhs ) const { return (*lhs)<(*rhs); }
};

class ScValidationDataList : public std::set<ScValidationData*, CompareScValidationDataPtr>
{
public:
    ScValidationDataList() {}
    ScValidationDataList(const ScValidationDataList& rList);
    ScValidationDataList(ScDocument* pNewDoc, const ScValidationDataList& rList);
    ~ScValidationDataList() {}

    void    InsertNew( ScValidationData* pNew )
                { if (!insert(pNew).second) delete pNew; }

    ScValidationData* GetData( sal_uInt32 nKey );

    void    CompileXML();
    void UpdateReference( sc::RefUpdateContext& rCxt );
    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt );

    sal_Bool    operator==( const ScValidationDataList& r ) const;      // for ref-undo
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
