/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoshap4.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:01:50 $
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
#include "precompiled_svx.hxx"

#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XLINKAGESUPPORT_HPP_
#include <com/sun/star/embed/XLinkageSupport.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#define _SVX_USE_UNOGLOBALS_

#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOMEDIA_HXX
#include <svx/svdomedia.hxx>
#endif

#ifndef SVX_LIGHT
#ifndef _SOT_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif
#ifndef _SFX_FRMDESCRHXX
#include <sfx2/frmdescr.hxx>
#endif
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif

#ifndef _SVX_SHAPEIMPL_HXX
#include "shapeimpl.hxx"
#endif

#include <svx/unoshprp.hxx>
#include "unoapi.hxx"

#ifndef _SVDGLOB_HXX
#include "svdglob.hxx"
#endif
#include "svdstr.hrc"

///////////////////////////////////////////////////////////////////////

using namespace ::osl;
using namespace ::vos;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

///////////////////////////////////////////////////////////////////////
SvxOle2Shape::SvxOle2Shape( SdrObject* pObject ) throw()
: SvxShape( pObject, aSvxMapProvider.GetMap(SVXMAP_OLE2)  )
{
}

SvxOle2Shape::SvxOle2Shape( SdrObject* pObject, const SfxItemPropertyMap* pPropertySet ) throw ()
: SvxShape( pObject, pPropertySet  )
{
}

SvxOle2Shape::~SvxOle2Shape() throw()
{
}

::com::sun::star::uno::Any SAL_CALL SvxOle2Shape::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    return SvxShape::queryAggregation( rType );
}

//XPropertySet
void SAL_CALL SvxOle2Shape::setPropertyValue( const OUString& aPropertyName, const Any& aValue )    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CLSID" ) ) )
    {
        OUString aCLSID;
        if( aValue >>= aCLSID )
        {
            // init a ole object with a global name
            if( mpObj.is() )
            {
                SvGlobalName aClassName;
                if( aClassName.MakeId( aCLSID ) )
                {
                    if( createObject( aClassName ) )
                        return;
                }
            }
        }

        throw IllegalArgumentException();
    }
    else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ThumbnailGraphicURL" ) ) )
    {
        OUString aURL;
        if( aValue >>= aURL )
        {
            SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
            if( pOle )
            {
                GraphicObject aGrafObj( CreateGraphicObjectFromURL( aURL ) );
                pOle->SetGraphic( &aGrafObj.GetGraphic() );
            }
            return;
        }

        throw IllegalArgumentException();
    }
    else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Graphic" ) ) )
    {
        uno::Reference< graphic::XGraphic > xGraphic;
        if( aValue >>= xGraphic )
        {
            SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
            if( pOle )
            {
                GraphicObject aGrafObj( xGraphic );
                const Graphic aGraphic( aGrafObj.GetGraphic() );
                pOle->SetGraphicToObj( aGraphic, rtl::OUString() );
            }
            return;
        }
        throw IllegalArgumentException();
    }
    else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( UNO_NAME_OLE2_PERSISTNAME ) ) )
    {
        OUString aPersistName;

        if( aValue >>= aPersistName )
        {
            SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );

            if( pOle )
                pOle->SetPersistName( aPersistName );

            return;
        }

        throw IllegalArgumentException();
    }
    else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "LinkURL" ) ) )
    {
        OUString aLinkURL;
        if ( aValue >>= aLinkURL )
        {
            if( mpObj.is() )
                createLink( aLinkURL );

            return;
        }

        throw IllegalArgumentException();
    }

    SvxShape::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL SvxOle2Shape::getPropertyValue( const OUString& PropertyName ) throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ThumbnailGraphicURL" ) ) )
    {
        OUString    aURL;
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );

        if( pOle )
        {
            Graphic* pGraphic = pOle->GetGraphic();

            // if there isn't already a preview graphic set, check if we need to generate
            // one if model says so
            if( pGraphic == NULL && !pOle->IsEmptyPresObj() && mpModel->IsSaveOLEPreview() )
                pGraphic = pOle->GetGraphic();

            if( pGraphic )
            {
                GraphicObject aObj( *pGraphic );
                aURL = OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
                aURL += OUString::createFromAscii( aObj.GetUniqueID().GetBuffer() );
            }
        }
        return makeAny( aURL );
    }
    else if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( UNO_NAME_OLE2_PERSISTNAME ) ) )
    {
        OUString    aPersistName;
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );

        if( pOle )
        {
            aPersistName = pOle->GetPersistName();
            if( aPersistName.getLength() )
            {
                ::comphelper::IEmbeddedHelper *pPersist = mpObj->GetModel()->GetPersist();
                if( (NULL == pPersist) || !pPersist->getEmbeddedObjectContainer().HasEmbeddedObject( pOle->GetPersistName() ) )
                    aPersistName = OUString();
            }
        }

        return makeAny( aPersistName );
    }
    else if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "LinkURL" ) ) )
    {
        OUString    aLinkURL;
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );

        if( pOle )
        {
            uno::Reference< embed::XLinkageSupport > xLink( pOle->GetObjRef(), uno::UNO_QUERY );
            if ( xLink.is() && xLink->isLink() )
                aLinkURL = xLink->getLinkURL();
        }

        return makeAny( aLinkURL );
    }
    else if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsChart" ) ) )
    {
        //property for use in slide show
        //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
        sal_Bool bIsChart = sal_False;
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
        if( pOle )
            bIsChart = pOle->IsChart();
        return makeAny( bIsChart );
    }
    else if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "XModel" ) ) )
    {
        //property for use in slide show
        //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
        uno::Reference< frame::XModel > xModel;
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
        if( pOle )
            xModel = pOle->getXModel();
        return makeAny( xModel );
    }

    return SvxShape::getPropertyValue( PropertyName );
}


sal_Bool SvxOle2Shape::createObject( const SvGlobalName &aClassName )
{
    DBG_TESTSOLARMUTEX();

    SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
    if ( !pOle2Obj && !pOle2Obj->IsEmpty() )
        return sal_False;

    // create storage and inplace object
    ::comphelper::IEmbeddedHelper*     pPersist = mpModel->GetPersist();
    ::rtl::OUString              aPersistName;
    OUString            aTmpStr;
    Any                 aAny( getPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ) ) );
    if( aAny >>= aTmpStr )
        aPersistName = aTmpStr;

    //TODO/LATER: how to cope with creation failure?!
    uno::Reference < embed::XEmbeddedObject > xObj( pPersist->getEmbeddedObjectContainer().CreateEmbeddedObject( aClassName.GetByteSequence(), aPersistName ) );
    if( xObj.is() )
    {
        Rectangle aRect = pOle2Obj->GetLogicRect();
        if ( aRect.GetWidth() == 100 && aRect.GetHeight() == 100 )
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
            Size aSize = pOle2Obj->GetLogicRect().GetSize();
            aSz.Width = aSize.Width();
            aSz.Height = aSize.Height();
            xObj->setVisualAreaSize(  pOle2Obj->GetAspect(), aSz );
        }

        // connect the object after the visual area is set
        aAny <<= ( aTmpStr = aPersistName );
        setPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ), aAny );

        // the object is inserted during setting of PersistName property usually
        if( pOle2Obj->IsEmpty() )
            pOle2Obj->SetObjRef( xObj );
    }

    return xObj.is();
}

sal_Bool SvxOle2Shape::createLink( const ::rtl::OUString& aLinkURL )
{
    DBG_TESTSOLARMUTEX();

    SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
    if ( !pOle2Obj || !pOle2Obj->IsEmpty() )
        return sal_False;

    ::rtl::OUString aPersistName;

    ::comphelper::IEmbeddedHelper* pPersist = mpModel->GetPersist();

    uno::Sequence< beans::PropertyValue > aMediaDescr( 1 );
    aMediaDescr[0].Name = ::rtl::OUString::createFromAscii( "URL" );
    aMediaDescr[0].Value <<= aLinkURL;

    uno::Reference< task::XInteractionHandler > xInteraction = pPersist->getInteractionHandler();
    if ( xInteraction.is() )
    {
        aMediaDescr.realloc( 2 );
        aMediaDescr[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ) );
        aMediaDescr[1].Value <<= xInteraction;
    }

    //TODO/LATER: how to cope with creation failure?!
    uno::Reference< embed::XEmbeddedObject > xObj =
            pPersist->getEmbeddedObjectContainer().InsertEmbeddedLink( aMediaDescr , aPersistName );

    if( xObj.is() )
    {
        Rectangle aRect = pOle2Obj->GetLogicRect();
        if ( aRect.GetWidth() == 100 && aRect.GetHeight() == 100 )
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
        setPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ), uno::makeAny( aPersistName ) );

        // the object is inserted during setting of PersistName property usually
        if ( pOle2Obj->IsEmpty() )
            pOle2Obj->SetObjRef( xObj );
    }

    return xObj.is();
}

///////////////////////////////////////////////////////////////////////
#ifndef SVX_LIGHT
SvxAppletShape::SvxAppletShape( SdrObject* pObject ) throw()
: SvxOle2Shape( pObject, aSvxMapProvider.GetMap(SVXMAP_APPLET)  )
{
    SetShapeType( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.AppletShape" ) ) );
}

SvxAppletShape::~SvxAppletShape() throw()
{
}

void SvxAppletShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw ()
{
    SvxShape::Create( pNewObj, pNewPage );
    const SvGlobalName aAppletClassId( SO3_APPLET_CLASSID );
    createObject(aAppletClassId);
    SetShapeType( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.AppletShape" ) ) );
}

//XPropertySet
void SAL_CALL SvxAppletShape::setPropertyValue( const OUString& aPropertyName, const Any& aValue )  throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Bool bOwn = sal_False;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(aPropertyName);

    if( pMap && mpObj.is() && mpModel )
    {
        if( pMap->nWID >= OWN_ATTR_APPLET_DOCBASE && pMap->nWID <= OWN_ATTR_APPLET_ISSCRIPT )
        {
            if ( !svt::EmbeddedObjectRef::TryRunningState( ((SdrOle2Obj*)mpObj.get())->GetObjRef() ) )
                return;

            uno::Reference < beans::XPropertySet > xSet( ((SdrOle2Obj*)mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if ( !xSet.is() )
                return;

            switch( pMap->nWID )
            {
                case OWN_ATTR_APPLET_CODEBASE:
                case OWN_ATTR_APPLET_NAME:
                case OWN_ATTR_APPLET_CODE:
                case OWN_ATTR_APPLET_COMMANDS:
                case OWN_ATTR_APPLET_ISSCRIPT:
                case OWN_ATTR_APPLET_DOCBASE:
                    // allow exceptions to pass through
                    xSet->setPropertyValue( aPropertyName, aValue );
                    bOwn = sal_True;
                    break;
                default:
                    throw IllegalArgumentException();
            }
        }
    }

    if( !bOwn )
        SvxOle2Shape::setPropertyValue( aPropertyName, aValue );

    if( mpModel )
    {
        ::comphelper::IEmbeddedHelper* pPersist = mpModel->GetPersist();
        if( pPersist && !pPersist->isEnableSetModified() )
        {
            SdrOle2Obj* pOle = static_cast< SdrOle2Obj* >( mpObj.get() );
            if( pOle && !pOle->IsEmpty() )
            {
                uno::Reference < util::XModifiable > xMod( ((SdrOle2Obj*)mpObj.get())->GetObjRef(), uno::UNO_QUERY );
                if( xMod.is() )
                    // TODO/MBA: what's this?!
                    xMod->setModified( sal_False );
            }
        }
    }
}

Any SAL_CALL SvxAppletShape::getPropertyValue( const OUString& PropertyName ) throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    if( pMap && mpObj.is() && mpModel )
    {
        if( pMap->nWID >= OWN_ATTR_APPLET_DOCBASE && pMap->nWID <= OWN_ATTR_APPLET_ISSCRIPT )
        {
            if ( !svt::EmbeddedObjectRef::TryRunningState( ((SdrOle2Obj*)mpObj.get())->GetObjRef() ) )
                return uno::Any();

            uno::Reference < beans::XPropertySet > xSet( ((SdrOle2Obj*)mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if ( !xSet.is() )
                return uno::Any();

            switch( pMap->nWID )
            {
                case OWN_ATTR_APPLET_CODEBASE:
                case OWN_ATTR_APPLET_NAME:
                case OWN_ATTR_APPLET_CODE:
                case OWN_ATTR_APPLET_COMMANDS:
                case OWN_ATTR_APPLET_ISSCRIPT:
                    return xSet->getPropertyValue( PropertyName );
                default:
                    throw IllegalArgumentException();
            }
        }
    }

    return SvxOle2Shape::getPropertyValue( PropertyName );
}

///////////////////////////////////////////////////////////////////////

SvxPluginShape::SvxPluginShape( SdrObject* pObject ) throw()
: SvxOle2Shape( pObject, aSvxMapProvider.GetMap(SVXMAP_PLUGIN)  )
{
    SetShapeType( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.PluginShape" ) ) );
}

SvxPluginShape::~SvxPluginShape() throw()
{
}

void SvxPluginShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw ()
{
    SvxShape::Create( pNewObj, pNewPage );
    const SvGlobalName aPluginClassId( SO3_PLUGIN_CLASSID );
    createObject(aPluginClassId);
    SetShapeType( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.PluginShape" ) ) );
}

//XPropertySet
void SAL_CALL SvxPluginShape::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Bool bOwn = sal_False;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(aPropertyName);

    if( pMap && mpObj.is() && mpModel )
    {
        if( pMap->nWID >= OWN_ATTR_PLUGIN_MIMETYPE && pMap->nWID <= OWN_ATTR_PLUGIN_COMMANDS )
        {
            if ( !svt::EmbeddedObjectRef::TryRunningState( ((SdrOle2Obj*)mpObj.get())->GetObjRef() ) )
                return;

            uno::Reference < beans::XPropertySet > xSet( ((SdrOle2Obj*)mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if ( !xSet.is() )
                return;

            switch( pMap->nWID )
            {
                case OWN_ATTR_PLUGIN_MIMETYPE:
                case OWN_ATTR_PLUGIN_URL:
                case OWN_ATTR_PLUGIN_COMMANDS:
                    // allow exceptions to pass through
                    xSet->setPropertyValue( aPropertyName, aValue );
                    bOwn = sal_True;
                    break;
                default:
                    throw IllegalArgumentException();
            }
        }
    }

    if( !bOwn )
        SvxOle2Shape::setPropertyValue( aPropertyName, aValue );


    if( mpModel )
    {
        ::comphelper::IEmbeddedHelper* pPersist = mpModel->GetPersist();
        if( pPersist && !pPersist->isEnableSetModified() )
        {
            SdrOle2Obj* pOle = static_cast< SdrOle2Obj* >( mpObj.get() );
            if( pOle && !pOle->IsEmpty() )
            {
                uno::Reference < util::XModifiable > xMod( ((SdrOle2Obj*)mpObj.get())->GetObjRef(), uno::UNO_QUERY );
                if( xMod.is() )
                    // TODO/MBA: what's this?!
                    xMod->setModified( sal_False );
            }
        }
    }
}

Any SAL_CALL SvxPluginShape::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    if( pMap && mpObj.is() && mpModel )
    {
        if( pMap->nWID >= OWN_ATTR_PLUGIN_MIMETYPE && pMap->nWID <= OWN_ATTR_PLUGIN_COMMANDS )
        {
            if ( !svt::EmbeddedObjectRef::TryRunningState( ((SdrOle2Obj*)mpObj.get())->GetObjRef() ) )
                return uno::Any();

            uno::Reference < beans::XPropertySet > xSet( ((SdrOle2Obj*)mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if ( !xSet.is() )
                return uno::Any();

            switch( pMap->nWID )
            {
                case OWN_ATTR_PLUGIN_MIMETYPE:
                case OWN_ATTR_PLUGIN_URL:
                case OWN_ATTR_PLUGIN_COMMANDS:
                    return xSet->getPropertyValue( PropertyName );
                default:
                    throw IllegalArgumentException();
            }
        }
    }

    return SvxOle2Shape::getPropertyValue( PropertyName );
}

///////////////////////////////////////////////////////////////////////

SvxFrameShape::SvxFrameShape( SdrObject* pObject ) throw()
: SvxOle2Shape( pObject, aSvxMapProvider.GetMap(SVXMAP_FRAME)  )
{
    SetShapeType( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.FrameShape" ) ) );
}

SvxFrameShape::~SvxFrameShape() throw()
{
}

void SvxFrameShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw ()
{
    SvxShape::Create( pNewObj, pNewPage );
    const SvGlobalName aIFrameClassId( SO3_IFRAME_CLASSID );
    createObject(aIFrameClassId);
    SetShapeType( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.FrameShape" ) ) );
}

//XPropertySet
void SAL_CALL SvxFrameShape::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Bool bOwn = sal_False;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(aPropertyName);

    Any aAny;
    if( pMap && mpObj.is() && mpModel )
    {
        if( pMap->nWID >= OWN_ATTR_FRAME_URL && pMap->nWID <= OWN_ATTR_FRAME_MARGIN_HEIGHT )
        {
            if ( !svt::EmbeddedObjectRef::TryRunningState( ((SdrOle2Obj*)mpObj.get())->GetObjRef() ) )
                return;

            uno::Reference < beans::XPropertySet > xSet( ((SdrOle2Obj*)mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if ( !xSet.is() )
                return;

            switch( pMap->nWID )
            {
                //TODO/LATER: more properties!
                case OWN_ATTR_FRAME_URL:
                case OWN_ATTR_FRAME_NAME:
                case OWN_ATTR_FRAME_ISAUTOSCROLL:
                case OWN_ATTR_FRAME_ISBORDER:
                case OWN_ATTR_FRAME_MARGIN_WIDTH:
                case OWN_ATTR_FRAME_MARGIN_HEIGHT:
                    // allow exceptions to pass through
                    xSet->setPropertyValue( aPropertyName, aValue );
                    bOwn = sal_True;
                    break;
                default:
                    throw IllegalArgumentException();
            }
        }
    }

    if( !bOwn )
        SvxOle2Shape::setPropertyValue( aPropertyName, aValue );

    if( mpModel )
    {
        ::comphelper::IEmbeddedHelper* pPersist = mpModel->GetPersist();
        if( pPersist && !pPersist->isEnableSetModified() )
        {
            SdrOle2Obj* pOle = static_cast< SdrOle2Obj* >( mpObj.get() );
               if( pOle && !pOle->IsEmpty() )
            {
                   uno::Reference < util::XModifiable > xMod( ((SdrOle2Obj*)mpObj.get())->GetObjRef(), uno::UNO_QUERY );
                   if( xMod.is() )
                       // TODO/MBA: what's this?!
                       xMod->setModified( sal_False );
            }
        }
    }
}

Any SAL_CALL SvxFrameShape::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    Any aAny;
    if( pMap && mpObj.is() && mpModel )
    {
        if( pMap->nWID >= OWN_ATTR_FRAME_URL && pMap->nWID <= OWN_ATTR_FRAME_MARGIN_HEIGHT )
        {
            if ( !svt::EmbeddedObjectRef::TryRunningState( ((SdrOle2Obj*)mpObj.get())->GetObjRef() ) )
                return uno::Any();

            uno::Reference < beans::XPropertySet > xSet( ((SdrOle2Obj*)mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if ( !xSet.is() )
                return uno::Any();

            switch( pMap->nWID )
            {
                //TODO/LATER: more properties!
                case OWN_ATTR_FRAME_URL:
                case OWN_ATTR_FRAME_NAME:
                case OWN_ATTR_FRAME_ISAUTOSCROLL:
                case OWN_ATTR_FRAME_ISBORDER:
                case OWN_ATTR_FRAME_MARGIN_WIDTH:
                case OWN_ATTR_FRAME_MARGIN_HEIGHT:
                    return xSet->getPropertyValue( PropertyName );
                default:
                    throw IllegalArgumentException();
            }
        }
    }

    return SvxOle2Shape::getPropertyValue( PropertyName );
}

/***********************************************************************
*                                                                      *
***********************************************************************/

SvxMediaShape::SvxMediaShape( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_MEDIA) )
{
    SetShapeType( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.MediaShape" ) ) );
}

//----------------------------------------------------------------------
SvxMediaShape::~SvxMediaShape() throw()
{
}

//----------------------------------------------------------------------

void SAL_CALL SvxMediaShape::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    OGuard                      aGuard( Application::GetSolarMutex() );
    const SfxItemPropertyMap*   pMap = maPropSet.getPropertyMapEntry( rPropertyName );
    bool                        bOwn = false;

    if( pMap && mpObj.is() && mpModel )
    {
        SdrMediaObj* pMedia = dynamic_cast< SdrMediaObj* >( mpObj.get() );

        if( pMedia && ( pMap->nWID >= OWN_ATTR_MEDIA_URL ) && ( pMap->nWID <= OWN_ATTR_MEDIA_ZOOM ) )
        {
            ::avmedia::MediaItem aItem;

            switch( pMap->nWID )
            {
                case OWN_ATTR_MEDIA_URL:
                {
                    OUString aURL;

                    if( rValue >>= aURL )
                        aItem.setURL( aURL );
                }
                break;

                case( OWN_ATTR_MEDIA_LOOP ):
                {
                    sal_Bool bLoop = sal_Bool();

                    if( rValue >>= bLoop )
                        aItem.setLoop( bLoop );
                }
                break;

                case( OWN_ATTR_MEDIA_MUTE ):
                {
                    sal_Bool bMute = sal_Bool();

                    if( rValue >>= bMute )
                        aItem.setMute( bMute );
                }
                break;

                case( OWN_ATTR_MEDIA_VOLUMEDB ):
                {
                    sal_Int16 nVolumeDB = sal_Int16();

                    if( rValue >>= nVolumeDB )
                        aItem.setVolumeDB( nVolumeDB );
                }
                break;

                case( OWN_ATTR_MEDIA_ZOOM ):
                {
                    ::com::sun::star::media::ZoomLevel eLevel;

                    if( rValue >>= eLevel )
                        aItem.setZoom( eLevel );
                }
                break;

                default:
                    throw IllegalArgumentException();
            }

            pMedia->setMediaProperties( aItem );
            bOwn = true;
        }
    }

    if( !bOwn )
        SvxShape::setPropertyValue( rPropertyName, rValue );
}

//----------------------------------------------------------------------

Any SAL_CALL SvxMediaShape::getPropertyValue( const OUString& rPropertyName )
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard                      aGuard( Application::GetSolarMutex() );
    const SfxItemPropertyMap*   pMap = maPropSet.getPropertyMapEntry( rPropertyName );

    if( pMap && mpObj.is() && mpModel )
    {
        SdrMediaObj* pMedia = dynamic_cast< SdrMediaObj* >( mpObj.get() );

        if( pMedia && ( pMap->nWID >= OWN_ATTR_MEDIA_URL ) && ( pMap->nWID <= OWN_ATTR_MEDIA_ZOOM ) )
        {
            const ::avmedia::MediaItem aItem( pMedia->getMediaProperties() );

            switch( pMap->nWID )
            {
                case OWN_ATTR_MEDIA_URL:
                    return makeAny( aItem.getURL() );

                case( OWN_ATTR_MEDIA_LOOP ):
                    return makeAny( (sal_Bool) aItem.isLoop() );

                case( OWN_ATTR_MEDIA_MUTE ):
                    return makeAny( (sal_Bool) aItem.isMute() );

                case( OWN_ATTR_MEDIA_VOLUMEDB ):
                    return makeAny( (sal_Int16) aItem.getVolumeDB() );

                case( OWN_ATTR_MEDIA_ZOOM ):
                    return makeAny( aItem.getZoom() );

                default:
                    throw IllegalArgumentException();
            }
        }
    }

    return SvxShape::getPropertyValue( rPropertyName );
}

#endif
