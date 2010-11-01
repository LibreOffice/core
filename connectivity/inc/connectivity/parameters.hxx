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
#ifndef CONNECTIVITY_PARAMETERS_HXX
#define CONNECTIVITY_PARAMETERS_HXX

#include <map>
#include <vector>

/** === begin UNO includes === **/
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
/** === end UNO includes === **/

#include "connectivity/dbtoolsdllapi.hxx"
#include "connectivity/paramwrapper.hxx"
#include <unotools/sharedunocomponent.hxx>
#include <comphelper/implementationreference.hxx>
#include <cppuhelper/interfacecontainer.hxx>

//........................................................................
namespace dbtools
{
//........................................................................

    typedef ::utl::SharedUNOComponent< ::com::sun::star::sdb::XSingleSelectQueryComposer, ::utl::DisposableComponent >
            SharedQueryComposer;

    //====================================================================
    //= ParameterManager
    //====================================================================
    class FilterManager;
    class OOO_DLLPUBLIC_DBTOOLS ParameterManager
    {
    public:
        /// classifies the origin of the data to fill a parameter
        enum ParameterClassification
        {
            /** parameters which are filled from the master-detail relationship, where the detail
                name is an explicit parameter name
            */
            eLinkedByParamName,
            /** parameters which are filled from the master-detail relationship, where the detail
                name is a column name, so an implicit parameter had to be generated for it
            */
            eLinkedByColumnName,
            /** parameters which are filled externally (i.e. by XParamaters::setXXX, or by the parameter listeners)
            */
            eFilledExternally
        };
        /** meta data about an inner parameter
        */
    private:
        struct ParameterMetaData
        {
            /// the type of the parameter
            ParameterClassification     eType;
            /// the column object for this parameter, as returned by the query composer
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                        xComposerColumn;
            /// the indicies of inner parameters which need to be filled when this concrete parameter is set
            ::std::vector< sal_Int32 >  aInnerIndexes;

            /// default ctor
            ParameterMetaData()
                :eType( eFilledExternally )
            {
            }

            /// ctor with composer column
            ParameterMetaData( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn )
                :eType           ( eFilledExternally )
                ,xComposerColumn ( _rxColumn         )
            {
            }
        };

        typedef ::std::map< ::rtl::OUString, ParameterMetaData >    ParameterInformation;

    private:
        ::osl::Mutex&                       m_rMutex;
        ::cppu::OInterfaceContainerHelper   m_aParameterListeners;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                            m_xORB;

        ::com::sun::star::uno::WeakReference< ::com::sun::star::beans::XPropertySet >
                                            m_xComponent;                // the database component whose parameters we're handling
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                                            m_xAggregatedRowSet;    // the aggregated row set - necessary for unwrapped access to some interfaces
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters >
                                            m_xInnerParamUpdate;    // write access to the inner parameters
        SharedQueryComposer                 m_xComposer;            // query composer wrapping the statement which the *aggregate* is based on
        SharedQueryComposer                 m_xParentComposer;      // query composer wrapping the statement of our parent database component
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                                            m_xInnerParamColumns;   // index access to the parameter columns, as got from the query composer

        ::dbtools::param::ParametersContainerRef
                                            m_pOuterParameters;     // the container of parameters which still need to be filled in by
                                                                    // external instances
        sal_Int32                           m_nInnerCount;          // overall number of parameters as required by the database component's aggregate

        ParameterInformation                m_aParameterInformation;

        ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aMasterFields;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aDetailFields;

        ::rtl::OUString                     m_sIdentifierQuoteString;
        ::rtl::OUString                     m_sSpecialCharacters;

        ::std::vector< bool >               m_aParametersVisited;

        bool                                m_bUpToDate;

    public:
        /** ctor
        */
        explicit ParameterManager(
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        /// late ctor
        void    initialize(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxComponent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >& _rxComponentAggregate
                );

        /// makes the object forgetting the references to the database component
        void    dispose( );

        /// clears the instance data
                void    clearAllParameterInformation();

        /// checks whether the parameter information are up-to-date
        inline  bool    isUpToDate() const { return m_bUpToDate; }

        /** updates all parameter information represented by the instance
        */
        void    updateParameterInfo( FilterManager& _rFilterManager );

        /** fills parameter values, as extensive as possible

            <p>In particular, all values which can be filled from the master-detail relationship of
            between our database component and it's parent are filled in.</p>

            @param _rxCompletionHandler
                an interaction handler which should be used to fill all parameters which
                cannot be filled by other means. May be <NULL/>
            @param _rClearForNotifies
                the mutex guard to be (temporarily) cleared for notifications

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>

            @return
                <TRUE/> if and only if the parameter filling has <em>not</em> been cancelled by the user
        */
        bool    fillParameterValues(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler,
                    ::osl::ResettableMutexGuard& _rClearForNotifies
                );

        /** sets all parameter values to null (via <member>XParameters::setNull</member>)

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>
        */
        void    setAllParametersNull() SAL_THROW( ( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) );

        /** resets all detail columns which are, via a parameter, linked to a master column, to
            the value of this master column.

            For instance, if the database component is bound to a statement <code>SELECT * from invoice where inv_id = :cid</code>,
            and there is <em>one</em> master-detail link from

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>
        */
        void    resetParameterValues() SAL_THROW(());

        /** tells the object that it's database component is being disposed

            The object then fires the <member>XEventListener::disposing</member> notification to
            the parameter listeners
        */
        void    disposing( const ::com::sun::star::lang::EventObject& _rDisposingEvent );

        /** adds the given listener to the list of parameter listeners
        */
        void    addParameterListener(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener >& _rxListener
                );

        /** removes the given listener from the list of parameter listeners
        */
        void    removeParameterListener(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener >& _rxListener
                );

        // XParameters equivalents
        void setNull            ( sal_Int32 _nIndex, sal_Int32 sqlType);
        void setObjectNull      ( sal_Int32 _nIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName);
        void setBoolean         ( sal_Int32 _nIndex, sal_Bool x);
        void setByte            ( sal_Int32 _nIndex, sal_Int8 x);
        void setShort           ( sal_Int32 _nIndex, sal_Int16 x);
        void setInt             ( sal_Int32 _nIndex, sal_Int32 x);
        void setLong            ( sal_Int32 _nIndex, sal_Int64 x);
        void setFloat           ( sal_Int32 _nIndex, float x);
        void setDouble          ( sal_Int32 _nIndex, double x);
        void setString          ( sal_Int32 _nIndex, const ::rtl::OUString& x);
        void setBytes           ( sal_Int32 _nIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x);
        void setDate            ( sal_Int32 _nIndex, const ::com::sun::star::util::Date& x);
        void setTime            ( sal_Int32 _nIndex, const ::com::sun::star::util::Time& x);
        void setTimestamp       ( sal_Int32 _nIndex, const ::com::sun::star::util::DateTime& x);
        void setBinaryStream    ( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length);
        void setCharacterStream ( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length);
        void setObject          ( sal_Int32 _nIndex, const ::com::sun::star::uno::Any& x);
        void setObjectWithInfo  ( sal_Int32 _nIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale);
        void setRef             ( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef>& x);
        void setBlob            ( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob>& x);
        void setClob            ( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob>& x);
        void setArray           ( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray>& x);
        void clearParameters();

    private:
        /// checkes whether the object is already initialized, and not yet disposed
        inline  bool    isAlive() const { return m_xComponent.get().is() && m_xInnerParamUpdate.is(); }

        /** creates a filter expression from a master-detail link where the detail denotes a column name
        */
        ::rtl::OUString
                createFilterConditionFromColumnLink(
                    const ::rtl::OUString& /* [in]  */ _rMasterColumn,
                    const ::rtl::OUString& /* [in]  */ _rDetailColumn,
                          ::rtl::OUString& /* [out] */ _rNewParamName
                );

        /** initializes our query composer, and the collection of inner parameter columns

            @param _rxComponent
                the database component to initialize from. Must not be <NULL/>
            @return
                <TRUE/> if and only if the initialization was successfull

            @postcond
                if and only if <TRUE/> is returned, then <member>m_xInnerParamColumns</member> contains the collection of
                inner parameters
        */
        bool    initializeComposerByComponent(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxComponent
                );

        /** collects initial meta information about inner paramaters (i.e. it initially fills
            <member>m_aParameterInformation</member>).

            @param _bSecondRun
                if <TRUE/>, this is the second run, because we ourself previously extended the filter of
                the RowSet

            @precond
                <member>m_xInnerParamColumns</member> is not <NULL/>
        */
        void    collectInnerParameters( bool _bSecondRun );

        /** analyzes the master-detail links for our database component, and initializes m_aMasterFields and m_aDetailFields

            @param _rFilterManager
                the filter manager of the database component
            @param _rColumnsInLinkDetails
                will be set to <TRUE/> if and only if there were link pairs where the detail field denoted
                a column name of our database component

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>
        */
        void    analyzeFieldLinks( FilterManager& _rFilterManager, bool& /* [out] */ _rColumnsInLinkDetails );

        /** classifies the link pairs

            @param  _rxParentColumns
                the columns of the parent database component

            @param  _rxColumns
                the columns of our own database component

            @param  _out_rAdditionalFilterComponents
                the additional filter components which are required for master-detail relationships where
                the detail part denotes a column name. In such a case, an additional filter needs to be created,
                containing a new parameter.

            @precond
                <member>m_aMasterFields</member> and <member>m_aDetailFields</member> have the same length
        */
        void    classifyLinks(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxParentColumns,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumns,
                    ::std::vector< ::rtl::OUString >& _out_rAdditionalFilterComponents
                )   SAL_THROW(( ::com::sun::star::uno::Exception ));

        /** finalizes our <member>m_pOuterParameters</member> so that it can be used for
            external parameter listeners

            @precond
                <member>m_pOuterParameters</member> is <NULL/>
            @precond
                <member>m_xInnerParamUpdate</member> is not <NULL/>
        */
        void    createOuterParameters();

        /** fills in the parameters values which result from the master-detail relationship
            between the database component and it's parent

            @param _rxParentColumns
                the columns of the parameter database component. Must not be <NULL/>
            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>
        */
        void    fillLinkedParameters(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxParentColumns
                );

        /** completes all missing parameters via an interaction handler

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>

            @return
                <TRUE/> if and only if the parameter filling has <em>not</em> been cancelled by the user
        */
        bool    completeParameters(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxCompletionHandler,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > _rxConnection
                );

        /** asks the parameter listeners to fill in final values

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>

            @return
                <TRUE/> if and only if the parameter filling has <em>not</em> been cancelled by the user
        */
        bool    consultParameterListeners( ::osl::ResettableMutexGuard& _rClearForNotifies );

        /** mark an externally filled parameter asvisited
        */
        void    externalParameterVisited( sal_Int32 _nIndex );

    private:
        /** retrieves the columns of the parent database component

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>
            @return
                <TRUE/> if and only if the columns could be successfully retrieved
        */
        bool    getParentColumns(
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& /* [out] */ _out_rxParentColumns,
                    bool _bFromComposer
                );

        /** retrieves the columns of our database component

            @param _bFromComposer
                if <TRUE/>, the columns are obtained from the composer, else from the living database component itself
            @return
                <TRUE/> if and only if the columns could be successfully retrieved
        */
        bool    getColumns(
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& /* [out] */ _rxColumns,
                    bool _bFromComposer
                ) SAL_THROW(( ::com::sun::star::uno::Exception ));

        /** retrieves the active connection of the database component
        */
        bool    getConnection(
                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& /* [out] */ _rxConnection
                );

        /** caches some info about the connection of our database component
        */
        void    cacheConnectionInfo() SAL_THROW(( ));

    private:
        ParameterManager();                                      // never implemented
        ParameterManager( const ParameterManager& );              // never implemented
        ParameterManager& operator=( const ParameterManager& );   // never implemented
    };

//........................................................................
} // namespacefrm
//........................................................................

#endif // CONNECTIVITY_PARAMETERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
