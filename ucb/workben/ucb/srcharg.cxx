/*************************************************************************
 *
 *  $RCSfile: srcharg.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:56:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <limits>

#ifndef _COM_SUN_STAR_UCB_RULEOPERATOR_HPP_
#include <com/sun/star/ucb/RuleOperator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_SEARCHINFO_HPP_
#include <com/sun/star/ucb/SearchInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef TOOLS_INETMIME_HXX
#include <tools/inetmime.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef CHAOS_UCBDEMO_SRCHARG_HXX
#include <srcharg.hxx>
#endif

#undef max // declared as a macro in <tools/solar.h>...
#undef min // declared as a macro in <tools/solar.h>...

namespace unnamed_chaos_ucbdemo_srcharg {}
using namespace unnamed_chaos_ucbdemo_srcharg;
    // unnamed namespaces don't work well yet...

using namespace com::sun::star;

//============================================================================
//
//  skipWhiteSpace
//
//============================================================================

namespace unnamed_chaos_ucbdemo_srcharg {

void skipWhiteSpace(sal_Unicode const *& rBegin, sal_Unicode const * pEnd)
{
    while (rBegin != pEnd
           && (*rBegin == '\n' || *rBegin == '\t' || *rBegin == ' '))
        ++rBegin;
}

//============================================================================
//
//  scanAtom
//
//============================================================================

String scanAtom(sal_Unicode const *& rBegin, sal_Unicode const * pEnd)
{
    sal_Unicode const * pTheBegin = rBegin;
    while (rBegin != pEnd && INetMIME::isAlpha(*rBegin))
        ++rBegin;
    return String(pTheBegin, rBegin - pTheBegin);
}

//============================================================================
//
//  scanProperty
//
//============================================================================

String scanProperty(sal_Unicode const *& rBegin, sal_Unicode const * pEnd)
{
    sal_Unicode const * pTheBegin = rBegin;
    while (rBegin != pEnd
           && !(*rBegin == '\n' || *rBegin == '\t' || *rBegin == ' '))
        ++rBegin;
    return String(pTheBegin, rBegin - pTheBegin);
}

//============================================================================
//
//  scanOperator
//
//============================================================================

String scanOperator(sal_Unicode const *& rBegin, sal_Unicode const * pEnd)
{
    sal_Unicode const * pTheBegin = rBegin;
    while (rBegin != pEnd
           && (INetMIME::isAlpha(*rBegin) || *rBegin == '!'
               || *rBegin >= '<' && *rBegin <= '>'))
        ++rBegin;
    return String(pTheBegin, rBegin - pTheBegin);
}

}

//============================================================================
//
//  parseSearchArgument
//
//============================================================================

bool parseSearchArgument(String const & rInput, ucb::SearchInfo & rInfo)
{
    /* Format of rInput:

       argument = *option [criterium *("OR" criterium)]

       option = ("--RECURSE" "=" ("NONE" / "ONE" / "DEEP"))
                    / (("--BASE" / "--FOLDERVIEW" / "--DOCVIEW"
                                / "--INDIRECT")
                           "=" bool)

       criterium = "EMPTY" / (term *("AND" term))

       term = text-term / date-term / numeric-term / bool-term

       text-term = property ("CONT" / "!CONT" / ">=" / "<=" / "==" / "!=")
                       string *("-C" / "-R")

       date-term = property
                       (((">=" / "<=" / "==" / "!=") date)
                            / (("OLDER" / "YOUNGER") number))

       numeric-term = property (">=" / "<=" / "==" / "!=") number

       bool-term = property ("TRUE" / "FALSE")

       property = 1*VCHAR

       string = DQUOTE
                    *(<any Unicode code point except DQUOTE or "\">
                          / ("\" %x75 4HEXDIG)  ; \uHHHH
                          / ("\" (DQUOTE / "\")))
                    DQUOTE

       date = 1*2DIGIT "/" 1*2DIGIT "/" 4DIGIT  ; mm/dd/yyyy

       number = ["+" / "-"] 1*DIGIT
    */

    sal_Unicode const * p = rInput.GetBuffer();
    sal_Unicode const * pEnd = p + rInput.Len();

    // Parse options:
    rInfo.Recursion = ucb::SearchRecursion_ONE_LEVEL;
    rInfo.IncludeBase = true;
    rInfo.RespectFolderViewRestrictions = true;
    rInfo.RespectDocViewRestrictions = false;
    rInfo.FollowIndirections = false;
    enum OptionID { OPT_RECURSE, OPT_BASE, OPT_FOLDERVIEW, OPT_DOCVIEW,
                    OPT_INDIRECT, OPT_Count };
    struct OptionInfo
    {
        bool m_bSpecified;
        sal_Bool * m_pValue;
    };
    OptionInfo aOptions[OPT_Count];
    aOptions[OPT_RECURSE].m_bSpecified = false;
    aOptions[OPT_RECURSE].m_pValue = 0;
    aOptions[OPT_BASE].m_bSpecified = false;
    aOptions[OPT_BASE].m_pValue = &rInfo.IncludeBase;
    aOptions[OPT_FOLDERVIEW].m_bSpecified = false;
    aOptions[OPT_FOLDERVIEW].m_pValue
        = &rInfo.RespectFolderViewRestrictions;
    aOptions[OPT_DOCVIEW].m_bSpecified = false;
    aOptions[OPT_DOCVIEW].m_pValue = &rInfo.RespectDocViewRestrictions;
    aOptions[OPT_INDIRECT].m_bSpecified = false;
    aOptions[OPT_INDIRECT].m_pValue = &rInfo.FollowIndirections;
    while (p != pEnd)
    {
        sal_Unicode const * q = p;

        skipWhiteSpace(q, pEnd);
        if (pEnd - q < 2 || *q++ != '-' || *q++ != '-')
            break;
        String aOption(scanAtom(q, pEnd));
        OptionID eID;
        if (aOption.EqualsIgnoreCaseAscii("recurse"))
            eID = OPT_RECURSE;
        else if (aOption.EqualsIgnoreCaseAscii("base"))
            eID = OPT_BASE;
        else if (aOption.EqualsIgnoreCaseAscii("folderview"))
            eID = OPT_FOLDERVIEW;
        else if (aOption.EqualsIgnoreCaseAscii("docview"))
            eID = OPT_DOCVIEW;
        else if (aOption.EqualsIgnoreCaseAscii("indirect"))
            eID = OPT_INDIRECT;
        else
            break;

        if (aOptions[eID].m_bSpecified)
            break;
        aOptions[eID].m_bSpecified = true;

        skipWhiteSpace(q, pEnd);
        if (q == pEnd || *q++ != '=')
            break;

        skipWhiteSpace(q, pEnd);
        String aValue(scanAtom(q, pEnd));
        if (eID == OPT_RECURSE)
        {
            if (aValue.EqualsIgnoreCaseAscii("none"))
                rInfo.Recursion = ucb::SearchRecursion_NONE;
            else if (aValue.EqualsIgnoreCaseAscii("one"))
                rInfo.Recursion = ucb::SearchRecursion_ONE_LEVEL;
            else if (aValue.EqualsIgnoreCaseAscii("deep"))
                rInfo.Recursion = ucb::SearchRecursion_DEEP;
            else
                break;
        }
        else if (aValue.EqualsIgnoreCaseAscii("true"))
            *aOptions[eID].m_pValue = true;
        else if (aValue.EqualsIgnoreCaseAscii("false"))
            *aOptions[eID].m_pValue = false;
        else
            break;

        p = q;
    }

    // Parse criteria:
    ucb::SearchCriterium aCriterium;
    for (;;)
    {
        sal_Unicode const * q = p;

        // Parse either property name or "empty":
        skipWhiteSpace(q, pEnd);
        String aProperty(scanProperty(q, pEnd));
        sal_Unicode const * pPropertyEnd = q;

        // Parse operator:
        skipWhiteSpace(q, pEnd);
        String aOperator(scanOperator(q, pEnd));
        struct Operator
        {
            sal_Char const * m_pName;
            sal_Int16 m_nText;
            sal_Int16 m_nDate;
            sal_Int16 m_nNumeric;
            sal_Int16 m_nBool;
        };
        static Operator const aOperators[]
            = { { "cont", ucb::RuleOperator::CONTAINS, 0, 0, 0 },
                { "!cont", ucb::RuleOperator::CONTAINSNOT, 0, 0, 0 },
                { ">=", ucb::RuleOperator::GREATEREQUAL,
                  ucb::RuleOperator::GREATEREQUAL,
                  ucb::RuleOperator::GREATEREQUAL, 0 },
                { "<=", ucb::RuleOperator::LESSEQUAL,
                  ucb::RuleOperator::LESSEQUAL, ucb::RuleOperator::LESSEQUAL,
                  0 },
                { "==", ucb::RuleOperator::EQUAL, ucb::RuleOperator::EQUAL,
                  ucb::RuleOperator::EQUAL, 0 },
                { "!=", ucb::RuleOperator::NOTEQUAL,
                  ucb::RuleOperator::NOTEQUAL, ucb::RuleOperator::NOTEQUAL,
                  0 },
                { "true", 0, 0, 0, ucb::RuleOperator::VALUE_TRUE },
                { "false", 0, 0, 0, ucb::RuleOperator::VALUE_FALSE } };
        int const nOperatorCount = sizeof aOperators / sizeof (Operator);
        Operator const * pTheOperator = 0;
        for (int i = 0; i < nOperatorCount; ++i)
            if (aOperator.EqualsIgnoreCaseAscii(aOperators[i].m_pName))
            {
                pTheOperator = aOperators + i;
                break;
            }
        bool bTerm = pTheOperator != 0;

        sal_Int16 nOperatorID;
        uno::Any aTheOperand;
        bool bCaseSensitive = false;
        bool bRegularExpression = false;
        if (bTerm)
        {
            skipWhiteSpace(q, pEnd);
            bool bHasOperand = false;

            // Parse string operand:
            if (!bHasOperand && pTheOperator->m_nText)
            {
                if (q != pEnd && *q == '"')
                {
                    String aString;
                    for (sal_Unicode const * r = q + 1;;)
                    {
                        if (r == pEnd)
                            break;
                        sal_Unicode c = *r++;
                        if (c == '"')
                        {
                            bHasOperand = true;
                            aTheOperand <<= rtl::OUString(aString);
                            nOperatorID = pTheOperator->m_nText;
                            q = r;
                            break;
                        }
                        if (c == '\\')
                        {
                            if (r == pEnd)
                                break;
                            c = *r++;
                            if (c == 'u')
                            {
                                if (pEnd - r < 4)
                                    break;
                                c = 0;
                                bool bBad = false;
                                for (int i = 0; i < 4; ++i)
                                {
                                    int nWeight
                                        = INetMIME::getHexWeight(*r++);
                                    if (nWeight < 0)
                                    {
                                        bBad = false;
                                        break;
                                    }
                                    c = sal_Unicode(c << 4 | nWeight);
                                }
                                if (bBad)
                                    break;
                            }
                            else if (c != '"' && c != '\\')
                                break;
                        }
                        aString += c;
                    }
                }

                // Parse "-C" and "-R":
                if (bHasOperand)
                    for (;;)
                    {
                        skipWhiteSpace(q, pEnd);
                        if (pEnd - q >= 2 && q[0] == '-'
                            && (q[1] == 'C' || q[1] == 'c')
                            && !bCaseSensitive)
                        {
                            bCaseSensitive = true;
                            q += 2;
                        }
                        else if (pEnd - q >= 2 && q[0] == '-'
                                 && (q[1] == 'R' || q[1] == 'r')
                                 && !bRegularExpression)
                        {
                            bRegularExpression = true;
                            q += 2;
                        }
                        else
                            break;
                    }
            }

            // Parse date operand:
            if (!bHasOperand && pTheOperator->m_nDate != 0)
            {
                sal_Unicode const * r = q;
                bool bOK = true;
                USHORT nMonth = 0;
                if (bOK && r != pEnd && INetMIME::isDigit(*r))
                    nMonth = INetMIME::getWeight(*r++);
                else
                    bOK = false;
                if (bOK && r != pEnd && INetMIME::isDigit(*r))
                    nMonth = 10 * nMonth + INetMIME::getWeight(*r++);
                if (!(bOK && r != pEnd && *r++ == '/'))
                    bOK = false;
                USHORT nDay = 0;
                if (bOK && r != pEnd && INetMIME::isDigit(*r))
                    nDay = INetMIME::getWeight(*r++);
                else
                    bOK = false;
                if (bOK && r != pEnd && INetMIME::isDigit(*r))
                    nDay = 10 * nDay + INetMIME::getWeight(*r++);
                if (!(bOK && r != pEnd && *r++ == '/'))
                    bOK = false;
                USHORT nYear = 0;
                for (int i = 0; bOK && i < 4; ++i)
                    if (r != pEnd && INetMIME::isDigit(*r))
                        nYear = 10 * nYear + INetMIME::getWeight(*r++);
                    else
                        bOK = false;
                if (bOK && Date(nDay, nMonth, nYear).IsValid())
                {
                    bHasOperand = true;
                    aTheOperand <<= util::Date(nDay, nMonth, nYear);
                    nOperatorID = pTheOperator->m_nDate;
                    q = r;
                }
            }

            // Parse number operand:
            if (!bHasOperand && pTheOperator->m_nNumeric != 0)
            {
                sal_Unicode const * r = q;
                bool bNegative = false;
                if (*r == '+')
                    ++r;
                else if (*r == '-')
                {
                    bNegative = true;
                    ++r;
                }
                sal_uInt64 nNumber = 0;
                bool bDigits = false;
                while (r != pEnd && INetMIME::isDigit(*r))
                {
                    nNumber = 10 * nNumber + INetMIME::getWeight(*r++);
                    if (nNumber > std::numeric_limits< sal_Int32 >::max())
                    {
                        bDigits = false;
                        break;
                    }
                }
                if (bDigits)
                {
                    bHasOperand = true;
                    aTheOperand
                        <<= sal_Int32(bNegative ? -sal_Int32(nNumber) :
                                                  sal_Int32(nNumber));
                    nOperatorID = pTheOperator->m_nNumeric;
                    q = r;
                }
            }

            // Bool operator has no operand:
            if (!bHasOperand && pTheOperator->m_nBool != 0)
            {
                bHasOperand = true;
                nOperatorID = pTheOperator->m_nBool;
            }

            bTerm = bHasOperand;
        }

        bool bEmpty = false;
        if (bTerm)
        {
            aCriterium.Terms.realloc(aCriterium.Terms.getLength() + 1);
            aCriterium.Terms[aCriterium.Terms.getLength() - 1]
                = ucb::RuleTerm(aProperty, aTheOperand, nOperatorID,
                                bCaseSensitive, bRegularExpression);
        }
        else if (aCriterium.Terms.getLength() == 0
                 && aProperty.EqualsIgnoreCaseAscii("empty"))
        {
            bEmpty = true;
            q = pPropertyEnd;
        }

        if (!(bTerm || bEmpty))
            break;

        p = q;
        skipWhiteSpace(p, pEnd);

        q = p;
        String aConnection(scanAtom(q, pEnd));
        if (p == pEnd || aConnection.EqualsIgnoreCaseAscii("or"))
        {
            rInfo.Criteria.realloc(rInfo.Criteria.getLength() + 1);
            rInfo.Criteria[rInfo.Criteria.getLength() - 1] = aCriterium;
            aCriterium = ucb::SearchCriterium();
            p = q;
        }
        else if (bTerm && aConnection.EqualsIgnoreCaseAscii("and"))
            p = q;
        else
            break;
    }

    skipWhiteSpace(p, pEnd);
    return p == pEnd;
}

