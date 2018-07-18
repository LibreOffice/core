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

#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>

#include <vcl/virdev.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpool.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <sfx2/frmdescr.hxx>
#include <vcl/svapp.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>

#include <sot/storage.hxx>
#include <sot/exchange.hxx>
#include <vcl/FilterConfigItem.hxx>

#include <svx/svdmodel.hxx>
#include "shapeimpl.hxx"

#include <svx/unoshprp.hxx>

#include <svx/unoapi.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <vcl/wmf.hxx>
#include <svtools/embedhlp.hxx>

#include <config_features.h>


using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;


SvxOle2Shape::SvxOle2Shape(SdrObject* pObject)
: SvxShapeText( pObject, getSvxMapProvider().GetMap(SVXMAP_OLE2),
                getSvxMapProvider().GetPropertySet(SVXMAP_OLE2,SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

SvxOle2Shape::SvxOle2Shape(SdrObject* pObject, const SfxItemPropertyMapEntry* pPropertyMap, const SvxItemPropertySet* pPropertySet)
: SvxShapeText( pObject, pPropertyMap, pPropertySet  )
{
}

SvxOle2Shape::~SvxOle2Shape() throw()
{
}

//XPropertySet
bool SvxOle2Shape::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_OLE_VISAREA:
    {
        // TODO/LATER: seems to make no sense for iconified object

        awt::Rectangle aVisArea;
        if( (rValue >>= aVisArea) && dynamic_cast<const SdrOle2Obj* >(GetSdrObject()) != nullptr)
        {
            Size aTmp( aVisArea.X + aVisArea.Width, aVisArea.Y + aVisArea.Height );
            uno::Reference < embed::XEmbeddedObject > xObj = static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef();
            if( xObj.is() )
            {
                try
                {
                    // the API handles with MapUnit::Map100thMM map mode
                    MapUnit aObjUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( embed::Aspects::MSOLE_CONTENT ) );
                    aTmp = OutputDevice::LogicToLogic(aTmp, MapMode(MapUnit::Map100thMM), MapMode(aObjUnit));
                    xObj->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, awt::Size( aTmp.Width(), aTmp.Height() ) );
                }
                catch( uno::Exception& )
                {
                    OSL_FAIL( "Couldn't set the visual area for the object!" );
                }
            }

            return true;
        }
        break;
    }
    case OWN_ATTR_OLE_ASPECT:
    {
        sal_Int64 nAspect = 0;
        if( rValue >>= nAspect )
        {
            static_cast<SdrOle2Obj*>(GetSdrObject())->SetAspect( nAspect );
            return true;
        }
        break;
    }
    case OWN_ATTR_CLSID:
    {
        OUString aCLSID;
        if( rValue >>= aCLSID )
        {
            // init a ole object with a global name
            SvGlobalName aClassName;
            if( aClassName.MakeId( aCLSID ) )
            {
                if( createObject( aClassName ) )
                    return true;
            }
        }
        break;
    }
    case OWN_ATTR_THUMBNAIL:
    {
        uno::Reference< graphic::XGraphic > xGraphic( rValue, uno::UNO_QUERY );
        if( xGraphic.is() )
        {
            const Graphic aGraphic(xGraphic);
            static_cast<SdrOle2Obj*>(GetSdrObject())->SetGraphic(aGraphic);
            return true;
        }
        break;
    }
    case OWN_ATTR_VALUE_GRAPHIC:
    {
        uno::Reference< graphic::XGraphic > xGraphic( rValue, uno::UNO_QUERY );
        if( xGraphic.is() )
        {
            SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( GetSdrObject() );
            if( pOle )
            {
                GraphicObject aGrafObj( xGraphic );
                const Graphic aGraphic( aGrafObj.GetGraphic() );
                pOle->SetGraphicToObj( aGraphic );
            }
            return true;
        }
        break;
    }
    case OWN_ATTR_PERSISTNAME:
    {
        OUString aPersistName;
        if( rValue >>= aPersistName )
        {
            SdrOle2Obj *pOle;
#if OSL_DEBUG_LEVEL > 0
            pOle = dynamic_cast<SdrOle2Obj*>(GetSdrObject());
            assert(pOle);
#else
            pOle = static_cast<SdrOle2Obj*>(GetSdrObject());
#endif
            pOle->SetPersistName( aPersistName );
            return true;
        }
        break;
    }
    case OWN_ATTR_OLE_LINKURL:
    {
        OUString aLinkURL;
        if( rValue >>= aLinkURL )
        {
            createLink( aLinkURL );
            return true;
        }
        break;
    }
    default:
        return SvxShapeText::setPropertyValueImpl( rName, pProperty, rValue );
    }

    throw IllegalArgumentException();
}

bool SvxOle2Shape::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_CLSID:
    {
        OUString aCLSID;
        GetClassName_Impl(aCLSID);
        rValue <<= aCLSID;
        break;
    }

    case OWN_ATTR_INTERNAL_OLE:
    {
        OUString sCLSID;
        rValue <<= SotExchange::IsInternal( GetClassName_Impl(sCLSID) );
        break;
    }

    case OWN_ATTR_METAFILE:
    {
        SdrOle2Obj* pObj = dynamic_cast<SdrOle2Obj*>(GetSdrObject());
        if( pObj )
        {
            const Graphic* pGraphic = pObj->GetGraphic();
            if( pGraphic )
            {
                bool bIsWMF = false;
                if ( pGraphic->IsGfxLink() )
                {
                    GfxLink aLnk = pGraphic->GetGfxLink();
                    if ( aLnk.GetType() == GfxLinkType::NativeWmf )
                    {
                        bIsWMF = true;
                        uno::Sequence<sal_Int8> aSeq(reinterpret_cast<sal_Int8 const *>(aLnk.GetData()), static_cast<sal_Int32>(aLnk.GetDataSize()));
                        rValue <<= aSeq;
                    }
                }
                if ( !bIsWMF )
                {
                    // #i119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
                    GDIMetaFile aMtf(pObj->GetGraphic()->GetGDIMetaFile());
                    SvMemoryStream aDestStrm( 65535, 65535 );
                    ConvertGDIMetaFileToWMF( aMtf, aDestStrm, nullptr, false );
                    const uno::Sequence<sal_Int8> aSeq(
                        static_cast< const sal_Int8* >(aDestStrm.GetData()),
                        aDestStrm.GetEndOfData());
                    rValue <<= aSeq;
                }
            }
        }
        else
        {
            rValue = GetBitmap( true );
        }
        break;
    }

    case OWN_ATTR_OLE_VISAREA:
    {
        awt::Rectangle aVisArea;
        if( dynamic_cast<const SdrOle2Obj* >(GetSdrObject()) != nullptr)
        {
            MapMode aMapMode( MapUnit::Map100thMM ); // the API uses this map mode
            Size aTmp = static_cast<SdrOle2Obj*>(GetSdrObject())->GetOrigObjSize( &aMapMode ); // get the size in the requested map mode
            aVisArea = awt::Rectangle( 0, 0, aTmp.Width(), aTmp.Height() );
        }

        rValue <<= aVisArea;
        break;
    }

    case OWN_ATTR_OLESIZE:
    {
        Size aTmp( static_cast<SdrOle2Obj*>(GetSdrObject())->GetOrigObjSize() );
        rValue <<= awt::Size( aTmp.Width(), aTmp.Height() );
        break;
    }

    case OWN_ATTR_OLE_ASPECT:
    {
        rValue <<= static_cast<SdrOle2Obj*>(GetSdrObject())->GetAspect();
        break;
    }

    case OWN_ATTR_OLEMODEL:
    case OWN_ATTR_OLE_EMBEDDED_OBJECT:
    case OWN_ATTR_OLE_EMBEDDED_OBJECT_NONEWCLIENT:
    {
        SdrOle2Obj* pObj = dynamic_cast<SdrOle2Obj*>( GetSdrObject() );
        if( pObj )
        {
            uno::Reference < embed::XEmbeddedObject > xObj( pObj->GetObjRef() );
            if ( xObj.is()
              && ( pProperty->nWID == OWN_ATTR_OLE_EMBEDDED_OBJECT || pProperty->nWID == OWN_ATTR_OLE_EMBEDDED_OBJECT_NONEWCLIENT || svt::EmbeddedObjectRef::TryRunningState( xObj ) ) )
            {
                // Discussed with CL fue to the before GetPaintingPageView
                // usage. Removed it, former fallback is used now
                if ( pProperty->nWID == OWN_ATTR_OLEMODEL || pProperty->nWID == OWN_ATTR_OLE_EMBEDDED_OBJECT )
                {
                    const bool bSuccess(pObj->AddOwnLightClient());
                    SAL_WARN_IF(!bSuccess, "svx.svdraw", "An object without client is provided!");
                }

                if ( pProperty->nWID == OWN_ATTR_OLEMODEL )
                    rValue <<= pObj->GetObjRef()->getComponent();
                else
                    rValue <<= xObj;
            }
        }
        break;
    }

    case OWN_ATTR_VALUE_GRAPHIC:
    {
        uno::Reference< graphic::XGraphic > xGraphic;
        const Graphic* pGraphic = static_cast<SdrOle2Obj*>( GetSdrObject() )->GetGraphic();
        if( pGraphic )
            xGraphic = pGraphic->GetXGraphic();
        rValue <<= xGraphic;
        break;
    }

    case OWN_ATTR_THUMBNAIL:
    {
        uno::Reference< graphic::XGraphic > xGraphic;
        const Graphic* pGraphic = static_cast<SdrOle2Obj*>( GetSdrObject() )->GetGraphic();
        if( pGraphic )
            xGraphic = pGraphic->GetXGraphic();
        rValue <<= xGraphic;
        break;
    }
    case OWN_ATTR_PERSISTNAME:
    {
        OUString    aPersistName;
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( GetSdrObject() );

        if( pOle )
        {
            aPersistName = pOle->GetPersistName();
            if( !aPersistName.isEmpty() )
            {
                ::comphelper::IEmbeddedHelper* pPersist(GetSdrObject()->getSdrModelFromSdrObject().GetPersist());
                if( (nullptr == pPersist) || !pPersist->getEmbeddedObjectContainer().HasEmbeddedObject( pOle->GetPersistName() ) )
                    aPersistName.clear();
            }
        }

        rValue <<= aPersistName;
        break;
    }
    case OWN_ATTR_OLE_LINKURL:
    {
        OUString    aLinkURL;
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( GetSdrObject() );

        if( pOle )
        {
            uno::Reference< embed::XLinkageSupport > xLink( pOle->GetObjRef(), uno::UNO_QUERY );
            if ( xLink.is() && xLink->isLink() )
                aLinkURL = xLink->getLinkURL();
        }

        rValue <<= aLinkURL;
        break;
    }
    default:
        return SvxShapeText::getPropertyValueImpl( rName, pProperty, rValue );
    }

    return true;
}

bool SvxOle2Shape::createObject( const SvGlobalName &aClassName )
{
    DBG_TESTSOLARMUTEX();

    SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( GetSdrObject() );
    if ( !pOle2Obj || !pOle2Obj->IsEmpty() )
        return false;

    // create storage and inplace object
    ::comphelper::IEmbeddedHelper*     pPersist = GetSdrObject()->getSdrModelFromSdrObject().GetPersist();
    OUString              aPersistName;
    OUString            aTmpStr;
    if( SvxShape::getPropertyValue( UNO_NAME_OLE2_PERSISTNAME ) >>= aTmpStr )
        aPersistName = aTmpStr;

    uno::Sequence<beans::PropertyValue> objArgs( comphelper::InitPropertySequence({
            { "DefaultParentBaseURL", Any(pPersist->getDocumentBaseURL()) }
        }));
    //TODO/LATER: how to cope with creation failure?!
    uno::Reference<embed::XEmbeddedObject> xObj(
        pPersist->getEmbeddedObjectContainer().CreateEmbeddedObject(
            aClassName.GetByteSequence(), objArgs, aPersistName));
    if( xObj.is() )
    {
        tools::Rectangle aRect = pOle2Obj->GetLogicRect();
        if ( aRect.GetWidth() == 101 && aRect.GetHeight() == 101 )
        {
            // TODO/LATER: is it possible that this method is used to create an iconified object?
            // default size
            try
            {
                awt::Size aSz = xObj->getVisualAreaSize( pOle2Obj->GetAspect() );
                aRect.SetSize( Size( aSz.Width, aSz.Height ) );
            }
            catch( embed::NoVisualAreaSizeException& )
            {}
            pOle2Obj->SetLogicRect( aRect );
        }
        else
        {
            awt::Size aSz;
            Size aSize = aRect.GetSize();
            aSz.Width = aSize.Width();
            aSz.Height = aSize.Height();
            if (aSz.Width != 0 || aSz.Height != 0)
            {
                //HACK: can aSz legally be empty?
                xObj->setVisualAreaSize(  pOle2Obj->GetAspect(), aSz );
            }
        }

        // connect the object after the visual area is set
        SvxShape::setPropertyValue( UNO_NAME_OLE2_PERSISTNAME, Any( aTmpStr = aPersistName ) );

        // the object is inserted during setting of PersistName property usually
        if( pOle2Obj->IsEmpty() )
            pOle2Obj->SetObjRef( xObj );
    }

    return xObj.is();
}

void SvxOle2Shape::createLink( const OUString& aLinkURL )
{
    DBG_TESTSOLARMUTEX();

    SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( GetSdrObject() );
    if ( !pOle2Obj || !pOle2Obj->IsEmpty() )
        return;

    OUString aPersistName;

    ::comphelper::IEmbeddedHelper* pPersist = GetSdrObject()->getSdrModelFromSdrObject().GetPersist();

    uno::Sequence< beans::PropertyValue > aMediaDescr( 1 );
    aMediaDescr[0].Name = "URL";
    aMediaDescr[0].Value <<= aLinkURL;

    uno::Reference< task::XInteractionHandler > xInteraction = pPersist->getInteractionHandler();
    if ( xInteraction.is() )
    {
        aMediaDescr.realloc( 2 );
        aMediaDescr[1].Name = "InteractionHandler";
        aMediaDescr[1].Value <<= xInteraction;
    }

    //TODO/LATER: how to cope with creation failure?!
    uno::Reference< embed::XEmbeddedObject > xObj =
            pPersist->getEmbeddedObjectContainer().InsertEmbeddedLink( aMediaDescr , aPersistName );

    if( xObj.is() )
    {
        tools::Rectangle aRect = pOle2Obj->GetLogicRect();
        if ( aRect.GetWidth() == 101 && aRect.GetHeight() == 101 )
        {
            // default size
            try
            {
                awt::Size aSz = xObj->getVisualAreaSize( pOle2Obj->GetAspect() );
                aRect.SetSize( Size( aSz.Width, aSz.Height ) );
            }
            catch( embed::NoVisualAreaSizeException& )
            {}
            pOle2Obj->SetLogicRect( aRect );
        }
        else
        {
            awt::Size aSz;
            Size aSize = pOle2Obj->GetLogicRect().GetSize();
            aSz.Width = aSize.Width();
            aSz.Height = aSize.Height();
            xObj->setVisualAreaSize(  pOle2Obj->GetAspect(), aSz );
        }

        // connect the object after the visual area is set
        SvxShape::setPropertyValue( UNO_NAME_OLE2_PERSISTNAME, uno::makeAny( aPersistName ) );

        // the object is inserted during setting of PersistName property usually
        if ( pOle2Obj->IsEmpty() )
            pOle2Obj->SetObjRef( xObj );
    }
}

void SvxOle2Shape::resetModifiedState()
{
    ::comphelper::IEmbeddedHelper* pPersist = GetSdrObject()->getSdrModelFromSdrObject().GetPersist();
    if( pPersist && !pPersist->isEnableSetModified() )
    {
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( GetSdrObject() );
        if( pOle && !pOle->IsEmpty() )
        {
            uno::Reference < util::XModifiable > xMod( pOle->GetObjRef(), uno::UNO_QUERY );
            if( xMod.is() )
                // TODO/MBA: what's this?!
                xMod->setModified( false );
        }
    }
}

const SvGlobalName SvxOle2Shape::GetClassName_Impl(OUString& rHexCLSID)
{
    DBG_TESTSOLARMUTEX();
    SvGlobalName aClassName;
    SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( GetSdrObject() );

    if( pOle2Obj )
    {
        rHexCLSID.clear();

        if( pOle2Obj->IsEmpty() )
        {
            ::comphelper::IEmbeddedHelper* pPersist = GetSdrObject()->getSdrModelFromSdrObject().GetPersist();
            if( pPersist )
            {
                uno::Reference < embed::XEmbeddedObject > xObj =
                        pPersist->getEmbeddedObjectContainer().GetEmbeddedObject( pOle2Obj->GetPersistName() );
                if ( xObj.is() )
                {
                    aClassName = SvGlobalName( xObj->getClassID() );
                    rHexCLSID = aClassName.GetHexName();
                }
            }
        }

        if (rHexCLSID.isEmpty())
        {
            uno::Reference < embed::XEmbeddedObject > xObj( pOle2Obj->GetObjRef() );
            if ( xObj.is() )
            {
                aClassName = SvGlobalName( xObj->getClassID() );
                rHexCLSID = aClassName.GetHexName();
            }
        }
    }

    return aClassName;
}

SvxAppletShape::SvxAppletShape(SdrObject* pObject)
    : SvxOle2Shape( pObject, getSvxMapProvider().GetMap(SVXMAP_APPLET), getSvxMapProvider().GetPropertySet(SVXMAP_APPLET, SdrObject::GetGlobalDrawObjectItemPool())  )
{
    SetShapeType( "com.sun.star.drawing.AppletShape" );
}

SvxAppletShape::~SvxAppletShape() throw()
{
}

void SvxAppletShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage )
{
    SvxShape::Create( pNewObj, pNewPage );
    const SvGlobalName aAppletClassId( SO3_APPLET_CLASSID );
    createObject(aAppletClassId);
    SetShapeType( "com.sun.star.drawing.AppletShape" );
}

void SAL_CALL SvxAppletShape::setPropertyValue( const OUString& aPropertyName, const css::uno::Any& rValue )
{
    SvxShape::setPropertyValue( aPropertyName, rValue );
    resetModifiedState();
}

void SAL_CALL SvxAppletShape::setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& rValues )
{
    SvxShape::setPropertyValues( aPropertyNames, rValues );
    resetModifiedState();
}

bool SvxAppletShape::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    if( (pProperty->nWID >= OWN_ATTR_APPLET_DOCBASE) && (pProperty->nWID <= OWN_ATTR_APPLET_ISSCRIPT) )
    {
        if ( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                // allow exceptions to pass through
                xSet->setPropertyValue( rName, rValue );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::setPropertyValueImpl( rName, pProperty, rValue );
    }
}

bool SvxAppletShape::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    if( (pProperty->nWID >= OWN_ATTR_APPLET_DOCBASE) && (pProperty->nWID <= OWN_ATTR_APPLET_ISSCRIPT) )
    {
        if ( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                rValue = xSet->getPropertyValue( rName );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::getPropertyValueImpl( rName, pProperty, rValue );
    }
}

SvxPluginShape::SvxPluginShape(SdrObject* pObject)
    : SvxOle2Shape( pObject, getSvxMapProvider().GetMap(SVXMAP_PLUGIN), getSvxMapProvider().GetPropertySet(SVXMAP_PLUGIN, SdrObject::GetGlobalDrawObjectItemPool()) )
{
    SetShapeType( "com.sun.star.drawing.PluginShape" );
}

SvxPluginShape::~SvxPluginShape() throw()
{
}

void SvxPluginShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage )
{
    SvxShape::Create( pNewObj, pNewPage );
    const SvGlobalName aPluginClassId( SO3_PLUGIN_CLASSID );
    createObject(aPluginClassId);
    SetShapeType( "com.sun.star.drawing.PluginShape" );
}

void SAL_CALL SvxPluginShape::setPropertyValue( const OUString& aPropertyName, const css::uno::Any& rValue )
{
    SvxShape::setPropertyValue( aPropertyName, rValue );
    resetModifiedState();
}

void SAL_CALL SvxPluginShape::setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& rValues )
{
    SvxShape::setPropertyValues( aPropertyNames, rValues );
    resetModifiedState();
}

bool SvxPluginShape::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    if( (pProperty->nWID >= OWN_ATTR_PLUGIN_MIMETYPE) && (pProperty->nWID <= OWN_ATTR_PLUGIN_COMMANDS) )
    {
        if( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                // allow exceptions to pass through
                xSet->setPropertyValue( rName, rValue );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::setPropertyValueImpl( rName, pProperty, rValue );
    }
}

bool SvxPluginShape::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    if( (pProperty->nWID >= OWN_ATTR_PLUGIN_MIMETYPE) && (pProperty->nWID <= OWN_ATTR_PLUGIN_COMMANDS) )
    {
        if( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                rValue = xSet->getPropertyValue( rName );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::getPropertyValueImpl( rName, pProperty, rValue );
    }
}


SvxFrameShape::SvxFrameShape(SdrObject* pObject)
: SvxOle2Shape( pObject, getSvxMapProvider().GetMap(SVXMAP_FRAME), getSvxMapProvider().GetPropertySet(SVXMAP_FRAME, SdrObject::GetGlobalDrawObjectItemPool())  )
{
    SetShapeType( "com.sun.star.drawing.FrameShape" );
}

SvxFrameShape::~SvxFrameShape() throw()
{
}

void SvxFrameShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage )
{
    SvxShape::Create( pNewObj, pNewPage );
    const SvGlobalName aIFrameClassId( SO3_IFRAME_CLASSID );
    createObject(aIFrameClassId);
    SetShapeType( "com.sun.star.drawing.FrameShape" );
}

void SAL_CALL SvxFrameShape::setPropertyValue( const OUString& aPropertyName, const css::uno::Any& rValue )
{
    SvxShape::setPropertyValue( aPropertyName, rValue );
    resetModifiedState();
}

void SAL_CALL SvxFrameShape::setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& rValues )
{
    SvxShape::setPropertyValues( aPropertyNames, rValues );
    resetModifiedState();
}

bool SvxFrameShape::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    if( (pProperty->nWID >= OWN_ATTR_FRAME_URL) && (pProperty->nWID <= OWN_ATTR_FRAME_MARGIN_HEIGHT) )
    {
        if( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                // allow exceptions to pass through
                xSet->setPropertyValue( rName, rValue );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::setPropertyValueImpl( rName, pProperty, rValue );
    }
}

bool SvxFrameShape::getPropertyValueImpl(const OUString& rName, const SfxItemPropertySimpleEntry* pProperty,
    css::uno::Any& rValue)
{
    if( (pProperty->nWID >= OWN_ATTR_FRAME_URL) && (pProperty->nWID <= OWN_ATTR_FRAME_MARGIN_HEIGHT) )
    {
        if( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(GetSdrObject())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                rValue = xSet->getPropertyValue( rName );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::getPropertyValueImpl( rName, pProperty, rValue );
    }
}
SvxMediaShape::SvxMediaShape(SdrObject* pObj, OUString const & referer)
:   SvxShape( pObj, getSvxMapProvider().GetMap(SVXMAP_MEDIA), getSvxMapProvider().GetPropertySet(SVXMAP_MEDIA, SdrObject::GetGlobalDrawObjectItemPool()) ),
    referer_(referer)
{
    SetShapeType( "com.sun.star.drawing.MediaShape" );
}


SvxMediaShape::~SvxMediaShape() throw()
{
}


bool SvxMediaShape::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    if( ((pProperty->nWID >= OWN_ATTR_MEDIA_URL) && (pProperty->nWID <= OWN_ATTR_MEDIA_ZOOM))
        || (pProperty->nWID == OWN_ATTR_MEDIA_STREAM)
        || (pProperty->nWID == OWN_ATTR_MEDIA_MIMETYPE) )
    {
        SdrMediaObj* pMedia = static_cast< SdrMediaObj* >( GetSdrObject() );
        ::avmedia::MediaItem aItem;
        bool bOk = false;

        switch( pProperty->nWID )
        {
        case OWN_ATTR_MEDIA_URL:
#if HAVE_FEATURE_AVMEDIA
        {
            OUString aURL;
            if( rValue >>= aURL )
            {
                bOk = true;
                aItem.setURL( aURL, "", referer_ );
            }
        }
#endif
        break;

        case OWN_ATTR_MEDIA_LOOP:
#if HAVE_FEATURE_AVMEDIA
        {
            bool bLoop;

            if( rValue >>= bLoop )
            {
                bOk = true;
                aItem.setLoop( bLoop );
            }
        }
#endif
        break;

        case OWN_ATTR_MEDIA_MUTE:
#if HAVE_FEATURE_AVMEDIA
        {
            bool bMute;

            if( rValue >>= bMute )
            {
                bOk = true;
                aItem.setMute( bMute );
            }
        }
#endif
        break;

        case OWN_ATTR_MEDIA_VOLUMEDB:
#if HAVE_FEATURE_AVMEDIA
        {
            sal_Int16 nVolumeDB = sal_Int16();

            if( rValue >>= nVolumeDB )
            {
                bOk = true;
                aItem.setVolumeDB( nVolumeDB );
            }
        }
#endif
        break;

        case OWN_ATTR_MEDIA_ZOOM:
#if HAVE_FEATURE_AVMEDIA
        {
            css::media::ZoomLevel eLevel;

            if( rValue >>= eLevel )
            {
                bOk = true;
                aItem.setZoom( eLevel );
            }
        }
#endif
        break;

        case OWN_ATTR_MEDIA_MIMETYPE:
#if HAVE_FEATURE_AVMEDIA
        {
            OUString sMimeType;
            if( rValue >>= sMimeType )
            {
                bOk = true;
                aItem.setMimeType( sMimeType );
            }
        }
#endif
        break;

        case OWN_ATTR_MEDIA_STREAM:
#if HAVE_FEATURE_AVMEDIA
            try
            {
                uno::Reference<io::XInputStream> xStream;
                if (rValue >>= xStream)
                {
                    pMedia->SetInputStream(xStream);
                }
            }
            catch (const css::ucb::ContentCreationException&)
            {
                css::uno::Any exc = cppu::getCaughtException();
                throw css::lang::WrappedTargetException(
                        "ContentCreationException Setting InputStream!",
                        static_cast<OWeakObject *>(this),
                        exc);
            }
            catch (const css::ucb::CommandFailedException&)
            {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw css::lang::WrappedTargetException(
                        "CommandFailedException Setting InputStream!",
                        static_cast<OWeakObject *>(this),
                        anyEx);
            }
#endif
        break;

        default:
            OSL_FAIL("SvxMediaShape::setPropertyValueImpl(), unknown argument!");
        }

        if( bOk )
        {
            pMedia->setMediaProperties( aItem );
            return true;
        }
    }
    else
    {
        return SvxShape::setPropertyValueImpl( rName, pProperty, rValue );
    }

    throw IllegalArgumentException();
}


bool SvxMediaShape::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    if (   ((pProperty->nWID >= OWN_ATTR_MEDIA_URL) &&
            (pProperty->nWID <= OWN_ATTR_MEDIA_ZOOM))
        || (pProperty->nWID == OWN_ATTR_MEDIA_STREAM)
        || (pProperty->nWID == OWN_ATTR_MEDIA_TEMPFILEURL)
        || (pProperty->nWID == OWN_ATTR_MEDIA_MIMETYPE)
        || (pProperty->nWID == OWN_ATTR_FALLBACK_GRAPHIC))
    {
        SdrMediaObj* pMedia = static_cast< SdrMediaObj* >( GetSdrObject() );
        const ::avmedia::MediaItem aItem( pMedia->getMediaProperties() );

        switch( pProperty->nWID )
        {
            case OWN_ATTR_MEDIA_URL:
#if HAVE_FEATURE_AVMEDIA
                rValue <<= aItem.getURL();
#endif
                break;

            case OWN_ATTR_MEDIA_LOOP:
#if HAVE_FEATURE_AVMEDIA
                rValue <<= aItem.isLoop();
#endif
                break;

            case OWN_ATTR_MEDIA_MUTE:
#if HAVE_FEATURE_AVMEDIA
                rValue <<= aItem.isMute();
#endif
                break;

            case OWN_ATTR_MEDIA_VOLUMEDB:
#if HAVE_FEATURE_AVMEDIA
                rValue <<= aItem.getVolumeDB();
#endif
                break;

            case OWN_ATTR_MEDIA_ZOOM:
#if HAVE_FEATURE_AVMEDIA
                rValue <<= aItem.getZoom();
#endif
                break;

            case OWN_ATTR_MEDIA_STREAM:
                try
                {
                    rValue <<= pMedia->GetInputStream();
                }
                catch (const css::ucb::ContentCreationException&)
                {
                    css::uno::Any anyEx = cppu::getCaughtException();
                    throw css::lang::WrappedTargetException(
                            "ContentCreationException Getting InputStream!",
                            static_cast < OWeakObject * > ( this ), anyEx );
                }
                catch (const css::ucb::CommandFailedException&)
                {
                    css::uno::Any anyEx = cppu::getCaughtException();
                    throw css::lang::WrappedTargetException(
                            "CommandFailedException Getting InputStream!",
                            static_cast < OWeakObject * > ( this ), anyEx );
                }

                break;

            case OWN_ATTR_MEDIA_TEMPFILEURL:
#if HAVE_FEATURE_AVMEDIA
                rValue <<= aItem.getTempURL();
#endif
                break;

            case OWN_ATTR_MEDIA_MIMETYPE:
#if HAVE_FEATURE_AVMEDIA
                rValue <<= aItem.getMimeType();
#endif
                break;

            case OWN_ATTR_FALLBACK_GRAPHIC:
                rValue <<= pMedia->getSnapshot();
                break;

            default:
                OSL_FAIL("SvxMediaShape::getPropertyValueImpl(), unknown property!");
        }
        return true;
    }
    else
    {
        return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
