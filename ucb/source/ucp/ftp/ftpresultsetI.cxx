/*************************************************************************
 *
 *  $RCSfile: ftpresultsetI.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: abi $ $Date: 2002-08-29 09:45:03 $
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

#include <ucbhelper/propertyvalueset.hxx>
#include <vos/ref.hxx>
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include "ftpresultsetI.hxx"



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
    for(unsigned i = 0; i < dirvec.size(); ++i)
        m_aPath.push_back(dirvec[i].m_aURL);

    // m_aIdents holds the contentidentifiers

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    for(unsigned n = 0; n < m_aItems.size(); ++n) {
        vos::ORef<ucb::PropertyValueSet> xRow =
            new ucb::PropertyValueSet(xMSF);

        for(i = 0; i < seqProp.getLength(); ++i) {
            const rtl::OUString& Name = seqProp[i].Name;
            if(Name.compareToAscii("ContentType") == 0 )
                xRow->appendString(seqProp[i],
                                   rtl::OUString::createFromAscii(
                                       "application/ftp" ));
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
            else
                xRow->appendVoid(seqProp[i]);
        }
        m_aItems[n] = Reference<XRow>(xRow.getBodyPtr());
    }
}
