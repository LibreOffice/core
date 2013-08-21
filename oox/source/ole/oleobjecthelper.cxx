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

#include "oox/ole/oleobjecthelper.hxx"

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "oox/helper/propertymap.hxx"

namespace oox {
namespace ole {

// ============================================================================

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

// ============================================================================

OleObjectInfo::OleObjectInfo() :
    mbLinked( false ),
    mbShowAsIcon( false ),
    mbAutoUpdate( false )
{
}

// ============================================================================

OleObjectHelper::OleObjectHelper( const Reference< XMultiServiceFactory >& rxModelFactory ) :
    maEmbeddedObjScheme( "vnd.sun.star.EmbeddedObject:" ),
    mnObjectId( 100 )
{
    if( rxModelFactory.is() ) try
    {
        mxResolver.set( rxModelFactory->createInstance( "com.sun.star.document.ImportEmbeddedObjectResolver" ), UNO_QUERY );
    }
    catch(const Exception& )
    {
    }
}

OleObjectHelper::~OleObjectHelper()
{
    try
    {
        Reference< XComponent > xResolverComp( mxResolver, UNO_QUERY_THROW );
        xResolverComp->dispose();
    }
    catch(const Exception& )
    {
    }
}

bool OleObjectHelper::importOleObject( PropertyMap& rPropMap, const OleObjectInfo& rOleObject, const awt::Size& rObjSize )
{
    bool bRet = false;

    if( rOleObject.mbLinked )
    {
        // linked OLE object - set target URL
        if( !rOleObject.maTargetLink.isEmpty() )
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
            OUString aObjectId = "Obj" + OUString::number( mnObjectId++ );

            Reference< XNameAccess > xResolverNA( mxResolver, UNO_QUERY_THROW );
            Reference< XOutputStream > xOutStrm( xResolverNA->getByName( aObjectId ), UNO_QUERY_THROW );
            xOutStrm->writeBytes( rOleObject.maEmbeddedData );
            xOutStrm->closeOutput();

            OUString aUrl = mxResolver->resolveEmbeddedObjectURL( aObjectId );
            OSL_ENSURE( aUrl.match( maEmbeddedObjScheme ), "OleObjectHelper::importOleObject - unexpected URL scheme" );
            OUString aPersistName = aUrl.copy( maEmbeddedObjScheme.getLength() );
            if( !aPersistName.isEmpty() )
            {
                rPropMap[ PROP_PersistName ] <<= aPersistName;
                bRet = true;
            }
        }
        catch(const Exception& )
        {
        }
    }

    if( bRet )
    {
        rPropMap[ PROP_Aspect ] <<= (rOleObject.mbShowAsIcon ? Aspects::MSOLE_ICON : Aspects::MSOLE_CONTENT);
        rPropMap[ PROP_VisualArea ] <<= awt::Rectangle( 0, 0, rObjSize.Width, rObjSize.Height );
    }
    return bRet;
}

// ============================================================================

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
