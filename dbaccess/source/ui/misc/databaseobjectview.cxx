/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: databaseobjectview.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:49:05 $
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

    //======================================================================
    //= DatabaseObjectView
    //======================================================================
    DatabaseObjectView::DatabaseObjectView( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XDatabaseDocumentUI >& _rxApplication, const ::rtl::OUString& _rComponentURL )
        :m_xORB( _rxORB )
        ,m_xApplication( _rxApplication )
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
                sal_Int32 nFrameSearchFlag = FrameSearchFlag::SELF;
                ::rtl::OUString sTarget(RTL_CONSTASCII_USTRINGPARAM("_self"));
                if ( !m_xFrameLoader.is() )
                {
                    m_xFrameLoader.set(m_xORB->createInstance(SERVICE_FRAME_DESKTOP),UNO_QUERY);
                    nFrameSearchFlag = FrameSearchFlag::TASKS | FrameSearchFlag::CREATE;
                    sTarget = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank"));
                }

                OSL_ENSURE( m_xFrameLoader.is(), "DatabaseObjectView::doDispatch: invalid frame loader!" );

                if ( m_xFrameLoader.is() )
                {
                    // create a new frame and remove it from the desktop, so we care for it
                    Reference<XFrame> xFrame = Reference<XFrame>(m_xFrameLoader,UNO_QUERY)->findFrame(sTarget,nFrameSearchFlag);
                    /*
                    ... no ... dont remove the frame from the desktop tree.
                    Otherwhise it can happen that sometimes no active frame will be defined
                    and some funstions behind the menu of this frame cant work.

                    see #124984# for further informations
                    */

                    Reference<XFramesSupplier> xSup(m_xFrameLoader,UNO_QUERY);
                    if ( xSup.is() )
                    {
                        Reference<XFrames> xFrames = xSup->getFrames();
                        xFrames->remove(xFrame);
                    }

                    Reference<XComponentLoader> xFrameLoad(xFrame,UNO_QUERY);
                    if ( xFrameLoad.is() )
                    {
                        xReturn = xFrameLoad->loadComponentFromURL(
                            m_sComponentURL,
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
                            FrameSearchFlag::SELF,
                            _rArgs
                        );
                        if ( xReturn.is() )
                            xReturn.set(xFrame,UNO_QUERY);
                    }
                }
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

        _rDispatchArguments[nPos  ].Name = PROPERTY_ACTIVECONNECTION;
        _rDispatchArguments[nPos++].Value <<= getConnection();
    }

    //======================================================================
    //= QueryDesigner
    //======================================================================
    //----------------------------------------------------------------------
    QueryDesigner::QueryDesigner( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication,
        sal_Bool _bCreateView, sal_Bool _bPreferSQLView )
        :DatabaseObjectView( _rxORB, _rxApplication, static_cast< ::rtl::OUString >( URL_COMPONENT_QUERYDESIGN ) )
        ,m_bCreateView( _bCreateView )
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
        _rDispatchArguments.realloc(_rDispatchArguments.getLength() + 2 + (bIncludeQueryName ? 1 : 0) );


        _rDispatchArguments[nPos  ].Name = PROPERTY_QUERYDESIGNVIEW;
        _rDispatchArguments[nPos++].Value <<= ::cppu::bool2any(!m_bPreferSQLView);

        _rDispatchArguments[nPos  ].Name = PROPERTY_CREATEVIEW;
        _rDispatchArguments[nPos++].Value <<= ::cppu::bool2any(m_bCreateView);

        if (bIncludeQueryName)
        {
            _rDispatchArguments[nPos  ].Name = PROPERTY_CURRENTQUERY;
            _rDispatchArguments[nPos++].Value <<= _rObjectName;
        }
    }

    //======================================================================
    //= TableDesigner
    //======================================================================
    //----------------------------------------------------------------------
    TableDesigner::TableDesigner( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication )
        :DatabaseObjectView( _rxORB, _rxApplication, static_cast< ::rtl::OUString >( URL_COMPONENT_TABLEDESIGN ) )
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
    ResultSetBrowser::ResultSetBrowser( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication,
            sal_Bool _bTable )
        :DatabaseObjectView( _rxORB, _rxApplication, static_cast < ::rtl::OUString >( URL_COMPONENT_DATASOURCEBROWSER ) )
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
        _rDispatchArguments.realloc( _rDispatchArguments.getLength() + 4 + (m_bTable ? 3 : 0) );

        _rDispatchArguments[nPos  ].Name = PROPERTY_COMMANDTYPE;
        _rDispatchArguments[nPos++].Value <<= (m_bTable ? CommandType::TABLE : CommandType::QUERY);

        _rDispatchArguments[nPos  ].Name = PROPERTY_COMMAND;
        _rDispatchArguments[nPos++].Value <<= _rQualifiedName;

        _rDispatchArguments[nPos  ].Name = PROPERTY_SHOWTREEVIEW;
        _rDispatchArguments[nPos++].Value <<= sal_False;

        _rDispatchArguments[nPos  ].Name = PROPERTY_SHOWTREEVIEWBUTTON;
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
    RelationDesigner::RelationDesigner( const Reference< XMultiServiceFactory >& _rxORB, const Reference< XDatabaseDocumentUI >& _rxApplication )
        :DatabaseObjectView( _rxORB, _rxApplication, static_cast< ::rtl::OUString >( URL_COMPONENT_RELATIONDESIGN ) )
    {
    }

// .........................................................................
}   // namespace dbaui
// .........................................................................

