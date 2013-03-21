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
#include "stringutil.hxx"

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

void ScCellValue::commit( ScDocument& rDoc, const ScAddress& rPos )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
