/*************************************************************************
 *
 *  $RCSfile: DIndexes.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-30 13:57:18 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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
#ifndef _CONNECTIVITY_DBASE_INDEXES_HXX_
#include "dbase/DIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbase
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif

using namespace utl;
using namespace connectivity::dbase;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace starutil      = ::com::sun::star::util;

Reference< XNamed > ODbaseIndexes::createObject(const ::rtl::OUString& _rName)
{
    //  Dir* pDir = m_pTable->getConnection()->getDir();
    //  String aPath = pDir->GetName();
    //  aPath += _rName.getStr();
    ::rtl::OUString sFile = m_pTable->getConnection()->getURL();
    sFile += STR_DELIMITER;
    sFile += _rName;
    sFile += ::rtl::OUString::createFromAscii(".ndx");
    if(!UCBContentHelper::Exists(sFile))
        throw SQLException(::rtl::OUString::createFromAscii("Index file doesn't exists!"),*m_pTable,SQLSTATE_GENERAL,1000,Any());

    Reference< XNamed > xRet;
    SvStream* pFileStream = UcbStreamHelper::CreateStream(sFile,STREAM_READ | STREAM_NOCREATE| STREAM_SHARE_DENYWRITE);
    if(pFileStream)
    {
        pFileStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        pFileStream->SetBufferSize(512);
        ODbaseIndex::NDXHeader aHeader;

        pFileStream->Seek(0);
        pFileStream->Read(&aHeader,512);
        delete pFileStream;

        ODbaseIndex* pIndex = new ODbaseIndex(m_pTable,aHeader,_rName);
        xRet = pIndex;
        pIndex->openIndexFile();
    }
    else
        throw SQLException(::rtl::OUString::createFromAscii("Could not open index file"),*m_pTable,SQLSTATE_GENERAL,1000,Any());
    return xRet;
}
// -------------------------------------------------------------------------
void ODbaseIndexes::impl_refresh(  ) throw(RuntimeException)
{
    if(m_pTable)
        m_pTable->refreshIndexes();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > ODbaseIndexes::createEmptyObject()
{
    ODbaseIndex* pRet = new ODbaseIndex(m_pTable);
    Reference< XPropertySet > xRet = pRet;
    return xRet;
}
typedef connectivity::sdbcx::OCollection ODbaseTables_BASE_BASE;
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL ODbaseIndexes::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    ObjectMap::iterator aIter = m_aNameMap.find(aName);
    if( aIter != m_aNameMap.end())
        throw ElementExistException(aName,*this);

    Reference<XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
        if(pIndex && pIndex->CreateImpl())
            ODbaseIndexes_BASE::appendByDescriptor(Reference< XPropertySet >(createObject(aName),UNO_QUERY));
    }
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL ODbaseIndexes::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    ObjectMap::iterator aIter = m_aNameMap.find(elementName);
    if( aIter == m_aNameMap.end())
        throw NoSuchElementException(elementName,*this);

    if(!aIter->second.is())
        aIter->second = createObject(elementName);

    Reference< XUnoTunnel> xTunnel(aIter->second.get(),UNO_QUERY);
    if(xTunnel.is())
    {
        ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
        if(pIndex && pIndex->DropImpl())
            ODbaseIndexes_BASE::dropByName(elementName);
    }

}
// -------------------------------------------------------------------------
void SAL_CALL ODbaseIndexes::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException(::rtl::OUString::valueOf(index),*this);

    dropByName((*m_aElements[index]).first);
}
// -------------------------------------------------------------------------


