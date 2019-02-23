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
#ifndef INCLUDED_CONNECTIVITY_PARAMETERS_HXX
#define INCLUDED_CONNECTIVITY_PARAMETERS_HXX

#include <map>
#include <vector>

#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/paramwrapper.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <comphelper/interfacecontainer2.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::container { class XIndexAccess; }
namespace com::sun::star::form { class XDatabaseParameterListener; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::sdbc { class XArray; }
namespace com::sun::star::sdbc { class XBlob; }
namespace com::sun::star::sdbc { class XClob; }
namespace com::sun::star::sdbc { class XConnection; }
namespace com::sun::star::sdbc { class XDatabaseMetaData; }
namespace com::sun::star::sdbc { class XParameters; }
namespace com::sun::star::sdbc { class XRef; }
namespace com::sun::star::task { class XInteractionHandler; }
namespace com::sun::star::uno { class XAggregation; }
namespace com::sun::star::uno { class XComponentContext; }

namespace dbtools
{


    typedef ::utl::SharedUNOComponent< css::sdb::XSingleSelectQueryComposer, ::utl::DisposableComponent >
            SharedQueryComposer;


    //= ParameterManager

    class FilterManager;
    class OOO_DLLPUBLIC_DBTOOLS ParameterManager
    {
    public:
        /// classifies the origin of the data to fill a parameter
        enum class ParameterClassification
        {
            /** parameters which are filled from the master-detail relationship, where the detail
                name is an explicit parameter name
            */
            LinkedByParamName,
            /** parameters which are filled from the master-detail relationship, where the detail
                name is a column name, so an implicit parameter had to be generated for it
            */
            LinkedByColumnName,
            /** parameters which are filled externally (i.e. by XParameters::setXXX, or by the parameter listeners)
            */
            FilledExternally
        };
        /** meta data about an inner parameter
        */
    private:
        struct ParameterMetaData
        {
            /// the type of the parameter
            ParameterClassification     eType;
            /// the column object for this parameter, as returned by the query composer
            css::uno::Reference< css::beans::XPropertySet >
                                        xComposerColumn;
            /// the indices of inner parameters which need to be filled when this concrete parameter is set
            ::std::vector< sal_Int32 >  aInnerIndexes;

            /// ctor with composer column
            ParameterMetaData( const css::uno::Reference< css::beans::XPropertySet >& _rxColumn )
                :eType           ( ParameterClassification::FilledExternally )
                ,xComposerColumn ( _rxColumn         )
            {
            }
        };

        typedef ::std::map< OUString, ParameterMetaData >    ParameterInformation;

    private:
        ::osl::Mutex&                       m_rMutex;
        ::comphelper::OInterfaceContainerHelper2  m_aParameterListeners;

        css::uno::Reference< css::uno::XComponentContext >
                                            m_xContext;

        css::uno::WeakReference< css::beans::XPropertySet >
                                            m_xComponent;                // the database component whose parameters we're handling
        css::uno::Reference< css::uno::XAggregation >
                                            m_xAggregatedRowSet;    // the aggregated row set - necessary for unwrapped access to some interfaces
        css::uno::Reference< css::sdbc::XParameters >
                                            m_xInnerParamUpdate;    // write access to the inner parameters
        SharedQueryComposer                 m_xComposer;            // query composer wrapping the statement which the *aggregate* is based on
        SharedQueryComposer                 m_xParentComposer;      // query composer wrapping the statement of our parent database component
        css::uno::Reference< css::container::XIndexAccess >
                                            m_xInnerParamColumns;   // index access to the parameter columns, as got from the query composer

        ::dbtools::param::ParametersContainerRef
                                            m_pOuterParameters;     // the container of parameters which still need to be filled in by
                                                                    // external instances
        sal_Int32                           m_nInnerCount;          // overall number of parameters as required by the database component's aggregate

        ParameterInformation                m_aParameterInformation;

        std::vector< OUString >             m_aMasterFields;
        std::vector< OUString >             m_aDetailFields;

        OUString                            m_sIdentifierQuoteString;
        OUString                            m_sSpecialCharacters;
        css::uno::Reference< css::sdbc::XDatabaseMetaData > m_xConnectionMetadata;

        ::std::vector< bool >               m_aParametersVisited;

        bool                                m_bUpToDate;

    public:
        /** ctor
        */
        explicit ParameterManager(
            ::osl::Mutex& _rMutex,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        /// late ctor
        void    initialize(
                    const css::uno::Reference< css::beans::XPropertySet >& _rxComponent,
                    const css::uno::Reference< css::uno::XAggregation >& _rxComponentAggregate
                );

        /// makes the object forgetting the references to the database component
        void    dispose( );

        /// clears the instance data
                void    clearAllParameterInformation();

        /// checks whether the parameter information are up-to-date
        bool    isUpToDate() const { return m_bUpToDate; }

        /** updates all parameter information represented by the instance
        */
        void    updateParameterInfo( FilterManager& _rFilterManager );

        /** fills parameter values, as extensive as possible

            <p>In particular, all values which can be filled from the master-detail relationship of
            between our database component and its parent are filled in.</p>

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
                    const css::uno::Reference< css::task::XInteractionHandler >& _rxCompletionHandler,
                    ::osl::ResettableMutexGuard& _rClearForNotifies
                );

        /** sets all parameter values to null (via <member>XParameters::setNull</member>)

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>
        */
        void    setAllParametersNull();

        /** resets all detail columns which are, via a parameter, linked to a master column, to
            the value of this master column.

            For instance, if the database component is bound to a statement <code>SELECT * from invoice where inv_id = :cid</code>,
            and there is <em>one</em> master-detail link from

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>
        */
        void    resetParameterValues();

        /** adds the given listener to the list of parameter listeners
        */
        void    addParameterListener(
                    const css::uno::Reference< css::form::XDatabaseParameterListener >& _rxListener
                );

        /** removes the given listener from the list of parameter listeners
        */
        void    removeParameterListener(
                    const css::uno::Reference< css::form::XDatabaseParameterListener >& _rxListener
                );

        // XParameters equivalents
        void setNull            ( sal_Int32 _nIndex, sal_Int32 sqlType);
        void setObjectNull      ( sal_Int32 _nIndex, sal_Int32 sqlType, const OUString& typeName);
        void setBoolean         ( sal_Int32 _nIndex, bool x);
        void setByte            ( sal_Int32 _nIndex, sal_Int8 x);
        void setShort           ( sal_Int32 _nIndex, sal_Int16 x);
        void setInt             ( sal_Int32 _nIndex, sal_Int32 x);
        void setLong            ( sal_Int32 _nIndex, sal_Int64 x);
        void setFloat           ( sal_Int32 _nIndex, float x);
        void setDouble          ( sal_Int32 _nIndex, double x);
        void setString          ( sal_Int32 _nIndex, const OUString& x);
        void setBytes           ( sal_Int32 _nIndex, const css::uno::Sequence< sal_Int8 >& x);
        void setDate            ( sal_Int32 _nIndex, const css::util::Date& x);
        void setTime            ( sal_Int32 _nIndex, const css::util::Time& x);
        void setTimestamp       ( sal_Int32 _nIndex, const css::util::DateTime& x);
        void setBinaryStream    ( sal_Int32 _nIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length);
        void setCharacterStream ( sal_Int32 _nIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length);
        void setObject          ( sal_Int32 _nIndex, const css::uno::Any& x);
        void setObjectWithInfo  ( sal_Int32 _nIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale);
        void setRef             ( sal_Int32 _nIndex, const css::uno::Reference< css::sdbc::XRef>& x);
        void setBlob            ( sal_Int32 _nIndex, const css::uno::Reference< css::sdbc::XBlob>& x);
        void setClob            ( sal_Int32 _nIndex, const css::uno::Reference< css::sdbc::XClob>& x);
        void setArray           ( sal_Int32 _nIndex, const css::uno::Reference< css::sdbc::XArray>& x);
        void clearParameters();

    private:
        /// checks whether the object is already initialized, and not yet disposed
        bool    isAlive() const { return m_xComponent.get().is() && m_xInnerParamUpdate.is(); }

        /** creates a filter expression from a master-detail link where the detail denotes a column name
        */
        OUString
                createFilterConditionFromColumnLink(
                    const OUString& /* [in]  */ _rMasterColumn,
                    const css::uno::Reference< css::beans::XPropertySet >& /* [in]  */ xDetailColumn,
                          OUString& /* [out] */ _rNewParamName
                );

        /** initializes our query composer, and the collection of inner parameter columns

            @param _rxComponent
                the database component to initialize from. Must not be <NULL/>
            @return
                <TRUE/> if and only if the initialization was successful

            @postcond
                if and only if <TRUE/> is returned, then <member>m_xInnerParamColumns</member> contains the collection of
                inner parameters
        */
        bool    initializeComposerByComponent(
                    const css::uno::Reference< css::beans::XPropertySet >& _rxComponent
                );

        /** collects initial meta information about inner parameters (i.e. it initially fills
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

            @param  _out_rAdditionalHavingComponents
                the additional having clause components which are required for master-detail relationships where
                the detail part denotes a column name. In such a case, an additional filter needs to be created,
                containing a new parameter.

            @precond
                <member>m_aMasterFields</member> and <member>m_aDetailFields</member> have the same length
        */
        void    classifyLinks(
                    const css::uno::Reference< css::container::XNameAccess >& _rxParentColumns,
                    const css::uno::Reference< css::container::XNameAccess >& _rxColumns,
                    ::std::vector< OUString >& _out_rAdditionalFilterComponents,
                    ::std::vector< OUString >& _out_rAdditionalHavingComponents
                );

        /** finalizes our <member>m_pOuterParameters</member> so that it can be used for
            external parameter listeners

            @precond
                <member>m_pOuterParameters</member> is <NULL/>
            @precond
                <member>m_xInnerParamUpdate</member> is not <NULL/>
        */
        void    createOuterParameters();

        /** fills in the parameters values which result from the master-detail relationship
            between the database component and its parent

            @param _rxParentColumns
                the columns of the parameter database component. Must not be <NULL/>
            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>
        */
        void    fillLinkedParameters(
                    const css::uno::Reference< css::container::XNameAccess >& _rxParentColumns
                );

        /** completes all missing parameters via an interaction handler

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>

            @return
                <TRUE/> if and only if the parameter filling has <em>not</em> been cancelled by the user
        */
        bool    completeParameters(
                    const css::uno::Reference< css::task::XInteractionHandler >& _rxCompletionHandler,
                    const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
                );

        /** asks the parameter listeners to fill in final values

            @precond
                the instance is alive, i.e. <member>isAlive</member> returns <TRUE/>

            @return
                <TRUE/> if and only if the parameter filling has <em>not</em> been cancelled by the user
        */
        bool    consultParameterListeners( ::osl::ResettableMutexGuard& _rClearForNotifies );

        /** mark an externally filled parameter as visited
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
                    css::uno::Reference< css::container::XNameAccess >& /* [out] */ _out_rxParentColumns,
                    bool _bFromComposer
                );

        /** retrieves the columns of our database component

            @param _bFromComposer
                if <TRUE/>, the columns are obtained from the composer, else from the living database component itself
            @return
                <TRUE/> if and only if the columns could be successfully retrieved
        */
        bool    getColumns(
                    css::uno::Reference< css::container::XNameAccess >& /* [out] */ _rxColumns,
                    bool _bFromComposer
                );

        /** retrieves the active connection of the database component
        */
        void    getConnection(
                    css::uno::Reference< css::sdbc::XConnection >& /* [out] */ _rxConnection
                );

        /** caches some info about the connection of our database component
        */
        void    cacheConnectionInfo();

    private:
        ParameterManager( const ParameterManager& ) = delete;
        ParameterManager& operator=( const ParameterManager& ) = delete;
    };


} // namespacefrm


#endif // INCLUDED_CONNECTIVITY_PARAMETERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
