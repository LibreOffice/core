/*************************************************************************
 *
 *  $RCSfile: XMLTextListAutoStylePool.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-31 09:00:40 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CNTRSRT_HXX
#include <svtools/cntnrsrt.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLNUME_HXX
#include "xmlnume.hxx"
#endif
#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#include "XMLTextListAutoStylePool.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;


int XMLTextListAutoStylePoolNameCmp_Impl( const OUString& r1,
                                           const OUString& r2 )
{
    return (int)r1.compareTo( r2 );
}

DECLARE_CONTAINER_SORT_DEL( XMLTextListAutoStylePoolNames_Impl,
                            OUString )
IMPL_CONTAINER_SORT( XMLTextListAutoStylePoolNames_Impl,
                     OUString,
                     XMLTextListAutoStylePoolNameCmp_Impl )

class XMLTextListAutoStylePoolEntry_Impl
{
    OUString    sName;
    OUString    sInternalName;
    Reference < XIndexReplace > xNumRules;
    sal_uInt32  nPos;
    sal_Bool    bIsNamed;


public:

    XMLTextListAutoStylePoolEntry_Impl(
            sal_uInt32 nPos,
            const Reference < XIndexReplace > & rNumRules,
            XMLTextListAutoStylePoolNames_Impl& rNames,
            const OUString& rPrefix,
            sal_uInt32& rName );

    XMLTextListAutoStylePoolEntry_Impl(
            const Reference < XIndexReplace > & rNumRules ) :
        nPos( 0 ),
        xNumRules( rNumRules ),
        bIsNamed( sal_False )
    {
        Reference < XNamed > xNamed( xNumRules, UNO_QUERY );
        if( xNamed.is() )
        {
            sInternalName = xNamed->getName();
            bIsNamed = sal_True;
        }
    }

    XMLTextListAutoStylePoolEntry_Impl(
            const OUString& rInternalName ) :
        nPos( 0 ),
        sInternalName( rInternalName ),
        bIsNamed( sal_True )
    {
    }

    const OUString& GetName() const { return sName; }
    const OUString& GetInternalName() const { return sInternalName; }
    const Reference < XIndexReplace > & GetNumRules() const { return xNumRules; }
    sal_uInt32 GetPos() const { return nPos; }
    sal_Bool IsNamed() const { return bIsNamed; }
};

XMLTextListAutoStylePoolEntry_Impl::XMLTextListAutoStylePoolEntry_Impl(
        sal_uInt32 nP,
        const Reference < XIndexReplace > & rNumRules,
        XMLTextListAutoStylePoolNames_Impl& rNames,
        const OUString& rPrefix,
        sal_uInt32& rName ) :
    nPos( nP ),
    xNumRules( rNumRules ),
    bIsNamed( sal_False )
{
    Reference < XNamed > xNamed( xNumRules, UNO_QUERY );
    if( xNamed.is() )
    {
        sInternalName = xNamed->getName();
        bIsNamed = sal_True;
    }

    // create a name that hasn't been used before. The created name has not
    // to be added to the array, because it will never tried again
    OUStringBuffer sBuffer( 7 );
    do
    {
        rName++;
        sBuffer.append( rPrefix );
        sBuffer.append( (sal_Int32)rName );
        sName = sBuffer.makeStringAndClear();
    }
    while( rNames.Seek_Entry( &sName, 0 ) );
}

int XMLTextListAutoStylePoolEntryCmp_Impl(
        const XMLTextListAutoStylePoolEntry_Impl& r1,
        const XMLTextListAutoStylePoolEntry_Impl& r2 )
{
    int nRet;
    if( r1.IsNamed() )
    {
        if( r2.IsNamed() )
             nRet = (int)r1.GetInternalName().compareTo( r2.GetInternalName());
        else
            nRet = -1;
    }
    else
    {
        if( r2.IsNamed() )
            nRet = 1;
        else
             nRet = (int)(r1.GetNumRules().get() - r2.GetNumRules().get());
    }

    return nRet;
}

typedef XMLTextListAutoStylePoolEntry_Impl *XMLTextListAutoStylePoolEntryPtr;
DECLARE_CONTAINER_SORT( XMLTextListAutoStylePool_Impl,
                        XMLTextListAutoStylePoolEntry_Impl )
IMPL_CONTAINER_SORT( XMLTextListAutoStylePool_Impl,
                     XMLTextListAutoStylePoolEntry_Impl,
                     XMLTextListAutoStylePoolEntryCmp_Impl )

XMLTextListAutoStylePool::XMLTextListAutoStylePool( SvXMLExport& rExp ) :
    rExport( rExp ),
    pPool( new XMLTextListAutoStylePool_Impl( 5, 5 ) ),
    pNames( new XMLTextListAutoStylePoolNames_Impl( 5, 5 ) ),
    nName( 0 ),
    sPrefix( RTL_CONSTASCII_USTRINGPARAM("L") )
{
}

XMLTextListAutoStylePool::~XMLTextListAutoStylePool()
{
    delete pPool;
    delete pNames;
}

void XMLTextListAutoStylePool::RegisterName( const OUString& rName )
{
    OUString *pName = new OUString( rName );
    if( !pNames->Insert( pName ) )
        delete pName;
}

sal_Bool XMLTextListAutoStylePool::HasName( const OUString& rName ) const
{
    return pNames->Seek_Entry( &rName, 0 );
}

OUString XMLTextListAutoStylePool::Add(
            const Reference < XIndexReplace > & rNumRules )
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rNumRules );
    sal_uInt32 nPos;
    if( pPool->Seek_Entry( &aTmp, &nPos ) )
    {
        sName = pPool->GetObject( nPos )->GetName();
    }
    else
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry =
            new XMLTextListAutoStylePoolEntry_Impl( pPool->Count(),
                                               rNumRules, *pNames, sPrefix,
                                               nName );
        pPool->Insert( pEntry );
        sName = pEntry->GetName();
    }

    return sName;
}

::rtl::OUString XMLTextListAutoStylePool::Find(
            const Reference < XIndexReplace > & rNumRules ) const
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rNumRules );
    sal_uInt32 nPos;
    if( pPool->Seek_Entry( &aTmp, &nPos ) )
        sName = pPool->GetObject( nPos )->GetName();

    return sName;
}

::rtl::OUString XMLTextListAutoStylePool::Find(
            const OUString& rInternalName ) const
{
    OUString sName;
    XMLTextListAutoStylePoolEntry_Impl aTmp( rInternalName );
    sal_uInt32 nPos;
    if( pPool->Seek_Entry( &aTmp, &nPos ) )
        sName = pPool->GetObject( nPos )->GetName();

    return sName;
}


void XMLTextListAutoStylePool::exportXML() const
{
    sal_uInt32 nCount = pPool->Count();
    if( !nCount )
        return;

    XMLTextListAutoStylePoolEntry_Impl **aExpEntries =
        new XMLTextListAutoStylePoolEntryPtr[nCount];

    sal_uInt32 i;
    for( i=0; i < nCount; i++ )
    {
        aExpEntries[i] = 0;
    }
    for( i=0; i < nCount; i++ )
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry = pPool->GetObject(i);
        DBG_ASSERT( pEntry->GetPos() < nCount, "Illegal pos" );
        aExpEntries[pEntry->GetPos()] = pEntry;
    }

    SvxXMLNumRuleExport aNumRuleExp( rExport );

    for( i=0; i < nCount; i++ )
    {
        XMLTextListAutoStylePoolEntry_Impl *pEntry = aExpEntries[i];
        aNumRuleExp.exportNumberingRule( pEntry->GetName(),
                                         pEntry->GetNumRules() );
    }
    delete aExpEntries;
}


