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

#ifndef INCLUDED_SC_INC_VALIDAT_HXX
#define INCLUDED_SC_INC_VALIDAT_HXX

#include "conditio.hxx"
#include "scdllapi.h"

namespace vcl { class Window; }
namespace weld { class Window; }

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

// Entry for validation (only one condition exists)

class SC_DLLPUBLIC ScValidationData : public ScConditionEntry
{
private:
    sal_uInt32 nKey;               // index in attributes

    ScValidationMode eDataMode;
    bool bShowInput;
    bool bShowError;
    ScValidErrorStyle eErrorStyle;
    sal_Int16 mnListType;         // selection list type: none, unsorted, sorted.
    OUString aInputTitle;
    OUString aInputMessage;
    OUString aErrorTitle;
    OUString aErrorMessage;

    bool DoMacro( const ScAddress& rPos, const OUString& rInput,
                                ScFormulaCell* pCell, weld::Window* pParent ) const;

    bool DoScript( const ScAddress& rPos, const OUString& rInput,
                                ScFormulaCell* pCell, weld::Window* pParent ) const;

    using ScConditionEntry::operator==;

public:
            ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                const OUString& rExpr1, const OUString& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const OUString& rExprNmsp1 = EMPTY_OUSTRING, const OUString& rExprNmsp2 = EMPTY_OUSTRING,
                                formula::FormulaGrammar::Grammar eGrammar1 = formula::FormulaGrammar::GRAM_DEFAULT,
                                formula::FormulaGrammar::Grammar eGrammar2 = formula::FormulaGrammar::GRAM_DEFAULT );
            ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos );
            ScValidationData( const ScValidationData& r );
            ScValidationData( ScDocument* pDocument, const ScValidationData& r );
    virtual ~ScValidationData() override;

    ScValidationData* Clone() const     // real copy
                    { return new ScValidationData( GetDocument(), *this ); }
    ScValidationData* Clone(ScDocument* pNew) const override
                    { return new ScValidationData( pNew, *this ); }

    void            ResetInput();
    void            ResetError();
    void            SetInput( const OUString& rTitle, const OUString& rMsg );
    void            SetError( const OUString& rTitle, const OUString& rMsg,
                                ScValidErrorStyle eStyle );

    bool            GetInput( OUString& rTitle, OUString& rMsg ) const
                        { rTitle = aInputTitle; rMsg = aInputMessage; return bShowInput; }
    bool            GetErrMsg( OUString& rTitle, OUString& rMsg, ScValidErrorStyle& rStyle ) const;

    bool            HasErrMsg() const       { return bShowError; }

    ScValidationMode GetDataMode() const    { return eDataMode; }

    sal_Int16 GetListType() const                { return mnListType; }
    void     SetListType( sal_Int16 nListType )  { mnListType = nListType; }

    /** Returns true, if the validation cell will show a selection list.
        @descr  Use this instead of GetListType() which returns the raw property
        regardless of the validation type. */
    bool            HasSelectionList() const;
    /** Tries to fill the passed collection with list validation entries.
        @descr  Fills the list only, if this is a list validation and IsShowList() is enabled.
        @param rStrings  (out-param) The string list to fill with list validation entries.
        @return  true = rStrings has been filled with at least one entry. */
    bool FillSelectionList(std::vector<ScTypedStrData>& rStrings, const ScAddress& rPos) const;

    //  with string: during input, with cell: for detective / RC_FORCED
    bool IsDataValid(
        const OUString& rTest, const ScPatternAttr& rPattern, const ScAddress& rPos ) const;

    // Custom validations (SC_VALID_CUSTOM) should be validated using this specific method.
    // Take care that internally this method commits to the to be validated cell the new input,
    // in order to be able to interpret the validating boolean formula on the new input.
    // After the formula has been evaluated the original cell content is restored.
    // At present is only used in ScInputHandler::EnterHandler: handling this case in the
    // regular IsDataValid method would have been unsafe since it can be invoked
    // by ScFormulaCell::InterpretTail.

    struct CustomValidationPrivateAccess
    {
        // so IsDataValidCustom can be invoked only by ScInputHandler methods
        friend class ScInputHandler;
    private:
        CustomValidationPrivateAccess() {}
    };

    bool IsDataValidCustom(
        const OUString& rTest, const ScPatternAttr& rPattern,
        const ScAddress& rPos, const CustomValidationPrivateAccess& ) const;

    bool IsDataValid( ScRefCellValue& rCell, const ScAddress& rPos ) const;

                    // TRUE -> break
    bool DoError(weld::Window* pParent, const OUString& rInput, const ScAddress& rPos) const;
    void DoCalcError( ScFormulaCell* pCell ) const;

    bool IsEmpty() const;
    sal_uInt32      GetKey() const          { return nKey; }
    void            SetKey(sal_uInt32 nNew) { nKey = nNew; }    // only if not inserted!

    bool            EqualEntries( const ScValidationData& r ) const;    // for undo

    //  sort (using std::set) by index
    bool operator < ( const ScValidationData& r ) const { return nKey <  r.nKey; }

private:
    /** Tries to fill the passed collection with list validation entries.
        @descr  Fills the list only if it is non-NULL,
        @param pStrings  (out-param) Optionally NULL, string list to fill with list validation entries.
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

//  list of conditions:

struct CompareScValidationDataPtr
{
  bool operator()( ScValidationData* const& lhs, ScValidationData* const& rhs ) const { return (*lhs)<(*rhs); }
};

class ScValidationDataList
{
private:
    typedef std::set<ScValidationData*, CompareScValidationDataPtr> ScValidationDataListDataType;
    ScValidationDataListDataType maData;

public:
    ScValidationDataList() {}
    ScValidationDataList(const ScValidationDataList& rList);
    ScValidationDataList(ScDocument* pNewDoc, const ScValidationDataList& rList);

    typedef ScValidationDataListDataType::iterator iterator;
    typedef ScValidationDataListDataType::const_iterator const_iterator;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    void InsertNew( ScValidationData* pNew )
                { if (!maData.insert(pNew).second) delete pNew; }

    ScValidationData* GetData( sal_uInt32 nKey );

    void CompileXML();
    void UpdateReference( sc::RefUpdateContext& rCxt );
    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt );

    void clear();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
