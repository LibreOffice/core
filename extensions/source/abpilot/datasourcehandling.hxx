/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datasourcehandling.hxx,v $
 * $Revision: 1.11 $
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

#ifndef EXTENSIONS_ABP_DATASOURCEHANDLING_HXX
#define EXTENSIONS_ABP_DATASOURCEHANDLING_HXX

#include <com/sun/star/uno/Reference.hxx>
#include "abptypes.hxx"

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

        /// creates a new Thunderbird data source
        ODataSource createNewThunderbird( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new Evolution local data source
        ODataSource createNewEvolution( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new Evolution LDAP data source
        ODataSource createNewEvolutionLdap( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new Evolution GROUPWISE data source
        ODataSource createNewEvolutionGroupwise( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new KDE address book data source
        ODataSource createNewKab( const ::rtl::OUString& _rName ) SAL_THROW (( ));

        /// creates a new Mac OS X address book data source
        ODataSource createNewMacab( const ::rtl::OUString& _rName ) SAL_THROW (( ));

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

        /** determines whether a given table exists
        */
        bool    hasTable( const ::rtl::OUString& _rTableName ) const;

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

