/*************************************************************************
 *
 *  $RCSfile: nmspmap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:31:44 $
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

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <nmspmap.hxx>
#endif

using namespace rtl;

typedef OUString *OUStringPtr;
SV_DECL_PTRARR_DEL( OUStringsDtor_Impl, OUStringPtr, 5, 5 )
SV_IMPL_PTRARR( OUStringsDtor_Impl, OUStringPtr )

SvXMLNamespaceMap::SvXMLNamespaceMap() :
    sXMLNS( OUString::createFromAscii(sXML_xmlns) ),
    pPrefixes( new OUStringsDtor_Impl ),
    pNames( new OUStringsDtor_Impl )
{
}

SvXMLNamespaceMap::SvXMLNamespaceMap( const SvXMLNamespaceMap& rMap ) :
    sXMLNS( OUString::createFromAscii(sXML_xmlns) ),
    pPrefixes( new OUStringsDtor_Impl ),
    pNames( new OUStringsDtor_Impl )
{
    USHORT nCount = rMap.aKeys.Count();
    for( USHORT i=0; i<nCount; i++ )
    {
        pPrefixes->Insert( new OUString( *(*rMap.pPrefixes)[i] ), i );
        pNames->Insert( new OUString( *(*rMap.pNames)[i] ), i );
        aKeys.Insert( rMap.aKeys[i], i );
    }
}

SvXMLNamespaceMap::~SvXMLNamespaceMap()
{
    delete pPrefixes;
    delete pNames;
}

int SvXMLNamespaceMap::operator ==( const SvXMLNamespaceMap& rCmp ) const
{
    BOOL bRet = aKeys.Count() == rCmp.aKeys.Count();
    if( bRet )
    {
        USHORT nCount = aKeys.Count();
        USHORT i;
        for( i=0; bRet && i<nCount; i++ )
            bRet = aKeys[i] == rCmp.aKeys[i];

        for( i=0; bRet && i<nCount; i++ )
            bRet = *(*pPrefixes)[i] == *(*rCmp.pPrefixes)[i] &&
                   *(*pNames)[i] == *(*rCmp.pNames)[i];
    }

    return (int)bRet;
}

USHORT SvXMLNamespaceMap::Add( const OUString& rPrefix, const OUString& rName,
                               USHORT nKey )
{
    if( XML_NAMESPACE_UNKNOWN==nKey )
        nKey = GetKeyByName( rName );

    DBG_ASSERT( XML_NAMESPACE_NONE != nKey,
                "SvXMLNamespaceMap::Add: invalid namespace key" );
    if( XML_NAMESPACE_NONE==nKey )
        return USHRT_MAX;

    USHORT nCount = aKeys.Count();
    USHORT i;
    for( i=0; i<nCount; i++ )
    {
        if( 0 == (*pPrefixes)[i]->getLength() ||
            *(*pPrefixes)[i] == rPrefix )
        {
            *(*pPrefixes)[i] = rPrefix;
            *(*pNames)[i] = rName;
            aKeys[i] = nKey;

            break;
        }
    }

    if( i == aKeys.Count() )
    {
        pPrefixes->Insert( new OUString(rPrefix), i );
        pNames->Insert( new OUString(rName), i );
        aKeys.Insert( nKey, i );
    }

    return i;
}

BOOL SvXMLNamespaceMap::AddAtIndex( USHORT nIdx, const OUString& rPrefix,
                                    const OUString& rName, USHORT nKey )
{
    if( XML_NAMESPACE_UNKNOWN==nKey )
        nKey = GetKeyByName( rName );

    DBG_ASSERT( XML_NAMESPACE_NONE != nKey,
                "SvXMLNamespaceMap::AddAtIndex: invalid namespace key" );
    if( XML_NAMESPACE_NONE==nKey )
        return FALSE;

    BOOL bAdded = TRUE;
    USHORT nCount = aKeys.Count();
    if( nIdx >= nCount )
    {
        USHORT i;
        for( i=nCount; i<nIdx; i++ )
        {
            pPrefixes->Insert( new OUString, i );
            pNames->Insert( new OUString, i );
            aKeys.Insert( XML_NAMESPACE_UNKNOWN, i );
        }
        pPrefixes->Insert( new OUString(rPrefix), nIdx );
        pNames->Insert( new OUString(rName), nIdx );
        aKeys.Insert( nKey, i );
    }
    else
    {
        if( 0 == (*pPrefixes)[nIdx]->getLength() )
        {
            *(*pPrefixes)[nIdx] = rPrefix;
            *(*pNames)[nIdx] = rName;
            aKeys[nIdx] = nKey;
        }
        else
        {
            bAdded = *(*pPrefixes)[nIdx] == rPrefix &&
                   *(*pNames)[nIdx] == rName &&
                   aKeys[nIdx] == nKey;
            DBG_ASSERT( bAdded,
        "SvXMLNamespaceMap::AddByIndex: reuse of indices is not supported" );
        }
    }

    return bAdded;
}

BOOL SvXMLNamespaceMap::AddAtIndex( USHORT nIdx, const sal_Char *pPrefix,
                                     const sal_Char *pName, USHORT nKey )
{
    OUString sPrefix( OUString::createFromAscii(pPrefix) );
    OUString sName( OUString::createFromAscii(pName) );

    return AddAtIndex( nIdx, sPrefix, sName, nKey );
}

USHORT SvXMLNamespaceMap::GetIndexByKey( USHORT nKey ) const
{
    USHORT nIdx = USHRT_MAX;
    if( XML_NAMESPACE_UNKNOWN != nKey )
    {
        if( nKey < aKeys.Count() && aKeys[nKey] == nKey )
        {
            nIdx = nKey;
        }
        else
        {
            for( USHORT i=0; i<aKeys.Count(); i++ )
            {
                if( aKeys[i] == nKey )
                {
                    nIdx = i;
                    break;
                }
            }
        }
    }

    return nIdx;
}

USHORT SvXMLNamespaceMap::GetIndexByPrefix( const OUString& rPrefix ) const
{
    USHORT nIdx = USHRT_MAX;
    for( USHORT i=0; i<pPrefixes->Count(); i++ )
    {
        if( *(*pPrefixes)[i] == rPrefix )
        {
            nIdx = i;
            break;
        }
    }

    return nIdx;
}

USHORT SvXMLNamespaceMap::GetKeyByName( const OUString& rName ) const
{
    USHORT nKey = XML_NAMESPACE_UNKNOWN;

    for( USHORT i=0; i<aKeys.Count(); i++ )
    {
        if( rName == *(*pNames)[i] )
        {
            nKey = aKeys[i];
            break;
        }
    }

    return nKey;
}

const OUString& SvXMLNamespaceMap::GetPrefixByIndex( USHORT nIdx ) const
{
    return pPrefixes ? *(*pPrefixes)[nIdx] : sEmpty;
}

const OUString& SvXMLNamespaceMap::GetNameByIndex( USHORT nIdx ) const
{
    return pNames ? *(*pNames)[nIdx] : sEmpty;
}

USHORT SvXMLNamespaceMap::GetKeyByIndex( USHORT nIdx ) const
{
    return aKeys[nIdx];
}

OUString SvXMLNamespaceMap::GetAttrNameByIndex( USHORT nIdx ) const
{
    DBG_ASSERT( nIdx < aKeys.Count(),
                "SvXMLNamespaceMap::GetAttrNameByIndex: invalid index" );
    OUStringBuffer sAttrName;
    if( nIdx < aKeys.Count() )
    {
        sAttrName.append( sXMLNS  );
        sAttrName.append( sal_Unicode(':') );
        sAttrName.append( *(*pPrefixes)[nIdx] );
    }

    return sAttrName.makeStringAndClear();
}

OUString SvXMLNamespaceMap::GetQNameByIndex( USHORT nIdx,
                                           const OUString& rLocalName ) const
{
    DBG_ASSERT( nIdx < aKeys.Count() || USHRT_MAX == nIdx,
                "SvXMLNamespaceMap::GetQNameByIndex: invalid index" );

    OUStringBuffer sQName;
    if( nIdx < aKeys.Count() )
    {
        sQName.append( *(*pPrefixes)[nIdx] );
        sQName.append( sal_Unicode(':') );
    }
    if( nIdx < aKeys.Count() || USHRT_MAX == nIdx )
        sQName.append( rLocalName );

    return sQName.makeStringAndClear();;
}

USHORT SvXMLNamespaceMap::GetKeyByAttrName( const OUString& rAttrName,
                                            OUString *pPrefix,
                                            OUString *pLocalName,
                                            OUString *pNamespace,
                                            USHORT nIdxGuess ) const
{
    USHORT nKey = XML_NAMESPACE_UNKNOWN;

    sal_Int32 nColonPos = rAttrName.indexOf( sal_Unicode(':') );
    if( -1L != nColonPos )
    {
        if( pPrefix )
            *pPrefix = rAttrName.copy( 0L, nColonPos );
        if( pLocalName )
            *pLocalName = rAttrName.copy( nColonPos + 1L );

        OUString aPrefix( rAttrName.copy( 0L, nColonPos ) );
        if( nIdxGuess < aKeys.Count() && *(*pPrefixes)[nIdxGuess] == aPrefix )
        {
            nKey = aKeys[nIdxGuess];
            if( pNamespace )
                *pNamespace = *(*pNames)[nIdxGuess];
        }
        else
        {
            for( USHORT i=0; i<aKeys.Count(); i++ )
            {
                if( *(*pPrefixes)[i] == aPrefix )
                {
                    nKey = aKeys[i];
                    if( pNamespace )
                        *pNamespace = *(*pNames)[i];
                    break;
                }
            }
            if( aKeys.Count() == i && sXMLNS == aPrefix )
                nKey = XML_NAMESPACE_XMLNS;
        }
    }
    else
    {
        nKey = XML_NAMESPACE_NONE;
        if( pPrefix )
            *pPrefix = sEmpty;
        if( pLocalName )
            *pLocalName = rAttrName;
        if( pNamespace )
            *pNamespace = sEmpty;
    }

    return nKey;
}

USHORT SvXMLNamespaceMap::GetFirstIndex() const
{
    USHORT nIdx = 0;
    while( nIdx < aKeys.Count() && 0 == (*pPrefixes)[nIdx]->getLength() )
        nIdx++;

    return nIdx == aKeys.Count() ? USHRT_MAX : nIdx;
}

USHORT SvXMLNamespaceMap::GetNextIndex( USHORT nOldIdx ) const
{
    if( nOldIdx >= aKeys.Count() )
        return USHRT_MAX;

    nOldIdx++;
    while( nOldIdx < aKeys.Count() && 0 == (*pPrefixes)[nOldIdx]->getLength() )
        nOldIdx++;

    return nOldIdx == aKeys.Count() ? USHRT_MAX : nOldIdx;
}



