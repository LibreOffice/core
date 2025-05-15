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
#include <svl/sharedstringpool.hxx>

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
        return rVal.getSharedString()->getString();

    if (rVal.getType() == CELLTYPE_EDIT)
    {
        OUStringBuffer aRet;
        sal_Int32 n = rVal.getEditText()->GetParagraphCount();
        for (sal_Int32 i = 0; i < n; ++i)
        {
            if (i > 0)
                aRet.append('\n');
            aRet.append(rVal.getEditText()->GetText(i));
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
            return equalsFormulaCells(left.getFormula(), right.getFormula());
        default:
            ;
    }
    return false;
}

bool equalsWithoutFormatImpl( const ScCellValue& left, const ScCellValue& right )
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
            return equalsFormulaCells(left.getFormula(), right.getFormula());
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
            rColumn.SetRawString(nRow, *rCell.getSharedString());
        break;
        case CELLTYPE_EDIT:
            rColumn.SetEditText(nRow, ScEditUtil::Clone(*rCell.getEditText(), rColumn.GetDoc()));
        break;
        case CELLTYPE_VALUE:
            rColumn.SetValue(nRow, rCell.getDouble());
        break;
        case CELLTYPE_FORMULA:
        {
            ScAddress aDestPos(rColumn.GetCol(), nRow, rColumn.GetTab());
            rColumn.SetFormulaCell(nRow, new ScFormulaCell(*rCell.getFormula(), rColumn.GetDoc(), aDestPos));
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

template <typename T>
OUString getStringImpl( const T& rCell, const ScDocument& rDoc )
{
    switch (rCell.getType())
    {
        case CELLTYPE_VALUE:
            return OUString::number(rCell.getDouble());
        case CELLTYPE_STRING:
            return rCell.getSharedString()->getString();
        case CELLTYPE_EDIT:
            if (rCell.getEditText())
                return ScEditUtil::GetString(*rCell.getEditText(), rDoc);
        break;
        case CELLTYPE_FORMULA:
            return rCell.getFormula()->GetString().getString();
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
            return OUString::number(rCell.getDouble());
        case CELLTYPE_STRING:
            return rCell.getSharedString()->getString();
        case CELLTYPE_EDIT:
            if (rCell.getEditText())
                return ScEditUtil::GetString(*rCell.getEditText(), rDoc);
        break;
        case CELLTYPE_FORMULA:
            return rCell.getFormula()->GetRawString().getString();
        default:
            ;
    }
    return OUString();
}

}

ScCellValue::ScCellValue() {}

ScCellValue::ScCellValue( const ScRefCellValue& rCell )
{
    switch (rCell.getType())
    {
        case CELLTYPE_STRING:
            maData = *rCell.getSharedString();
        break;
        case CELLTYPE_EDIT:
            maData = rCell.getEditText()->Clone().release();
        break;
        case CELLTYPE_FORMULA:
            maData = rCell.getFormula()->Clone();
        break;
        case CELLTYPE_VALUE:
            maData = rCell.getDouble();
        break;
        default: ;
    }
}

ScCellValue::ScCellValue( double fValue ) : maData(fValue) {}

ScCellValue::ScCellValue( const svl::SharedString& rString ) : maData(rString) {}

ScCellValue::ScCellValue( std::unique_ptr<EditTextObject> xEdit ) : maData(xEdit.release()) {}

ScCellValue::ScCellValue( const ScCellValue& r )
{
    switch (r.getType())
    {
        case CELLTYPE_STRING:
            maData = *r.getSharedString();
        break;
        case CELLTYPE_EDIT:
            maData = r.getEditText()->Clone().release();
        break;
        case CELLTYPE_FORMULA:
            maData = r.getFormula()->Clone();
        break;
        case CELLTYPE_VALUE:
            maData = r.getDouble();
        break;
        default: ;
    }
}

void ScCellValue::reset_to_empty()
{
    suppress_fun_call_w_exception(maData = std::monostate()); // reset to empty;
}

ScCellValue::ScCellValue(ScCellValue&& r) noexcept
    : maData(std::move(r.maData))
{
    r.reset_to_empty();
}

ScCellValue::~ScCellValue()
{
    clear();
}

CellType ScCellValue::getType() const
{
    switch (maData.index())
    {
        case 0: return CELLTYPE_NONE;
        case 1: return CELLTYPE_VALUE;
        case 2: return CELLTYPE_STRING;
        case 3: return CELLTYPE_EDIT;
        case 4: return CELLTYPE_FORMULA;
        default:
            assert(false);
            return CELLTYPE_NONE;
    }
}

void ScCellValue::clear() noexcept
{
    switch (getType())
    {
        case CELLTYPE_EDIT:
            suppress_fun_call_w_exception(delete getEditText());
        break;
        case CELLTYPE_FORMULA:
            suppress_fun_call_w_exception(delete getFormula());
        break;
        default:
            ;
    }

    // Reset to empty value.
    reset_to_empty();
}

void ScCellValue::set( double fValue )
{
    clear();
    maData = fValue;
}

void ScCellValue::set( const svl::SharedString& rStr )
{
    clear();
    maData = rStr;
}

void ScCellValue::set( const EditTextObject& rEditText )
{
    clear();
    maData = rEditText.Clone().release();
}

void ScCellValue::set( std::unique_ptr<EditTextObject> xEditText )
{
    clear();
    maData = xEditText.release();
}

void ScCellValue::set( ScFormulaCell* pFormula )
{
    clear();
    maData = pFormula;
}

void ScCellValue::assign( const ScDocument& rDoc, const ScAddress& rPos )
{
    clear();

    ScRefCellValue aRefVal(const_cast<ScDocument&>(rDoc), rPos);

    switch (aRefVal.getType())
    {
        case CELLTYPE_STRING:
            maData = *aRefVal.getSharedString();
        break;
        case CELLTYPE_EDIT:
            maData = aRefVal.getEditText() ? aRefVal.getEditText()->Clone().release() : static_cast<EditTextObject*>(nullptr);
        break;
        case CELLTYPE_VALUE:
            maData = aRefVal.getDouble();
        break;
        case CELLTYPE_FORMULA:
            maData = aRefVal.getFormula()->Clone();
        break;
        default: ; // leave empty
    }
}

void ScCellValue::assign(const ScCellValue& rOther, ScDocument& rDestDoc, ScCloneFlags nCloneFlags)
{
    clear();

    switch (rOther.getType())
    {
        case CELLTYPE_STRING:
            maData = rOther.maData;
        break;
        case CELLTYPE_EDIT:
        {
            // Switch to the pool of the destination document.
            ScFieldEditEngine& rEngine = rDestDoc.GetEditEngine();
            if (rOther.getEditText()->HasOnlineSpellErrors())
            {
                EEControlBits nControl = rEngine.GetControlWord();
                const EEControlBits nSpellControl = EEControlBits::ONLINESPELLING | EEControlBits::ALLOWBIGOBJS;
                bool bNewControl = ((nControl & nSpellControl) != nSpellControl);
                if (bNewControl)
                    rEngine.SetControlWord(nControl | nSpellControl);
                rEngine.SetTextCurrentDefaults(*rOther.getEditText());
                maData = rEngine.CreateTextObject().release();
                if (bNewControl)
                    rEngine.SetControlWord(nControl);
            }
            else
            {
                rEngine.SetTextCurrentDefaults(*rOther.getEditText());
                maData = rEngine.CreateTextObject().release();
            }
        }
        break;
        case CELLTYPE_VALUE:
            maData = rOther.maData;
        break;
        case CELLTYPE_FORMULA:
            // Switch to the destination document.
            maData = new ScFormulaCell(*rOther.getFormula(), rDestDoc, rOther.getFormula()->aPos, nCloneFlags);
        break;
        default: ; // leave empty
    }
}

void ScCellValue::commit( ScDocument& rDoc, const ScAddress& rPos ) const
{
    switch (getType())
    {
        case CELLTYPE_STRING:
        {
            ScSetStringParam aParam;
            aParam.setTextInput();
            rDoc.SetString(rPos, getSharedString()->getString(), &aParam);
        }
        break;
        case CELLTYPE_EDIT:
            rDoc.SetEditText(rPos, getEditText()->Clone());
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(rPos, getDouble());
        break;
        case CELLTYPE_FORMULA:
            rDoc.SetFormulaCell(rPos, getFormula()->Clone());
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
    switch (getType())
    {
        case CELLTYPE_STRING:
        {
            // Currently, string cannot be placed without copying.
            ScSetStringParam aParam;
            aParam.setTextInput();
            rDoc.SetString(rPos, getSharedString()->getString(), &aParam);
        }
        break;
        case CELLTYPE_EDIT:
            // Cell takes the ownership of the text object.
            rDoc.SetEditText(rPos, std::unique_ptr<EditTextObject>(getEditText()));
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(rPos, getDouble());
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            rDoc.SetFormulaCell(rPos, getFormula());
        break;
        default:
            rDoc.SetEmptyCell(rPos);
    }

    reset_to_empty(); // reset to empty
}

void ScCellValue::release( ScColumn& rColumn, SCROW nRow, sc::StartListeningType eListenType )
{
    switch (getType())
    {
        case CELLTYPE_STRING:
        {
            // Currently, string cannot be placed without copying.
            rColumn.SetRawString(nRow, *getSharedString());
        }
        break;
        case CELLTYPE_EDIT:
            // Cell takes the ownership of the text object.
            rColumn.SetEditText(nRow, std::unique_ptr<EditTextObject>(getEditText()));
        break;
        case CELLTYPE_VALUE:
            rColumn.SetValue(nRow, getDouble());
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            rColumn.SetFormulaCell(nRow, getFormula(), eListenType);
        break;
        default:
            rColumn.DeleteContent(nRow);
    }

    reset_to_empty(); // reset to empty
}

OUString ScCellValue::getString( const ScDocument& rDoc ) const
{
    return getStringImpl(*this, rDoc);
}

bool ScCellValue::isEmpty() const
{
    return getType() == CELLTYPE_NONE;
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
    maData = std::move(rCell.maData);
    rCell.reset_to_empty(); // reset to empty;
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
    std::swap(maData, r.maData);
}

ScRefCellValue::ScRefCellValue() : meType(CELLTYPE_NONE), mfValue(0.0) {}
ScRefCellValue::ScRefCellValue( double fValue ) : meType(CELLTYPE_VALUE), mfValue(fValue) {}
ScRefCellValue::ScRefCellValue( const svl::SharedString* pString ) : meType(CELLTYPE_STRING), mpString(pString) {}
ScRefCellValue::ScRefCellValue( const EditTextObject* pEditText ) : meType(CELLTYPE_EDIT), mpEditText(pEditText) {}
ScRefCellValue::ScRefCellValue( ScFormulaCell* pFormula ) : meType(CELLTYPE_FORMULA), mpFormula(pFormula) {}

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
    mfValue = 0.0;
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

bool ScRefCellValue::hasError() const
{
    return meType == CELLTYPE_FORMULA && mpFormula->GetErrCode() != FormulaError::NONE;
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

double ScRefCellValue::getRawValue() const
{
    switch (meType)
    {
        case CELLTYPE_VALUE:
            return mfValue;
        case CELLTYPE_FORMULA:
            return mpFormula->GetRawValue();
        default:
            ;
    }
    return 0.0;
}

OUString ScRefCellValue::getString( const ScDocument& rDoc ) const
{
    return getStringImpl(*this, rDoc);
}

svl::SharedString ScRefCellValue::getSharedString( const ScDocument& rDoc, svl::SharedStringPool& rStrPool ) const
{
    switch (getType())
    {
        case CELLTYPE_VALUE:
            return rStrPool.intern(OUString::number(getDouble()));
        case CELLTYPE_STRING:
            return *getSharedString();
        case CELLTYPE_EDIT:
            if (auto pEditText = getEditText())
                return rStrPool.intern(ScEditUtil::GetString(*pEditText, rDoc));
            break;
        case CELLTYPE_FORMULA:
            return getFormula()->GetString();
        default:
            ;
    }
    return svl::SharedString::getEmptyString();
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
        return mpFormula->IsEmpty();

    return false;
}

bool ScRefCellValue::equalsWithoutFormat( const ScRefCellValue& r ) const
{
    return equalsWithoutFormatImpl(*this, r);
}

bool ScRefCellValue::operator==( const ScRefCellValue& r ) const
{
    if (meType != r.meType)
        return false;

    switch (meType)
    {
        case CELLTYPE_NONE:
            return true;
        case CELLTYPE_VALUE:
            return mfValue == r.mfValue;
        case CELLTYPE_STRING:
            return mpString == r.mpString;
        case CELLTYPE_FORMULA:
            return equalsFormulaCells(getFormula(), r.getFormula());
        default:
            return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
