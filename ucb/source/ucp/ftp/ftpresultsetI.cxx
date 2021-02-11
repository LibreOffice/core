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


#include <ucbhelper/propertyvalueset.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/ucb/Command.hpp>
#include "ftpresultsetI.hxx"
#include "ftpcontent.hxx"


using namespace std;
using namespace ftp;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;


ResultSetI::ResultSetI(const Reference<XComponentContext>&  rxContext,
                       const Reference<XContentProvider>&  xProvider,
                       const Sequence<Property>& seqProp,
                       const std::vector<FTPDirentry>&  dirvec)
    : ResultSetBase(rxContext,xProvider,seqProp)
{
    for(const auto & i : dirvec)
        m_aPath.push_back(i.m_aURL);

    // m_aIdents holds the content identifiers

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    for(size_t n = 0; n < m_aItems.size(); ++n) {
        rtl::Reference<ucbhelper::PropertyValueSet> xRow =
            new ucbhelper::PropertyValueSet(rxContext);

        for( const auto& rProp : seqProp) {
            const OUString& Name = rProp.Name;
            if(Name == "ContentType")
                xRow->appendString(rProp,
                                   OUString( "application/ftp" ));
            else if(Name == "Title")
                xRow->appendString(rProp,dirvec[n].m_aName);
            else if(Name == "IsReadOnly")
                xRow->appendBoolean(rProp,
                                    (dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_WRITE) == INETCOREFTP_FILEMODE_WRITE);
            else if(Name == "IsDocument")
                xRow->appendBoolean(rProp,
                                    (dirvec[n].m_nMode &
                                               INETCOREFTP_FILEMODE_ISDIR) != INETCOREFTP_FILEMODE_ISDIR);
            else if(Name == "IsFolder")
                xRow->appendBoolean(rProp,
                                    ( dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_ISDIR) == INETCOREFTP_FILEMODE_ISDIR);
            else if(Name == "Size")
                xRow->appendLong(rProp,
                                 dirvec[n].m_nSize);
            else if(Name == "DateCreated")
                xRow->appendTimestamp(rProp,
                                      dirvec[n].m_aDate);
            else if(Name == "CreatableContentsInfo")
                xRow->appendObject(
                    rProp,
                    makeAny(FTPContent::queryCreatableContentsInfo_Static()));
            else
                xRow->appendVoid(rProp);
        }
        m_aItems[n].set(xRow);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
