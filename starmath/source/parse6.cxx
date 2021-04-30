/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <parse6.hxx>
#include <smmod.hxx>

// Parser helpers
#include <unotools/syslocale.hxx>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>

// Namespace for ParseResult
using namespace ::com::sun::star::i18n;

// Helper
/*************************************************************************************************/

// Parse token
/*************************************************************************************************/

/**
  * To see UnicodeType:  /core/offapi/com/sun/star/i18n/UnicodeType.idl
  * To see KParseType:   /core/offapi/com/sun/star/i18n/KParseType.idl
  * To see ParseResult:  /core/offapi/com/sun/star/i18n/ParseResult.idl
  * To see kParseTokens: /core/offapi/com/sun/star/i18n/KParseTokens.idl
  */

/**
  * Helps with the parsing.
  * Checks out the unicode type of the current position in the buffer.
  * @param aUnicodeType
  * @return match unicode type
  */
#define isUnicodeType(X) bool(X & m_pSysCC->getType(m_aBufferString, m_nBufferIndex))

// Token definitions
static std::unique_ptr<const CharClass> m_pSysCC(nullptr);
static const int_fast32_t identTokens = KParseTokens::ANY_LETTER_OR_NUMBER;
static const int_fast32_t numTokens
    = KParseTokens::GROUP_SEPARATOR_IN_NUMBER | KParseTokens::ANY_NUMBER;
//static const int_fast32_t num16Tokens =  KParseTokens::GROUP_SEPARATOR_IN_NUMBER | KParseTokens::ANY_LETTER_OR_NUMBER;
//static const int_fast32_t wnum16Tokens =  KParseTokens::ASC_DOT | KParseTokens::ASC_ALNUM;
static const int_fast32_t snumTokens
    = KParseTokens::GROUP_SEPARATOR_IN_NUMBER | KParseTokens::ANY_NUMBER;
static const int_fast32_t textTokens = KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;

void SmParser6::NextToken()
{
    ParseResult aRes;

    // Skip empty spaces
    while (isUnicodeType(UnicodeType::SPACE_SEPARATOR)
           || isUnicodeType(UnicodeType::LINE_SEPARATOR))
        ++m_nBufferIndex;
    {
        if (isUnicodeType(UnicodeType::LINE_SEPARATOR))
        {
            m_nRow++;
            m_nColOff = 0;
        }
        if (isUnicodeType(UnicodeType::LINE_SEPARATOR))
            m_nColOff++;
    }

    // We gonna parse a variable name
    if (m_pSysCC->isLetter(m_aBufferString, m_nBufferIndex))
    {
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, identTokens, "",
                                       identTokens, "");
        aRes.TokenType = KParseType::IDENTNAME;
    }

    // We gonna parse a number
    else if (m_pSysCC->isDigit(m_aBufferString, m_nBufferIndex))
    {
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, numTokens, "", numTokens,
                                       "");
        aRes.TokenType = KParseType::ANY_NUMBER;
    }

    // We gonna parse a number .123
    else if (m_aBufferString[m_nBufferIndex] == u'.' || m_aBufferString[m_nBufferIndex] == u',')
    {
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, snumTokens, "", snumTokens,
                                       "");
        if (aRes.TokenType == 0)
            aRes = ParseResult(0, m_nBufferIndex + 1, 1, 0.0, KParseType::ONE_SINGLE_CHAR, 0, 0,
                               u"");
        else
            aRes.TokenType = KParseType::ANY_NUMBER;
    }

    // We gonna parse text
    else if (m_aBufferString[m_nBufferIndex] == '\"')
    {
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, textTokens, "", textTokens,
                                       "");
        aRes.TokenType = KParseType::DOUBLE_QUOTE_STRING;
    }

    // We gonna parse an operator
    else if (UnicodeType::MATH_SYMBOL == m_pSysCC->getType(m_aBufferString, m_nBufferIndex))
    {
        aRes = ParseResult(0, m_nBufferIndex + 1, 1, 0.0, KParseType::ONE_SINGLE_CHAR, 0, 0, u"");
        aRes.TokenType = KParseType::ONE_SINGLE_CHAR;
    }

    // We give up
    else
    {
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, 0, "", 0, "");
    }

    // set index of current token
    m_aCurToken.nRow = m_nRow;
    m_aCurToken.nCol = m_nColOff;
    OUString aName(m_aBufferString.copy(m_nBufferIndex, aRes.EndPos - m_nBufferIndex));

    if (aRes.TokenType & KParseType::IDENTNAME)
    {
        m_aCurToken.eType = TIDENT;
        m_aCurToken.cMathChar = aName;
        m_aCurToken.nGroup = TG::NONE;
        m_aCurToken.nPriority = -1;
        m_aCurToken.aText = aName;
    }
    else if (aRes.TokenType & KParseType::ANY_NUMBER)
    {
        m_aCurToken.eType = TNUMBER;
        m_aCurToken.cMathChar = aName;
        m_aCurToken.nGroup = TG::NONE;
        m_aCurToken.nPriority = -1;
        m_aCurToken.aText = aName;
    }
    else if (aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING)
    {
        m_aCurToken.eType = TTEXT;
        m_aCurToken.cMathChar = aRes.DequotedNameOrString;
        m_aCurToken.nGroup = TG::NONE;
        m_aCurToken.nPriority = -1;
        m_aCurToken.aText = "%";
        // TODO: not exeptions
        // Temporal while build
        throw std::invalid_argument("Not implemented yet");
    }
    else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
    {
        // TODO: not exeptions
        // Temporal while build
        throw std::invalid_argument("Not implemented yet");
    }
    else
    {
        // TODO: not exeptions
        // Temporal while build
        throw std::invalid_argument("Not implemented yet");
    }

    // "Move cursor"
    m_nRow += aRes.EndPos - m_nBufferIndex;
    m_nBufferIndex = aRes.EndPos;
}

/**
  * Note: we changed parser model.
  * This parser allows limitless depth (until overflow ~ 2^31 nodes or full memory consumption)
  * However it's more complex
  * Some depth limit will be added later in expert config (needs heavy refractoring)
  */
void SmParser6::ParseTokens()
{
    SmTableNode* pTree = new SmTableNode(m_aCurToken);
    SmNode* parent = pTree;
    NextToken();

    while (true)
    {
        switch (m_aCurToken.nGroup)
        {
            case TG::NONE:
            {
                switch (m_aCurToken.eType)
                {
                    case TNUMBER:
                    {
                        SmTextNode* pTextNode = new SmTextNode(m_aCurToken, FNT_NUMBER);
                    }
                    break;

                    case TIDENT:
                    {
                        SmTextNode* pTextNode = new SmTextNode(m_aCurToken, FNT_VARIABLE);
                    }
                    break;

                    default:
                        break;
                }
            }
            break;

            case TG::BinMo:
            {
                SmStructureNode* pStructureNode;
                SmNode* pOperNode;
                switch (m_aCurToken.eType)
                {
                    case TOVER:
                    {
                        pStructureNode = new SmBinVerNode(m_aCurToken);
                        pOperNode = new SmRectangleNode(m_aCurToken);
                    }
                    break;

                    default:
                    {
                        pStructureNode = new SmBinHorNode(m_aCurToken);
                        pOperNode = new SmMathSymbolNode(m_aCurToken);
                    }
                    break;
                }
            }
            break;

            case TG::UnMo:
            {
                switch (m_aCurToken.eType)
                {
                    default:
                        break;
                }
            }
            break;

            case TG::BinUnMo:
            {
            }
            break;

            default:
            {
                switch (m_aCurToken.eType)
                {
                    default:
                        break;
                }
            }
            break;
        }
    }
}

// Class construction
/*************************************************************************************************/

SmParser6::SmParser6()
    : AbstractSmParser()
    , m_aBufferString(u"")
    , m_nBufferIndex(0)
    , m_nRow(1)
    , m_nColOff(0)
    , nDepth(0)
{
    if (!m_pSysCC)
        m_pSysCC.reset(SM_MOD()->GetSysLocale().GetCharClassPtr());
    m_aCurToken.nLevel = 0;
}

const OUString& SmParser6::GetText() const { return m_aBufferString; }

#undef isUnicodeType
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
