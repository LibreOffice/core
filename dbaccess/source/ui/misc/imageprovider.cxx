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

#include <imageprovider.hxx>
#include <bitmaps.hlst>

#include <com/sun/star/graphic/GraphicColorMode.hpp>
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>

#include <tools/diagnose_ex.h>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::graphic::XGraphic;
    using ::com::sun::star::sdb::application::XTableUIProvider;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::sdbcx::XViewsSupplier;
    using ::com::sun::star::uno::UNO_SET_THROW;

    namespace GraphicColorMode = css::graphic::GraphicColorMode;
    namespace DatabaseObject = css::sdb::application::DatabaseObject;

    // ImageProvider_Data
    struct ImageProvider_Data
    {
        /// the connection we work with
        Reference< XConnection >        xConnection;
        /// the views of the connection, if the DB supports views
        Reference< XNameAccess >        xViews;
        /// interface for providing table's UI
        Reference< XTableUIProvider >   xTableUI;
    };

    namespace
    {
        void lcl_getConnectionProvidedTableIcon_nothrow(  const ImageProvider_Data& _rData,
            const OUString& _rName, Reference< XGraphic >& _out_rxGraphic )
        {
            try
            {
                if ( _rData.xTableUI.is() )
                    _out_rxGraphic = _rData.xTableUI->getTableIcon( _rName, GraphicColorMode::NORMAL );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }

        void lcl_getTableImageResourceID_nothrow( const ImageProvider_Data& _rData, const OUString& _rName,
            OUString& _out_rResourceID)
        {
            _out_rResourceID = OUString();
            try
            {
                bool bIsView = _rData.xViews.is() && _rData.xViews->hasByName( _rName );
                if ( bIsView )
                {
                    _out_rResourceID = VIEW_TREE_ICON;
                }
                else
                {
                    _out_rResourceID = TABLE_TREE_ICON;
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
    }
    // ImageProvider
    ImageProvider::ImageProvider()
        :m_pData( std::make_shared<ImageProvider_Data>() )
    {
    }

    ImageProvider::ImageProvider( const Reference< XConnection >& _rxConnection )
        :m_pData( std::make_shared<ImageProvider_Data>() )
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
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }

    void ImageProvider::getImages( const OUString& _rName, const sal_Int32 _nDatabaseObjectType, Image& _out_rImage )
    {
        if ( _nDatabaseObjectType != DatabaseObject::TABLE )
        {
            // for types other than tables, the icon does not depend on the concrete object
            _out_rImage = getDefaultImage( _nDatabaseObjectType );
        }
        else
        {
            // check whether the connection can give us an icon
            Reference< XGraphic > xGraphic;
            lcl_getConnectionProvidedTableIcon_nothrow( *m_pData, _rName, xGraphic );
            if ( xGraphic.is() )
                _out_rImage = Image( xGraphic );

            if ( !_out_rImage )
            {
                // no -> determine by type
                OUString sImageResourceID;
                lcl_getTableImageResourceID_nothrow( *m_pData, _rName, sImageResourceID );

                if (!sImageResourceID.isEmpty() && !_out_rImage)
                    _out_rImage = Image(StockImage::Yes, sImageResourceID);
            }
        }
    }

    OUString ImageProvider::getImageId(const OUString& _rName, const sal_Int32 _nDatabaseObjectType)
    {
        if (_nDatabaseObjectType != DatabaseObject::TABLE)
        {
            // for types other than tables, the icon does not depend on the concrete object
            return getDefaultImageResourceID( _nDatabaseObjectType );
        }
        else
        {
            // no -> determine by type
            OUString sImageResourceID;
            lcl_getTableImageResourceID_nothrow( *m_pData, _rName, sImageResourceID );
            return sImageResourceID;
        }
    }

    Reference<XGraphic> ImageProvider::getXGraphic(const OUString& _rName, const sal_Int32 _nDatabaseObjectType)
    {
        Reference<XGraphic> xGraphic;
        if (_nDatabaseObjectType == DatabaseObject::TABLE)
        {
            // check whether the connection can give us an icon
            lcl_getConnectionProvidedTableIcon_nothrow( *m_pData, _rName, xGraphic );
        }
        return xGraphic;
    }

    Image ImageProvider::getDefaultImage( sal_Int32 _nDatabaseObjectType )
    {
        Image aObjectImage;
        OUString sImageResourceID( getDefaultImageResourceID( _nDatabaseObjectType) );
        if (!sImageResourceID.isEmpty())
            aObjectImage = Image(StockImage::Yes, sImageResourceID);
        return aObjectImage;
    }

    OUString ImageProvider::getDefaultImageResourceID( sal_Int32 _nDatabaseObjectType)
    {
        OUString sImageResourceID;
        switch ( _nDatabaseObjectType )
        {
        case DatabaseObject::QUERY:
            sImageResourceID = QUERY_TREE_ICON;
            break;
        case DatabaseObject::FORM:
            sImageResourceID = FORM_TREE_ICON;
            break;
        case DatabaseObject::REPORT:
            sImageResourceID = REPORT_TREE_ICON;
            break;
        case DatabaseObject::TABLE:
            sImageResourceID = TABLE_TREE_ICON;
            break;
        default:
            OSL_FAIL( "ImageProvider::getDefaultImage: invalid database object type!" );
            break;
        }
        return sImageResourceID;
    }

    Image ImageProvider::getFolderImage( sal_Int32 _nDatabaseObjectType )
    {
        OUString sImageResourceID;
        switch ( _nDatabaseObjectType )
        {
        case DatabaseObject::QUERY:
            sImageResourceID = QUERYFOLDER_TREE_ICON;
            break;
        case DatabaseObject::FORM:
            sImageResourceID = FORMFOLDER_TREE_ICON;
            break;
        case DatabaseObject::REPORT:
            sImageResourceID = REPORTFOLDER_TREE_ICON;
            break;
        case DatabaseObject::TABLE:
            sImageResourceID = TABLEFOLDER_TREE_ICON;
            break;
        default:
            OSL_FAIL( "ImageProvider::getDefaultImage: invalid database object type!" );
            break;
        }

        Image aFolderImage;
        if (!sImageResourceID.isEmpty())
            aFolderImage = Image(StockImage::Yes, sImageResourceID);
        return aFolderImage;
    }

    OUString ImageProvider::getFolderImageId( sal_Int32 _nDatabaseObjectType )
    {
        OUString sImageResourceID;
        switch ( _nDatabaseObjectType )
        {
        case DatabaseObject::QUERY:
            sImageResourceID = QUERYFOLDER_TREE_ICON;
            break;
        case DatabaseObject::FORM:
            sImageResourceID = FORMFOLDER_TREE_ICON;
            break;
        case DatabaseObject::REPORT:
            sImageResourceID = REPORTFOLDER_TREE_ICON;
            break;
        case DatabaseObject::TABLE:
            sImageResourceID = TABLEFOLDER_TREE_ICON;
            break;
        default:
            OSL_FAIL( "ImageProvider::getDefaultImage: invalid database object type!" );
            break;
        }

        return sImageResourceID;
    }

    OUString ImageProvider::getDatabaseImage()
    {
        return DATABASE_TREE_ICON;
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
