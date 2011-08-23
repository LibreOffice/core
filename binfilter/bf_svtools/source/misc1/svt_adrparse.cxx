/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <tools/inetmime.hxx>

#include <adrparse.hxx>


//============================================================================
namespace binfilter
{

enum ElementType { ELEMENT_START, ELEMENT_DELIM, ELEMENT_ITEM, ELEMENT_END };

//============================================================================
struct ParsedAddrSpec
{
    sal_Unicode const * m_pBegin;
    sal_Unicode const * m_pEnd;
    ElementType m_eLastElem;
    bool m_bAtFound;
    bool m_bReparse;

    ParsedAddrSpec() { reset(); }

    bool isPoorlyValid() const { return m_eLastElem >= ELEMENT_ITEM; }

    bool isValid() const { return isPoorlyValid() && m_bAtFound; }

    inline void reset();

    inline void finish();
};

inline void ParsedAddrSpec::reset()
{
    m_pBegin = 0;
    m_pEnd = 0;
    m_eLastElem = ELEMENT_START;
    m_bAtFound = false;
    m_bReparse = false;
}

inline void ParsedAddrSpec::finish()
{
    if (isPoorlyValid())
        m_eLastElem = ELEMENT_END;
    else
        reset();
}

//============================================================================
class SvAddressParser_Impl
{
    enum State { BEFORE_COLON, BEFORE_LESS, AFTER_LESS, AFTER_GREATER };

    enum TokenType { TOKEN_QUOTED = 0x80000000, TOKEN_DOMAIN, TOKEN_COMMENT,
                     TOKEN_ATOM };

    sal_Unicode const * m_pInputPos;
    sal_Unicode const * m_pInputEnd;
    sal_uInt32 m_nCurToken;
    sal_Unicode const * m_pCurTokenBegin;
    sal_Unicode const * m_pCurTokenEnd;
    sal_Unicode const * m_pCurTokenContentBegin;
    sal_Unicode const * m_pCurTokenContentEnd;
    bool m_bCurTokenReparse;
    ParsedAddrSpec m_aOuterAddrSpec;
    ParsedAddrSpec m_aInnerAddrSpec;
    ParsedAddrSpec * m_pAddrSpec;
    sal_Unicode const * m_pRealNameBegin;
    sal_Unicode const * m_pRealNameEnd;
    sal_Unicode const * m_pRealNameContentBegin;
    sal_Unicode const * m_pRealNameContentEnd;
    bool m_bRealNameReparse;
    bool m_bRealNameFinished;
    sal_Unicode const * m_pFirstCommentBegin;
    sal_Unicode const * m_pFirstCommentEnd;
    bool m_bFirstCommentReparse;
    State m_eState;
    TokenType m_eType;

    inline void resetRealNameAndFirstComment();

    inline void reset();

    inline void addTokenToAddrSpec(ElementType eTokenElem);

    inline void addTokenToRealName();

    bool readToken();

    static UniString reparse(sal_Unicode const * pBegin,
                             sal_Unicode const * pEnd, bool bAddrSpec);

    static UniString reparseComment(sal_Unicode const * pBegin,
                                    sal_Unicode const * pEnd);

public:
    SvAddressParser_Impl(SvAddressParser * pParser, UniString const & rInput);
};

inline void SvAddressParser_Impl::resetRealNameAndFirstComment()
{
    m_pRealNameBegin = 0;
    m_pRealNameEnd = 0;
    m_pRealNameContentBegin = 0;
    m_pRealNameContentEnd = 0;
    m_bRealNameReparse = false;
    m_bRealNameFinished = false;
    m_pFirstCommentBegin = 0;
    m_pFirstCommentEnd = 0;
    m_bFirstCommentReparse = false;
}

inline void SvAddressParser_Impl::reset()
{
    m_aOuterAddrSpec.reset();
    m_aInnerAddrSpec.reset();
    m_pAddrSpec = &m_aOuterAddrSpec;
    resetRealNameAndFirstComment();
    m_eState = BEFORE_COLON;
    m_eType = TOKEN_ATOM;
}

inline void SvAddressParser_Impl::addTokenToAddrSpec(ElementType eTokenElem)
{
    if (!m_pAddrSpec->m_pBegin)
        m_pAddrSpec->m_pBegin = m_pCurTokenBegin;
    else if (m_pAddrSpec->m_pEnd < m_pCurTokenBegin)
        m_pAddrSpec->m_bReparse = true;
    m_pAddrSpec->m_pEnd = m_pCurTokenEnd;
    m_pAddrSpec->m_eLastElem = eTokenElem;
}

inline void SvAddressParser_Impl::addTokenToRealName()
{
    if (!m_bRealNameFinished && m_eState != AFTER_LESS)
    {
        if (!m_pRealNameBegin)
            m_pRealNameBegin = m_pRealNameContentBegin = m_pCurTokenBegin;
        else if (m_pRealNameEnd < m_pCurTokenBegin - 1
                 || m_pRealNameEnd == m_pCurTokenBegin - 1
                    && *m_pRealNameEnd != ' ')
            m_bRealNameReparse = true;
        m_pRealNameEnd = m_pRealNameContentEnd = m_pCurTokenEnd;
    }
}

//============================================================================
//
//  SvAddressParser_Impl
//
//============================================================================

bool SvAddressParser_Impl::readToken()
{
    m_nCurToken = m_eType;
    m_bCurTokenReparse = false;
    switch (m_eType)
    {
        case TOKEN_QUOTED:
        {
            m_pCurTokenBegin = m_pInputPos - 1;
            m_pCurTokenContentBegin = m_pInputPos;
            bool bEscaped = false;
            for (;;)
            {
                if (m_pInputPos >= m_pInputEnd)
                    return false;
                sal_Unicode cChar = *m_pInputPos++;
                if (bEscaped)
                {
                    m_bCurTokenReparse = true;
                    bEscaped = false;
                }
                else if (cChar == '"')
                {
                    m_pCurTokenEnd = m_pInputPos;
                    m_pCurTokenContentEnd = m_pInputPos - 1;
                    return true;
                }
                else if (cChar == '\\')
                    bEscaped = true;
            }
        }

        case TOKEN_DOMAIN:
        {
            m_pCurTokenBegin = m_pInputPos - 1;
            m_pCurTokenContentBegin = m_pInputPos;
            bool bEscaped = false;
            for (;;)
            {
                if (m_pInputPos >= m_pInputEnd)
                    return false;
                sal_Unicode cChar = *m_pInputPos++;
                if (bEscaped)
                    bEscaped = false;
                else if (cChar == ']')
                {
                    m_pCurTokenEnd = m_pInputPos;
                    return true;
                }
                else if (cChar == '\\')
                    bEscaped = true;
            }
        }

        case TOKEN_COMMENT:
        {
            m_pCurTokenBegin = m_pInputPos - 1;
            m_pCurTokenContentBegin = 0;
            m_pCurTokenContentEnd = 0;
            bool bEscaped = false;
            xub_StrLen nLevel = 0;
            for (;;)
            {
                if (m_pInputPos >= m_pInputEnd)
                    return false;
                sal_Unicode cChar = *m_pInputPos++;
                if (bEscaped)
                {
                    m_bCurTokenReparse = true;
                    m_pCurTokenContentEnd = m_pInputPos;
                    bEscaped = false;
                }
                else if (cChar == '(')
                {
                    if (!m_pCurTokenContentBegin)
                        m_pCurTokenContentBegin = m_pInputPos - 1;
                    m_pCurTokenContentEnd = m_pInputPos;
                    ++nLevel;
                }
                else if (cChar == ')')
                    if (nLevel)
                    {
                        m_pCurTokenContentEnd = m_pInputPos;
                        --nLevel;
                    }
                    else
                        return true;
                else if (cChar == '\\')
                {
                    if (!m_pCurTokenContentBegin)
                        m_pCurTokenContentBegin = m_pInputPos - 1;
                    bEscaped = true;
                }
                else if (cChar > ' ' && cChar != 0x7F) // DEL
                {
                    if (!m_pCurTokenContentBegin)
                        m_pCurTokenContentBegin = m_pInputPos - 1;
                    m_pCurTokenContentEnd = m_pInputPos;
                }
            }
        }

        default:
        {
            sal_Unicode cChar;
            for (;;)
            {
                if (m_pInputPos >= m_pInputEnd)
                    return false;
                cChar = *m_pInputPos++;
                if (cChar > ' ' && cChar != 0x7F) // DEL
                    break;
            }
            m_pCurTokenBegin = m_pInputPos - 1;
            if (cChar == '"' || cChar == '(' || cChar == ')' || cChar == ','
                || cChar == '.' || cChar == ':' || cChar == ';'
                || cChar == '<' || cChar == '>' || cChar == '@'
                || cChar == '[' || cChar == '\\' || cChar == ']')
            {
                m_nCurToken = cChar;
                m_pCurTokenEnd = m_pInputPos;
                return true;
            }
            else
                for (;;)
                {
                    if (m_pInputPos >= m_pInputEnd)
                    {
                        m_pCurTokenEnd = m_pInputPos;
                        return true;
                    }
                    cChar = *m_pInputPos++;
                    if (cChar <= ' ' || cChar == '"' || cChar == '('
                        || cChar == ')' || cChar == ',' || cChar == '.'
                        || cChar == ':' || cChar == ';' || cChar == '<'
                        || cChar == '>' || cChar == '@' || cChar == '['
                        || cChar == '\\' || cChar == ']'
                        || cChar == 0x7F) // DEL
                    {
                        m_pCurTokenEnd = --m_pInputPos;
                        return true;
                    }
                }
        }
    }
}

//============================================================================
// static
UniString SvAddressParser_Impl::reparse(sal_Unicode const * pBegin,
                                        sal_Unicode const * pEnd,
                                        bool bAddrSpec)
{
    UniString aResult;
    TokenType eMode = TOKEN_ATOM;
    bool bEscaped = false;
    bool bEndsWithSpace = false;
    xub_StrLen nLevel = 0;
    while (pBegin < pEnd)
    {
        sal_Unicode cChar = *pBegin++;
        switch (eMode)
        {
            case TOKEN_QUOTED:
                if (bEscaped)
                {
                    aResult += cChar;
                    bEscaped = false;
                }
                else if (cChar == '"')
                {
                    if (bAddrSpec)
                        aResult += cChar;
                    eMode = TOKEN_ATOM;
                }
                else if (cChar == '\\')
                {
                    if (bAddrSpec)
                        aResult += cChar;
                    bEscaped = true;
                }
                else
                    aResult += cChar;
                break;

            case TOKEN_DOMAIN:
                if (bEscaped)
                {
                    aResult += cChar;
                    bEscaped = false;
                }
                else if (cChar == ']')
                {
                    aResult += cChar;
                    eMode = TOKEN_ATOM;
                }
                else if (cChar == '\\')
                {
                    if (bAddrSpec)
                        aResult += cChar;
                    bEscaped = true;
                }
                else
                    aResult += cChar;
                break;

            case TOKEN_COMMENT:
                if (bEscaped)
                    bEscaped = false;
                else if (cChar == '(')
                    ++nLevel;
                else if (cChar == ')')
                    if (nLevel)
                        --nLevel;
                    else
                        eMode = TOKEN_ATOM;
                else if (cChar == '\\')
                    bEscaped = true;
                break;

            case TOKEN_ATOM:
                if (cChar <= ' ' || cChar == 0x7F) // DEL
                {
                    if (!bAddrSpec && !bEndsWithSpace)
                    {
                        aResult += ' ';
                        bEndsWithSpace = true;
                    }
                }
                else if (cChar == '(')
                {
                    if (!bAddrSpec && !bEndsWithSpace)
                    {
                        aResult += ' ';
                        bEndsWithSpace = true;
                    }
                    eMode = TOKEN_COMMENT;
                }
                else
                {
                    bEndsWithSpace = false;
                    if (cChar == '"')
                    {
                        if (bAddrSpec)
                            aResult += cChar;
                        eMode = TOKEN_QUOTED;
                    }
                    else if (cChar == '[')
                    {
                        aResult += cChar;
                        eMode = TOKEN_QUOTED;
                    }
                    else
                        aResult += cChar;
                }
                break;
        }
    }
    return aResult;
}

//============================================================================
// static
UniString SvAddressParser_Impl::reparseComment(sal_Unicode const * pBegin,
                                               sal_Unicode const * pEnd)
{
    UniString aResult;
    while (pBegin < pEnd)
    {
        sal_Unicode cChar = *pBegin++;
        if (cChar == '\\')
            cChar = *pBegin++;
        aResult += cChar;
    }
    return aResult;
}

//============================================================================
SvAddressParser_Impl::SvAddressParser_Impl(SvAddressParser * pParser,
                                           UniString const & rInput)
{
    m_pInputPos = rInput.GetBuffer();
    m_pInputEnd = m_pInputPos + rInput.Len();

    reset();
    bool bDone = false;
    for (;;)
    {
        if (!readToken())
        {
            m_bRealNameFinished = true;
            if (m_eState == AFTER_LESS)
                m_nCurToken = '>';
            else
            {
                m_nCurToken = ',';
                bDone = true;
            }
        }
        switch (m_nCurToken)
        {
            case TOKEN_QUOTED:
                if (m_pAddrSpec->m_eLastElem != ELEMENT_END)
                {
                    if (m_pAddrSpec->m_bAtFound
                        || m_pAddrSpec->m_eLastElem <= ELEMENT_DELIM)
                        m_pAddrSpec->reset();
                    addTokenToAddrSpec(ELEMENT_ITEM);
                }
                if (!m_bRealNameFinished && m_eState != AFTER_LESS)
                    if (m_bCurTokenReparse)
                    {
                        if (!m_pRealNameBegin)
                            m_pRealNameBegin = m_pCurTokenBegin;
                        m_pRealNameEnd = m_pCurTokenEnd;
                        m_bRealNameReparse = true;
                    }
                    else if (m_bRealNameReparse)
                        m_pRealNameEnd = m_pCurTokenEnd;
                    else if (!m_pRealNameBegin)
                    {
                        m_pRealNameBegin = m_pCurTokenBegin;
                        m_pRealNameContentBegin = m_pCurTokenContentBegin;
                        m_pRealNameEnd = m_pRealNameContentEnd
                            = m_pCurTokenContentEnd;
                    }
                    else
                    {
                        m_pRealNameEnd = m_pCurTokenEnd;
                        m_bRealNameReparse = true;
                    }
                m_eType = TOKEN_ATOM;
                break;

            case TOKEN_DOMAIN:
                if (m_pAddrSpec->m_eLastElem != ELEMENT_END)
                    if (m_pAddrSpec->m_bAtFound
                        && m_pAddrSpec->m_eLastElem == ELEMENT_DELIM)
                        addTokenToAddrSpec(ELEMENT_ITEM);
                    else
                        m_pAddrSpec->reset();
                addTokenToRealName();
                m_eType = TOKEN_ATOM;
                break;

            case TOKEN_COMMENT:
                if (!m_bRealNameFinished && m_eState != AFTER_LESS
                    && !m_pFirstCommentBegin && m_pCurTokenContentBegin)
                {
                    m_pFirstCommentBegin = m_pCurTokenContentBegin;
                    m_pFirstCommentEnd = m_pCurTokenContentEnd;
                    m_bFirstCommentReparse = m_bCurTokenReparse;
                }
                m_eType = TOKEN_ATOM;
                break;

            case TOKEN_ATOM:
                if (m_pAddrSpec->m_eLastElem != ELEMENT_END)
                {
                    if (m_pAddrSpec->m_eLastElem != ELEMENT_DELIM)
                        m_pAddrSpec->reset();
                    addTokenToAddrSpec(ELEMENT_ITEM);
                }
                addTokenToRealName();
                break;

            case '(':
                m_eType = TOKEN_COMMENT;
                break;

            case ')':
            case '\\':
            case ']':
                m_pAddrSpec->finish();
                addTokenToRealName();
                break;

            case '<':
                switch (m_eState)
                {
                    case BEFORE_COLON:
                    case BEFORE_LESS:
                        m_aOuterAddrSpec.finish();
                        if (m_pRealNameBegin)
                            m_bRealNameFinished = true;
                        m_pAddrSpec = &m_aInnerAddrSpec;
                        m_eState = AFTER_LESS;
                        break;

                    case AFTER_LESS:
                        m_aInnerAddrSpec.finish();
                        break;

                    case AFTER_GREATER:
                        m_aOuterAddrSpec.finish();
                        addTokenToRealName();
                        break;
                }
                break;

            case '>':
                if (m_eState == AFTER_LESS)
                {
                    m_aInnerAddrSpec.finish();
                    if (m_aInnerAddrSpec.isValid())
                        m_aOuterAddrSpec.m_eLastElem = ELEMENT_END;
                    m_pAddrSpec = &m_aOuterAddrSpec;
                    m_eState = AFTER_GREATER;
                }
                else
                {
                    m_aOuterAddrSpec.finish();
                    addTokenToRealName();
                }
                break;

            case '@':
                if (m_pAddrSpec->m_eLastElem != ELEMENT_END)
                    if (!m_pAddrSpec->m_bAtFound
                        && m_pAddrSpec->m_eLastElem == ELEMENT_ITEM)
                    {
                        addTokenToAddrSpec(ELEMENT_DELIM);
                        m_pAddrSpec->m_bAtFound = true;
                    }
                    else
                        m_pAddrSpec->reset();
                addTokenToRealName();
                break;

            case ',':
            case ';':
                if (m_eState == AFTER_LESS)
                    if (m_nCurToken == ',')
                    {
                        if (m_aInnerAddrSpec.m_eLastElem
                             != ELEMENT_END)
                            m_aInnerAddrSpec.reset();
                    }
                    else
                        m_aInnerAddrSpec.finish();
                else
                {
                    m_pAddrSpec = m_aInnerAddrSpec.isValid()
                                  || !m_aOuterAddrSpec.isValid()
                                         && m_aInnerAddrSpec.isPoorlyValid() ?
                                      &m_aInnerAddrSpec :
                                  m_aOuterAddrSpec.isPoorlyValid() ?
                                      &m_aOuterAddrSpec : 0;
                    if (m_pAddrSpec)
                    {
                        UniString aTheAddrSpec;
                        if (m_pAddrSpec->m_bReparse)
                            aTheAddrSpec = reparse(m_pAddrSpec->m_pBegin,
                                                   m_pAddrSpec->m_pEnd, true);
                        else
                        {
                            xub_StrLen nLen =
                                sal::static_int_cast< xub_StrLen >(
                                    m_pAddrSpec->m_pEnd
                                    - m_pAddrSpec->m_pBegin);
                            if (nLen == rInput.Len())
                                aTheAddrSpec = rInput;
                            else
                                aTheAddrSpec
                                    = rInput.Copy(
                                        sal::static_int_cast< xub_StrLen >(
                                            m_pAddrSpec->m_pBegin
                                            - rInput.GetBuffer()),
                                        nLen);
                        }
                        UniString aTheRealName;
                        if (!m_pRealNameBegin
                            || m_pAddrSpec == &m_aOuterAddrSpec
                               && m_pRealNameBegin
                                      == m_aOuterAddrSpec.m_pBegin
                               && m_pRealNameEnd == m_aOuterAddrSpec.m_pEnd
                               && m_pFirstCommentBegin)
                            if (!m_pFirstCommentBegin)
                                aTheRealName = aTheAddrSpec;
                            else if (m_bFirstCommentReparse)
                                aTheRealName
                                    = reparseComment(m_pFirstCommentBegin,
                                                     m_pFirstCommentEnd);
                            else
                                aTheRealName
                                    = rInput.Copy(
                                        sal::static_int_cast< xub_StrLen >(
                                            m_pFirstCommentBegin
                                            - rInput.GetBuffer()),
                                        sal::static_int_cast< xub_StrLen >(
                                            m_pFirstCommentEnd
                                            - m_pFirstCommentBegin));
                        else if (m_bRealNameReparse)
                            aTheRealName = reparse(m_pRealNameBegin,
                                                   m_pRealNameEnd, false);
                        else
                        {
                            xub_StrLen nLen =
                                sal::static_int_cast< xub_StrLen >(
                                    m_pRealNameContentEnd
                                    - m_pRealNameContentBegin);
                            if (nLen == rInput.Len())
                                aTheRealName = rInput;
                            else
                                aTheRealName
                                    = rInput.Copy(
                                        sal::static_int_cast< xub_StrLen >(
                                            m_pRealNameContentBegin
                                            - rInput.GetBuffer()),
                                        nLen);
                        }
                        if (pParser->m_bHasFirst)
                            pParser->m_aRest.Insert(new SvAddressEntry_Impl(
                                                            aTheAddrSpec,
                                                            aTheRealName),
                                                    LIST_APPEND);
                        else
                        {
                            pParser->m_bHasFirst = true;
                            pParser->m_aFirst.m_aAddrSpec = aTheAddrSpec;
                            pParser->m_aFirst.m_aRealName = aTheRealName;
                        }
                    }
                    if (bDone)
                        return;
                    reset();
                }
                break;

            case ':':
                switch (m_eState)
                {
                    case BEFORE_COLON:
                        m_aOuterAddrSpec.reset();
                        resetRealNameAndFirstComment();
                        m_eState = BEFORE_LESS;
                        break;

                    case BEFORE_LESS:
                    case AFTER_GREATER:
                        m_aOuterAddrSpec.finish();
                        addTokenToRealName();
                        break;

                    case AFTER_LESS:
                        m_aInnerAddrSpec.reset();
                        break;
                }
                break;

            case '"':
                m_eType = TOKEN_QUOTED;
                break;

            case '.':
                if (m_pAddrSpec->m_eLastElem != ELEMENT_END)
                    if (m_pAddrSpec->m_eLastElem != ELEMENT_DELIM)
                        addTokenToAddrSpec(ELEMENT_DELIM);
                    else
                        m_pAddrSpec->reset();
                addTokenToRealName();
                break;

            case '[':
                m_eType = TOKEN_DOMAIN;
                break;
        }
    }
}

//============================================================================
//
//  SvAddressParser
//
//============================================================================

SvAddressParser::SvAddressParser(UniString const & rInput): m_bHasFirst(false)
{
    SvAddressParser_Impl(this, rInput);
}

//============================================================================
SvAddressParser::~SvAddressParser()
{
    for (ULONG i = m_aRest.Count(); i != 0;)
        delete m_aRest.Remove(--i);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
