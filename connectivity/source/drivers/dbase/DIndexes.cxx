/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <dbase/DIndexes.hxx>
#include <dbase/DIndex.hxx>
#include <comphelper/servicehelper.hxx>
#include <connectivity/dbexception.hxx>
#include <unotools/ucbhelper.hxx>
#include <strings.hrc>

using namespace utl;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::connectivity::dbase;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

sdbcx::ObjectType ODbaseIndexes::createObject(const OUString& _rName)
{
    OUString sFile = m_pTable->getConnection()->getURL() +
        OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELIMITER) +
        _rName + ".ndx";
    if ( !UCBContentHelper::Exists(sFile) )
    {
        const OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_FILE,
                "$filename$", sFile
            ) );
        ::dbtools::throwGenericSQLException( sError, *m_pTable );
    }

    sdbcx::ObjectType xRet;
    std::unique_ptr<SvStream> pFileStream = ::connectivity::file::OFileTable::createStream_simpleError(sFile, StreamMode::READ | StreamMode::NOCREATE | StreamMode::SHARE_DENYWRITE);
    if(pFileStream)
    {
        pFileStream->SetEndian(SvStreamEndian::LITTLE);
        pFileStream->SetBufferSize(DINDEX_PAGE_SIZE);
        ODbaseIndex::NDXHeader aHeader;

        pFileStream->Seek(0);
        ReadHeader(*pFileStream, aHeader);
        pFileStream.reset();

        rtl::Reference<ODbaseIndex> pIndex = new ODbaseIndex(m_pTable,aHeader,_rName);
        xRet = pIndex;
        pIndex->openIndexFile();
    }
    else
    {
        const OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_FILE,
                "$filename$", sFile
             ) );
        ::dbtools::throwGenericSQLException( sError, *m_pTable );
    }
    return xRet;
}

void ODbaseIndexes::impl_refresh(  )
{
    if(m_pTable)
        m_pTable->refreshIndexes();
}

Reference< XPropertySet > ODbaseIndexes::createDescriptor()
{
    return new ODbaseIndex(m_pTable);
}

// XAppend
sdbcx::ObjectType ODbaseIndexes::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ODbaseIndex* pIndex = dynamic_cast<ODbaseIndex*>(descriptor.get());
    if(pIndex)
        pIndex->CreateImpl();

    return createObject( _rForName );
}

// XDrop
void ODbaseIndexes::dropObject(sal_Int32 _nPos, const OUString& /*_sElementName*/)
{
    rtl::Reference<ODbaseIndex> pIndex = dynamic_cast<ODbaseIndex*>(getObject(_nPos).get());
    if ( pIndex )
        pIndex->DropImpl();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
