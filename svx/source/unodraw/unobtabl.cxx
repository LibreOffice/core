/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unobtabl.cxx,v $
 * $Revision: 1.19 $
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
#include "precompiled_svx.hxx"
#include <svl/itempool.hxx>
#include <vcl/cvtgrf.hxx>
#include <svl/itemset.hxx>
#include <svx/xit.hxx>
#ifndef SVX_LIGHT
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#endif
#include "UnoNameItemTable.hxx"

#include <svx/xbtmpit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xflhtit.hxx>
#include "unoapi.hxx"
#include "svx/impgrf.hxx"
#include <svx/unomid.hxx>
#include <svx/unoprnms.hxx>
#include "unofill.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoBitmapTable : public SvxUnoNameItemTable
{
public:
    SvxUnoBitmapTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoBitmapTable() throw();

    virtual NameOrIndex* createItem() const throw();
    virtual bool isValid( const NameOrIndex* pItem ) const;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
};

SvxUnoBitmapTable::SvxUnoBitmapTable( SdrModel* pModel ) throw()
: SvxUnoNameItemTable( pModel, XATTR_FILLBITMAP, MID_GRAFURL )
{
}

SvxUnoBitmapTable::~SvxUnoBitmapTable() throw()
{
}

bool SvxUnoBitmapTable::isValid( const NameOrIndex* pItem ) const
{
    if( SvxUnoNameItemTable::isValid( pItem ) )
    {
        const XFillBitmapItem* pBitmapItem = dynamic_cast< const XFillBitmapItem* >( pItem );
        if( pBitmapItem )
        {
            const GraphicObject& rGraphic = pBitmapItem->GetBitmapValue().GetGraphicObject();
            return rGraphic.GetSizeBytes() > 0;
        }
    }

    return false;
}

OUString SAL_CALL SvxUnoBitmapTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoBitmapTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoBitmapTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.BitmapTable" ));
    return aSNS;
}

NameOrIndex* SvxUnoBitmapTable::createItem() const throw()
{
    return new XFillBitmapItem();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoBitmapTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType( (const ::rtl::OUString*)0 );
}

/**
 * Create a bitmaptable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoBitmapTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoBitmapTable(pModel);
}
#include <tools/stream.hxx>
#include <unotools/localfilehelper.hxx>

/** returns a GraphicObject for this URL */
GraphicObject CreateGraphicObjectFromURL( const ::rtl::OUString &rURL ) throw()
{
    const String aURL( rURL ), aPrefix( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX) );

    if( aURL.Search( aPrefix ) == 0 )
    {
        // graphic manager url
        ByteString aUniqueID( String(rURL.copy( sizeof( UNO_NAME_GRAPHOBJ_URLPREFIX ) - 1 )), RTL_TEXTENCODING_UTF8 );
        return GraphicObject( aUniqueID );
    }
    else
    {
        Graphic     aGraphic;

#ifndef SVX_LIGHT
        if ( aURL.Len() )
        {
            SfxMedium   aMedium( aURL, STREAM_READ, TRUE );
            SvStream*   pStream = aMedium.GetInStream();

            if( pStream )
                GraphicConverter::Import( *pStream, aGraphic );
        }
#else
        String aSystemPath( rURL );
        utl::LocalFileHelper::ConvertURLToSystemPath( aSystemPath, aSystemPath );
        SvFileStream aFile( aSystemPath, STREAM_READ );
        GraphicConverter::Import( aFile, aGraphic );
#endif


        return GraphicObject( aGraphic );
    }
}
