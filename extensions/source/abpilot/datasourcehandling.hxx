/*************************************************************************
 *
 *  $RCSfile: datasourcehandling.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:36:51 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_ABP_DATASOURCEHANDLING_HXX
#define EXTENSIONS_ABP_DATASOURCEHANDLING_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef EXTENSIONS_ABP_ABPTYPES_HXX
#include "abptypes.hxx"
#endif

//========================================================================
namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace beans {
        class XPropertySet;
    }
} } }

class Window;


//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= ODataSourceContext
    //=====================================================================
    struct ODataSourceContextImpl;
    class ODataSource;
    /// a non-UNO wrapper for the data source context
    class ODataSourceContext
    {
    private:
        ODataSourceContextImpl*     m_pImpl;

    public:
        ODataSourceContext(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        /// retrieves the names of all data sources
        void    getDataSourceNames( StringBag& _rNames ) const SAL_THROW (( ));

        /// disambiguates the given name by appending auccessive numbers
        ::rtl::OUString& disambiguate(::rtl::OUString& _rDataSourceName);

        /// creates a new MORK data source
        ODataSource createNewMORK( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new Evolution data source
        ODataSource createNewEvolution( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new LDAP data source
        ODataSource createNewLDAP( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new Outlook data source
        ODataSource createNewOutlook( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new Outlook express data source
        ODataSource createNewOE( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new dBase data source
        ODataSource createNewDBase( const ::rtl::OUString& _rName ) SAL_THROW (( ));
    };

    //=====================================================================
    //= ODataSource
    //=====================================================================
    struct ODataSourceImpl;
    struct PackageAccessControl;
    /** a non-UNO wrapper for a data source
        <p>This class allows to access data sources without the need to compile the respective file with
        exception handling enabled (hopefully :).</p>
        <p>In addition to wrapping an UNO data source, an instance of this class can handle at most
        one valid connection, as obtained from the data source.</p>
    */
    class ODataSource
    {
    private:
        ODataSourceImpl*    m_pImpl;

    public:
        // ----------------------------------------------------------------
        // - ctor/dtor/assignment
        // ----------------------------------------------------------------
        /** ctor
            @param _rxORB
                the service factory to use to access the UNO objects
            @param _rName
                the name of the data source the object should represent
        */
        ODataSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::rtl::OUString& _rName
        );

        /// constructs an object which is initially invalid
        ODataSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        /// copy ctor
        ODataSource( const ODataSource& _rSource );

        /// dtor
        ~ODataSource( );

        /// assignment
        ODataSource& operator=( const ODataSource& _rSource );

        // ----------------------------------------------------------------
        /// checks whether or not the object represents a valid data source
        sal_Bool    isValid() const SAL_THROW (( ));

        // ----------------------------------------------------------------
        /// removes the data source represented by the object from the data source context
        void        remove() SAL_THROW (( ));
            // TODO: put this into the context class

        /// returns the name of the data source
        ::rtl::OUString
                    getName() const SAL_THROW (( ));

        /// renames the data source
        sal_Bool    rename( const ::rtl::OUString& _rName ) SAL_THROW (( ));
            // TODO: put this into the context class

        // ----------------------------------------------------------------
        // - connection handling
        // ----------------------------------------------------------------
        /** connects to the data source represented by this object
            @param _pMessageParent
                the window to use as parent for any error messages. If this is <NULL/>, no messages are displayed
                at all.
            @see isConnected
        */
        sal_Bool    connect( Window* _pMessageParent ) SAL_THROW (( ));

        /// returns <TRUE/> if the object has a valid connection, obtained from it's data source
        sal_Bool    isConnected( ) const SAL_THROW (( ));

        /// disconnects from the data source (i.e. disposes the UNO connection hold internally)
        void        disconnect( ) SAL_THROW (( ));

        /// stores the database file
        void        store() SAL_THROW (( ));

        /// register the data source under the given name in the configuration
        void        registerDataSource( const ::rtl::OUString& _sRegisteredDataSourceName )  SAL_THROW (( ));

        // ----------------------------------------------------------------
        /** retrieves the tables names from the connection
            <p>to be called when <method>isConnection</method> returns <TRUE/> only</p>
        */
        const StringBag&    getTableNames() const SAL_THROW (( ));

        /// return the intern data source object
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getDataSource() const SAL_THROW (( ));


        // ----------------------------------------------------------------
        /** set a new data source.
            <p>Available to selected clients only</p>
        */
        void        setDataSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDS
            ,const ::rtl::OUString& _sName
            ,PackageAccessControl
        );

    private:
        ODataSource( ); // never implemented
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_DATASOURCEHANDLING_HXX

