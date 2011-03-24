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
#include "precompiled_dbaccess.hxx"

#include "databaseobjectview.hxx"
#include "dbustrings.hrc"
#include "asyncmodaldialog.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
/** === end UNO includes === **/

#include <comphelper/extract.hxx>
#include <comphelper/sequence.hxx>
#include <connectivity/dbtools.hxx>
#include <osl/diagnose.h>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>

// .........................................................................
namespace dbaui
{
// .........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdb::application;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::awt;

    //======================================================================
    //= DatabaseObjectView
    //======================================================================
    DatabaseObjectView::DatabaseObjectView( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XDatabaseDocumentUI >& _rxApplication,
            const Reference< XFrame >& _rxParentFrame,
            const ::rtl::OUString& _rComponentURL )
        :m_xORB             ( _rxORB            )
        ,m_xParentFrame     ( _rxParentFrame    )
        ,m_xFrameLoader     (                   )
        ,m_xApplication     ( _rxApplication    )
        ,m_sComponentURL    ( _rComponentURL    )
    {
        OSL_ENSURE( m_xORB.is(), "DatabaseObjectView::DatabaseObjectView: invalid service factory!" );
        OSL_ENSURE( m_xApplication.is(), "DatabaseObjectView::DatabaseObjectView: invalid connection!" );
    }

    //----------------------------------------------------------------------
    Reference< XConnection > DatabaseObjectView::getConnection() const
    {
        Reference< XConnection > xConnection;
        if ( m_xApplication.is() )
            xConnection = m_xApplication->getActiveConnection();
        return xConnection;
    }

    //----------------------------------------------------------------------
    Reference< XComponent > DatabaseObjectView::createNew( const Reference< XDataSource >& _xDataSource, const ::comphelper::NamedValueCollection& i_rDispatchArgs )
    {
        return doCreateView( makeAny( _xDataSource ), ::rtl::OUString(), i_rDispatchArgs );
    }

    //----------------------------------------------------------------------
    Reference< XComponent > DatabaseObjectView::openExisting( const Any& _rDataSource, const ::rtl::OUString& _rName,
            const ::comphelper::NamedValueCollection& i_rDispatchArgs )
    {
        return doCreateView( _rDataSource, _rName, i_rDispatchArgs );
    }

    //----------------------------------------------------------------------
    Reference< XComponent > DatabaseObjectView::doCreateView( const Any& _rDataSource, const ::rtl::OUString& _rObjectName,
        const ::comphelper::NamedValueCollection& i_rCreationArgs )
    {
        ::comphelper::NamedValueCollection aDispatchArgs;

        aDispatchArgs.merge( i_rCreationArgs, false );    // false => do not overwrite
        fillDispatchArgs( aDispatchArgs, _rDataSource, _rObjectName );
        aDispatchArgs.merge( i_rCreationArgs, true );    // true => do overwrite

        return doDispatch( aDispatchArgs );
    }

    //----------------------------------------------------------------------
    Reference< XComponent > DatabaseObjectView::doDispatch( const ::comphelper::NamedValueCollection& i_rDispatchArgs )
    {
        Reference< XComponent > xReturn;
        if ( m_xORB.is() )
        {
            try
            {
                // if we have no externally provided frame, create one
                if ( !m_xFrameLoader.is() )
                {
                    Reference< XSingleServiceFactory > xFact(m_xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.TaskCreator")), UNO_QUERY_THROW);
                    Sequence< Any > lArgs(2);
                    NamedValue      aProp;
                    sal_Int32       nArg = 0;

                    aProp.Name    = ::rtl::OUString::createFromAscii("ParentFrame");
                    aProp.Value <<= m_xParentFrame;
                    lArgs[nArg++] <<= aProp;

                    aProp.Name    = ::rtl::OUString::createFromAscii("TopWindow");
                    aProp.Value <<= sal_True;
                    lArgs[nArg++] <<= aProp;

                    m_xFrameLoader.set(xFact->createInstanceWithArguments(lArgs), UNO_QUERY_THROW);

                    // everything we load can be considered a "top level document", so set the respective bit at the window.
                    // This, amongst other things, triggers that the component in this task participates in the
                    // "ThisComponent"-game for the global application Basic.
                    const Reference< XFrame > xFrame( m_xFrameLoader, UNO_QUERY_THROW );
                    const Reference< XWindow > xFrameWindow( xFrame->getContainerWindow(), UNO_SET_THROW );
                    Window* pContainerWindow = VCLUnoHelper::GetWindow( xFrameWindow );
                    ENSURE_OR_THROW( pContainerWindow, "no implementation access to the frame's container window!" );
                    pContainerWindow->SetExtendedStyle( pContainerWindow->GetExtendedStyle() | WB_EXT_DOCUMENT );
                }

                Reference< XComponentLoader > xFrameLoader( m_xFrameLoader, UNO_QUERY_THROW );
                xReturn = xFrameLoader->loadComponentFromURL(
                    m_sComponentURL,
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
                    0,
                    i_rDispatchArgs.getPropertyValues()
                );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return xReturn;
    }

    //----------------------------------------------------------------------
    void DatabaseObjectView::fillDispatchArgs(
            ::comphelper::NamedValueCollection& i_rDispatchArgs,
            const Any& _aDataSource,
            const ::rtl::OUString& /* _rName */
        )
    {
        ::rtl::OUString sDataSource;
        Reference<XDataSource> xDataSource;
        if ( _aDataSource >>= sDataSource )
        {
            i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_DATASOURCENAME, sDataSource );
        }
        else if ( _aDataSource >>= xDataSource )
        {
            i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_DATASOURCE, xDataSource );
        }

        i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_ACTIVE_CONNECTION, getConnection() );
    }

    //======================================================================
    //= QueryDesigner
    //======================================================================
    //----------------------------------------------------------------------
    QueryDesigner::QueryDesigner( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication,
        const Reference< XFrame >& _rxParentFrame, bool _bCreateView )
        :DatabaseObjectView( _rxORB, _rxApplication, _rxParentFrame, _bCreateView ? URL_COMPONENT_VIEWDESIGN : URL_COMPONENT_QUERYDESIGN )
        ,m_nCommandType( _bCreateView ? CommandType::TABLE : CommandType::QUERY )
    {
    }

    //----------------------------------------------------------------------
    void QueryDesigner::fillDispatchArgs( ::comphelper::NamedValueCollection& i_rDispatchArgs, const Any& _aDataSource,
        const ::rtl::OUString& _rObjectName )
    {
        DatabaseObjectView::fillDispatchArgs( i_rDispatchArgs, _aDataSource, _rObjectName );

        const bool bIncludeQueryName = 0 != _rObjectName.getLength();
        const bool bGraphicalDesign = i_rDispatchArgs.getOrDefault( (::rtl::OUString)PROPERTY_GRAPHICAL_DESIGN, sal_True );
        const bool bEditViewAsSQLCommand = ( m_nCommandType == CommandType::TABLE ) && !bGraphicalDesign;

        i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_COMMAND_TYPE, m_nCommandType );

        if ( bIncludeQueryName )
        {
            i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_COMMAND, _rObjectName );
        }

        if ( bEditViewAsSQLCommand )
        {
            i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_ESCAPE_PROCESSING, sal_False );
        }
    }

    //======================================================================
    //= TableDesigner
    //======================================================================
    //----------------------------------------------------------------------
    TableDesigner::TableDesigner( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication, const Reference< XFrame >& _rxParentFrame )
        :DatabaseObjectView( _rxORB, _rxApplication, _rxParentFrame, static_cast< ::rtl::OUString >( URL_COMPONENT_TABLEDESIGN ) )
    {
    }

    //----------------------------------------------------------------------
    void TableDesigner::fillDispatchArgs( ::comphelper::NamedValueCollection& i_rDispatchArgs, const Any& _aDataSource,
        const ::rtl::OUString& _rObjectName )
    {
        DatabaseObjectView::fillDispatchArgs( i_rDispatchArgs, _aDataSource, _rObjectName );

        if ( 0 != _rObjectName.getLength() )
        {
            i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_CURRENTTABLE, _rObjectName );
        }
    }

    //----------------------------------------------------------------------
    Reference< XComponent > TableDesigner::doCreateView( const Any& _rDataSource, const ::rtl::OUString& _rObjectName,
        const ::comphelper::NamedValueCollection& i_rCreationArgs )
    {
        bool bIsNewDesign = ( _rObjectName.getLength() == 0 );

        // let's see whether the connection can provide a dedicated table desginer
        Reference< XInterface > xDesigner;
        if ( !bIsNewDesign )
            xDesigner = impl_getConnectionProvidedDesigner_nothrow( _rObjectName );

        if ( !xDesigner.is() )
            return DatabaseObjectView::doCreateView( _rDataSource, _rObjectName, i_rCreationArgs );

        // try whether the designer is a dialog
        Reference< XExecutableDialog > xDialog( xDesigner, UNO_QUERY_THROW );
        if ( xDialog.is() )
        {
            try { AsyncDialogExecutor::executeModalDialogAsync( xDialog ); }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
            return NULL;
        }

        Reference< XComponent > xDesignerComponent( xDesigner, UNO_QUERY );
        OSL_ENSURE( xDesignerComponent.is(), "TableDesigner::doCreateView: a designer which is no dialog and no component?" );
        return xDesignerComponent;
    }

    //----------------------------------------------------------------------
    Reference< XInterface > TableDesigner::impl_getConnectionProvidedDesigner_nothrow( const ::rtl::OUString& _rTableName )
    {
        Reference< XInterface > xDesigner;
        try
        {
            Reference< XTableUIProvider > xTableUIProv( getConnection(), UNO_QUERY );
            if ( xTableUIProv.is() )
                xDesigner = xTableUIProv->getTableEditor( getApplicationUI(), _rTableName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xDesigner;
    }

    //======================================================================
    //= ResultSetBrowser
    //======================================================================
    //----------------------------------------------------------------------
    ResultSetBrowser::ResultSetBrowser( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication, const Reference< XFrame >& _rxParentFrame,
            sal_Bool _bTable )
        :DatabaseObjectView( _rxORB, _rxApplication, _rxParentFrame, static_cast < ::rtl::OUString >( URL_COMPONENT_DATASOURCEBROWSER ) )
        ,m_bTable(_bTable)
    {
    }

    //----------------------------------------------------------------------
    void ResultSetBrowser::fillDispatchArgs( ::comphelper::NamedValueCollection& i_rDispatchArgs, const Any& _aDataSource,
        const ::rtl::OUString& _rQualifiedName)
    {
        DatabaseObjectView::fillDispatchArgs( i_rDispatchArgs, _aDataSource, _rQualifiedName );
        OSL_ENSURE( 0 != _rQualifiedName.getLength(),"A Table name must be set");
        ::rtl::OUString sCatalog;
        ::rtl::OUString sSchema;
        ::rtl::OUString sTable;
        if ( m_bTable )
            ::dbtools::qualifiedNameComponents( getConnection()->getMetaData(), _rQualifiedName, sCatalog, sSchema, sTable, ::dbtools::eInDataManipulation );

        i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_COMMAND_TYPE, (m_bTable ? CommandType::TABLE : CommandType::QUERY) );
        i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_COMMAND, _rQualifiedName );
        i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_ENABLE_BROWSER, sal_False );

        if ( m_bTable )
        {
            i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_UPDATE_CATALOGNAME, sCatalog );
            i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_UPDATE_SCHEMANAME, sSchema );
            i_rDispatchArgs.put( (::rtl::OUString)PROPERTY_UPDATE_TABLENAME, sTable );
        }
    }

    //======================================================================
    //= RelationDesigner
    //======================================================================
    //----------------------------------------------------------------------
    RelationDesigner::RelationDesigner( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication, const Reference< XFrame >& _rxParentFrame )
        :DatabaseObjectView( _rxORB, _rxApplication, _rxParentFrame, static_cast< ::rtl::OUString >( URL_COMPONENT_RELATIONDESIGN ) )
    {
    }
// .........................................................................
}   // namespace dbaui
// .........................................................................

