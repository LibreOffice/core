/*************************************************************************
 *
 *  $RCSfile: configpath.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 09:04:02 $
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

#include "configpath.hxx"
#include "configexcept.hxx"

#include <algorithm>

#define OLD_SETELEMENT_NAMES 1

namespace configmgr
{
    namespace configuration
    {

//-----------------------------------------------------------------------------
        const sal_Unicode c_cDelimiter = '/';
        const sal_Unicode c_quot = '\"';
        const sal_Unicode c_apos = '\'';

        const sal_Unicode c_lBracket = '[', c_rBracket = ']';

        const sal_Size c_nBraLen = 2;
        const sal_Unicode c_lBraQuot[c_nBraLen] = { c_lBracket, c_quot };
        const sal_Unicode c_rBraQuot[c_nBraLen] = { c_quot, c_rBracket };
        const sal_Unicode c_lBraApos[c_nBraLen] = { c_lBracket, c_apos };
        const sal_Unicode c_rBraApos[c_nBraLen] = { c_apos, c_rBracket };

//-----------------------------------------------------------------------------
namespace
{
    //-------------------------------------------------------------------------
    // Helper functions for name validation
    //-------------------------------------------------------------------------
    bool isBracketed(OUString const& sName)
    {
#if OLD_SETELEMENT_NAMES
        return false;
#endif
        sal_Int32 nLen = sName.getLength();
        return sName.getLength() > 2*c_nBraLen &&

                sName[0]        == c_lBracket &&
                sName[nLen-1]   == c_rBracket &&
                ((sName[1] == c_quot && sName[nLen-2] == c_quot) ||
                 (sName[1] == c_apos && sName[nLen-2] == c_apos) );

    }

    //-------------------------------------------------------------------------
    inline OUString makeBrace(sal_Unicode const (&chars)[c_nBraLen])
    { return OUString(chars,c_nBraLen); }

    OUString bracket(OUString const& sName)
    {
#if OLD_SETELEMENT_NAMES
        return OUString();
#endif
        if (sName.indexOf(c_cDelimiter) >= 0)
            return OUString();

        else if (isBracketed(sName))
            return sName;

        else if (sName.indexOf(c_quot) < 0)
            return makeBrace(c_lBraQuot) += OUString(sName) += makeBrace(c_rBraQuot);

        else if (sName.indexOf(c_quot) < 0)
            return makeBrace(c_lBraQuot) += OUString(sName) += makeBrace(c_rBraQuot);

        else
            return OUString();
    }

    //-------------------------------------------------------------------------
    OUString unbracket(OUString const& sName)
    {
        if (sName.indexOf(c_cDelimiter) >= 0)
            return OUString();

        else if (isBracketed(sName))
            return sName.copy(c_nBraLen,sName.getLength()-c_nBraLen);

        else
            return sName;
    }

    //-------------------------------------------------------------------------
    inline
    bool isValidNameStart(sal_Unicode ch)
    {
        return (sal_Unicode('A') <= ch && ch <= sal_Unicode('Z')) ||
               (sal_Unicode('a') <= ch && ch <= sal_Unicode('z')) ||
                sal_Unicode('_') == ch;
    }
    inline
    bool isValidNameCont(sal_Unicode ch)
    {
        return  (   (sal_Unicode('0') <= ch && ch <= sal_Unicode('9'))
                ||  (sal_Unicode('-') == ch)
                ||  (sal_Unicode('.') == ch)
                );
    }

    //-------------------------------------------------------------------------
    bool isValidName(OUString const& sName)
    {
        sal_Unicode const* const pStr = sName.getStr();
        sal_Unicode const* const pEnd = pStr + sName.getLength();

        for (sal_Unicode const* pValidate = pStr; pValidate != pEnd; ++pValidate)
        {
            if (!isValidNameStart(*pValidate))
            {
                if (pValidate == pStr)
                    return isBracketed(sName) && sName.indexOf(c_cDelimiter) < 0;
                else if (!isValidNameCont(*pValidate))
                    return false;
            }
        }

        return true;
    }
    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------
// class configuration::Name
//-----------------------------------------------------------------------------

// Performance: Could optimize memory usage by using a string pool
Name::Name(OUString const& aString, NoValidate)
: m_sRep(aString)
{

}
//-----------------------------------------------------------------------------

Name makeName(OUString const& sName)
{
    return Name( unbracket(sName), Name::NoValidate() );
}
//-----------------------------------------------------------------------------

Name validateNodeName(OUString const& sName)
{
    if (!isValidName(sName))
        throw InvalidName(sName, "is not a valid configuration name");
    if (isBracketed(sName))
        throw InvalidName(sName, "is not a valid name for an configuration Node (bracketed names are not allowed");

    return Name( sName, Name::NoValidate() );
}
//-----------------------------------------------------------------------------

Name validateElementName(OUString const& sName)
{
    if (!isValidName(sName))
        throw InvalidName(sName, "is not a valid configuration name");

    return Name( unbracket(sName), Name::NoValidate() );
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class configuration::PathRep
//-----------------------------------------------------------------------------

PathRep::PathRep(Components const& aComponents)
: m_aComponents(aComponents)
{
}
//-----------------------------------------------------------------------------

PathRep PathRep::compose(PathRep const& aRelativePath) const
{
    Components aResult( components() );
    aResult.insert(aResult.end(), aRelativePath.begin(), aRelativePath.end());
    return PathRep( aResult );
}
//-----------------------------------------------------------------------------

Name PathRep::getLocalName() const
{
    if (isEmpty())
        return Name();
    else
        return components().back();
}

//-----------------------------------------------------------------------------

PathRep PathRep::child(Name const& aChild) const
{
    Components aResult( components() );
    aResult.push_back(aChild);
    return PathRep( aResult );
}
//-----------------------------------------------------------------------------

PathRep PathRep::parent() const
{
    Components aResult( components() );
    OSL_ENSURE( !aResult.empty(), "ERROR: Getting parent path of empty path" );
    if (aResult.empty())
        throw InvalidName(OUString(RTL_CONSTASCII_USTRINGPARAM("<empty>")), " - is empty. Cannot construct parent path");

    aResult.pop_back();
    return PathRep( aResult );
}
//-----------------------------------------------------------------------------

OUString PathRep::toString() const
{
    OUString const sDelimiter(&c_cDelimiter, 1);

    Iterator cur = begin();
    Iterator const stop = end();

    OUString sRet;
    if (cur != stop)
    {
        sRet = cur->toString();

        while(++cur != stop)
            sRet += sDelimiter.concat( cur->toString() );

        if ((--cur)->isEmpty()) // special behavior for root & friends
            sRet += sDelimiter;
    }
    return sRet;
}
//-----------------------------------------------------------------------------

size_t PathRep::hashCode() const
{
    const unsigned long mangle_factor = 11; // 1011 (2)
    unsigned long nHash = 0;
    for (Iterator it = begin(), stop = end(); it != stop; ++it)
    {
        nHash = mangle_factor*nHash + it->hashCode();
    }
    return nHash;
}
//-----------------------------------------------------------------------------

bool PathRep::before(PathRep const& rhs) const
{
    return std::lexicographical_compare(this->begin(),this->end(),rhs.begin(),rhs.end());
}
//-----------------------------------------------------------------------------

bool operator==(PathRep const& lhs, PathRep const& rhs)
{
    return (lhs.m_aComponents.size() != rhs.m_aComponents.size()) &&
            std::equal(lhs.begin(),lhs.end(),rhs.begin());
}
//-----------------------------------------------------------------------------

namespace
{
    // Absolute path starts its rep with an empty name
    bool isAbsolute(PathRep const& aRep)
    {
        return !aRep.isEmpty() &&
                aRep.begin()->isEmpty();
    }
    // Textually Absolute path starts with a slash
    bool isAbsolute(OUString const& sPath)
    {
        return  sPath.getLength() != 0 &&
                sPath.getStr()[0] == c_cDelimiter;
    }
    PathRep::Components implParsePath(OUString const& aString, Path::Type eType)
    {
        PathRep::Components aRet;

        sal_Int32 nPos = 0;
        sal_Int32 const nEnd = aString.getLength();

        if (isAbsolute(aString))
        {
            OSL_ENSURE(eType == PathType::eABSOLUTE || eType == PathType::ePATH,"Configuration: PARSE WARNING: Absolute Path String provided in an inapplicable context");
            eType = PathType::eABSOLUTE;
            ++nPos;
        }

        if (eType == PathType::eABSOLUTE)
            aRet.push_back(Name());

        while(nPos < nEnd)
        {
            sal_Int32 nCut = aString.indexOf(c_cDelimiter, nPos);
            if (nCut < 0) nCut = nEnd;

            OSL_ENSURE(nPos < nCut, "Configuration: PARSE WARNING: Invalid Path containing empty component");

            aRet.push_back(makeName(aString.copy(nPos,nCut-nPos)));
            nPos = nCut + 1;
        }

        return aRet;
    }
}

//-----------------------------------------------------------------------------
// class RelativePath
//-----------------------------------------------------------------------------

// Absolute path starts its rep with an empty name
// we must ensure that there is no empty name for a relative path
// see AbsolutePath::init
void RelativePath::init()
{
    if (!m_aRep.isEmpty())
    {
        OSL_ENSURE(!m_aRep.begin()->isEmpty(), "WARNING: Wrong representation passed to RelativePath");
        while(begin()->isEmpty()) // but now correcting this condition
        {
            m_aRep.removeComponent(m_aRep.begin_mutate());
        }
    }
    OSL_ASSERT( !isAbsolute(m_aRep) );
}
//-----------------------------------------------------------------------------

RelativePath::RelativePath()
: m_aRep( Components() )
{
}
//-----------------------------------------------------------------------------

RelativePath::RelativePath(OUString const& aString, NoValidate)
: m_aRep(implParsePath(aString, PathType::eRELATIVE))
{
    init();
}
//-----------------------------------------------------------------------------

RelativePath::RelativePath(Name const& aName)
: m_aRep( Components(1,aName) )
{
    if (aName.isEmpty()) m_aRep.clearComponents();
    OSL_ASSERT( !isAbsolute(m_aRep) );
}
//-----------------------------------------------------------------------------

RelativePath RelativePath::compose(RelativePath const& aPath) const
{
    return RelativePath( m_aRep.compose(aPath.rep()) );
}
//-----------------------------------------------------------------------------

RelativePath RelativePath::child(Name const& aChild) const
{
    return RelativePath( m_aRep.child(aChild) );
}
//-----------------------------------------------------------------------------

RelativePath RelativePath::parent() const
{
    return RelativePath( m_aRep.parent() );
}
//-----------------------------------------------------------------------------

OUString RelativePath::toString() const
{
    return m_aRep.toString();
}

//-----------------------------------------------------------------------------
// class AbsolutePath
//-----------------------------------------------------------------------------

// Starts the rep with an empty name
// - this also ensures correct toString() behavior
// - skipping this extra element is done using REP_OFF
void AbsolutePath::init()
{
    if (!isAbsolute(m_aRep))
        m_aRep.insertComponent(m_aRep.begin_mutate());
    OSL_ASSERT( isAbsolute(m_aRep) );
}
//-----------------------------------------------------------------------------

AbsolutePath::AbsolutePath(OUString const& aString, NoValidate)
: m_aRep(implParsePath(aString, PathType::eABSOLUTE))
{
    init();
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::root()
{
    Components aRep(1);
    return AbsolutePath(aRep);
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::detachedRoot()
{
    Components aRep(2);
    return AbsolutePath(aRep);
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::compose(RelativePath const& aPath) const
{
    return AbsolutePath( m_aRep.compose(aPath.rep()));
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::child(Name const& aChild) const
{
    return AbsolutePath( m_aRep.child(aChild) );
}
//-----------------------------------------------------------------------------

AbsolutePath AbsolutePath::parent() const
{
    return AbsolutePath( m_aRep.parent() );
}
//-----------------------------------------------------------------------------

bool AbsolutePath::isRoot() const
{
    return m_aRep.countComponents() == REP_OFF;
}
//-----------------------------------------------------------------------------

OUString AbsolutePath::toString() const
{
    return m_aRep.toString();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// class Path
//-----------------------------------------------------------------------------

PathRep::Components Path::parse(OUString const& aString,  Type eType)
{
    return implParsePath(aString, eType);
}
//-----------------------------------------------------------------------------

void Path::init()
{
    using namespace PathType;
    if (m_eType == ePATH)
    {
        m_eType =   isAbsolute(m_aRep)              ? eABSOLUTE
                    :m_aRep.countComponents() == 1  ? eNAME
                                                    : eRELATIVE;
    }

    OSL_ASSERT( isAbsolute(m_aRep) == (m_eType == eABSOLUTE) );
}
//-----------------------------------------------------------------------------

Path::Path(OUString const& aString, NoValidate, Type eType)
: m_aRep(implParsePath(aString,eType))
, m_eType(eType)
{
    init();
}
//-----------------------------------------------------------------------------

Path::Path(PathRep const& aRep, Type eType)
: m_aRep(aRep)
, m_eType(eType)
{
    init();
}
//-----------------------------------------------------------------------------

Path::Path(Name const& aName)
: m_aRep( Components(1,aName) )
, m_eType(PathType::eNAME)
{
    OSL_ASSERT( !isAbsolute(m_aRep) );
}
//-----------------------------------------------------------------------------

Path::Path(AbsolutePath const& aPath)
: m_aRep(aPath.rep())
, m_eType(PathType::eABSOLUTE)
{
    OSL_ASSERT( isAbsolute(m_aRep) );
}
//-----------------------------------------------------------------------------

Path::Path(RelativePath const& aPath)
: m_aRep(aPath.rep())
, m_eType(PathType::eRELATIVE)
{
    OSL_ASSERT( !isAbsolute(m_aRep) );
}
//-----------------------------------------------------------------------------

Path Path::compose(RelativePath const& aPath) const
{
    return Path( m_aRep.compose(aPath.rep()), m_eType);
}
//-----------------------------------------------------------------------------

Path Path::child(Name const& aChild) const
{
    return Path( m_aRep.child(aChild), m_eType );
}
//-----------------------------------------------------------------------------

Path Path::parent() const
{
    return Path( m_aRep.parent(), m_eType);
}
//-----------------------------------------------------------------------------

OUString Path::toString() const
{
    return m_aRep.toString();
}

//-----------------------------------------------------------------------------
    }
}

