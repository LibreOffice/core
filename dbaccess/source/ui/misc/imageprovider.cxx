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

#include "imageprovider.hxx"
#include "dbu_resource.hrc"
#include "moduledbu.hxx"
#include "dbustrings.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicColorMode.hpp>
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::graphic::XGraphic;
    using ::com::sun::star::sdb::application::XTableUIProvider;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::sdbcx::XViewsSupplier;
    using ::com::sun::star::uno::UNO_SET_THROW;
    /** === end UNO using === **/
    namespace GraphicColorMode = ::com::sun::star::graphic::GraphicColorMode;

    //====================================================================
    //= ImageProvider_Data
    //====================================================================
    struct ImageProvider_Data
    {
        /// the connection we work with
        Reference< XConnection >        xConnection;
        /// the views of the connection, if the DB supports views
        Reference< XNameAccess >        xViews;
        /// interface for providing table's UI
        Reference< XTableUIProvider >   xTableUI;
    };

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        static void lcl_getConnectionProvidedTableIcon_nothrow(  const ImageProvider_Data& _rData,
            const ::rtl::OUString& _rName, Reference< XGraphic >& _out_rxGraphic, Reference< XGraphic >& _out_rxGraphicHC )
        {
            try
            {
                if ( _rData.xTableUI.is() )
                {
                    _out_rxGraphic = _rData.xTableUI->getTableIcon( _rName, GraphicColorMode::NORMAL );
                    _out_rxGraphicHC = _rData.xTableUI->getTableIcon( _rName, GraphicColorMode::HIGH_CONTRAST );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        //................................................................
        static void lcl_getTableImageResourceID_nothrow( const ImageProvider_Data& _rData, const ::rtl::OUString& _rName,
            sal_uInt16& _out_rResourceID, sal_uInt16& _out_rResourceID_HC )
        {
            _out_rResourceID = _out_rResourceID_HC = 0;
            try
            {
                bool bIsView = _rData.xViews.is() && _rData.xViews->hasByName( _rName );
                if ( bIsView )
                {
                    _out_rResourceID = VIEW_TREE_ICON;
                    _out_rResourceID_HC = VIEW_TREE_ICON_SCH;
                }
                else
                {
                    _out_rResourceID = TABLE_TREE_ICON;
                    _out_rResourceID_HC = TABLE_TREE_ICON_SCH;
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
    //====================================================================
    //= ImageProvider
    //====================================================================
    //--------------------------------------------------------------------
    ImageProvider::ImageProvider()
        :m_pData( new ImageProvider_Data )
    {
    }

    //--------------------------------------------------------------------
    ImageProvider::ImageProvider( const Reference< XConnection >& _rxConnection )
        :m_pData( new ImageProvider_Data )
    {
        m_pData->xConnection = _rxConnection;
        try
        {
            Reference< XViewsSupplier > xSuppViews( m_pData->xConnection, UNO_QUERY );
            if ( xSuppViews.is() )
                m_pData->xViews.set( xSuppViews->getViews(), UNO_SET_THROW );

            m_pData->xTableUI.set( _rxConnection, UNO_QUERY );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void ImageProvider::getImages( const String& _rName, const sal_Int32 _nDatabaseObjectType, Image& _out_rImage, Image& _out_rImageHC )
    {
        if ( _nDatabaseObjectType != DatabaseObject::TABLE )
        {
            // for types other than tables, the icon does not depend on the concrete object
            _out_rImage = getDefaultImage( _nDatabaseObjectType, false );
            _out_rImageHC = getDefaultImage( _nDatabaseObjectType, true );
        }
        else
        {
            // check whether the connection can give us an icon
            Reference< XGraphic > xGraphic;
            Reference< XGraphic > xGraphicHC;
            lcl_getConnectionProvidedTableIcon_nothrow( *m_pData, _rName, xGraphic, xGraphicHC );
            if ( xGraphic.is() )
                _out_rImage = Image( xGraphic );
            if ( xGraphicHC.is() )
                _out_rImageHC = Image( xGraphicHC );

            if ( !_out_rImage || !_out_rImageHC )
            {
                // no -> determine by type
                sal_uInt16 nImageResourceID = 0;
                sal_uInt16 nImageResourceID_HC = 0;
                lcl_getTableImageResourceID_nothrow( *m_pData, _rName, nImageResourceID, nImageResourceID_HC );

                if ( nImageResourceID && !_out_rImage )
                    _out_rImage = Image( ModuleRes( nImageResourceID ) );
                if ( nImageResourceID_HC && !_out_rImageHC )
                    _out_rImageHC = Image( ModuleRes( nImageResourceID_HC ) );
            }
        }
    }

    //--------------------------------------------------------------------
    Image ImageProvider::getDefaultImage( sal_Int32 _nDatabaseObjectType, bool _bHighContrast )
    {
        Image aObjectImage;
        sal_uInt16 nImageResourceID( getDefaultImageResourceID( _nDatabaseObjectType, _bHighContrast ) );
        if ( nImageResourceID )
            aObjectImage = Image( ModuleRes( nImageResourceID ) );
        return aObjectImage;
    }

    //--------------------------------------------------------------------
    sal_uInt16 ImageProvider::getDefaultImageResourceID( sal_Int32 _nDatabaseObjectType, bool _bHighContrast )
    {
        sal_uInt16 nImageResourceID( 0 );
        switch ( _nDatabaseObjectType )
        {
        case DatabaseObject::QUERY:
            nImageResourceID = _bHighContrast ? QUERY_TREE_ICON_SCH : QUERY_TREE_ICON;
            break;
        case DatabaseObject::FORM:
            nImageResourceID = _bHighContrast ? FORM_TREE_ICON_SCH : FORM_TREE_ICON;
            break;
        case DatabaseObject::REPORT:
            nImageResourceID = _bHighContrast ? REPORT_TREE_ICON_SCH : REPORT_TREE_ICON;
            break;
        case DatabaseObject::TABLE:
            nImageResourceID = _bHighContrast ? TABLE_TREE_ICON_SCH : TABLE_TREE_ICON;
            break;
        default:
            OSL_ENSURE( false, "ImageProvider::getDefaultImage: invalid database object type!" );
            break;
        }
        return nImageResourceID;
    }

    //--------------------------------------------------------------------
    Image ImageProvider::getFolderImage( sal_Int32 _nDatabaseObjectType, bool _bHighContrast )
    {
        sal_uInt16 nImageResourceID( 0 );
        switch ( _nDatabaseObjectType )
        {
        case DatabaseObject::QUERY:
            nImageResourceID = _bHighContrast ? QUERYFOLDER_TREE_ICON_SCH : QUERYFOLDER_TREE_ICON;
            break;
        case DatabaseObject::FORM:
            nImageResourceID = _bHighContrast ? FORMFOLDER_TREE_ICON_SCH : FORMFOLDER_TREE_ICON;
            break;
        case DatabaseObject::REPORT:
            nImageResourceID = _bHighContrast ? REPORTFOLDER_TREE_ICON_SCH : REPORTFOLDER_TREE_ICON;
            break;
        case DatabaseObject::TABLE:
            nImageResourceID = _bHighContrast ? TABLEFOLDER_TREE_ICON_SCH : TABLEFOLDER_TREE_ICON;
            break;
        default:
            OSL_ENSURE( false, "ImageProvider::getDefaultImage: invalid database object type!" );
            break;
        }

        Image aFolderImage;
        if ( nImageResourceID )
            aFolderImage = Image( ModuleRes( nImageResourceID ) );
        return aFolderImage;
    }

    //--------------------------------------------------------------------
    Image ImageProvider::getDatabaseImage( bool _bHighContrast )
    {
        return Image( ModuleRes( _bHighContrast ? DATABASE_TREE_ICON_SCH : DATABASE_TREE_ICON ) );
    }

//........................................................................
} // namespace dbaui
//........................................................................

