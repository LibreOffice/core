/*************************************************************************
 *
 *  $RCSfile: UITools.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2001-07-18 08:51:09 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_TOOLS_HXX
#define DBAUI_TOOLS_HXX

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif

// we only need forward decl here
namespace com { namespace sun { namespace star {

    namespace beans     { class XPropertySet;}
    namespace container { class XNameAccess;}
    namespace lang
    {
        class XEventListener;
        class XMultiServiceFactory;
    }
    namespace sdbc
    {
        class XDatabaseMetaData;
        class XConnection;
    }

}}}

class Window;
// .........................................................................
namespace dbaui
{
// .........................................................................

    /** compose a complete table name from it's up to three parts, regarding to the database meta data composing rules
        the PropertySet must be support the service com::sun::star::sdbc::table
    */
    void composeTableName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxMetaData,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxTable,
                            ::rtl::OUString& _rComposedName,
                            sal_Bool _bQuote);

    /** creates a new connection and appends the eventlistener
        @param  _rsDataSourceName       name of the datasource
        @param  _xDatabaseContext       the database context
        @param  _rEvtLst                the eventlistener which will be added to the new created connection
        @param  _rOUTConnection         this parameter will be filled with the new created connection
        @return SQLExceptionInfo        contains a SQLException, SQLContext or a SQLWarning when they araised else .isValid() will return false
    */
    ::dbtools::SQLExceptionInfo createConnection(
                                    const ::rtl::OUString& _rsDataSourceName,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xDatabaseContext,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rMF,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rEvtLst,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rOUTConnection );

    /**  creates a error dialog which displays the SQLExceptionInfo. Also it supports a "more" button where detailed information are available
        @param  _rInfo                  the error which should be shown, if the info is not valid no error dialog will appear
        @param  _pParent                the parent of the error dialog
        @param  _xFactory               need to create the dialog
    */
    void showError( const ::dbtools::SQLExceptionInfo& _rInfo,
                    Window* _pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory);

    /** return a vector which contains all key columns for the @see com::sun::star::sdbc::KeyType _nKeyType
        @param  _rxTable                the table which must be a @see com::sun::star::sdbcx::XColumnsSupplier
        @param  _nKeyType               @see com::sun::star::sdbc::KeyType
    */

    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> >
        getKeyColumns(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxTable,
                        sal_Int32 _nKeyType);

    /** fills a map and a vector with localized type names
        @param  _rxConnection   the connection to acces the metadata
        @param  _rsTypeNames    a list of localized type names seperated with ';'
        @param  _rTypeInfoMap   the filled map with the type names
        @param  _rTypeInfoIters the vector filled with map iterators
    */
    void fillTypeInfo(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection,
                        const String& _rsTypeNames,
                        OTypeInfoMap& _rTypeInfoMap,
                        ::std::vector<OTypeInfoMap::iterator>& _rTypeInfoIters);

    /** fill a column with data of a field description
        @param  _rxColumn   the column which should be filled
        @param  _pFieldDesc the source of the data
    */
    class OFieldDescription;
    void setColumnProperties(   const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
                                const OFieldDescription* _pFieldDesc);

    ::rtl::OUString createDefaultName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xTables,
                                        const ::rtl::OUString& _sName);

    /** checks if the given name exists in the database context
    */
    sal_Bool checkDataSourceAvailable(  const ::rtl::OUString& _sDataSourceName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory_xORB);
// .........................................................................
}
// .........................................................................
#endif // DBAUI_TOOLS_HXX


