/*************************************************************************
 *
 *  $RCSfile: unoshap4.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: cl $ $Date: 2001-08-24 08:22:46 $
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

    SvxShape::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL SvxOle2Shape::getPropertyValue( const OUString& PropertyName ) throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ThumbnailGraphicURL" ) ) )
    {
        OUString aURL;

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

    return SvxShape::getPropertyValue( PropertyName );
}


sal_Bool SvxOle2Shape::createObject( const SvGlobalName &aClassName )
{
#ifndef SVX_LIGHT
    const SvInPlaceObjectRef& rIPRef = ((SdrOle2Obj*)pObj)->GetObjRef();
    if(rIPRef.Is() )
        return sal_False;

    // create storage and inplace object
    String aEmptyStr;
    SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
    SvInPlaceObjectRef aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit( aClassName, aStor);

    SvPersist* pPersist = pModel->GetPersist();

    String aName = getName();

    sal_Bool bOk = sal_False;
    // if we already have a shape name check if its a unique
    // storage name
    if( aName.Len() && !pPersist->Find( aName ) )
    {
        SvInfoObjectRef xSub = new SvEmbeddedInfoObject( aIPObj, aName );
        bOk = pPersist->Move( xSub, aName );
    }
    else
    {
        // generate a unique name

        aName = ImpGetResStr( STR_ObjOLE2NamePrefix );
        aName += sal_Unicode(' ');
        String aStr;
        sal_Int32 i = 1;
        HACK(Wegen Storage Bug 46033)
        // for-Schleife wegen Storage Bug 46033
        for( sal_Int16 n = 0; n < 100; n++ )
        {
            do
            {
                aStr = aName;
                aStr += String::CreateFromInt32( i );
                i++;
            } while ( pPersist->Find( aStr ) );

            SvInfoObjectRef xSub = new SvEmbeddedInfoObject( aIPObj, aStr );
            if( pPersist->Move( xSub, aStr ) ) // Eigentuemer Uebergang
            {
                bOk = sal_True;
                aName = aStr;
                break;
            }
        }
    }

    DBG_ASSERT( bOk, "could not create move ole stream!" )

    if( bOk )
        pObj->SetName( aName );

    ((SdrOle2Obj*)pObj)->SetObjRef(aIPObj);

    Rectangle aRect( ( (SdrOle2Obj*) pObj)->GetLogicRect() );
    aIPObj->SetVisAreaSize( aRect.GetSize() );

    return bOk;
#else
    return sal_False;
#endif
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

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    if( pObj && pModel )
    {
        if(pMap == NULL )
            throw UnknownPropertyException();

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
                            return;
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
                            return;
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
                            return;
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
                                return;
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
                            return;
                        }
                    }
                    break;
            }

            throw IllegalArgumentException();
        }
    }

    SvxOle2Shape::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL SvxAppletShape::getPropertyValue( const OUString& PropertyName ) throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    if( pObj && pModel )
    {
        if(pMap == NULL )
            throw UnknownPropertyException();

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

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    if( pObj && pModel )
    {
        if(pMap == NULL )
            throw UnknownPropertyException();

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
                            return;
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
                            return;
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
                                return;
                            }
                        }
                    }
                    break;
            }

            throw IllegalArgumentException();
        }
    }

    SvxOle2Shape::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL SvxPluginShape::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    if( pObj && pModel )
    {
        if(pMap == NULL )
            throw UnknownPropertyException();

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

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    Any aAny;
    if( pObj && pModel )
    {
        if(pMap == NULL )
            throw UnknownPropertyException();

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
                            return;
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
                            return;
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
                            return;
                        }
                        else if( aValue >>= bScroll )
                        {
                            pDescriptor->SetScrollingMode( bScroll ? ScrollingYes : ScrollingNo );
                            xFrame->SetFrameDescriptor( pDescriptor );
                            return;
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
                            return;
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
                            return;
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
                            return;
                        }
                    }
                    break;
            }

            throw IllegalArgumentException();
        }
    }

    SvxOle2Shape::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL SvxFrameShape::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    Any aAny;
    if( pObj && pModel )
    {
        if(pMap == NULL )
            throw UnknownPropertyException();

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

#endif
