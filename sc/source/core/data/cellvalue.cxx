/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cellvalue.hxx"
#include "document.hxx"
#include "column.hxx"
#include "formulacell.hxx"
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include "stringutil.hxx"
#include "editutil.hxx"
#include "tokenarray.hxx"
#include <formula/token.hxx>
#include <svl/sharedstring.hxx>

namespace {

CellType adjustCellType( CellType eOrig )
{
    switch (eOrig)
    {
        case CELLTYPE_EDIT:
            return CELLTYPE_STRING;
        default:
            ;
    }
    return eOrig;
}

template<typename T>
OUString getString( const T& rVal )
{
    if (rVal.meType == CELLTYPE_STRING)
        return rVal.mpString->getString();

    if (rVal.meType == CELLTYPE_EDIT)
    {
        OUStringBuffer aRet;
        sal_Int32 n = rVal.mpEditText->GetParagraphCount();
        for (sal_Int32 i = 0; i < n; ++i)
        {
            if (i > 0)
                aRet.append('\n');
            aRet.append(rVal.mpEditText->GetText(i));
        }
        return aRet.makeStringAndClear();
    }

    return EMPTY_OUSTRING;
}

bool equalsFormulaCells( const ScFormulaCell* p1, const ScFormulaCell* p2 )
{
    const ScTokenArray* pCode1 = p1->GetCode();
    const ScTokenArray* pCode2 = p2->GetCode();

    if (pCode1->GetLen() != pCode2->GetLen())
        return false;

    sal_uInt16 n = pCode1->GetLen();
    formula::FormulaToken** ppToken1 = pCode1->GetArray();
    formula::FormulaToken** ppToken2 = pCode2->GetArray();
    for (sal_uInt16 i = 0; i < n; ++i)
    {
        if (!ppToken1[i]->TextEqual(*(ppToken2[i])))
            return false;
    }

    return true;
}

template<typename T>
bool equalsWithoutFormatImpl( const T& left, const T& right )
{
    CellType eType1 = adjustCellType(left.meType);
    CellType eType2 = adjustCellType(right.meType);
    if (eType1 != eType2)
        return false;

    switch (eType1)
    {
        case CELLTYPE_NONE:
            return true;
        case CELLTYPE_VALUE:
            return left.mfValue == right.mfValue;
        case CELLTYPE_STRING:
        {
            OUString aStr1 = getString(left);
            OUString aStr2 = getString(right);
            return aStr1 == aStr2;
        }
        case CELLTYPE_FORMULA:
            return equalsFormulaCells(left.mpFormula, right.mpFormula);
        default:
            ;
    }
    return false;
}

void commitToColumn( const ScCellValue& rCell, ScColumn& rColumn, SCROW nRow )
{
    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
            rColumn.SetRawString(nRow, *rCell.mpString);
        break;
        case CELLTYPE_EDIT:
            rColumn.SetEditText(nRow, ScEditUtil::Clone(*rCell.mpEditText, rColumn.GetDoc()));
        break;
        case CELLTYPE_VALUE:
            rColumn.SetValue(nRow, rCell.mfValue);
        break;
        case CELLTYPE_FORMULA:
        {
            ScAddress aDestPos(rColumn.GetCol(), nRow, rColumn.GetTab());
            rColumn.SetFormulaCell(nRow, new ScFormulaCell(*rCell.mpFormula, rColumn.GetDoc(), aDestPos));
        }
        break;
        default:
            rColumn.DeleteContent(nRow);
    }
}

bool hasStringImpl( CellType eType, ScFormulaCell* pFormula )
{
    switch (eType)
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            return true;
        case CELLTYPE_FORMULA:
            return !pFormula->IsValue();
        default:
            return false;
    }
}

bool hasNumericImpl( CellType eType, ScFormulaCell* pFormula )
{
    switch (eType)
    {
        case CELLTYPE_VALUE:
            return true;
        case CELLTYPE_FORMULA:
            return pFormula->IsValue();
        default:
            return false;
    }
}

template<typename CellT>
OUString getStringImpl( const CellT& rCell, const ScDocument* pDoc )
{
    switch (rCell.meType)
    {
        case CELLTYPE_VALUE:
            return OUString::number(rCell.mfValue);
        case CELLTYPE_STRING:
            return rCell.mpString->getString();
        case CELLTYPE_EDIT:
            if (rCell.mpEditText)
                return ScEditUtil::GetString(*rCell.mpEditText, pDoc);
        break;
        case CELLTYPE_FORMULA:
            return rCell.mpFormula->GetString().getString();
        default:
            ;
    }
    return EMPTY_OUSTRING;
}

}

ScCellValue::ScCellValue() : meType(CELLTYPE_NONE), mfValue(0.0) {}

ScCellValue::ScCellValue( const ScRefCellValue& rCell ) : meType(rCell.meType), mfValue(rCell.mfValue)
{
    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
            mpString = new svl::SharedString(*rCell.mpString);
        break;
        case CELLTYPE_EDIT:
            mpEditText = rCell.mpEditText->Clone();
        break;
        case CELLTYPE_FORMULA:
            mpFormula = rCell.mpFormula->Clone();
        break;
        default:
            ;
    }
}

ScCellValue::ScCellValue( double fValue ) : meType(CELLTYPE_VALUE), mfValue(fValue) {}

ScCellValue::ScCellValue( const svl::SharedString& rString ) : meType(CELLTYPE_STRING), mpString(new svl::SharedString(rString)) {}

ScCellValue::ScCellValue( const ScCellValue& r ) : meType(r.meType), mfValue(r.mfValue)
{
    switch (r.meType)
    {
        case CELLTYPE_STRING:
            mpString = new svl::SharedString(*r.mpString);
        break;
        case CELLTYPE_EDIT:
            mpEditText = r.mpEditText->Clone();
        break;
        case CELLTYPE_FORMULA:
            mpFormula = r.mpFormula->Clone();
        break;
        default:
            ;
    }
}

ScCellValue::~ScCellValue()
{
    clear();
}

void ScCellValue::clear()
{
    switch (meType)
    {
        case CELLTYPE_STRING:
            delete mpString;
        break;
        case CELLTYPE_EDIT:
            delete mpEditText;
        break;
        case CELLTYPE_FORMULA:
            delete mpFormula;
        break;
        default:
            ;
    }

    // Reset to empty value.
    meType = CELLTYPE_NONE;
    mfValue = 0.0;
}

void ScCellValue::set( double fValue )
{
    clear();
    meType = CELLTYPE_VALUE;
    mfValue = fValue;
}

void ScCellValue::set( const svl::SharedString& rStr )
{
    clear();
    meType = CELLTYPE_STRING;
    mpString = new svl::SharedString(rStr);
}

void ScCellValue::set( const EditTextObject& rEditText )
{
    clear();
    meType = CELLTYPE_EDIT;
    mpEditText = rEditText.Clone();
}

void ScCellValue::set( EditTextObject* pEditText )
{
    clear();
    meType = CELLTYPE_EDIT;
    mpEditText = pEditText;
}

void ScCellValue::set( ScFormulaCell* pFormula )
{
    clear();
    meType = CELLTYPE_FORMULA;
    mpFormula = pFormula;
}

void ScCellValue::assign( const ScDocument& rDoc, const ScAddress& rPos )
{
    clear();

    ScRefCellValue aRefVal(const_cast<ScDocument&>(rDoc), rPos);

    meType = aRefVal.meType;
    switch (meType)
    {
        case CELLTYPE_STRING:
            mpString = new svl::SharedString(*aRefVal.mpString);
        break;
        case CELLTYPE_EDIT:
            if (aRefVal.mpEditText)
                mpEditText = aRefVal.mpEditText->Clone();
        break;
        case CELLTYPE_VALUE:
            mfValue = aRefVal.mfValue;
        break;
        case CELLTYPE_FORMULA:
            mpFormula = aRefVal.mpFormula->Clone();
        break;
        default:
            meType = CELLTYPE_NONE; // reset to empty.
    }
}

void ScCellValue::assign(const ScCellValue& rOther, ScDocument& rDestDoc, ScCloneFlags nCloneFlags)
{
    clear();

    meType = rOther.meType;
    switch (meType)
    {
        case CELLTYPE_STRING:
            mpString = new svl::SharedString(*rOther.mpString);
        break;
        case CELLTYPE_EDIT:
        {
            // Switch to the pool of the destination document.
            ScFieldEditEngine& rEngine = rDestDoc.GetEditEngine();
            if (rOther.mpEditText->HasOnlineSpellErrors())
            {
                EEControlBits nControl = rEngine.GetControlWord();
                const EEControlBits nSpellControl = EEControlBits::ONLINESPELLING | EEControlBits::ALLOWBIGOBJS;
                bool bNewControl = ((nControl & nSpellControl) != nSpellControl);
                if (bNewControl)
                    rEngine.SetControlWord(nControl | nSpellControl);
                rEngine.SetText(*rOther.mpEditText);
                mpEditText = rEngine.CreateTextObject();
                if (bNewControl)
                    rEngine.SetControlWord(nControl);
            }
            else
            {
                rEngine.SetText(*rOther.mpEditText);
                mpEditText = rEngine.CreateTextObject();
            }
        }
        break;
        case CELLTYPE_VALUE:
            mfValue = rOther.mfValue;
        break;
        case CELLTYPE_FORMULA:
            // Switch to the destination document.
            mpFormula = new ScFormulaCell(*rOther.mpFormula, rDestDoc, rOther.mpFormula->aPos, nCloneFlags);
        break;
        default:
            meType = CELLTYPE_NONE; // reset to empty.
    }
}

void ScCellValue::commit( ScDocument& rDoc, const ScAddress& rPos ) const
{
    switch (meType)
    {
        case CELLTYPE_STRING:
        {
            ScSetStringParam aParam;
            aParam.setTextInput();
            rDoc.SetString(rPos, mpString->getString(), &aParam);
        }
        break;
        case CELLTYPE_EDIT:
            rDoc.SetEditText(rPos, mpEditText->Clone());
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(rPos, mfValue);
        break;
        case CELLTYPE_FORMULA:
            rDoc.SetFormulaCell(rPos, mpFormula->Clone());
        break;
        default:
            rDoc.SetEmptyCell(rPos);
    }
}

void ScCellValue::commit( ScColumn& rColumn, SCROW nRow ) const
{
    commitToColumn(*this, rColumn, nRow);
}

void ScCellValue::release( ScDocument& rDoc, const ScAddress& rPos )
{
    switch (meType)
    {
        case CELLTYPE_STRING:
        {
            // Currently, string cannot be placed without copying.
            ScSetStringParam aParam;
            aParam.setTextInput();
            rDoc.SetString(rPos, mpString->getString(), &aParam);
            delete mpString;
        }
        break;
        case CELLTYPE_EDIT:
            // Cell takes the ownership of the text object.
            rDoc.SetEditText(rPos, mpEditText);
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(rPos, mfValue);
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            rDoc.SetFormulaCell(rPos, mpFormula);
        break;
        default:
            rDoc.SetEmptyCell(rPos);
    }

    meType = CELLTYPE_NONE;
    mfValue = 0.0;
}

void ScCellValue::release( ScColumn& rColumn, SCROW nRow, sc::StartListeningType eListenType )
{
    switch (meType)
    {
        case CELLTYPE_STRING:
        {
            // Currently, string cannot be placed without copying.
            rColumn.SetRawString(nRow, *mpString);
            delete mpString;
        }
        break;
        case CELLTYPE_EDIT:
            // Cell takes the ownership of the text object.
            rColumn.SetEditText(nRow, mpEditText);
        break;
        case CELLTYPE_VALUE:
            rColumn.SetValue(nRow, mfValue);
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            rColumn.SetFormulaCell(nRow, mpFormula, eListenType);
        break;
        default:
            rColumn.DeleteContent(nRow);
    }

    meType = CELLTYPE_NONE;
    mfValue = 0.0;
}

OUString ScCellValue::getString( const ScDocument* pDoc )
{
    return getStringImpl(*this, pDoc);
}

bool ScCellValue::isEmpty() const
{
    return meType == CELLTYPE_NONE;
}

bool ScCellValue::equalsWithoutFormat( const ScCellValue& r ) const
{
    return equalsWithoutFormatImpl(*this, r);
}

ScCellValue& ScCellValue::operator= ( const ScCellValue& r )
{
    ScCellValue aTmp(r);
    swap(aTmp);
    return *this;
}

ScCellValue& ScCellValue::operator= ( const ScRefCellValue& r )
{
    ScCellValue aTmp(r);
    swap(aTmp);
    return *this;
}

void ScCellValue::swap( ScCellValue& r )
{
    std::swap(meType, r.meType);

    // double is 8 bytes, whereas a pointer may be 4 or 8 bytes depending on
    // the platform. Swap by double values.
    std::swap(mfValue, r.mfValue);
}

ScRefCellValue::ScRefCellValue() : meType(CELLTYPE_NONE), mfValue(0.0) {}
ScRefCellValue::ScRefCellValue( double fValue ) : meType(CELLTYPE_VALUE), mfValue(fValue) {}
ScRefCellValue::ScRefCellValue( const svl::SharedString* pString ) : meType(CELLTYPE_STRING), mpString(pString) {}
ScRefCellValue::ScRefCellValue( const EditTextObject* pEditText ) : meType(CELLTYPE_EDIT), mpEditText(pEditText) {}
ScRefCellValue::ScRefCellValue( ScFormulaCell* pFormula ) : meType(CELLTYPE_FORMULA), mpFormula(pFormula) {}

// It should be enough to copy the double value, which is at least as large
// as the pointer values.
ScRefCellValue::ScRefCellValue( const ScRefCellValue& r ) : meType(r.meType), mfValue(r.mfValue) {}

ScRefCellValue::ScRefCellValue( ScDocument& rDoc, const ScAddress& rPos )
{
    assign( rDoc, rPos);
}

ScRefCellValue::~ScRefCellValue()
{
}

void ScRefCellValue::clear()
{
    // Reset to empty value.
    meType = CELLTYPE_NONE;
    mfValue = 0.0;
}

void ScRefCellValue::assign( ScDocument& rDoc, const ScAddress& rPos )
{
    *this = rDoc.GetRefCellValue(rPos);
}

void ScRefCellValue::commit( ScDocument& rDoc, const ScAddress& rPos ) const
{
    switch (meType)
    {
        case CELLTYPE_STRING:
        {
            ScSetStringParam aParam;
            aParam.setTextInput();
            rDoc.SetString(rPos, mpString->getString(), &aParam);
        }
        break;
        case CELLTYPE_EDIT:
            rDoc.SetEditText(rPos, ScEditUtil::Clone(*mpEditText, rDoc));
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(rPos, mfValue);
        break;
        case CELLTYPE_FORMULA:
            rDoc.SetFormulaCell(rPos, new ScFormulaCell(*mpFormula, rDoc, rPos));
        break;
        default:
            rDoc.SetEmptyCell(rPos);
    }
}

bool ScRefCellValue::hasString() const
{
    return hasStringImpl(meType, mpFormula);
}

bool ScRefCellValue::hasNumeric() const
{
    return hasNumericImpl(meType, mpFormula);
}

double ScRefCellValue::getValue()
{
    switch (meType)
    {
        case CELLTYPE_VALUE:
            return mfValue;
        case CELLTYPE_FORMULA:
            return mpFormula->GetValue();
        default:
            ;
    }
    return 0.0;
}

OUString ScRefCellValue::getString( const ScDocument* pDoc )
{
    return getStringImpl(*this, pDoc);
}

bool ScRefCellValue::isEmpty() const
{
    return meType == CELLTYPE_NONE;
}

bool ScRefCellValue::hasEmptyValue()
{
    if (isEmpty())
        return true;

    if (meType == CELLTYPE_FORMULA)
        return mpFormula->IsEmpty();

    return false;
}

bool ScRefCellValue::equalsWithoutFormat( const ScRefCellValue& r ) const
{
    return equalsWithoutFormatImpl(*this, r);
}

ScRefCellValue& ScRefCellValue::operator= ( const ScRefCellValue& r )
{
    // So we *could* have a copy-swap-idiom here for exception-safety if we had
    // to slow down things.. but then implement an explicit move-ctor and pass
    // r by-value instead of manually creating a temporary so the compiler can
    // take advantage. And initialize
    // ScRefCellValue(ScDocument&,const ScAddress&) with default ctor.
    meType = r.meType;
    mfValue = r.mfValue;    // largest member of union
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
