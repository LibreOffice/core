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

#include <oox/ole/oleobjecthelper.hxx>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <osl/diagnose.h>
#include <comphelper/sequenceashashmap.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/token/properties.hxx>

namespace oox::ole {

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

OleObjectInfo::OleObjectInfo() :
    mbLinked( false ),
    mbShowAsIcon( false ),
    mbAutoUpdate( false )
{
}

const char g_aEmbeddedObjScheme[] = "vnd.sun.star.EmbeddedObject:";

OleObjectHelper::OleObjectHelper(
        const Reference< XMultiServiceFactory >& rxModelFactory,
        uno::Reference<frame::XModel> const& xModel)
    : m_xModel(xModel)
    , mnObjectId( 100 )
{
    assert(m_xModel.is());
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

// TODO: this is probably a sub-optimal approach: ideally the media type
// of the stream from [Content_Types].xml should be stored somewhere for this
// purpose, but currently the media type of all OLE streams in the storage is
// just "application/vnd.sun.star.oleobject"
void SaveInteropProperties(uno::Reference<frame::XModel> const& xModel,
       OUString const& rObjectName, OUString const*const pOldObjectName,
       OUString const& rProgId)
{
    static const char sEmbeddingsPropName[] = "EmbeddedObjects";

    // get interop grab bag from document
    uno::Reference<beans::XPropertySet> const xDocProps(xModel, uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGrabBag(xDocProps->getPropertyValue("InteropGrabBag"));

    // get EmbeddedObjects property inside grab bag
    comphelper::SequenceAsHashMap objectsList;
    if (aGrabBag.find(sEmbeddingsPropName) != aGrabBag.end())
        objectsList << aGrabBag[sEmbeddingsPropName];

    uno::Sequence< beans::PropertyValue > aGrabBagAttribute(1);
    aGrabBagAttribute[0].Name = "ProgID";
    aGrabBagAttribute[0].Value <<= rProgId;

    // If we got an "old name", erase that first.
    if (pOldObjectName)
    {
        comphelper::SequenceAsHashMap::iterator it = objectsList.find(*pOldObjectName);
        if (it != objectsList.end())
            objectsList.erase(it);
    }

    objectsList[rObjectName] <<= aGrabBagAttribute;

    // put objects list back into the grab bag
    aGrabBag[sEmbeddingsPropName] <<= objectsList.getAsConstPropertyValueList();

    // put grab bag back into the document
    xDocProps->setPropertyValue("InteropGrabBag", uno::Any(aGrabBag.getAsConstPropertyValueList()));
}

bool OleObjectHelper::importOleObject( PropertyMap& rPropMap, const OleObjectInfo& rOleObject, const awt::Size& rObjSize )
{
    bool bRet = false;

    if( rOleObject.mbLinked )
    {
        // linked OLE object - set target URL
        if( !rOleObject.maTargetLink.isEmpty() )
        {
            rPropMap.setProperty( PROP_LinkURL, rOleObject.maTargetLink);
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

            SaveInteropProperties(m_xModel, aObjectId, nullptr, rOleObject.maProgId);

            OUString aUrl = mxResolver->resolveEmbeddedObjectURL( aObjectId );
            OSL_ENSURE( aUrl.match( g_aEmbeddedObjScheme ), "OleObjectHelper::importOleObject - unexpected URL scheme" );
            OUString aPersistName = aUrl.copy( strlen(g_aEmbeddedObjScheme) );
            if( !aPersistName.isEmpty() )
            {
                rPropMap.setProperty( PROP_PersistName, aPersistName);
                bRet = true;
            }
        }
        catch(const Exception& )
        {
        }
    }

    if( bRet )
    {
        rPropMap.setProperty( PROP_Aspect, (rOleObject.mbShowAsIcon ? Aspects::MSOLE_ICON : Aspects::MSOLE_CONTENT));
        rPropMap.setProperty( PROP_VisualArea, awt::Rectangle( 0, 0, rObjSize.Width, rObjSize.Height ));
    }
    return bRet;
}

} // namespace oox::ole

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
