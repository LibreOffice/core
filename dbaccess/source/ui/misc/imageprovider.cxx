/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imageprovider.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:49:19 $
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

#ifndef DBACCESS_IMAGEPROVIDER_HXX
#include "imageprovider.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_GRAPHICCOLORMODE_HPP_
#include <com/sun/star/graphic/GraphicColorMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_APPLICATION_XTABLEUIPROVIDER_HPP_
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#endif
/** === end UNO includes === **/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::sdbcx::XTablesSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::graphic::XGraphic;
    using ::com::sun::star::sdb::application::XTableUIProvider;
    using ::com::sun::star::uno::UNO_QUERY;
    /** === end UNO using === **/
    namespace GraphicColorMode = ::com::sun::star::graphic::GraphicColorMode;

    //====================================================================
    //= ImageProvider_Impl
    //====================================================================
    struct ImageProvider_Impl
    {
        Reference< XConnection >    xConnection;
    };

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        static Reference< XGraphic > lcl_getConnectionProvidedTableIcon_nothrow(
            const Reference< XConnection >& _rxConnection, const ::rtl::OUString& _rName, bool _bHighContrast )
        {
            try
            {
                Reference< XTableUIProvider > xProvider( _rxConnection, UNO_QUERY );
                if ( xProvider.is() )
                    return xProvider->getTableIcon( _rName, _bHighContrast ? GraphicColorMode::HIGH_CONTRAST : GraphicColorMode::NORMAL );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return NULL;
        }

        //................................................................
        static USHORT lcl_getTableImageResourceID_nothrow( const Reference< XConnection >& _rxConnection, const ::rtl::OUString& _rName, bool _bHighContrast )
        {
            USHORT nResourceID( 0 );
            try
            {
                Reference< XTablesSupplier > xTableSupp( _rxConnection, UNO_QUERY_THROW );
                Reference< XNameAccess > xTables( xTableSupp->getTables(), UNO_QUERY_THROW );

                OSL_ENSURE( xTables->hasByName( _rName ), "lcl_getTableImageResourceID_nothrow: table with the given name does not exist!" );

                Reference< XPropertySet > xTableProps( xTables->getByName( _rName ), UNO_QUERY_THROW );
                ::rtl::OUString sTableType;
                OSL_VERIFY( xTableProps->getPropertyValue( PROPERTY_TYPE ) >>= sTableType );
                bool bIsView = sTableType.equalsAscii( "VIEW" );

                if ( bIsView )
                    nResourceID = _bHighContrast ? VIEW_TREE_ICON_SCH : VIEW_TREE_ICON;
                else
                    nResourceID = _bHighContrast ? TABLE_TREE_ICON_SCH : TABLE_TREE_ICON;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return nResourceID;
        }
    }
    //====================================================================
    //= ImageProvider
    //====================================================================
    //--------------------------------------------------------------------
    ImageProvider::ImageProvider()
        :m_pImpl( new ImageProvider_Impl )
    {
    }

    //--------------------------------------------------------------------
    ImageProvider::ImageProvider( const Reference< XConnection >& _rxConnection )
        :m_pImpl( new ImageProvider_Impl )
    {
        m_pImpl->xConnection = _rxConnection;
    }

    //--------------------------------------------------------------------
    Image ImageProvider::getImage( const String& _rName, sal_Int32 _nDatabaseObjectType, bool _bHighContrast )
    {
        Image aObjectImage;

        if ( _nDatabaseObjectType != DatabaseObject::TABLE )
            // for types other than tables, the icon does not depend on the concrete object
            aObjectImage = getDefaultImage( _nDatabaseObjectType, _bHighContrast );
        else
        {
            // check whether the connection can give us an icon
            Reference< XGraphic > xGraphic = lcl_getConnectionProvidedTableIcon_nothrow( m_pImpl->xConnection, _rName, _bHighContrast );
            if ( xGraphic.is() )
                aObjectImage = Image( xGraphic );

            if ( !aObjectImage )
            {
                // no -> determine by type
                USHORT nImageResourceID = lcl_getTableImageResourceID_nothrow( m_pImpl->xConnection, _rName, _bHighContrast );
                if ( nImageResourceID )
                    aObjectImage = Image( ModuleRes( nImageResourceID ) );
            }
        }

        return aObjectImage;
    }

    //--------------------------------------------------------------------
    Image ImageProvider::getDefaultImage( sal_Int32 _nDatabaseObjectType, bool _bHighContrast )
    {
        Image aObjectImage;
        USHORT nImageResourceID( getDefaultImageResourceID( _nDatabaseObjectType, _bHighContrast ) );
        if ( nImageResourceID )
            aObjectImage = Image( ModuleRes( nImageResourceID ) );
        return aObjectImage;
    }

    //--------------------------------------------------------------------
    USHORT ImageProvider::getDefaultImageResourceID( sal_Int32 _nDatabaseObjectType, bool _bHighContrast )
    {
        USHORT nImageResourceID( 0 );
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
        USHORT nImageResourceID( 0 );
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

