/*************************************************************************
 *
 *  $RCSfile: unoshap4.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:06:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SVX_USE_UNOGLOBALS_

#ifndef _SVDOOLE2_HXX
#include "svdoole2.hxx"
#endif
#ifndef _SVDOMEDIA_HXX
#include "svdomedia.hxx"
#endif

#include <so3/outplace.hxx>

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

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVX_SHAPEIMPL_HXX
#include "shapeimpl.hxx"
#endif

#include "unoshprp.hxx"
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
    Any aAny;
    SvxShape::queryAggregation( rType, aAny );
    return aAny;
}

//XPropertySet
void SAL_CALL SvxOle2Shape::setPropertyValue( const OUString& aPropertyName, const Any& aValue )    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CLSID" ) ) )
    {
#ifndef SVX_LIGHT
        OUString aCLSID;
        if( aValue >>= aCLSID )
        {
            // init a ole object with a global name
            if( pObj )
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
#endif
    }
    else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ThumbnailGraphicURL" ) ) )
    {
#ifndef SVX_LIGHT
        // only allow setting of thumbnail for player
        return;
#else
        OUString aURL;
        if( aValue >>= aURL )
        {
            SdrOle2Obj* pOle = PTR_CAST( SdrOle2Obj, pObj );
            if( pOle )
            {
                GraphicObject aGrafObj( CreateGraphicObjectFromURL( aURL ) );
                pOle->SetGraphic( &aGrafObj.GetGraphic() );
            }
            return;
        }

        throw IllegalArgumentException();
#endif
    }
    else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( UNO_NAME_OLE2_PERSISTNAME ) ) )
    {
        OUString aPersistName;

        if( aValue >>= aPersistName )
        {
            SdrOle2Obj* pOle = PTR_CAST( SdrOle2Obj, pObj );

            if( pOle )
                pOle->SetPersistName( aPersistName );

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
        SdrOle2Obj* pOle = PTR_CAST( SdrOle2Obj, pObj );

        if( pOle )
        {
            Graphic* pGraphic = pOle->GetGraphic();

            // if there isn't already a preview graphic set, check if we need to generate
            // one if model says so
            if( pGraphic == NULL && !pOle->IsEmptyPresObj() && pModel->IsSaveOLEPreview() )
            {
                const GDIMetaFile* pMetaFile = pOle->GetGDIMetaFile();
                if( pMetaFile )
                {
                    Graphic aNewGrf( *pMetaFile );
                    pOle->SetGraphic( &aNewGrf );
                    pGraphic = pOle->GetGraphic();
                }
            }

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
        SdrOle2Obj* pOle = PTR_CAST( SdrOle2Obj, pObj );

        if( pOle )
        {
            aPersistName = pOle->GetPersistName();
            if( aPersistName.getLength() )
            {
                SvPersist *pPersist = pObj->GetModel()->GetPersist();
                if( (NULL == pPersist) || ( NULL == pPersist->Find( static_cast< SdrOle2Obj* >( pObj )->GetPersistName() ) ) )
                    aPersistName = OUString();
            }
        }

        return makeAny( aPersistName );
    }

    return SvxShape::getPropertyValue( PropertyName );
}


sal_Bool SvxOle2Shape::createObject( const SvGlobalName &aClassName )
{
    const SvInPlaceObjectRef& rIPRef = static_cast< SdrOle2Obj* >( pObj )->GetObjRef();

    if( rIPRef.Is() )
        return sal_False;

    // create storage and inplace object
    String              aEmptyStr;
    SvStorageRef        aStor;
    SvInPlaceObjectRef aIPObj = SvInPlaceObject::CreateObject( aClassName );

    SvPersist*          pPersist = pModel->GetPersist();
    sal_Bool            bOk = sal_False;
    String              aPersistName;
    OUString            aTmpStr;
    Any                 aAny( getPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ) ) );

    if( aAny >>= aTmpStr )
        aPersistName = aTmpStr;

    // if we already have a shape name check if its a unique
    // storage name
    if( aPersistName.Len() && !pPersist->Find( aPersistName ) )
    {
        SvInfoObjectRef xSub = new SvEmbeddedInfoObject( aIPObj, aPersistName );
        bOk = pPersist->Move( xSub, aPersistName );
    }
    else
    {
        // generate a unique name
        String aStr( aPersistName = String( RTL_CONSTASCII_USTRINGPARAM("Object ") ) );

        // for-Schleife wegen Storage Bug 46033
        for( sal_uInt32 i = 1, n = 0; n < 100; n++ )
        {
            do
            {
                aStr = aPersistName;
                aStr += String::CreateFromInt32( i++ );
            }
            while( pPersist->Find( aStr ) );

            SvInfoObjectRef xSub( new SvEmbeddedInfoObject( aIPObj, aStr ) );

            if( pPersist->Move( xSub, aStr ) ) // Eigentuemer Uebergang
            {
                bOk = sal_True;
                aPersistName = aStr;
                break;
            }
        }
    }

    DBG_ASSERT( bOk, "could not create move ole stream!" )

    if( bOk )
    {
        aAny <<= ( aTmpStr = aPersistName );
        setPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ), aAny );
    }

    static_cast< SdrOle2Obj* >( pObj )->SetObjRef( aIPObj );
    Rectangle aRect = static_cast< SdrOle2Obj* >( pObj )->GetLogicRect();
    if ( aRect.GetWidth() == 100 && aRect.GetHeight() == 100 )
    {
        // default size
        aRect.SetSize( aIPObj->GetVisArea().GetSize() );
        static_cast< SdrOle2Obj* >( pObj )->SetLogicRect( aRect );
    }
    else
        aIPObj->SetVisAreaSize( static_cast< SdrOle2Obj* >( pObj )->GetLogicRect().GetSize() );
    return bOk;
}

///////////////////////////////////////////////////////////////////////

// the following code is currently not working in the player
#ifndef SVX_LIGHT

static sal_Bool SvxImplFillCommandList( const Sequence< PropertyValue >& aCommandSequence, SvCommandList& aNewCommands )
{
    const sal_Int32 nCount = aCommandSequence.getLength();

    String aCommand, aArg;
    OUString aApiArg;
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        aCommand = aCommandSequence[nIndex].Name;

        if( !( aCommandSequence[nIndex].Value >>= aApiArg ) )
            return sal_False;

        aArg = aApiArg;
        aNewCommands.Append( aCommand, aArg );
    }

    return sal_True;
}

static void SvxImplFillCommandSequence( const SvCommandList& aCommands, Sequence< PropertyValue >& aCommandSequence )
{
    const sal_Int32 nCount = aCommands.Count();
    aCommandSequence.realloc( nCount );

    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const SvCommand& rCommand = aCommands[ nIndex ];

        aCommandSequence[nIndex].Name = rCommand.GetCommand();
        aCommandSequence[nIndex].Handle = -1;
        aCommandSequence[nIndex].Value = makeAny( OUString( rCommand.GetArgument() ) );
        aCommandSequence[nIndex].State = PropertyState_DIRECT_VALUE;

    }
}

///////////////////////////////////////////////////////////////////////

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

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    if( pMap && pObj && pModel )
    {
        if( pMap->nWID >= OWN_ATTR_APPLET_CODEBASE && pMap->nWID <= OWN_ATTR_APPLET_ISSCRIPT )
        {
            SvAppletObjectRef xApplet = SvAppletObjectRef( ((SdrOle2Obj*)pObj)->GetObjRef() );
            DBG_ASSERT( xApplet.Is(), "wrong ole object inside applet" );
            if( !xApplet.Is() )
                return;

            switch( pMap->nWID )
            {
                case OWN_ATTR_APPLET_CODEBASE:
                    {
                        OUString aCodeBase;
                        if( aValue >>= aCodeBase )
                        {
                            const String aStrCodeBase( aCodeBase );
                            xApplet->SetCodeBase( aStrCodeBase );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_APPLET_NAME:
                    {
                        OUString aName;
                        if( aValue >>= aName )
                        {
                            const String aStrName( aName );
                            xApplet->SetName( aStrName );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_APPLET_CODE:
                    {
                        OUString aCode;
                        if( aValue >>= aCode )
                        {
                            const String aStrCode( aCode );
                            xApplet->SetClass( aStrCode );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_APPLET_COMMANDS:
                    {
                        Sequence< PropertyValue > aCommandSequence;
                        if( aValue >>= aCommandSequence )
                        {
                            SvCommandList aNewCommands;
                            if( SvxImplFillCommandList( aCommandSequence, aNewCommands ) )
                            {
                                xApplet->SetCommandList( aNewCommands );
                                bOwn = sal_True;
                            }
                        }
                    }
                    break;
                case OWN_ATTR_APPLET_ISSCRIPT:
                    {
                        sal_Bool bScript;
                        if( aValue >>= bScript )
                        {
                            xApplet->SetMayScript( bScript );
                            bOwn = sal_True;
                        }
                    }
                    break;
            }

            if( !bOwn )
                throw IllegalArgumentException();
        }
    }

    if( !bOwn )
        SvxOle2Shape::setPropertyValue( aPropertyName, aValue );

    if( pModel )
    {
        SvPersist* pPersist = pModel->GetPersist();
        if( pPersist && !pPersist->IsEnableSetModified() )
        {
            SdrOle2Obj* pOle = static_cast< SdrOle2Obj* >( pObj );
            if( pOle && ! pOle->IsEmpty() )
            {
                const SvInPlaceObjectRef& rIPRef = pOle->GetObjRef();

                if( rIPRef.Is() )
                    rIPRef->SetModified( sal_False );
            }
        }
    }
}

Any SAL_CALL SvxAppletShape::getPropertyValue( const OUString& PropertyName ) throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    if( pMap && pObj && pModel )
    {
        if( pMap->nWID >= OWN_ATTR_APPLET_CODEBASE && pMap->nWID <= OWN_ATTR_APPLET_ISSCRIPT )
        {
            SvAppletObjectRef xApplet = SvAppletObjectRef( ((SdrOle2Obj*)pObj)->GetObjRef() );
            DBG_ASSERT( xApplet.Is(), "wrong ole object inside applet" );
            if( xApplet.Is() )
            {

                switch( pMap->nWID )
                {
                    case OWN_ATTR_APPLET_CODEBASE:
                        return makeAny( OUString( xApplet->GetCodeBase() ) );
                    case OWN_ATTR_APPLET_NAME:
                        return makeAny( OUString( xApplet->GetName() ) );
                    case OWN_ATTR_APPLET_CODE:
                        return makeAny( OUString( xApplet->GetClass() ) );
                    case OWN_ATTR_APPLET_COMMANDS:
                        {
                            Sequence< PropertyValue > aCommandSequence;
                            SvxImplFillCommandSequence( xApplet->GetCommandList(), aCommandSequence );
                            return makeAny( aCommandSequence );
                        }
                    case OWN_ATTR_APPLET_ISSCRIPT:
                        return makeAny( (sal_Bool) xApplet->IsMayScript() );
                }
            }

            Any aAny;
            return aAny;
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

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    if( pMap && pObj && pModel )
    {
        if( pMap->nWID >= OWN_ATTR_PLUGIN_MIMETYPE && pMap->nWID <= OWN_ATTR_PLUGIN_COMMANDS )
        {
            SvPlugInObjectRef xPlugin = SvPlugInObjectRef( ((SdrOle2Obj*)pObj)->GetObjRef() );
            DBG_ASSERT( xPlugin.Is(), "wrong ole object inside plugin" );
            if( !xPlugin.Is() )
                return;

            switch( pMap->nWID )
            {
                case OWN_ATTR_PLUGIN_MIMETYPE:
                    {
                        OUString aMimeType;
                        if( aValue >>= aMimeType )
                        {
                            const String aStrMimeType( aMimeType );
                            xPlugin->SetMimeType( aStrMimeType );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_PLUGIN_URL:
                    {
                        OUString aURL;
                        if( aValue >>= aURL )
                        {
                            const String aStrURL( aURL );
                            xPlugin->SetURL( aStrURL );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_PLUGIN_COMMANDS:
                    {
                        Sequence< PropertyValue > aCommandSequence;
                        if( aValue >>= aCommandSequence )
                        {
                            SvCommandList aNewCommands;
                            if( SvxImplFillCommandList( aCommandSequence, aNewCommands ) )
                            {
                                xPlugin->SetCommandList( aNewCommands );
                                bOwn = sal_True;
                            }
                        }
                    }
                    break;
            }

            if( !bOwn )
                throw IllegalArgumentException();
        }
    }

    if( !bOwn )
        SvxOle2Shape::setPropertyValue( aPropertyName, aValue );

    if( pModel )
    {
        SvPersist* pPersist = pModel->GetPersist();
        if( pPersist && !pPersist->IsEnableSetModified() )
        {
            SdrOle2Obj* pOle = static_cast< SdrOle2Obj* >( pObj );
            if( pOle && ! pOle->IsEmpty() )
            {
                const SvInPlaceObjectRef& rIPRef = pOle->GetObjRef();

                if( rIPRef.Is() )
                    rIPRef->SetModified( sal_False );
            }
        }
    }
}

Any SAL_CALL SvxPluginShape::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    if( pMap && pObj && pModel )
    {
        if( pMap->nWID >= OWN_ATTR_PLUGIN_MIMETYPE && pMap->nWID <= OWN_ATTR_PLUGIN_COMMANDS )
        {
            SvPlugInObjectRef xPlugin = SvPlugInObjectRef( ((SdrOle2Obj*)pObj)->GetObjRef() );
            DBG_ASSERT( xPlugin.Is(), "wrong ole object inside plugin" );
            if( xPlugin.Is() )
            {

                switch( pMap->nWID )
                {
                    case OWN_ATTR_PLUGIN_MIMETYPE:
                        return makeAny( OUString( xPlugin->GetMimeType() ) );
                    case OWN_ATTR_PLUGIN_URL:
                    {
                        OUString aURL;
                        DBG_ASSERT( xPlugin->GetURL(), "Plugin without a URL!" );
                        if( xPlugin->GetURL() )
                            aURL = xPlugin->GetURL()->GetMainURL( INetURLObject::NO_DECODE );
                        return makeAny( aURL );
                    }
                    case OWN_ATTR_PLUGIN_COMMANDS:
                        {
                            Sequence< PropertyValue > aCommandSequence;
                            SvxImplFillCommandSequence( xPlugin->GetCommandList(), aCommandSequence );
                            return makeAny( aCommandSequence );
                        }
                }
            }

            Any aAny;
            return aAny;
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

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    Any aAny;
    if( pMap && pObj && pModel )
    {
        if( pMap->nWID >= OWN_ATTR_FRAME_URL && pMap->nWID <= OWN_ATTR_FRAME_MARGIN_HEIGHT )
        {
            SfxFrameObjectRef xFrame = SfxFrameObjectRef( ((SdrOle2Obj*)pObj)->GetObjRef() );
            DBG_ASSERT( xFrame.Is(), "wrong ole object inside frame" );
            if( !xFrame.Is() )
                return;

            SfxFrameDescriptor *pDescriptor = xFrame->GetFrameDescriptor()->Clone( NULL );
            DBG_ASSERT( pDescriptor, "no descriptor for frame!" );
            if( !pDescriptor )
                return;

            switch( pMap->nWID )
            {
                case OWN_ATTR_FRAME_URL:
                    {
                        OUString aURL;
                        if( aValue >>= aURL )
                        {
                            const String aStrURL( aURL );
                            pDescriptor->SetURL( aStrURL );
                            xFrame->SetFrameDescriptor( pDescriptor );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_FRAME_NAME:
                    {
                        OUString aName;
                        if( aValue >>= aName )
                        {
                            const String aStrName( aName );
                            pDescriptor->SetName( aStrName );
                            xFrame->SetFrameDescriptor( pDescriptor );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_FRAME_ISAUTOSCROLL:
                    {
                        sal_Bool bScroll;
                        if( !aValue.hasValue() )
                        {
                            pDescriptor->SetScrollingMode( ScrollingAuto );
                            xFrame->SetFrameDescriptor( pDescriptor );
                            bOwn = sal_True;
                        }
                        else if( aValue >>= bScroll )
                        {
                            pDescriptor->SetScrollingMode( bScroll ? ScrollingYes : ScrollingNo );
                            xFrame->SetFrameDescriptor( pDescriptor );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_FRAME_ISBORDER:
                    {
                        sal_Bool bBorder;
                        if( aValue >>= bBorder )
                        {
                            pDescriptor->SetFrameBorder( bBorder );
                            xFrame->SetFrameDescriptor( pDescriptor );
                            bOwn = sal_True;
                        }
                    }
                    break;

                case OWN_ATTR_FRAME_MARGIN_WIDTH:
                    {
                        sal_Int32 nMargin;
                        if( aValue >>= nMargin )
                        {
                            const Size aNewMargin( nMargin, pDescriptor->GetMargin().Height() );
                            pDescriptor->SetMargin( aNewMargin );
                            xFrame->SetFrameDescriptor( pDescriptor );
                            bOwn = sal_True;
                        }
                    }
                    break;
                case OWN_ATTR_FRAME_MARGIN_HEIGHT:
                    {
                        sal_Int32 nMargin;
                        if( aValue >>= nMargin )
                        {
                            const Size aNewMargin( pDescriptor->GetMargin().Width(), nMargin );
                            pDescriptor->SetMargin( aNewMargin );
                            xFrame->SetFrameDescriptor( pDescriptor );
                            bOwn = sal_True;
                        }
                    }
                    break;
            }

            if( !bOwn )
                throw IllegalArgumentException();
        }
    }

    if( !bOwn )
        SvxOle2Shape::setPropertyValue( aPropertyName, aValue );

    if( pModel )
    {
        SvPersist* pPersist = pModel->GetPersist();
        if( pPersist && !pPersist->IsEnableSetModified() )
        {
            SdrOle2Obj* pOle = static_cast< SdrOle2Obj* >( pObj );
            if( pOle && ! pOle->IsEmpty() )
            {
                const SvInPlaceObjectRef& rIPRef = pOle->GetObjRef();

                if( rIPRef.Is() )
                    rIPRef->SetModified( sal_False );
            }
        }
    }
}

Any SAL_CALL SvxFrameShape::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    Any aAny;
    if( pMap && pObj && pModel )
    {
        if( pMap->nWID >= OWN_ATTR_FRAME_URL && pMap->nWID <= OWN_ATTR_FRAME_MARGIN_HEIGHT )
        {
            SfxFrameObjectRef xFrame = SfxFrameObjectRef( ((SdrOle2Obj*)pObj)->GetObjRef() );
            DBG_ASSERT( xFrame.Is(), "wrong ole object inside frame" );
            if( !xFrame.Is() )
            {
                Any aAny;
                return aAny;
            }

            const SfxFrameDescriptor *pDescriptor = xFrame->GetFrameDescriptor();
            DBG_ASSERT( pDescriptor, "no descriptor for frame!" );

            switch( pMap->nWID )
            {
                case OWN_ATTR_FRAME_URL:
                    return makeAny( OUString( pDescriptor->GetURL().GetMainURL( INetURLObject::NO_DECODE ) ) );
                case OWN_ATTR_FRAME_NAME:
                    return makeAny( OUString( pDescriptor->GetName() ) );
                case OWN_ATTR_FRAME_ISAUTOSCROLL:
                    if( pDescriptor->GetScrollingMode() == ScrollingAuto )
                    {
                        Any aAny;
                        return aAny;
                    }
                    else
                    {
                        return makeAny( (sal_Bool)(pDescriptor->GetScrollingMode() == ScrollingYes) );
                    }
                case OWN_ATTR_FRAME_ISBORDER:
                    return makeAny( (sal_Bool)pDescriptor->IsFrameBorderOn() );
                case OWN_ATTR_FRAME_MARGIN_WIDTH:
                    return makeAny( (sal_Int32)pDescriptor->GetMargin().Width() );
                case OWN_ATTR_FRAME_MARGIN_HEIGHT:
                    return makeAny( (sal_Int32)pDescriptor->GetMargin().Height() );
            }
            throw IllegalArgumentException();
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
    const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMapEntry( rPropertyName );
    bool                        bOwn = false;

    if( pMap && pObj && pModel )
    {
        SdrMediaObj* pMedia = PTR_CAST( SdrMediaObj, pObj );

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
                    sal_Bool bLoop;

                    if( rValue >>= bLoop )
                        aItem.setLoop( bLoop );
                }
                break;

                case( OWN_ATTR_MEDIA_MUTE ):
                {
                    sal_Bool bMute;

                    if( rValue >>= bMute )
                        aItem.setMute( bMute );
                }
                break;

                case( OWN_ATTR_MEDIA_VOLUMEDB ):
                {
                    sal_Int16 nVolumeDB;

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
    const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMapEntry( rPropertyName );

    if( pMap && pObj && pModel )
    {
        SdrMediaObj* pMedia = PTR_CAST( SdrMediaObj, pObj );

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
