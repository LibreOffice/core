/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
                       sal_Int32 nOpenMode,
                       const Sequence<Property>& seqProp,
                       const Sequence< NumberedSortingInfo >& seqSort,
                       const std::vector<FTPDirentry>&  dirvec)
    : ResultSetBase(rxContext,xProvider,nOpenMode,seqProp,seqSort)
{
    for( unsigned int i = 0; i < dirvec.size(); ++i)
        m_aPath.push_back(dirvec[i].m_aURL);

    

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    for(unsigned n = 0; n < m_aItems.size(); ++n) {
        rtl::Reference<ucbhelper::PropertyValueSet> xRow =
            new ucbhelper::PropertyValueSet(rxContext);

        for( int i = 0; i < seqProp.getLength(); ++i) {
            const OUString& Name = seqProp[i].Name;
            if(Name.equalsAscii("ContentType") )
                xRow->appendString(seqProp[i],
                                   OUString( "application/ftp" ));
            else if(Name.equalsAscii("Title"))
                xRow->appendString(seqProp[i],dirvec[n].m_aName);
            else if(Name.equalsAscii("IsReadOnly"))
                xRow->appendBoolean(seqProp[i],
                                    sal_Bool(dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_WRITE));
            else if(Name.equalsAscii("IsDocument"))
                xRow->appendBoolean(seqProp[i],
                                    ! sal_Bool(dirvec[n].m_nMode &
                                               INETCOREFTP_FILEMODE_ISDIR));
            else if(Name.equalsAscii("IsFolder"))
                xRow->appendBoolean(seqProp[i],
                                    sal_Bool(dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_ISDIR));
            else if(Name.equalsAscii("Size"))
                xRow->appendLong(seqProp[i],
                                 dirvec[n].m_nSize);
            else if(Name.equalsAscii("DateCreated"))
                xRow->appendTimestamp(seqProp[i],
                                      dirvec[n].m_aDate);
            else if(Name.equalsAscii("CreatableContentsInfo"))
                xRow->appendObject(
                    seqProp[i],
                    makeAny(FTPContent::queryCreatableContentsInfo_Static()));
            else
                xRow->appendVoid(seqProp[i]);
        }
        m_aItems[n] = Reference<XRow>(xRow.get());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
