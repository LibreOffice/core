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

#include <comphelper/diagnose_ex.hxx>

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

    namespace
    {
        void lcl_getConnectionProvidedTableIcon_nothrow(
            const css::uno::Reference< css::sdb::application::XTableUIProvider >& _xTableUI,
            const OUString& _rName, Reference< XGraphic >& _out_rxGraphic )
        {
            try
            {
                if ( _xTableUI.is() )
                    _out_rxGraphic = _xTableUI->getTableIcon( _rName, GraphicColorMode::NORMAL );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }

        void lcl_getTableImageResourceID_nothrow(
            const css::uno::Reference< css::container::XNameAccess >& _xViews,
            const OUString& _rName,
            OUString& _out_rResourceID)
        {
            _out_rResourceID = OUString();
            try
            {
                bool bIsView = _xViews.is() && _xViews->hasByName( _rName );
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
    {
    }

    ImageProvider::ImageProvider( const Reference< XConnection >& _rxConnection )
        : mxConnection(_rxConnection)
    {
        try
        {
            Reference< XViewsSupplier > xSuppViews( mxConnection, UNO_QUERY );
            if ( xSuppViews.is() )
                mxViews.set( xSuppViews->getViews(), UNO_SET_THROW );

            mxTableUI.set( _rxConnection, UNO_QUERY );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
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
            lcl_getTableImageResourceID_nothrow( mxViews, _rName, sImageResourceID );
            return sImageResourceID;
        }
    }

    Reference<XGraphic> ImageProvider::getXGraphic(const OUString& _rName, const sal_Int32 _nDatabaseObjectType)
    {
        Reference<XGraphic> xGraphic;
        if (_nDatabaseObjectType == DatabaseObject::TABLE)
        {
            // check whether the connection can give us an icon
            lcl_getConnectionProvidedTableIcon_nothrow( mxTableUI, _rName, xGraphic );
        }
        return xGraphic;
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

    const OUString & ImageProvider::getDatabaseImage()
    {
        return DATABASE_TREE_ICON;
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
