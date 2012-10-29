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
#include "globstr.hrc"

namespace {

rtl::OUString getTextForType(ScCondFormatEntryType eType)
{
    switch(eType)
    {
        case CONDITION:
            return ScGlobal::GetRscString(STR_COND_CONDITION);
        case COLORSCALE:
            return ScGlobal::GetRscString(STR_COND_COLORSCALE);
        case DATABAR:
            return ScGlobal::GetRscString(STR_COND_DATABAR);
        case FORMULA:
            return ScGlobal::GetRscString(STR_COND_FORMULA);
        case ICONSET:
            return ScGlobal::GetRscString(STR_COND_ICONSET);
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
            return ScGlobal::GetRscString(STR_COND_BETWEEN);
        case 7:
            return ScGlobal::GetRscString(STR_COND_NOTBETWEEN);
        case 8:
            return ScGlobal::GetRscString(STR_COND_DUPLICATE);
        case 9:
            return ScGlobal::GetRscString(STR_COND_UNIQUE);
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
                        aBuffer.append(" ");
                        aBuffer.append(pEntry->GetExpression(rPos, 0));
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
                        else if(eMode <= SC_COND_NOTEQUAL)
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
            case condformat::ICONSET:
                aBuffer.append(getTextForType(ICONSET));
                break;
        }
    }
    return aBuffer.makeStringAndClear();
}

rtl::OUString ScCondFormatHelper::GetExpression( ScCondFormatEntryType eType, sal_Int32 nIndex,
        rtl::OUString aStr1, rtl::OUString aStr2 )
{
    rtl::OUStringBuffer aBuffer(getTextForType(eType));
    aBuffer.append(rtl::OUString(" "));
    if(eType == CONDITION)
    {
        aBuffer.append(getExpression(nIndex));
        if(nIndex <= 7)
        {
            aBuffer.append(" ").append(aStr1);
            if(nIndex == 6 || nIndex == 7)
            {
                aBuffer.append(" and ").append(aStr2);
            }
        }
    }
    else if(eType == FORMULA)
    {
        aBuffer.append(" ").append(aStr1);
    }

    return aBuffer.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
