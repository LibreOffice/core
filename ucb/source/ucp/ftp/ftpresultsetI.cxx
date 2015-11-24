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


#include "comphelper/processfactory.hxx"
#include "ucbhelper/propertyvalueset.hxx"
#include "rtl/ref.hxx"
#include "com/sun/star/ucb/Command.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/ucb/XCommandProcessor.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
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
                       const Sequence< NumberedSortingInfo >& seqSort,
                       const std::vector<FTPDirentry>&  dirvec)
    : ResultSetBase(rxContext,xProvider,seqProp,seqSort)
{
    for( size_t i = 0; i < dirvec.size(); ++i)
        m_aPath.push_back(dirvec[i].m_aURL);

    // m_aIdents holds the contentidentifiers

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    for(size_t n = 0; n < m_aItems.size(); ++n) {
        rtl::Reference<ucbhelper::PropertyValueSet> xRow =
            new ucbhelper::PropertyValueSet(rxContext);

        for( int i = 0; i < seqProp.getLength(); ++i) {
            const OUString& Name = seqProp[i].Name;
            if(Name == "ContentType")
                xRow->appendString(seqProp[i],
                                   OUString( "application/ftp" ));
            else if(Name == "Title")
                xRow->appendString(seqProp[i],dirvec[n].m_aName);
            else if(Name == "IsReadOnly")
                xRow->appendBoolean(seqProp[i],
                                    (dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_WRITE) == INETCOREFTP_FILEMODE_WRITE);
            else if(Name == "IsDocument")
                xRow->appendBoolean(seqProp[i],
                                    (dirvec[n].m_nMode &
                                               INETCOREFTP_FILEMODE_ISDIR) != INETCOREFTP_FILEMODE_ISDIR);
            else if(Name == "IsFolder")
                xRow->appendBoolean(seqProp[i],
                                    ( dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_ISDIR) == INETCOREFTP_FILEMODE_ISDIR);
            else if(Name == "Size")
                xRow->appendLong(seqProp[i],
                                 dirvec[n].m_nSize);
            else if(Name == "DateCreated")
                xRow->appendTimestamp(seqProp[i],
                                      dirvec[n].m_aDate);
            else if(Name == "CreatableContentsInfo")
                xRow->appendObject(
                    seqProp[i],
                    makeAny(FTPContent::queryCreatableContentsInfo_Static()));
            else
                xRow->appendVoid(seqProp[i]);
        }
        m_aItems[n].set(xRow.get());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
