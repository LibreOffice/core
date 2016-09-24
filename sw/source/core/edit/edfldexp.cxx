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

#include <editsh.hxx>
#include <dbfld.hxx>
#include <dbmgr.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <docary.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <edimp.hxx>
#include <flddat.hxx>
#include <calbck.hxx>

using namespace com::sun::star;

bool SwEditShell::IsFieldDataSourceAvailable(OUString& rUsedDataSource) const
{
    const SwFieldTypes * pFieldTypes = GetDoc()->getIDocumentFieldsAccess().GetFieldTypes();
    uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<sdb::XDatabaseContext> xDBContext = sdb::DatabaseContext::create(xContext);
    for(const auto pFieldType : *pFieldTypes)
    {
        if(IsUsed(*pFieldType))
        {
            switch(pFieldType->Which())
            {
                case RES_DBFLD:
                {
                    SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
                    SwFormatField* pFormatField = aIter.First();
                    while(pFormatField)
                    {
                        if(pFormatField->IsFieldInDoc())
                        {
                            const SwDBData& rData =
                                    static_cast<SwDBFieldType*>(pFormatField->GetField()->GetTyp())->GetDBData();
                            try
                            {
                                return xDBContext->getByName(rData.sDataSource).hasValue();
                            }
                            catch(uno::Exception const &)
                            {
                                rUsedDataSource = rData.sDataSource;
                                return false;
                            }
                        }
                        pFormatField = aIter.Next();
                    }
                }
                break;
            }
        }
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
