/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

OleObjectInfo::OleObjectInfo() :
    mbLinked( false ),
    mbShowAsIcon( false ),
    mbAutoUpdate( false )
{
}

// ============================================================================

OleObjectHelper::OleObjectHelper( const Reference< XMultiServiceFactory >& rxModelFactory ) :
    maEmbeddedObjScheme( CREATE_OUSTRING( "vnd.sun.star.EmbeddedObject:" ) ),
    mnObjectId( 100 )
{
    if( rxModelFactory.is() ) try
    {
        mxResolver.set( rxModelFactory->createInstance( CREATE_OUSTRING( "com.sun.star.document.ImportEmbeddedObjectResolver" ) ), UNO_QUERY );
    }
    catch( Exception& )
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
        rPropMap[ PROP_Aspect ] <<= (rOleObject.mbShowAsIcon ? Aspects::MSOLE_ICON : Aspects::MSOLE_CONTENT);
        rPropMap[ PROP_VisualArea ] <<= Rectangle( 0, 0, rObjSize.Width, rObjSize.Height );
    }
    return bRet;
}

// ============================================================================

} // namespace ole
} // namespace oox
