/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: olehelper.cxx,v $
 * $Revision: 1.1 $
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

#include "oox/ole/oleobjecthelper.hxx"
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include "properties.hxx"
#include "oox/helper/propertymap.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::Size;

namespace oox {
namespace ole {

// ============================================================================

OleObjectInfo::OleObjectInfo() :
    mbLinked( false ),
    mbShowAsIcon( false ),
    mbAutoUpdate( false )
{
}

// ============================================================================

OleObjectHelper::OleObjectHelper( const Reference< XMultiServiceFactory >& rxFactory ) :
    maEmbeddedObjScheme( CREATE_OUSTRING( "vnd.sun.star.EmbeddedObject:" ) ),
    mnObjectId( 100 )
{
    if( rxFactory.is() )
        mxResolver.set( rxFactory->createInstance( CREATE_OUSTRING( "com.sun.star.document.ImportEmbeddedObjectResolver" ) ), UNO_QUERY );
}

OleObjectHelper::~OleObjectHelper()
{
    try
    {
        Reference< XComponent > xResolverComp( mxResolver, UNO_QUERY_THROW );
        xResolverComp->dispose();
    }
    catch( Exception& )
    {
    }
}

bool OleObjectHelper::importOleObject( PropertyMap& rPropMap, const OleObjectInfo& rOleObject, const Size& rObjSize )
{
    bool bRet = false;

    if( rOleObject.mbLinked )
    {
        // linked OLE object - set target URL
        if( rOleObject.maTargetLink.getLength() > 0 )
        {
            rPropMap[ PROP_LinkURL ] <<= rOleObject.maTargetLink;
            bRet = true;
        }
    }
    else
    {
        // embedded OLE object - import the embedded data
        if( rOleObject.maEmbeddedData.hasElements() && mxResolver.is() ) try
        {
            OUString aObjectId = CREATE_OUSTRING( "Obj" ) + OUString::valueOf( mnObjectId++ );

            Reference< XNameAccess > xResolverNA( mxResolver, UNO_QUERY_THROW );
            Reference< XOutputStream > xOutStrm( xResolverNA->getByName( aObjectId ), UNO_QUERY_THROW );
            xOutStrm->writeBytes( rOleObject.maEmbeddedData );
            xOutStrm->closeOutput();

            OUString aUrl = mxResolver->resolveEmbeddedObjectURL( aObjectId );
            OSL_ENSURE( aUrl.match( maEmbeddedObjScheme ), "OleObjectHelper::importOleObject - unexpected URL scheme" );
            OUString aPersistName = aUrl.copy( maEmbeddedObjScheme.getLength() );
            if( aPersistName.getLength() > 0 )
            {
                rPropMap[ PROP_PersistName ] <<= aPersistName;
                bRet = true;
            }
        }
        catch( Exception& )
        {
        }
    }

    if( bRet )
    {
        // aspect mode
        using namespace ::com::sun::star::embed::Aspects;
        sal_Int64 nAspect = rOleObject.mbShowAsIcon ? MSOLE_ICON : MSOLE_CONTENT;
        rPropMap[ PROP_Aspect ] <<= nAspect;
        // visual area
        rPropMap[ PROP_VisualArea ] <<= Rectangle( 0, 0, rObjSize.Width, rObjSize.Height );
    }
    return bRet;
}

// ============================================================================

} // namespace ole
} // namespace oox

