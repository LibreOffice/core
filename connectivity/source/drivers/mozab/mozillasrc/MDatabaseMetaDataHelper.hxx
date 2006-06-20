/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MDatabaseMetaDataHelper.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:50:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_
#define _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_

#ifndef CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX
#include "MErrorResource.hxx"
#endif

#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif

#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#include "FDatabaseMetaDataResultSet.hxx"
#endif

#ifndef _CONNECTIVITY_MAB_CONNECTION_HXX_
#include <MConnection.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_MOZILLA_MOZILLPRODUCTTYPE_HPP_
#include <com/sun/star/mozilla/MozillaProductType.hpp>
#endif

namespace connectivity
{
    namespace mozab
    {
        class MDatabaseMetaDataHelper : public ErrorResourceAccess
        {
        private:
            sal_Bool                                        m_bProfileExists ;
            ::std::vector< ::rtl::OUString >                m_aTableNames;
            ::std::vector< ::rtl::OUString >                m_aTableTypes;
            ::com::sun::star::mozilla::MozillaProductType   m_ProductType;
            ::rtl::OUString                                 m_ProfileName;

        public:
            MDatabaseMetaDataHelper();
            ~MDatabaseMetaDataHelper();

            //
            sal_Bool getTableStrings( OConnection*                        _pCon,
                                      ::std::vector< ::rtl::OUString >&   _rStrings,
                                      ::std::vector< ::rtl::OUString >&   _rTypes);

            sal_Bool getTables( OConnection* _pCon,
                                const ::rtl::OUString& tableNamePattern,
                                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& types,
                                ODatabaseMetaDataResultSet::ORows& _rRows);
            sal_Bool   testLDAPConnection( OConnection* _pCon );
            sal_Bool   NewAddressBook( OConnection* _pCon,const ::rtl::OUString & aTableName);

        private:
            void setAbSpecificError( OConnection* _pCon, sal_Bool bGivenURI );
        };
    }

}
#endif // _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_

