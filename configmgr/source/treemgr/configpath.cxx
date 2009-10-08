/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configpath.cxx,v $
 * $Revision: 1.16 $
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
#include "precompiled_configmgr.hxx"

#include "configpath.hxx"
#include "configexcept.hxx"
#include <rtl/ustrbuf.hxx>

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

#ifndef CFG_PATH_STRICT
//#define CFG_PATH_STRICT 1
#endif

#define dprint(a,b,c)

namespace configmgr
{
    namespace configuration
    {

    //-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Name validation
//-----------------------------------------------------------------------------
namespace
{
    //-------------------------------------------------------------------------
    inline
    bool isValidNameStart(sal_Unicode ch) SAL_THROW(())
    {
        return (sal_Unicode('A') <= ch && ch <= sal_Unicode('Z')) ||
               (sal_Unicode('a') <= ch && ch <= sal_Unicode('z')) ||
                sal_Unicode('_') == ch;
    }
    inline
    bool isValidNameCont(sal_Unicode ch) SAL_THROW(())
    {
        return  (   (sal_Unicode('0') <= ch && ch <= sal_Unicode('9'))
                ||  (sal_Unicode('.') == ch) // eg for module names
                ||  (sal_Unicode('-') == ch) // eg for locale names
                ||  (sal_Unicode(':') == ch) // support special namespaced names
                );
    }

    //-------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------

bool isSimpleName(rtl::OUString const& sName) SAL_THROW(())
{
    sal_Unicode const* const pStr = sName.getStr();
    sal_Unicode const* const pEnd = pStr + sName.getLength();

    if ( (pStr == pEnd) || !isValidNameStart(*pStr) )
        return false;

    for (sal_Unicode const* pValidate = pStr+1; pValidate != pEnd; ++pValidate)
    {
        if (!isValidNameStart(*pValidate) && !isValidNameCont(*pValidate))
            return false;
    }

    return true;
}
//-----------------------------------------------------------------------------

rtl::OUString validateNodeName(rtl::OUString const& sName)
{
    if (!isSimpleName(sName))
        throw InvalidName(sName, "is not a valid name for a configuration node");

    return sName;
}
//-----------------------------------------------------------------------------

rtl::OUString validateElementName(rtl::OUString const& sName)
{
    if (sName.getLength() == 0)
        throw InvalidName(sName, "is not a valid name for a configuration item (empty names are not permitted)");

    return sName;
}
//-----------------------------------------------------------------------------

namespace // path helpers I
{
//-----------------------------------------------------------------------------
    const sal_Unicode c_cDelimiter = '/';

    const sal_Unicode c_lBracket = '[', c_rBracket = ']';

    const sal_Unicode c_cAnytype = '*';
//-----------------------------------------------------------------------------

    // Textually an Absolute path starts with a slash
    static
    inline
    bool detectAbsolutePath(sal_Unicode const* _pPath) SAL_THROW(())
    {
        OSL_ASSERT( _pPath != NULL );
        return  *_pPath == c_cDelimiter;
    }
//-----------------------------------------------------------------------------

    static
    inline
    rtl::OUString makeWildcardType() SAL_THROW(())
    {
        return rtl::OUString(&c_cAnytype,1);
    }
//-----------------------------------------------------------------------------

    // even handles empty strings (if NUL-terminated)
    static
    inline
    bool isWildcardType(sal_Unicode const* _sType) SAL_THROW(())
    {
        OSL_ASSERT( _sType != NULL );
        return  _sType[0] == c_cAnytype &&
                _sType[1] == 0;
    }
//-----------------------------------------------------------------------------

    static
    inline
    bool isEmptyString(sal_Unicode const* _sType) SAL_THROW(())
    {
        OSL_ASSERT( _sType != NULL );
        return  _sType[0] == 0;
    }
//-----------------------------------------------------------------------------
    static
    inline
    sal_Unicode lastChar(rtl::OUString const& _sString) SAL_THROW(())
    {
        sal_Int32 const nLen = _sString.getLength();

        OSL_PRECOND( nLen > 0, "Non-empty string expected");

        return _sString[nLen-1];
    }
//-----------------------------------------------------------------------------

    rtl::OUString implMakeCompositeName(rtl::OUString const& _sBaseName, rtl::OUString const& _sPredicate) SAL_THROW((InvalidName));
    void implSplitCompositeName(rtl::OUString const& _aCompositeName, rtl::OUString& _rBaseName, rtl::OUString& _rPredicate) SAL_THROW(());
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
namespace Path
{
//-----------------------------------------------------------------------------
// class configuration::Path::Component
//-----------------------------------------------------------------------------

inline // though public, this method is not available outside this translation unit
Component::Component(rtl::OUString const& _sName) SAL_THROW(())
: m_aName(_sName)
{
}
//-----------------------------------------------------------------------------

bool Component::isSimpleName()    const SAL_THROW(())
{
    return m_aName.getLength() != 0 && lastChar(m_aName) != c_rBracket;
}
//-----------------------------------------------------------------------------

rtl::OUString Component::getName()    const SAL_THROW(())
{
    if (isSimpleName()) return m_aName;

    rtl::OUString sName, sType;
    implSplitCompositeName(m_aName,sType,sName);

    return sName;
}
//-----------------------------------------------------------------------------

rtl::OUString Component::getTypeName()      const SAL_THROW(())
{
    if (isSimpleName()) return rtl::OUString();

    rtl::OUString sName, sType;
    implSplitCompositeName(m_aName,sType,sName);

    return sType;
}
//-----------------------------------------------------------------------------

Component makeEmptyComponent() SAL_THROW(())
{
    return Component( rtl::OUString() );
}
//-----------------------------------------------------------------------------

Component wrapSimpleName(rtl::OUString const& _sName)
{
    OSL_ENSURE( isSimpleName(_sName), "Simple name expected creating path component");
    if (!isSimpleName(_sName))
        throw InvalidName(_sName, "is not a simple name. Cannot convert to path component");

    return Component( _sName );
}
//-----------------------------------------------------------------------------

Component makeCompositeName(rtl::OUString const& _sElementName, rtl::OUString const& _sTypeName)
{
    return Component( implMakeCompositeName(_sTypeName,_sElementName) );
}
//-----------------------------------------------------------------------------


bool matches(Component const& lhs,Component const& rhs) SAL_THROW(())
{
    // this extra preflight check might be left out (is it good for performance ?)
    if (lhs.getInternalName() == rhs.getInternalName())
        return true;

    if (lhs.getName() != rhs.getName())
        return false;

    // simple names are considered equivalent to wildcard namess
    if (lhs.isSimpleName() || rhs.isSimpleName())
        return true;

    rtl::OUString aTypeLHS = lhs.getTypeName();
    rtl::OUString aTypeRHS = rhs.getTypeName();

    // this would need an extra test without our preflight check
    OSL_ASSERT(aTypeLHS != aTypeRHS); // would have been dicovered by first check

    if ( isWildcardType(aTypeLHS) || isWildcardType(aTypeRHS) )
        return true;

    return false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// weak comparison of components
//-----------------------------------------------------------------------------
bool before(Component const& lhs, Component const& rhs) SAL_THROW(())
{ return lhs.getName() < rhs.getName(); }

//-----------------------------------------------------------------------------
bool equiv(Component const& lhs, Component const& rhs) SAL_THROW(())
{ return lhs.getName() == rhs.getName(); }

//-----------------------------------------------------------------------------
size_t hashCode(Component const& comp)  SAL_THROW(())
{ return comp.getName().hashCode(); }

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class configuration::Path::Rep
//-----------------------------------------------------------------------------
void Rep::check_not_empty() const
{
    if (m_aComponents.empty())
    {
        OSL_ENSURE(!m_aComponents.empty(),"Trying to access components of an empty path");
        throw Exception("Trying to access components of an empty path");
    }
}
//-----------------------------------------------------------------------------

void Rep::prepend(Rep const& _aOther) SAL_THROW(())
{
    // to prepend the other path append its components
    m_aComponents.insert( m_aComponents.end(),
                            _aOther.m_aComponents.begin(),
                            _aOther.m_aComponents.end());

}
//-----------------------------------------------------------------------------

rtl::OUString Rep::toString(bool _bAbsolute) const SAL_THROW(())
{
    std::vector<Component>::const_reverse_iterator cur = begin();
    std::vector<Component>::const_reverse_iterator const stop = end();

    rtl::OUStringBuffer sRet;

    if (!_bAbsolute && cur != stop)
        sRet = cur++->toPathString();

    for ( ;cur != stop; ++cur)
        sRet.append( c_cDelimiter ).append( cur->toPathString() );

    return sRet.makeStringAndClear();
}
//-----------------------------------------------------------------------------

size_t Rep::hashCode() const  SAL_THROW(())
{
    const unsigned long mangle_factor = 11; // 1011 (2)
    unsigned long nHash = 0;
    for (std::vector<Component>::const_reverse_iterator it = begin(), stop = end(); it != stop; ++it)
    {
        nHash = mangle_factor*nHash + Path::hashCode(*it);
    }
    return nHash;
}
//-----------------------------------------------------------------------------

bool before(Rep const& lhs, Rep const& rhs) SAL_THROW(())
{
    return std::lexicographical_compare(lhs.begin(),lhs.end(),rhs.begin(),rhs.end(), Before());
}
//-----------------------------------------------------------------------------

bool equiv(Rep const& lhs, Rep const& rhs) SAL_THROW(())
{
    return (lhs.countComponents() == rhs.countComponents()) &&
            std::equal(lhs.begin(),lhs.end(),rhs.begin(),Equiv());
}
//-----------------------------------------------------------------------------

bool matches(Rep const& lhs, Rep const& rhs) SAL_THROW(())
{
    return (lhs.countComponents() == rhs.countComponents()) &&
            std::equal(lhs.begin(),lhs.end(),rhs.begin(),Matches());
}
//-----------------------------------------------------------------------------

bool isAbsolutePath(rtl::OUString const& _sPath) SAL_THROW(())
{
    return detectAbsolutePath(_sPath);
}
//-----------------------------------------------------------------------------

bool hasMatchingPrefix(Rep const& _aPath, Rep const& _aPrefix) SAL_THROW(())
{
    return (_aPath.countComponents() >= _aPrefix.countComponents()) &&
            std::equal( _aPrefix.begin(), _aPrefix.end(), _aPath.begin(), Matches());
}
//-----------------------------------------------------------------------------

Rep stripMatchingPrefix(Rep const& _aPath,Rep const& _aPrefix) //  SAL_THROW((InvalidName))
{
    Rep aResult(_aPath);

    for (std::vector<Component>::const_reverse_iterator it = _aPrefix.begin(); it != _aPrefix.end(); ++it)
    {
        if (aResult.isEmpty() || !matches(*it,aResult.getFirstName()))
            throw InvalidName(aResult.getFirstName().toPathString(), "does not match the expected location.");

        aResult.dropFirstName();
    }

    return aResult;
}
//-----------------------------------------------------------------------------
} // namespace Path
//-----------------------------------------------------------------------------

namespace
{
//-----------------------------------------------------------------------------
    const sal_Unicode c_quot = '\"';
    const sal_Unicode c_apos = '\'';
    const sal_Unicode c_amp = '&';

    const sal_Unicode c_end_escape = ';';

    const sal_Unicode c_normal_quot = c_apos;
    //-------------------------------------------
    static sal_Char const c_amp_name[]  = "&amp;";
    static sal_Char const c_apos_name[] = "&apos;";
    static sal_Char const c_quot_name[] = "&quot;";

    const sal_Int32 c_nMinEscapeLen = sizeof c_amp_name  - 1;
    const sal_Int32 c_nMaxEscapeLen = sizeof c_quot_name - 1;
//-------------------------------------------------------------------------
    /// distinguishes which kind of path is held in a path object
    enum PathType { eRELATIVE = 1, eABSOLUTE = 2 };

//-----------------------------------------------------------------------------
    // missing or mis leading in SAL/rtl: pStr1[nLength] must NOT be evaluated
    static
    sal_Int32 cfg_ustr_ascii_compare_WithLength( const sal_Unicode* pStr1,
                                                          sal_Int32 nStr1Len,
                                                          const sal_Char* pStr2 )
    {
        while( nStr1Len )
        {
            sal_Int32 nRet = static_cast<sal_Int32>(*pStr1)-
                             static_cast<sal_Int32>(static_cast<unsigned char>(*pStr2));

            if (nRet != 0 || *pStr2 == 0) return nRet;

            ++pStr1;
            ++pStr2;
            --nStr1Len;
        }

        return -static_cast<sal_Int32>(static_cast<unsigned char>(*pStr2));
    }
//-----------------------------------------------------------------------------


    /** find the char being escaped by the escape sequence in the given string range
        @return
            the char being escaped or zero, if the range is no known escape
    */
    sal_Unicode implParseEscape(sal_Unicode const *  pBegin, sal_Unicode const *  pEnd) SAL_THROW(())
    {
        OSL_PRECOND( pBegin <  pEnd, "Nonempty string range expected" );
        OSL_PRECOND( pBegin[0] == c_amp,        "String range is not a possible escape: missing start marker" );
        OSL_PRECOND( pEnd[-1]  == c_end_escape, "String range is not a possible escape: missing end marker" );

        sal_Int32 const nLen = pEnd - pBegin;

        sal_Unicode chResult;

        if ( c_nMinEscapeLen > nLen || nLen > c_nMaxEscapeLen) // quick check, if there is no possible match
            chResult = 0;
        // the standard escapes
        else if (0 == cfg_ustr_ascii_compare_WithLength(pBegin,nLen,c_amp_name))   chResult = c_amp;
        else if (0 == cfg_ustr_ascii_compare_WithLength(pBegin,nLen,c_apos_name))  chResult = c_apos;
        else if (0 == cfg_ustr_ascii_compare_WithLength(pBegin,nLen,c_quot_name))  chResult = c_quot;
        // extra escapes for XML compatibility
        else if (0 == cfg_ustr_ascii_compare_WithLength(pBegin,nLen,"&lt;"))    chResult = sal_Unicode('<');
        else if (0 == cfg_ustr_ascii_compare_WithLength(pBegin,nLen,"&gt;"))    chResult = sal_Unicode('>');
        else chResult = 0;

        return chResult;
    }

//-----------------------------------------------------------------------------

    /** find the escape sequence to use for the given char
        @return
            an escape sequence, or NULL, if the char should not be escaped
    */
    inline
    sal_Char const* implGetEscape(sal_Unicode ch ) SAL_THROW(())
    {
        switch (ch)
        {
        case c_amp:  return c_amp_name;
        case c_apos: return c_apos_name;
        case c_quot: return c_quot_name;

        default: return NULL;
        }
    }

//-----------------------------------------------------------------------------

    /** find the start of the path component ending before pEnd in the string starting at pBegin
        @return
            a pointer to the last character before pEnd that is not a name delimiter
    */
    sal_Unicode const *  implFindNameStart(sal_Unicode const *  pBegin, sal_Unicode const *  pEnd) SAL_THROW(())
    {
        OSL_PRECOND(pBegin <= pEnd, "Invalid string range");

        sal_Int32 const nLen = pEnd-pBegin;
        sal_Int32 const nPos = rtl_ustr_lastIndexOfChar_WithLength(pBegin, nLen, c_cDelimiter) + 1;

        OSL_ASSERT(0 <= nPos && nPos <= nLen);

        return pBegin + nPos;
    }
//-----------------------------------------------------------------------------

    /** find the start of the bracketed & quoted predicate ending before pEnd in the string starting at pBegin
        @return
            <ul><li>a pointer to the opening bracket matching the closing bracket at pEnd[-1], if found</li>
            <li><var>pEnd</var>, if no bracketed string was found</li>
            <li>NULL, if there was a closing bracket, but the beginning could not be discovered</li></ul>
    */
    sal_Unicode const *  implFindPredicateStart(sal_Unicode const *  pBegin, sal_Unicode const *  pEnd) SAL_THROW(())
    {
        OSL_PRECOND(pBegin < pEnd, "Nonempty string range required");

        if (pEnd == pBegin || pEnd[-1] != c_rBracket) return pEnd;

        if (--pEnd == pBegin)
        {
            OSL_ENSURE(false, "Invalid path component: single ']'");
            return NULL; // string was only "]"
        }

        sal_Unicode chQuote = *--pEnd;

        if (chQuote != c_quot && chQuote != c_apos)
        {
            // should we support empty brackets ?
            if (chQuote == c_lBracket)
            {
                OSL_ENSURE(false, "Empty predicate brackets found");
                return NULL; // for now we don't

            }

            // should we support brackets with non-quoted strings ?
            chQuote = c_lBracket; // for now we do
        }

        sal_Int32 nStart = rtl_ustr_lastIndexOfChar_WithLength(pBegin, pEnd-pBegin, chQuote);

        if (chQuote != c_lBracket) // needed to support non-quoted strings
            --nStart;

        if (nStart < 0)
        {
            OSL_ENSURE(false, "Could not find opening quote or bracket for bracketed predicate");
            return NULL;
        }

        if (pBegin[nStart] != c_lBracket)
        {
            OSL_ENSURE(false, "Illegal quote character in string");
            return NULL; // for now we don't
        }

        return pBegin + nStart;
    }
//-----------------------------------------------------------------------------

    /// find the position of the given char in the range given.
    inline
    sal_Int32 indexOfCharInRange(sal_Unicode const *  pBegin, sal_Unicode const *  pEnd, sal_Unicode ch) SAL_THROW(())
    {
        return rtl_ustr_indexOfChar_WithLength(pBegin, pEnd-pBegin, ch);
    }
//-----------------------------------------------------------------------------

    /// find the position of the given char in the range given.
    inline
    bool containsChar(sal_Unicode const * pString, sal_Unicode ch) SAL_THROW(())
    {
        return rtl_ustr_indexOfChar(pString, ch) >= 0;
    }
//-----------------------------------------------------------------------------

    /** validate and normalize a bracketed & quoted predicate from content the string range [pBegin,pEnd)
        @param pRequiredEscapes
            contains a list of characters that must be preescaped or are otherwise invalid
            if NULL is passed, the source range is presumed to contain no escaped data
            otherwise the ampersand (&) and all characters in the list are required to be escaped
        @return
            the normalized, bracketed and quoted predicate
        @throw
            InvalidName, if the predicate data is not valid
    */
    rtl::OUString implMakeNormalizedPredicate(sal_Unicode const *  pBeginContent, sal_Unicode const *  pEndContent, sal_Unicode const* pRequiredEscapes) SAL_THROW((InvalidName))
    {
        OSL_PRECOND(pBeginContent <= pEndContent, "Invalid string range");
        if (pBeginContent == pEndContent)
            return rtl::OUString();

        rtl::OUStringBuffer aNormalized(pEndContent-pBeginContent + 4); // reserve approximate size initially

        // prefix: opening bracket and quote
        aNormalized.append(c_lBracket).append(c_normal_quot);

        // content: copy over each char and handle escaping
        for(sal_Unicode const *  pCur = pBeginContent; pCur != pEndContent; ++pCur)
        {
            sal_Unicode ch = *pCur;

            // maybe parse contained escaping
            if (pRequiredEscapes)
            {
                if (ch == c_amp)
                {
                    // find an escape end marker (after pCur). Result is pCur, if the end marker is not there
                    sal_Unicode const *  pEndEscape = pCur + 1 + indexOfCharInRange(pCur+1,pEndContent,c_end_escape);
                    sal_Unicode ch2   = pCur != pEndEscape ? implParseEscape(pCur,pEndEscape+1) : 0;

                    if (ch2 != 0) // found and read a valid escape sequence
                    {
                        ch = ch2;
                        pCur = pEndEscape;
                        OSL_ASSERT(*pCur == c_end_escape);
                    }
                    else
                    {
                        OSL_ENSURE(false, "Character '&' must be escaped in this context");
                        #if 0
                            throw InvalidName(rtl::OUString(pBeginContent,pEndContent-pBeginContent),
                                          "is not a valid element name string. "
                                          "Character '&' must be escaped in this context");
                        #endif
                    }
                }
                else if ( containsChar(pRequiredEscapes, ch) )
                {
                    throw InvalidName(rtl::OUString(pBeginContent,pEndContent-pBeginContent),
                                        "is not a valid element name string. "
                                        "Some characters must be escaped in this context");
                }
            }

            // now append (escape if normal)
            if (sal_Char const * pEscape = implGetEscape(ch))
                aNormalized.appendAscii( pEscape );

            else
                aNormalized.append( ch );
        }

        // suffix: closing quote and bracket
        aNormalized.append(c_normal_quot).append(c_rBracket);

        return aNormalized.makeStringAndClear();
    }
//-----------------------------------------------------------------------------

    /** extract and unescape the normalized predicate content in the string range [pBegin,pEnd)
        @return
            the denormalized predicate content
    */
    rtl::OUString implReadPredicate(sal_Unicode const *  pBegin, sal_Unicode const *  pEnd) SAL_THROW(())
    {
        OSL_PRECOND(pBegin <= pEnd, "Invalid string range");

        rtl::OUStringBuffer aContent(pEnd-pBegin); // reserve approximate size initially

        sal_Unicode const *  pReadPos = pBegin;

        // content: copy data, handling escapes
        for(sal_Unicode const *  pCur = pReadPos; pCur != pEnd; ++pCur)
        {
            if (*pCur != c_amp) continue; // no escape here

        // handle an escape
            // find an escape end marker (after pCur). Result is pCur, if the end marker is not there
            sal_Unicode const *  pEndEscape = pCur + 1 + indexOfCharInRange(pCur+1,pEnd,c_end_escape);

            OSL_ENSURE(pEndEscape != pCur, "Found dangling ampersand in normalized data");

            sal_Unicode ch = implParseEscape(pCur,pEndEscape+1);

            OSL_ENSURE(ch != 0, "Found unreckognized escape in normalized data");

            if (ch != 0) // found and read a valid escape sequence
            {
                // do copy of preceding data
                aContent.append(pReadPos, pCur-pReadPos).append(ch);
                pCur = pReadPos = pEndEscape;

                ++pReadPos;

                OSL_ASSERT(*pCur == c_end_escape);
            }
            // otherwise just treat the ampersand as a mormal character
        }

        // do copy of remaining data
        if (pReadPos != pEnd)
            aContent.append(pReadPos, pEnd-pReadPos);

        return aContent.makeStringAndClear();
    }
//-----------------------------------------------------------------------------

    /** validate and normalize the bracketed & quoted predicate in the string range [pBegin,pEnd)
        @return
            the normalized predicate
        @throw
            InvalidName, if the predicate is not valid
    */
    rtl::OUString implNormalizePredicate(sal_Unicode const *  pBegin, sal_Unicode const *  pEnd) SAL_THROW((InvalidName))
    {
        sal_Unicode sStopCharBuf[2];
        sal_Unicode const * pStopChars;

        OSL_PRECOND(pBegin < pEnd,          "Nonempty string range expected");
        OSL_PRECOND(pEnd-pBegin >= 2,       "Bracketed string range expected");
        OSL_PRECOND(pBegin[0] == c_lBracket,"Bracketed string range expected");
        OSL_PRECOND(pEnd[-1] == c_rBracket, "Bracketed string range expected");

        ++pBegin; --pEnd; // skip brackets

        sal_Unicode const chUsedQuot = *pBegin;
        if (chUsedQuot == c_apos || chUsedQuot == c_quot)
        {
            OSL_PRECOND(pBegin < pEnd && pEnd-pBegin >= 2,  "Bracketed quoted string range expected");
            OSL_PRECOND(pEnd[-1] == chUsedQuot,             "Non-matching quotes in bracketed quoted string");

            if (pEnd-pBegin <= 1 || pEnd[-1] != chUsedQuot)
                throw InvalidName( rtl::OUString(pBegin, pEnd-pBegin), "is not a valid element predicate: quotes do not match");

            ++pBegin; --pEnd; // skip quotes

            sStopCharBuf[0] = chUsedQuot;
            sStopCharBuf[1] = 0;

            pStopChars = sStopCharBuf;
        }

        // non-quoted strings are not really valid, but we tolerate them
        else
        {
            OSL_ENSURE(false, "Warning: Invalid path - non-quoted data in bracketed predicate");

            static sal_Unicode const sGeneralStoppers[] = { c_quot, c_apos, c_rBracket, c_lBracket, 0 };

            pStopChars = sGeneralStoppers;
        }

        if (pBegin == pEnd)
            throw InvalidName(rtl::OUString(pBegin-1,2),"Empty element name in predicate");

        return implMakeNormalizedPredicate(pBegin, pEnd, pStopChars);
    }
//-----------------------------------------------------------------------------
    /// parse a path into a sequence of components
    Path::Rep implParsePath(rtl::OUString const& _aPathString, PathType eType) SAL_THROW((InvalidName))
    {
        Path::Rep aResult;

        dprint (stderr, "implParsePath '%s' ",
                 rtl::OUStringToOString(_aPathString, RTL_TEXTENCODING_UTF8).getStr());

        sal_Unicode const *  pBegin = _aPathString.getStr();
        sal_Unicode const *  pEnd   = pBegin + _aPathString.getLength();

        if (eType == eABSOLUTE)
        {
            if ( detectAbsolutePath(_aPathString) )
                ++pBegin; // skip the leading slash

#ifdef CFG_PATH_STRICT
            else
                OSL_ENSURE(false, "Warning: trying to parse relative path as absolute");
#endif
        }
        else
            OSL_ENSURE(!detectAbsolutePath(_aPathString), "ERROR: trying to parse absolute path as relative one");

        if (detectAbsolutePath(pBegin))
            throw InvalidName(_aPathString, "is not a valid path. Illegal empty first component");

        else if (pBegin != pEnd && pEnd[-1] == '/')
        {
#ifdef CFG_PATH_STRICT
            OSL_ENSURE(false, "Illegal configuration path. Terminating '/' found.");
#endif
            --pEnd;
        }

        while (pEnd != pBegin)
        {
            // check for predicate
            sal_Unicode const *  pQuoteStart = implFindPredicateStart(pBegin, pEnd);
            if (pQuoteStart == NULL)
                throw InvalidName(_aPathString, "is not a valid path. Invalid name or predicate syntax");

            sal_Unicode const *  pNameStart = implFindNameStart(pBegin, pQuoteStart);

            rtl::OUString aElementName(pNameStart, pQuoteStart-pNameStart);

            if (!isSimpleName(aElementName))
            {
                // this is OK only for few cases WITH predicate
                if (pQuoteStart == pEnd)
                    throw InvalidName(_aPathString, "is not a valid path. Invalid name");

                if (isEmptyString(aElementName))
                    aElementName = makeWildcardType();

                else if ( !isWildcardType(aElementName))
                    throw InvalidName(_aPathString, "is not a valid path. Invalid type tag for predicate");
            }
            if (pQuoteStart != pEnd)
            {
                dprint (stderr, "add 'normalize predicate'", "");
                rtl::OUString aPred = implNormalizePredicate(pQuoteStart,pEnd);
                aElementName += aPred;
                dprint (stderr, " [result pred '%s']",
                         rtl::OUStringToOString(aPred, RTL_TEXTENCODING_UTF8).getStr());
            }

            aResult.prepend( Path::Component(aElementName) );

            pEnd = pNameStart;
            if (pNameStart != pBegin) --pEnd;
        }
        dprint (stderr, "\n", "");
        return aResult;
    }
//-----------------------------------------------------------------------------

    /// build a composite path component from a base name (type) and a (somewhat optional) predicate
    rtl::OUString implMakeCompositeName(rtl::OUString const& _sBaseName, rtl::OUString const& _sPredicate) SAL_THROW((InvalidName))
    {
        rtl::OUString sComposite(_sBaseName);

        if (isEmptyString(_sBaseName))
            sComposite = makeWildcardType();

        else if (!isWildcardType(_sBaseName) && !isSimpleName(_sBaseName))
            throw InvalidName(_sBaseName, "The base-name (type) part of a composite node name must be a simple word");

        dprint (stderr, "implMakeNormalizePred '%s' ",
                rtl::OUStringToOString(_sPredicate, RTL_TEXTENCODING_UTF8).getStr());

        sal_Unicode const *  pPredStart = _sPredicate.getStr();
        sal_Unicode const *  pPredEnd   = pPredStart + _sPredicate.getLength();

        if (pPredStart != pPredEnd)
            sComposite += implMakeNormalizedPredicate(pPredStart, pPredEnd, NULL);

        dprint (stderr, " [result pred '%s']\n",
                rtl::OUStringToOString(sComposite, RTL_TEXTENCODING_UTF8).getStr());

        return sComposite;
    }
//-----------------------------------------------------------------------------

    /// split a composite path component into a base name (type) and a predicate (if present)
    void implSplitCompositeName(rtl::OUString const& _aCompositeName, rtl::OUString& _rBaseName, rtl::OUString& _rPredicate) SAL_THROW(())
    {
        sal_Int32 nPos = _aCompositeName.indexOf(c_lBracket);

        if (nPos >= 0)
        {
            OSL_ENSURE( nPos > 0, "Invalid name: Only predicate, no base type");

            _rBaseName = _aCompositeName.copy(0,nPos);

            sal_Unicode const *  pBeginPred = _aCompositeName.getStr() + nPos;
            sal_Unicode const *  pEndPred   = _aCompositeName.getStr() + _aCompositeName.getLength();

            OSL_ASSERT(pBeginPred[0] == c_lBracket);
            OSL_ENSURE(pBeginPred[1] == c_normal_quot, "Missing or unexpected quote mark");
            OSL_ENSURE(pEndPred[-1] == c_rBracket, "Invalid name: Predicate brackets not closed");
            OSL_ENSURE(pEndPred[-2] == c_normal_quot, "Missing or unexpected quote mark");

            // skip brackets and quotes - then read data
            _rPredicate = implReadPredicate(pBeginPred+2, pEndPred-2);
        }
        else
        {
            OSL_ENSURE( _aCompositeName.indexOf(c_rBracket) < 0, "Invalid name: Predicate brackets not opened");
            _rBaseName = _aCompositeName;
            _rPredicate = rtl::OUString();
        }
    }
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class RelativePath
//-----------------------------------------------------------------------------

// Currently unused method to check/ensure validity
void RelativePath::init() SAL_THROW(())
{
}
//-----------------------------------------------------------------------------

RelativePath RelativePath::parse(rtl::OUString const& aString)
{
    return RelativePath( implParsePath(aString, eRELATIVE) );
}
//-----------------------------------------------------------------------------

RelativePath::RelativePath(Path::Component const& aName) SAL_THROW(())
: m_aRep(aName)
{
    if (aName.isEmpty()) m_aRep.clearComponents();
}
//-----------------------------------------------------------------------------

RelativePath RelativePath::compose(RelativePath const& aPath) const SAL_THROW(())
{
    Path::Rep aResult = aPath.rep();
    aResult.prepend( this->m_aRep );
    return RelativePath( aResult );
}
//-----------------------------------------------------------------------------
rtl::OUString RelativePath::toString() const SAL_THROW(())
{
    return m_aRep.toString(false);
}

//-----------------------------------------------------------------------------
// class AbsolutePath
//-----------------------------------------------------------------------------

// Currently unused method to check/ensure validity
void AbsolutePath::init() SAL_THROW(())
{
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::parse(rtl::OUString const& aString)
{
    return AbsolutePath( implParsePath(aString, eABSOLUTE) );
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::root() SAL_THROW(())
{
    return AbsolutePath( Path::Rep() );
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::detachedRoot() SAL_THROW(())
{
    Path::Rep aRep( Path::makeEmptyComponent() ); // use 1 empty component here, to start detached names
    return AbsolutePath( aRep );
}
//-----------------------------------------------------------------------------

static inline Path::Component implMakeSafeModuleName(rtl::OUString const& _sModuleName) SAL_THROW(())
{
    OSL_ENSURE( isSimpleName(_sModuleName), "A module name must be a simple name");

    // if (isSimpleName(_sModuleName)) sModuleName = escape_name( _sModuleName );

    return Path::Component(_sModuleName);
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::makeModulePath(rtl::OUString const& _sModuleName) SAL_THROW(())
{
    return AbsolutePath( Path::Rep( implMakeSafeModuleName(_sModuleName) ) );
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::compose(RelativePath const& aPath) const SAL_THROW(())
{
    Path::Rep aResult = aPath.rep();
    aResult.prepend( this->m_aRep );
    return AbsolutePath( aResult );
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::getParentPath() const
{
    // or: m_aRep.check_not_empty();
    OSL_ENSURE(!isRoot(), "ERROR: Requesting the parent of a root path");
    if (isRoot()) return *this;

    OSL_ENSURE(!isDetached(), "ERROR: Requesting the parent of a detached path");

    return AbsolutePath( Path::Rep(begin(),end()-1) );
}
#if OSL_DEBUG_LEVEL > 0
//-----------------------------------------------------------------------------

bool AbsolutePath::isDetached() const SAL_THROW(())
{
    return !m_aRep.isEmpty() && begin()->isEmpty();
}
#endif
//-----------------------------------------------------------------------------

rtl::OUString AbsolutePath::toString() const SAL_THROW(())
{
    return m_aRep.toString(true);
}
//-----------------------------------------------------------------------------
    }
}

