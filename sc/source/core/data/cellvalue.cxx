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
#include "cell.hxx"
#include "editeng/editobj.hxx"
#include "editeng/editstat.hxx"
#include "stringutil.hxx"
#include "editutil.hxx"
#include "formula/token.hxx"

ScCellValue::ScCellValue() : meType(CELLTYPE_NONE), mfValue(0.0) {}
ScCellValue::ScCellValue( double fValue ) : meType(CELLTYPE_VALUE), mfValue(fValue) {}
ScCellValue::ScCellValue( const OUString& rString ) : meType(CELLTYPE_STRING), mpString(new OUString(rString)) {}
ScCellValue::ScCellValue( const EditTextObject& rEditText ) : meType(CELLTYPE_EDIT), mpEditText(rEditText.Clone()) {}
ScCellValue::ScCellValue( const ScFormulaCell& rFormula ) : meType(CELLTYPE_FORMULA), mpFormula(rFormula.Clone()) {}

ScCellValue::ScCellValue( const ScCellValue& r ) : meType(r.meType), mfValue(r.mfValue)
{
    switch (r.meType)
    {
        case CELLTYPE_STRING:
            mpString = new OUString(*r.mpString);
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
            mpFormula->Delete();
        break;
        default:
            ;
    }

    // Reset to empty value.
    meType = CELLTYPE_NONE;
    mfValue = 0.0;
}

void ScCellValue::assign( const ScDocument& rDoc, const ScAddress& rPos )
{
    clear();

    meType = rDoc.GetCellType(rPos);
    switch (meType)
    {
        case CELLTYPE_STRING:
            mpString = new OUString(rDoc.GetString(rPos));
        break;
        case CELLTYPE_EDIT:
            mpEditText = rDoc.GetEditText(rPos)->Clone();
        break;
        case CELLTYPE_VALUE:
            mfValue = rDoc.GetValue(rPos);
        break;
        case CELLTYPE_FORMULA:
            mpFormula = rDoc.GetFormulaCell(rPos)->Clone();
        break;
        default:
            meType = CELLTYPE_NONE; // reset to empty.
    }
}

void ScCellValue::assign( const ScCellValue& rOther, ScDocument& rDestDoc, int nCloneFlags )
{
    clear();

    meType = rOther.meType;
    switch (meType)
    {
        case CELLTYPE_STRING:
            mpString = new OUString(*rOther.mpString);
        break;
        case CELLTYPE_EDIT:
        {
            // Switch to the pool of the destination document.
            ScFieldEditEngine& rEngine = rDestDoc.GetEditEngine();
            if (rOther.mpEditText->HasOnlineSpellErrors())
            {
                sal_uLong nControl = rEngine.GetControlWord();
                const sal_uLong nSpellControl = EE_CNTRL_ONLINESPELLING | EE_CNTRL_ALLOWBIGOBJS;
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

void ScCellValue::assign( const ScBaseCell& rCell )
{
    clear();

    meType = rCell.GetCellType();
    switch (meType)
    {
        case CELLTYPE_STRING:
            mpString = new OUString(static_cast<const ScStringCell&>(rCell).GetString());
        break;
        case CELLTYPE_EDIT:
        {
            const EditTextObject* p = static_cast<const ScEditCell&>(rCell).GetData();
            if (p)
                mpEditText = p->Clone();
        }
        break;
        case CELLTYPE_VALUE:
            mfValue = static_cast<const ScValueCell&>(rCell).GetValue();
        break;
        case CELLTYPE_FORMULA:
            mpFormula = static_cast<const ScFormulaCell&>(rCell).Clone();
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
            rDoc.SetString(rPos, *mpString, &aParam);
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

void ScCellValue::release( ScDocument& rDoc, const ScAddress& rPos )
{
    switch (meType)
    {
        case CELLTYPE_STRING:
        {
            // Currently, string cannot be placed without copying.
            ScSetStringParam aParam;
            aParam.setTextInput();
            rDoc.SetString(rPos, *mpString, &aParam);
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

bool ScCellValue::hasString() const
{
    switch (meType)
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            return true;
        case CELLTYPE_FORMULA:
            return !mpFormula->IsValue();
        default:
            return false;
    }
}

bool ScCellValue::hasNumeric() const
{
    switch (meType)
    {
        case CELLTYPE_VALUE:
            return true;
        case CELLTYPE_FORMULA:
            return mpFormula->IsValue();
        default:
            return false;
    }
}

bool ScCellValue::isEmpty() const
{
    return meType == CELLTYPE_NOTE || meType == CELLTYPE_NONE;
}

namespace {

CellType adjustCellType( CellType eOrig )
{
    switch (eOrig)
    {
        case CELLTYPE_NOTE:
            return CELLTYPE_NONE;
        case CELLTYPE_EDIT:
            return CELLTYPE_STRING;
        default:
            ;
    }
    return eOrig;
}

OUString getString( const ScCellValue& rVal )
{
    if (rVal.meType == CELLTYPE_STRING)
        return *rVal.mpString;

    if (rVal.meType == CELLTYPE_EDIT)
    {
        OUStringBuffer aRet;
        size_t n = rVal.mpEditText->GetParagraphCount();
        for (size_t i = 0; i < n; ++i)
        {
            if (i > 0)
                aRet.append('\n');
            aRet.append(rVal.mpEditText->GetText(i));
        }
        return aRet.makeStringAndClear();
    }

    return EMPTY_OUSTRING;
}

}

bool ScCellValue::equalsWithoutFormat( const ScCellValue& r ) const
{
    CellType eType1 = adjustCellType(meType);
    CellType eType2 = adjustCellType(r.meType);
    if (eType1 != eType2)
        return false;

    switch (meType)
    {
        case CELLTYPE_NONE:
            return true;
        case CELLTYPE_VALUE:
            return mfValue == r.mfValue;
        case CELLTYPE_STRING:
        {
            OUString aStr1 = getString(*this);
            OUString aStr2 = getString(r);
            return aStr1 == aStr2;
        }
        case CELLTYPE_FORMULA:
        {
            ScTokenArray* pCode1 = mpFormula->GetCode();
            ScTokenArray* pCode2 = r.mpFormula->GetCode();

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
        default:
            ;
    }
    return false;
}

ScCellValue& ScCellValue::operator= ( const ScCellValue& r )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
