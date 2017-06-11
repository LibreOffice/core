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

OUString getTextForType(ScCondFormatEntryType eType)
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
        case DATE:
            return ScGlobal::GetRscString(STR_COND_DATE);
        default:
            break;
    }

    return OUString();
}

OUString getExpression(sal_Int32 nIndex)
{
    switch(nIndex)
    {
        case 0:
            return OUString("=");
        case 1:
            return OUString("<");
        case 2:
            return OUString(">");
        case 3:
            return OUString("<=");
        case 4:
            return OUString(">=");
        case 5:
            return OUString("!=");
        case 6:
            return ScGlobal::GetRscString(STR_COND_BETWEEN);
        case 7:
            return ScGlobal::GetRscString(STR_COND_NOTBETWEEN);
        case 8:
            return ScGlobal::GetRscString(STR_COND_DUPLICATE);
        case 9:
            return ScGlobal::GetRscString(STR_COND_UNIQUE);

        case 11:
            return ScGlobal::GetRscString(STR_COND_TOP10);
        case 12:
            return ScGlobal::GetRscString(STR_COND_BOTTOM10);
        case 13:
            return ScGlobal::GetRscString(STR_COND_TOP_PERCENT);
        case 14:
            return ScGlobal::GetRscString(STR_COND_BOTTOM_PERCENT);
        case 15:
            return ScGlobal::GetRscString(STR_COND_ABOVE_AVERAGE);
        case 16:
            return ScGlobal::GetRscString(STR_COND_BELOW_AVERAGE);
        case 17:
            return ScGlobal::GetRscString(STR_COND_ABOVE_EQUAL_AVERAGE);
        case 18:
            return ScGlobal::GetRscString(STR_COND_BELOW_EQUAL_AVERAGE);
        case 19:
            return ScGlobal::GetRscString(STR_COND_ERROR);
        case 20:
            return ScGlobal::GetRscString(STR_COND_NOERROR);
        case 21:
            return ScGlobal::GetRscString(STR_COND_BEGINS_WITH);
        case 22:
            return ScGlobal::GetRscString(STR_COND_ENDS_WITH);
        case 23:
            return ScGlobal::GetRscString(STR_COND_CONTAINS);
        case 24:
            return ScGlobal::GetRscString(STR_COND_NOT_CONTAINS);

        case 10:
            assert(false);
    }
    return OUString();
}

OUString getDateString(sal_Int32 nIndex)
{
    const char* aCondStrs[] =
    {
        STR_COND_TODAY,
        STR_COND_YESTERDAY,
        STR_COND_TOMORROW,
        STR_COND_LAST7DAYS,
        STR_COND_THISWEEK,
        STR_COND_LASTWEEK,
        STR_COND_NEXTWEEK,
        STR_COND_THISMONTH,
        STR_COND_LASTMONTH,
        STR_COND_NEXTMONTH,
        STR_COND_THISYEAR,
        STR_COND_LASTYEAR,
        STR_COND_NEXTYEAR
    };

    if (nIndex >= 0 && static_cast<sal_uInt32>(nIndex) < SAL_N_ELEMENTS(aCondStrs))
        return ScGlobal::GetRscString(aCondStrs[nIndex]);
    assert(false);
    return OUString();
}

}

OUString ScCondFormatHelper::GetExpression(const ScConditionalFormat& rFormat, const ScAddress& rPos)
{
    OUStringBuffer aBuffer;
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
                        aBuffer.append(" ");
                        aBuffer.append(getExpression(static_cast<sal_Int32>(eMode)));
                        aBuffer.append(" ");
                        if(eMode == SC_COND_BETWEEN || eMode == SC_COND_NOTBETWEEN)
                        {
                            aBuffer.append(pEntry->GetExpression(rPos, 0));
                            aBuffer.append(" ");
                            aBuffer.append(ScGlobal::GetRscString(STR_COND_AND));
                            aBuffer.append(" ");
                            aBuffer.append(pEntry->GetExpression(rPos, 1));
                        }
                        else if(eMode <= SC_COND_NOTEQUAL || eMode >= SC_COND_BEGINS_WITH)
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
            case condformat::DATE:
                {
                    aBuffer.append(getTextForType(DATE));
                    aBuffer.append(" ");
                    sal_Int32 nDateEntry = static_cast<sal_Int32>(static_cast<const ScCondDateFormatEntry*>(rFormat.GetEntry(0))->GetDateType());
                    aBuffer.append(getDateString(nDateEntry));
                }
                break;
        }
    }
    return aBuffer.makeStringAndClear();
}

OUString ScCondFormatHelper::GetExpression( ScCondFormatEntryType eType, sal_Int32 nIndex,
        const OUString& aStr1, const OUString& aStr2 )
{
    OUStringBuffer aBuffer(getTextForType(eType));
    aBuffer.append(" ");
    if(eType == CONDITION)
    {
        // workaround missing FORMULA option in the conditions case
        // FORMULA is handled later
        if(nIndex > 9)
            ++nIndex;
        aBuffer.append(getExpression(nIndex));
        if(nIndex <= 7 || nIndex >= 19)
        {
            aBuffer.append(" ").append(aStr1);
            if(nIndex == 6 || nIndex == 7)
            {
                aBuffer.append(" ");
                aBuffer.append(ScGlobal::GetRscString(STR_COND_AND));
                aBuffer.append(" ");
                aBuffer.append(aStr2);
            }
        }
    }
    else if(eType == FORMULA)
    {
        aBuffer.append(" ").append(aStr1);
    }
    else if(eType == DATE)
    {
        aBuffer.append(getDateString(nIndex));
    }

    return aBuffer.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
