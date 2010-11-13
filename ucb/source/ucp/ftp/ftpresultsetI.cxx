/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_ucb.hxx"

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


ResultSetI::ResultSetI(const Reference<XMultiServiceFactory>&  xMSF,
                       const Reference<XContentProvider>&  xProvider,
                       sal_Int32 nOpenMode,
                       const Sequence<Property>& seqProp,
                       const Sequence< NumberedSortingInfo >& seqSort,
                       const std::vector<FTPDirentry>&  dirvec)
    : ResultSetBase(xMSF,xProvider,nOpenMode,seqProp,seqSort)
{
    for( unsigned int i = 0; i < dirvec.size(); ++i)
        m_aPath.push_back(dirvec[i].m_aURL);

    // m_aIdents holds the contentidentifiers

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    for(unsigned n = 0; n < m_aItems.size(); ++n) {
        rtl::Reference<ucbhelper::PropertyValueSet> xRow =
            new ucbhelper::PropertyValueSet(xMSF);

        for( int i = 0; i < seqProp.getLength(); ++i) {
            const rtl::OUString& Name = seqProp[i].Name;
            if(Name.compareToAscii("ContentType") == 0 )
                xRow->appendString(seqProp[i],
                                   rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                       "application/ftp" )));
            else if(Name.compareToAscii("Title") == 0)
                xRow->appendString(seqProp[i],dirvec[n].m_aName);
            else if(Name.compareToAscii("IsReadOnly") == 0)
                xRow->appendBoolean(seqProp[i],
                                    sal_Bool(dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_WRITE));
            else if(Name.compareToAscii("IsDocument") == 0)
                xRow->appendBoolean(seqProp[i],
                                    ! sal_Bool(dirvec[n].m_nMode &
                                               INETCOREFTP_FILEMODE_ISDIR));
            else if(Name.compareToAscii("IsFolder") == 0)
                xRow->appendBoolean(seqProp[i],
                                    sal_Bool(dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_ISDIR));
            else if(Name.compareToAscii("Size") == 0)
                xRow->appendLong(seqProp[i],
                                 dirvec[n].m_nSize);
            else if(Name.compareToAscii("DateCreated") == 0)
                xRow->appendTimestamp(seqProp[i],
                                      dirvec[n].m_aDate);
            else if(Name.compareToAscii("CreatableContentsInfo") == 0)
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
