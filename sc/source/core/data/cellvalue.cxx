/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cellvalue.hxx>
#include <document.hxx>
#include <column.hxx>
#include <formulacell.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <stringutil.hxx>
#include <editutil.hxx>
#include <tokenarray.hxx>
#include <formula/token.hxx>
#include <formula/errorcodes.hxx>
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
    if (rVal.getType() == CELLTYPE_STRING)
        return rVal.mpString->getString();

    if (rVal.getType() == CELLTYPE_EDIT)
    {
        OUStringBuffer aRet;
        sal_Int32 n = rVal.mpEditText1->GetParagraphCount();
        for (sal_Int32 i = 0; i < n; ++i)
        {
            if (i > 0)
                aRet.append('\n');
            aRet.append(rVal.mpEditText1->GetText(i));
        }
        return aRet.makeStringAndClear();
    }

    return OUString();
}

bool equalsFormulaCells( const ScFormulaCell* p1, const ScFormulaCell* p2 )
{
    const ScTokenArray* pCode1 = p1->GetCode();
    const ScTokenArray* pCode2 = p2->GetCode();

    if (pCode1->GetLen() != pCode2->GetLen())
        return false;

    if (pCode1->GetCodeError() != pCode2->GetCodeError())
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
    CellType eType1 = adjustCellType(left.getType());
    CellType eType2 = adjustCellType(right.getType());
    if (eType1 != eType2)
        return false;

    switch (eType1)
    {
        case CELLTYPE_NONE:
            return true;
        case CELLTYPE_VALUE:
            return left.getDouble() == right.getDouble();
        case CELLTYPE_STRING:
        {
            OUString aStr1 = getString(left);
            OUString aStr2 = getString(right);
            return aStr1 == aStr2;
        }
        case CELLTYPE_FORMULA:
            return equalsFormulaCells(left.mpFormula1, right.mpFormula1);
        default:
            ;
    }
    return false;
}

void commitToColumn( const ScCellValue& rCell, ScColumn& rColumn, SCROW nRow )
{
    switch (rCell.getType())
    {
        case CELLTYPE_STRING:
            rColumn.SetRawString(nRow, *rCell.mpString);
        break;
        case CELLTYPE_EDIT:
            rColumn.SetEditText(nRow, ScEditUtil::Clone(*rCell.mpEditText1, rColumn.GetDoc()));
        break;
        case CELLTYPE_VALUE:
            rColumn.SetValue(nRow, rCell.getDouble());
        break;
        case CELLTYPE_FORMULA:
        {
            ScAddress aDestPos(rColumn.GetCol(), nRow, rColumn.GetTab());
            rColumn.SetFormulaCell(nRow, new ScFormulaCell(*rCell.mpFormula1, rColumn.GetDoc(), aDestPos));
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
    switch (rCell.getType())
    {
        case CELLTYPE_VALUE:
            return OUString::number(rCell.getDouble());
        case CELLTYPE_STRING:
            return rCell.mpString->getString();
        case CELLTYPE_EDIT:
            if (rCell.mpEditText1)
                return ScEditUtil::GetString(*rCell.mpEditText1, pDoc);
        break;
        case CELLTYPE_FORMULA:
            return rCell.mpFormula1->GetString().getString();
        default:
            ;
    }
    return OUString();
}

template<typename CellT>
OUString getRawStringImpl( const CellT& rCell, const ScDocument& rDoc )
{
    switch (rCell.getType())
    {
        case CELLTYPE_VALUE:
            return OUString::number(rCell.mfValue1);
        case CELLTYPE_STRING:
            return rCell.mpString->getString();
        case CELLTYPE_EDIT:
            if (rCell.mpEditText1)
                return ScEditUtil::GetString(*rCell.mpEditText1, &rDoc);
        break;
        case CELLTYPE_FORMULA:
            return rCell.mpFormula1->GetRawString().getString();
        default:
            ;
    }
    return OUString();
}

}

ScCellValue::ScCellValue() : meType(CELLTYPE_NONE), mfValue1(0.0) {}

ScCellValue::ScCellValue( const ScRefCellValue& rCell ) : meType(rCell.getType()), mfValue1(rCell.mfValue1)
{
    switch (rCell.getType())
    {
        case CELLTYPE_STRING:
            mpString = new svl::SharedString(*rCell.mpString);
        break;
        case CELLTYPE_EDIT:
            mpEditText1 = rCell.mpEditText1->Clone().release();
        break;
        case CELLTYPE_FORMULA:
            mpFormula1 = rCell.mpFormula1->Clone();
        break;
        default:
            ;
    }
}

ScCellValue::ScCellValue( double fValue ) : meType(CELLTYPE_VALUE), mfValue1(fValue) {}

ScCellValue::ScCellValue( const svl::SharedString& rString ) : meType(CELLTYPE_STRING), mpString(new svl::SharedString(rString)) {}

ScCellValue::ScCellValue( std::unique_ptr<EditTextObject> xEdit ) : meType(CELLTYPE_EDIT), mpEditText1(xEdit.release()) {}

ScCellValue::ScCellValue( const ScCellValue& r ) : meType(r.meType), mfValue1(r.mfValue1)
{
    switch (r.meType)
    {
        case CELLTYPE_STRING:
            mpString = new svl::SharedString(*r.mpString);
        break;
        case CELLTYPE_EDIT:
            mpEditText1 = r.mpEditText1->Clone().release();
        break;
        case CELLTYPE_FORMULA:
            mpFormula1 = r.mpFormula1->Clone();
        break;
        default:
            ;
    }
}

ScCellValue::ScCellValue(ScCellValue&& r) noexcept
    : meType(r.meType)
    , mfValue1(r.mfValue1)
{
    switch (r.meType)
    {
        case CELLTYPE_STRING:
            mpString = r.mpString;
        break;
        case CELLTYPE_EDIT:
            mpEditText1 = r.mpEditText1;
        break;
        case CELLTYPE_FORMULA:
            mpFormula1 = r.mpFormula1;
        break;
        default:
            ;
    }
    r.meType = CELLTYPE_NONE;
}

ScCellValue::~ScCellValue()
{
    clear();
}

void ScCellValue::clear() noexcept
{
    switch (meType)
    {
        case CELLTYPE_STRING:
            delete mpString;
        break;
        case CELLTYPE_EDIT:
            delete mpEditText1;
        break;
        case CELLTYPE_FORMULA:
            delete mpFormula1;
        break;
        default:
            ;
    }

    // Reset to empty value.
    meType = CELLTYPE_NONE;
    mfValue1 = 0.0;
}

void ScCellValue::set( double fValue )
{
    clear();
    meType = CELLTYPE_VALUE;
    mfValue1 = fValue;
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
    mpEditText1 = rEditText.Clone().release();
}

void ScCellValue::set( std::unique_ptr<EditTextObject> xEditText )
{
    clear();
    meType = CELLTYPE_EDIT;
    mpEditText1 = xEditText.release();
}

void ScCellValue::set( ScFormulaCell* pFormula )
{
    clear();
    meType = CELLTYPE_FORMULA;
    mpFormula1 = pFormula;
}

void ScCellValue::assign( const ScDocument& rDoc, const ScAddress& rPos )
{
    clear();

    ScRefCellValue aRefVal(const_cast<ScDocument&>(rDoc), rPos);

    meType = aRefVal.getType();
    switch (meType)
    {
        case CELLTYPE_STRING:
            mpString = new svl::SharedString(*aRefVal.mpString);
        break;
        case CELLTYPE_EDIT:
            if (aRefVal.mpEditText1)
                mpEditText1 = aRefVal.mpEditText1->Clone().release();
        break;
        case CELLTYPE_VALUE:
            mfValue1 = aRefVal.mfValue1;
        break;
        case CELLTYPE_FORMULA:
            mpFormula1 = aRefVal.mpFormula1->Clone();
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
            if (rOther.mpEditText1->HasOnlineSpellErrors())
            {
                EEControlBits nControl = rEngine.GetControlWord();
                const EEControlBits nSpellControl = EEControlBits::ONLINESPELLING | EEControlBits::ALLOWBIGOBJS;
                bool bNewControl = ((nControl & nSpellControl) != nSpellControl);
                if (bNewControl)
                    rEngine.SetControlWord(nControl | nSpellControl);
                rEngine.SetTextCurrentDefaults(*rOther.mpEditText1);
                mpEditText1 = rEngine.CreateTextObject().release();
                if (bNewControl)
                    rEngine.SetControlWord(nControl);
            }
            else
            {
                rEngine.SetTextCurrentDefaults(*rOther.mpEditText1);
                mpEditText1 = rEngine.CreateTextObject().release();
            }
        }
        break;
        case CELLTYPE_VALUE:
            mfValue1 = rOther.mfValue1;
        break;
        case CELLTYPE_FORMULA:
            // Switch to the destination document.
            mpFormula1 = new ScFormulaCell(*rOther.mpFormula1, rDestDoc, rOther.mpFormula1->aPos, nCloneFlags);
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
            rDoc.SetEditText(rPos, mpEditText1->Clone());
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(rPos, mfValue1);
        break;
        case CELLTYPE_FORMULA:
            rDoc.SetFormulaCell(rPos, mpFormula1->Clone());
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
            rDoc.SetEditText(rPos, std::unique_ptr<EditTextObject>(mpEditText1));
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(rPos, mfValue1);
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            rDoc.SetFormulaCell(rPos, mpFormula1);
        break;
        default:
            rDoc.SetEmptyCell(rPos);
    }

    meType = CELLTYPE_NONE;
    mfValue1 = 0.0;
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
            rColumn.SetEditText(nRow, std::unique_ptr<EditTextObject>(mpEditText1));
        break;
        case CELLTYPE_VALUE:
            rColumn.SetValue(nRow, mfValue1);
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            rColumn.SetFormulaCell(nRow, mpFormula1, eListenType);
        break;
        default:
            rColumn.DeleteContent(nRow);
    }

    meType = CELLTYPE_NONE;
    mfValue1 = 0.0;
}

OUString ScCellValue::getString( const ScDocument& rDoc ) const
{
    return getStringImpl(*this, &rDoc);
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

ScCellValue& ScCellValue::operator=(ScCellValue&& rCell) noexcept
{
    clear();

    meType = rCell.meType;
    mfValue1 = rCell.mfValue1;
    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
            mpString = rCell.mpString;
        break;
        case CELLTYPE_EDIT:
            mpEditText1 = rCell.mpEditText1;
        break;
        case CELLTYPE_FORMULA:
            mpFormula1 = rCell.mpFormula1;
        break;
        default:
            ;
    }
    //we don't need to reset mpString/mpEditText1/mpFormula1 if we
    //set meType to NONE as the ScCellValue dtor keys off the meType
    rCell.meType = CELLTYPE_NONE;

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
    std::swap(mfValue1, r.mfValue1);
}

ScRefCellValue::ScRefCellValue() : meType(CELLTYPE_NONE), mfValue1(0.0) {}
ScRefCellValue::ScRefCellValue( double fValue ) : meType(CELLTYPE_VALUE), mfValue1(fValue) {}
ScRefCellValue::ScRefCellValue( const svl::SharedString* pString ) : meType(CELLTYPE_STRING), mpString(pString) {}
ScRefCellValue::ScRefCellValue( const EditTextObject* pEditText ) : meType(CELLTYPE_EDIT), mpEditText1(pEditText) {}
ScRefCellValue::ScRefCellValue( ScFormulaCell* pFormula ) : meType(CELLTYPE_FORMULA), mpFormula1(pFormula) {}

ScRefCellValue::ScRefCellValue( ScDocument& rDoc, const ScAddress& rPos )
{
    assign( rDoc, rPos);
}

ScRefCellValue::ScRefCellValue( ScDocument& rDoc, const ScAddress& rPos, sc::ColumnBlockPosition& rBlockPos )
{
    assign( rDoc, rPos, rBlockPos );
}

void ScRefCellValue::clear()
{
    // Reset to empty value.
    meType = CELLTYPE_NONE;
    mfValue1 = 0.0;
}

void ScRefCellValue::assign( ScDocument& rDoc, const ScAddress& rPos )
{
    *this = rDoc.GetRefCellValue(rPos);
}

void ScRefCellValue::assign( ScDocument& rDoc, const ScAddress& rPos, sc::ColumnBlockPosition& rBlockPos )
{
    *this = rDoc.GetRefCellValue(rPos, rBlockPos);
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
            rDoc.SetEditText(rPos, ScEditUtil::Clone(*mpEditText1, rDoc));
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(rPos, mfValue1);
        break;
        case CELLTYPE_FORMULA:
            rDoc.SetFormulaCell(rPos, new ScFormulaCell(*mpFormula1, rDoc, rPos));
        break;
        default:
            rDoc.SetEmptyCell(rPos);
    }
}

bool ScRefCellValue::hasString() const
{
    return hasStringImpl(meType, mpFormula1);
}

bool ScRefCellValue::hasNumeric() const
{
    return hasNumericImpl(meType, mpFormula1);
}

bool ScRefCellValue::hasError() const
{
    return meType == CELLTYPE_FORMULA && mpFormula1->GetErrCode() != FormulaError::NONE;
}

double ScRefCellValue::getValue()
{
    switch (meType)
    {
        case CELLTYPE_VALUE:
            return mfValue1;
        case CELLTYPE_FORMULA:
            return mpFormula1->GetValue();
        default:
            ;
    }
    return 0.0;
}

double ScRefCellValue::getRawValue() const
{
    switch (meType)
    {
        case CELLTYPE_VALUE:
            return mfValue1;
        case CELLTYPE_FORMULA:
            return mpFormula1->GetRawValue();
        default:
            ;
    }
    return 0.0;
}

OUString ScRefCellValue::getString( const ScDocument* pDoc ) const
{
    return getStringImpl(*this, pDoc);
}

OUString ScRefCellValue::getRawString( const ScDocument& rDoc ) const
{
    return getRawStringImpl(*this, rDoc);
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
        return mpFormula1->IsEmpty();

    return false;
}

bool ScRefCellValue::equalsWithoutFormat( const ScRefCellValue& r ) const
{
    return equalsWithoutFormatImpl(*this, r);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
