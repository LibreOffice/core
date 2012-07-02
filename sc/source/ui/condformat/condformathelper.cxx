/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustrbuf.hxx>
#include "condformathelper.hxx"

namespace {

rtl::OUString getTextForType(ScCondFormatEntryType eType)
{
    switch(eType)
    {
        case CONDITION:
            return rtl::OUString("Cell value");
        case COLORSCALE:
            return rtl::OUString("Color scale");
        case DATABAR:
            return rtl::OUString("Data Bar");
        case FORMULA:
            return rtl::OUString("Formula is");
        default:
            break;
    }

    return rtl::OUString("");
}

rtl::OUString getExpression(sal_Int32 nIndex)
{
    switch(nIndex)
    {
        case 0:
            return rtl::OUString("=");
        case 1:
            return rtl::OUString("<");
        case 2:
            return rtl::OUString(">");
        case 3:
            return rtl::OUString("<=");
        case 4:
            return rtl::OUString(">=");
        case 5:
            return rtl::OUString("!=");
        case 6:
            return rtl::OUString("between");
        case 7:
            return rtl::OUString("not between");
        case 8:
            return rtl::OUString("duplicate");
        case 9:
            return rtl::OUString("unique");
    }
    return rtl::OUString();
}

}

rtl::OUString ScCondFormatHelper::GetExpression(const ScConditionalFormat& rFormat, const ScAddress& rPos)
{
    rtl::OUStringBuffer aBuffer;
    if(!rFormat.IsEmpty())
    {
        switch(rFormat.GetEntry(0)->GetType())
        {
            case condformat::CONDITION:
                {
                    const ScConditionEntry* pEntry = static_cast<const ScConditionEntry*>(rFormat.GetEntry(0));
                    ScConditionMode eMode = pEntry->GetOperation();
                    if(eMode == SC_COND_DIRECT)
                    {
                        aBuffer.append(getTextForType(FORMULA));
                    }
                    else
                    {
                        aBuffer.append(getTextForType(CONDITION));
                        aBuffer.append(rtl::OUString(" "));
                        aBuffer.append(getExpression(static_cast<sal_Int32>(eMode)));
                        aBuffer.append(rtl::OUString(" "));
                        if(eMode == SC_COND_BETWEEN || eMode == SC_COND_NOTBETWEEN)
                        {
                            aBuffer.append(pEntry->GetExpression(rPos, 0));
                            aBuffer.append(rtl::OUString(" and "));
                            aBuffer.append(pEntry->GetExpression(rPos, 1));
                        }
                        else
                        {
                            aBuffer.append(pEntry->GetExpression(rPos, 0));
                        }
                    }
                }

                break;
            case condformat::DATABAR:
                aBuffer.append(getTextForType(DATABAR));
                break;
            case condformat::COLORSCALE:
                aBuffer.append(getTextForType(COLORSCALE));
                break;
        }
    }
    return aBuffer.makeStringAndClear();
}

rtl::OUString ScCondFormatHelper::GetExpression( ScCondFormatEntryType eType, sal_Int32 nIndex )
{
    rtl::OUStringBuffer aBuffer(getTextForType(eType));
    aBuffer.append(rtl::OUString(" "));
    if(eType == CONDITION)
        aBuffer.append(getExpression(nIndex));

    return aBuffer.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
