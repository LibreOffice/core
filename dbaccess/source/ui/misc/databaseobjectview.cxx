/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: databaseobjectview.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:03:00 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBACCESS_DATABASE_OBJECT_VIEW_HXX
#include "databaseobjectview.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBACCESS_ASYNCMODALDIALOG_HXX
#include "asyncmodaldialog.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMES_HPP_
#include <com/sun/star/frame/XFrames.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_APPLICATION_XTABLEUIPROVIDER_HPP_
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
/** === end UNO includes === **/

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

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
        :m_xORB         ( _rxORB         )
        ,m_xParentFrame ( _rxParentFrame )
        ,m_xFrameLoader (                )
        ,m_xApplication ( _rxApplication )
        ,m_sComponentURL( _rComponentURL )
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
    Reference< XComponent > DatabaseObjectView::createNew(const Reference< XDataSource >& _xDataSource )
    {
        return doCreateView( makeAny( _xDataSource ), ::rtl::OUString(), Sequence< PropertyValue >() );
    }

    //----------------------------------------------------------------------
    Reference< XComponent > DatabaseObjectView::openExisting( const Any& _rDataSource, const ::rtl::OUString& _rName, const Sequence< PropertyValue >& _rArgs )
    {
        return doCreateView( _rDataSource, _rName, _rArgs );
    }

    //----------------------------------------------------------------------
    Reference< XComponent > DatabaseObjectView::doCreateView( const Any& _rDataSource, const ::rtl::OUString& _rObjectName,
        const Sequence< PropertyValue >& _rCreationArgs )
    {
        Sequence< PropertyValue > aDispatchArgs;
        fillDispatchArgs( aDispatchArgs, _rDataSource, _rObjectName );
        return doDispatch( ::comphelper::concatSequences( aDispatchArgs, _rCreationArgs ) );
    }

    //----------------------------------------------------------------------
    Reference< XComponent > DatabaseObjectView::doDispatch( const Sequence< PropertyValue >& _rArgs )
    {
        Reference< XComponent > xReturn;
        if ( m_xORB.is() )
        {
            try
            {
                // get the desktop object
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
                }

                Reference< XComponentLoader > xFrameLoader(m_xFrameLoader, UNO_QUERY_THROW);
                xReturn = xFrameLoader->loadComponentFromURL(
                    m_sComponentURL,
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
                    0,
                    _rArgs
                );

                if ( xReturn.is() )
                    xReturn.set(m_xFrameLoader,UNO_QUERY);
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "DatabaseObjectView::getDispatcher: caught an exception while loading the component!");
            }
        }
        return xReturn;
    }

    //----------------------------------------------------------------------
    void DatabaseObjectView::fillDispatchArgs(
            Sequence< PropertyValue >& _rDispatchArguments,
            const Any& _aDataSource,
            const ::rtl::OUString& /* _rName */
        )
    {
        sal_Int32 nPos = _rDispatchArguments.getLength();
        _rDispatchArguments.realloc( nPos + 2 );

        ::rtl::OUString sDataSource;
        Reference<XDataSource> xDataSource;
        if ( _aDataSource >>= sDataSource )
        {
            _rDispatchArguments[nPos  ].Name = PROPERTY_DATASOURCENAME;
            _rDispatchArguments[nPos++].Value <<= sDataSource;
        }
        else if ( _aDataSource >>= xDataSource )
        {
            _rDispatchArguments[nPos  ].Name = PROPERTY_DATASOURCE;
            _rDispatchArguments[nPos++].Value <<= xDataSource;
        }

        _rDispatchArguments[nPos  ].Name = PROPERTY_ACTIVE_CONNECTION;
        _rDispatchArguments[nPos++].Value <<= getConnection();
    }

    //======================================================================
    //= QueryDesigner
    //======================================================================
    //----------------------------------------------------------------------
    QueryDesigner::QueryDesigner( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication,
        const Reference< XFrame >& _rxParentFrame,
        bool _bCreateView, sal_Bool _bPreferSQLView )
        :DatabaseObjectView( _rxORB, _rxApplication, _rxParentFrame, static_cast< ::rtl::OUString >( _bCreateView ? URL_COMPONENT_VIEWDESIGN : URL_COMPONENT_QUERYDESIGN ) )
        ,m_nCommandType( _bCreateView ? CommandType::TABLE : CommandType::QUERY )
        ,m_bPreferSQLView( _bPreferSQLView )
    {
    }

    //----------------------------------------------------------------------
    void QueryDesigner::fillDispatchArgs( Sequence< PropertyValue >& _rDispatchArguments, const Any& _aDataSource,
        const ::rtl::OUString& _rObjectName )
    {
        DatabaseObjectView::fillDispatchArgs( _rDispatchArguments, _aDataSource, _rObjectName );

        sal_Bool bIncludeQueryName = 0 != _rObjectName.getLength();

        sal_Int32 nPos = _rDispatchArguments.getLength();
        _rDispatchArguments.realloc(_rDispatchArguments.getLength() + 2 + ( bIncludeQueryName ? 1 : 0 ) );

        _rDispatchArguments[nPos  ].Name = PROPERTY_GRAPHICAL_DESIGN;
        _rDispatchArguments[nPos++].Value <<= ::cppu::bool2any( !m_bPreferSQLView );

        _rDispatchArguments[nPos  ].Name = PROPERTY_COMMAND_TYPE;
        _rDispatchArguments[nPos++].Value <<= m_nCommandType;

        if ( bIncludeQueryName )
        {
            _rDispatchArguments[nPos  ].Name = PROPERTY_COMMAND;
            _rDispatchArguments[nPos++].Value <<= _rObjectName;
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
    void TableDesigner::fillDispatchArgs( Sequence< PropertyValue >& _rDispatchArguments, const Any& _aDataSource,
        const ::rtl::OUString& _rObjectName )
    {
        DatabaseObjectView::fillDispatchArgs( _rDispatchArguments, _aDataSource, _rObjectName );
        sal_Bool bIncludeName = 0 != _rObjectName.getLength();

        if (bIncludeName)
        {
            sal_Int32 nPos = _rDispatchArguments.getLength();
            _rDispatchArguments.realloc(_rDispatchArguments.getLength() + 1);
            _rDispatchArguments[nPos  ].Name = PROPERTY_CURRENTTABLE;
            _rDispatchArguments[nPos++].Value <<= _rObjectName;
        }
    }

    //----------------------------------------------------------------------
    Reference< XComponent > TableDesigner::doCreateView( const Any& _rDataSource, const ::rtl::OUString& _rObjectName,
        const Sequence< PropertyValue >& _rCreationArgs )
    {
        bool bIsNewDesign = ( _rObjectName.getLength() == 0 );

        // let's see whether the connection can provide a dedicated table desginer
        Reference< XInterface > xDesigner;
        if ( !bIsNewDesign )
            xDesigner = impl_getConnectionProvidedDesigner_nothrow( _rObjectName );

        if ( !xDesigner.is() )
            return DatabaseObjectView::doCreateView( _rDataSource, _rObjectName, _rCreationArgs );

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
    void ResultSetBrowser::fillDispatchArgs(Sequence< PropertyValue >& _rDispatchArguments, const Any& _aDataSource,
        const ::rtl::OUString& _rQualifiedName)
    {
        DatabaseObjectView::fillDispatchArgs( _rDispatchArguments, _aDataSource, _rQualifiedName );
        OSL_ENSURE( 0 != _rQualifiedName.getLength(),"A Table name must be set");
        ::rtl::OUString sCatalog;
        ::rtl::OUString sSchema;
        ::rtl::OUString sTable;
        if ( m_bTable )
            ::dbtools::qualifiedNameComponents( getConnection()->getMetaData(), _rQualifiedName, sCatalog, sSchema, sTable, ::dbtools::eInDataManipulation );

        sal_Int32 nPos = _rDispatchArguments.getLength();
        _rDispatchArguments.realloc( _rDispatchArguments.getLength() + 3 + ( m_bTable ? 3 : 0 ) );

        _rDispatchArguments[nPos  ].Name = PROPERTY_COMMAND_TYPE;
        _rDispatchArguments[nPos++].Value <<= (m_bTable ? CommandType::TABLE : CommandType::QUERY);

        _rDispatchArguments[nPos  ].Name = PROPERTY_COMMAND;
        _rDispatchArguments[nPos++].Value <<= _rQualifiedName;

        _rDispatchArguments[nPos  ].Name = PROPERTY_ENABLE_BROWSER;
        _rDispatchArguments[nPos++].Value <<= sal_False;

        if ( m_bTable )
        {
            _rDispatchArguments[nPos  ].Name = PROPERTY_UPDATE_CATALOGNAME;
            _rDispatchArguments[nPos++].Value <<= sCatalog;
            _rDispatchArguments[nPos  ].Name = PROPERTY_UPDATE_SCHEMANAME;
            _rDispatchArguments[nPos++].Value <<= sSchema;
            _rDispatchArguments[nPos  ].Name = PROPERTY_UPDATE_TABLENAME;
            _rDispatchArguments[nPos++].Value <<= sTable;
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
    //======================================================================
    //= ReportDesigner
    //======================================================================
    //----------------------------------------------------------------------
    ReportDesigner::ReportDesigner( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication , const Reference< XFrame >& _rxParentFrame)
        :DatabaseObjectView( _rxORB, _rxApplication,  _rxParentFrame,static_cast< ::rtl::OUString >( URL_COMPONENT_REPORTDESIGN ) )
    {
    }


// .........................................................................
}   // namespace dbaui
// .........................................................................

