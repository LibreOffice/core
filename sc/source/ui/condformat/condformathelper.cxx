/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <rtl/ustrbuf.hxx>
#include <condformathelper.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <conditio.hxx>

namespace {

OUString getTextForType(ScCondFormatEntryType eType)
{
    switch(eType)
    {
        case CONDITION:
            return ScResId(STR_COND_CONDITION);
        case COLORSCALE:
            return ScResId(STR_COND_COLORSCALE);
        case DATABAR:
            return ScResId(STR_COND_DATABAR);
        case FORMULA:
            return ScResId(STR_COND_FORMULA);
        case ICONSET:
            return ScResId(STR_COND_ICONSET);
        case DATE:
            return ScResId(STR_COND_DATE);
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
            return "=";
        case 1:
            return "<";
        case 2:
            return ">";
        case 3:
            return "<=";
        case 4:
            return ">=";
        case 5:
            return "!=";
        case 6:
            return ScResId(STR_COND_BETWEEN);
        case 7:
            return ScResId(STR_COND_NOTBETWEEN);
        case 8:
            return ScResId(STR_COND_DUPLICATE);
        case 9:
            return ScResId(STR_COND_UNIQUE);

        case 11:
            return ScResId(STR_COND_TOP10);
        case 12:
            return ScResId(STR_COND_BOTTOM10);
        case 13:
            return ScResId(STR_COND_TOP_PERCENT);
        case 14:
            return ScResId(STR_COND_BOTTOM_PERCENT);
        case 15:
            return ScResId(STR_COND_ABOVE_AVERAGE);
        case 16:
            return ScResId(STR_COND_BELOW_AVERAGE);
        case 17:
            return ScResId(STR_COND_ABOVE_EQUAL_AVERAGE);
        case 18:
            return ScResId(STR_COND_BELOW_EQUAL_AVERAGE);
        case 19:
            return ScResId(STR_COND_ERROR);
        case 20:
            return ScResId(STR_COND_NOERROR);
        case 21:
            return ScResId(STR_COND_BEGINS_WITH);
        case 22:
            return ScResId(STR_COND_ENDS_WITH);
        case 23:
            return ScResId(STR_COND_CONTAINS);
        case 24:
            return ScResId(STR_COND_NOT_CONTAINS);

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

    if (nIndex >= 0 && o3tl::make_unsigned(nIndex) < SAL_N_ELEMENTS(aCondStrs))
        return ScResId(aCondStrs[nIndex]);
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
            case ScFormatEntry::Type::Condition:
            case ScFormatEntry::Type::ExtCondition:
                {
                    const ScConditionEntry* pEntry = static_cast<const ScConditionEntry*>(rFormat.GetEntry(0));
                    ScConditionMode eMode = pEntry->GetOperation();
                    if(eMode == ScConditionMode::Direct)
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
                        if(eMode == ScConditionMode::Between || eMode == ScConditionMode::NotBetween)
                        {
                            aBuffer.append(pEntry->GetExpression(rPos, 0));
                            aBuffer.append(" ");
                            aBuffer.append(ScResId(STR_COND_AND));
                            aBuffer.append(" ");
                            aBuffer.append(pEntry->GetExpression(rPos, 1));
                        }
                        else if(eMode <= ScConditionMode::NotEqual || eMode >= ScConditionMode::BeginsWith)
                        {
                            aBuffer.append(pEntry->GetExpression(rPos, 0));
                        }
                    }
                }

                break;
            case ScFormatEntry::Type::Databar:
                aBuffer.append(getTextForType(DATABAR));
                break;
            case ScFormatEntry::Type::Colorscale:
                aBuffer.append(getTextForType(COLORSCALE));
                break;
            case ScFormatEntry::Type::Iconset:
                aBuffer.append(getTextForType(ICONSET));
                break;
            case ScFormatEntry::Type::Date:
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
        std::u16string_view aStr1, std::u16string_view aStr2 )
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
                aBuffer.append(ScResId(STR_COND_AND));
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
