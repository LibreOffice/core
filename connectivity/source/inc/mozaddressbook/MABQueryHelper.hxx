/*************************************************************************
 *
 *  $RCSfile: MABQueryHelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dkenny $ $Date: 2001-04-17 19:27:22 $
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

#ifndef _CONNECTIVITY_MAB_QUERYHELPER_HXX_
#define _CONNECTIVITY_MAB_QUERYHELPER_HXX_

#include <map>
#include <vector>
#include <rtl/ustring.hxx>

#include "nsCOMPtr.h"
#include "nsISupportsArray.h"
#include "nsString.h"
#include "nsRDFCID.h"
#include "nsXPIDLString.h"
#include "nsIRDFService.h"
#include "nsIRDFResource.h"
#include "msgCore.h"
#include "nsIServiceManager.h"
#include "nsIAbCard.h"
#include "nsAbBaseCID.h"
#include "nsAbAddressCollecter.h"
#include "nsIPref.h"
#include "nsIAddrBookSession.h"
#include "nsIMsgHeaderParser.h"
#include "nsIAbDirectory.h"
#include "nsAbDirectoryQuery.h"
#include "nsIAbDirectoryQuery.h"
#include "nsISupportsUtils.h"
#include "nsIAbDirectoryQuery.h"

namespace connectivity
{
    namespace mozaddressbook
    {
        class OMozabQueryHelperResultEntry {
        private:
            struct ltstr
            {
                bool operator()( ::rtl::OUString &s1, ::rtl::OUString &s2) const;
            };

            typedef std::map< rtl::OUString, rtl::OUString, ltstr >  fieldMap;

            fieldMap    m_Fields;
        public:
            ~OMozabQueryHelperResultEntry();

            void insert( rtl::OUString &key, rtl::OUString &value );
            rtl::OUString getValue( const rtl::OUString &key ) const;
        };

        class OMozabQueryHelper : public nsIAbDirectoryQueryResultListener {
        private:
            typedef std::vector< OMozabQueryHelperResultEntry* > resultsArray;

            resultsArray    m_aResults;
            sal_Int32       m_nIndex;
            bool            m_bHasMore;
            bool            m_bAtEnd;

            void            append(OMozabQueryHelperResultEntry* resEnt );

            void            clear_results();

        public:

            NS_DECL_ISUPPORTS
            NS_DECL_NSIABDIRECTORYQUERYRESULTLISTENER

                                            OMozabQueryHelper();

                                            ~OMozabQueryHelper();

            void                            reset();

            void                            rewind();

            OMozabQueryHelperResultEntry*   next();

            OMozabQueryHelperResultEntry*   getByIndex( sal_Int32 nRow );

            bool                            hasMore() const;

            bool                            atEnd() const;

            sal_Int32                       getResultCount() const;

        };
    }
}
#endif // _CONNECTIVITY_MAB_QUERYHELPER_HXX_

