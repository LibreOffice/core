/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoshap4.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-13 09:07:26 $
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

#include <vcl/virdev.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdomedia.hxx>

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

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>

#include <sot/storage.hxx>
#include <sot/exchange.hxx>
#include <svtools/FilterConfigItem.hxx>

#include <svx/svdmodel.hxx>
#include "shapeimpl.hxx"

#include <svx/unoshprp.hxx>

#include "unoapi.hxx"
#include "svx/svdpagv.hxx"
#include "svx/svdview.hxx"
#include "svdglob.hxx"
#include "svdstr.hrc"

///////////////////////////////////////////////////////////////////////

extern sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pFilterConfigItem = NULL, sal_Bool bPlaceable = sal_True );

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
bool SvxOle2Shape::setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    switch( pProperty->nWID )
    {
/*
    case OWN_ATTR_CLSID:
    {
        OUString aCLSID;
        if( rValue >>= aCLSID )
        {
            // init an ole object with a global name
            SdrOle2Obj* pOle2 = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
            if( pOle2 )
            {
                uno::Reference < embed::XEmbeddedObject > xObj = pOle2->GetObjRef();
                if ( !xObj.is() )
                {
                    SvGlobalName aClassName;
                    if( aClassName.MakeId( aCLSID ) )
                    {
                        SfxObjectShell* pPersist = mpModel->GetPersist();
                        ::rtl::OUString aPersistName;
                        Any aAny( getPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ) ) );
                        aAny >>= aPersistName;

                        //TODO/LATER: how to cope with creation failure?!
                        xObj = pPersist->GetEmbeddedObjectContainer().CreateEmbeddedObject( aClassName.GetByteSequence(), aPersistName );
                        if( xObj.is() )
                        {
                            aAny <<= aPersistName;
                            setPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ), aAny );
                            pOle2->SetObjRef( xObj );

                            Rectangle aRect = pOle2->GetLogicRect();
                            awt::Size aSz;
                            Size aSize( pOle2->GetLogicRect().GetSize() );
                            aSz.Width = aSize.Width();
                            aSz.Height = aSize.Height();
                            xObj->setVisualAreaSize( pOle2->GetAspect(), aSz );
                        }
                    }
                }
            }
            return true;
        }
        break;
    }
*/
    case OWN_ATTR_OLE_VISAREA:
    {
        // TODO/LATER: seems to make no sence for iconified object

        awt::Rectangle aVisArea;
        if( (rValue >>= aVisArea) && mpObj->ISA(SdrOle2Obj))
        {
            Size aTmp( aVisArea.X + aVisArea.Width, aVisArea.Y + aVisArea.Height );
            uno::Reference < embed::XEmbeddedObject > xObj = ((SdrOle2Obj*)mpObj.get())->GetObjRef();
            if( xObj.is() )
            {
                try
                {
                    MapUnit aMapUnit( MAP_100TH_MM ); // the API handles with MAP_100TH_MM map mode
                    MapUnit aObjUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( embed::Aspects::MSOLE_CONTENT ) );
                    aTmp = OutputDevice::LogicToLogic( aTmp, aMapUnit, aObjUnit );
                    xObj->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, awt::Size( aTmp.Width(), aTmp.Height() ) );
                }
                catch( uno::Exception& )
                {
                    OSL_ENSURE( sal_False, "Couldn't set the visual area for the object!\n" );
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
            static_cast<SdrOle2Obj*>(mpObj.get())->SetAspect( nAspect );
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
        OUString aURL;
        if( rValue >>= aURL )
        {
            GraphicObject aGrafObj( CreateGraphicObjectFromURL( aURL ) );
            static_cast<SdrOle2Obj*>(mpObj.get())->SetGraphic( &aGrafObj.GetGraphic() );
            return true;
        }
        break;
    }
    case OWN_ATTR_VALUE_GRAPHIC:
    {
        uno::Reference< graphic::XGraphic > xGraphic( rValue, uno::UNO_QUERY );
        if( xGraphic.is() )
        {
            SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
            if( pOle )
            {
                GraphicObject aGrafObj( xGraphic );
                const Graphic aGraphic( aGrafObj.GetGraphic() );
                pOle->SetGraphicToObj( aGraphic, rtl::OUString() );
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
            static_cast<SdrOle2Obj*>(mpObj.get())->SetPersistName( aPersistName );
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
        return SvxShape::setPropertyValueImpl( pProperty, rValue );
    }

    throw IllegalArgumentException();
}

bool SvxOle2Shape::getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_CLSID:
    {
        OUString aCLSID;
        SvGlobalName aClassName = GetClassName_Impl(aCLSID);
        rValue <<= aCLSID;
        break;
    }

    case OWN_ATTR_INTERNAL_OLE:
    {
        rtl::OUString sCLSID;
        rValue <<= SotExchange::IsInternal( GetClassName_Impl(sCLSID) );
        break;
    }

    case OWN_ATTR_METAFILE:
    {
        SdrOle2Obj* pObj = dynamic_cast<SdrOle2Obj*>(mpObj.get());
        if( pObj )
        {
            Graphic* pGraphic = pObj->GetGraphic();
            if( pGraphic )
            {
                BOOL bIsWMF = FALSE;
                if ( pGraphic->IsLink() )
                {
                    GfxLink aLnk = pGraphic->GetLink();
                    if ( aLnk.GetType() == GFX_LINK_TYPE_NATIVE_WMF )
                    {
                        bIsWMF = TRUE;
                        uno::Sequence<sal_Int8> aSeq((sal_Int8*)aLnk.GetData(), (sal_Int32) aLnk.GetDataSize());
                        rValue <<= aSeq;
                    }
                }
                if ( !bIsWMF )
                {
                    GDIMetaFile aMtf;
                    if ( pGraphic->GetType() != GRAPHIC_BITMAP )
                        aMtf = pObj->GetGraphic()->GetGDIMetaFile();
                    else
                    {
                        VirtualDevice aVirDev;
                        aMtf.Record( &aVirDev );
                        pGraphic->Draw( &aVirDev, Point(),  pGraphic->GetPrefSize() );
                        aMtf.Stop();
                        aMtf.SetPrefSize( pGraphic->GetPrefSize() );
                        aMtf.SetPrefMapMode( pGraphic->GetPrefMapMode() );
                    }
                    SvMemoryStream aDestStrm( 65535, 65535 );
                    ConvertGDIMetaFileToWMF( aMtf, aDestStrm, NULL, sal_False );
                    uno::Sequence<sal_Int8> aSeq((sal_Int8*)aDestStrm.GetData(), aDestStrm.GetSize());
                    rValue <<= aSeq;
                }
            }
        }
        else
        {
            rValue = GetBitmap( sal_True );
        }
        break;
    }

    case OWN_ATTR_OLE_VISAREA:
    {
        awt::Rectangle aVisArea;
        if( mpObj->ISA(SdrOle2Obj))
        {
            MapMode aMapMode( MAP_100TH_MM ); // the API uses this map mode
            Size aTmp = ((SdrOle2Obj*)mpObj.get())->GetOrigObjSize( &aMapMode ); // get the size in the requested map mode
            aVisArea = awt::Rectangle( 0, 0, aTmp.Width(), aTmp.Height() );
        }

        rValue <<= aVisArea;
        break;
    }

    case OWN_ATTR_OLESIZE:
    {
        Size aTmp( static_cast<SdrOle2Obj*>(mpObj.get())->GetOrigObjSize() );
        rValue <<= awt::Size( aTmp.Width(), aTmp.Height() );
        break;
    }

    case OWN_ATTR_OLE_ASPECT:
    {
        rValue <<= static_cast<SdrOle2Obj*>(mpObj.get())->GetAspect();
        break;
    }

    case OWN_ATTR_OLEMODEL:
    case OWN_ATTR_OLE_EMBEDDED_OBJECT:
    {
        SdrOle2Obj* pObj = dynamic_cast<SdrOle2Obj*>( mpObj.get() );
        if( pObj )
        {
            uno::Reference < embed::XEmbeddedObject > xObj( pObj->GetObjRef() );
            if ( xObj.is() && ( pProperty->nWID == OWN_ATTR_OLE_EMBEDDED_OBJECT || svt::EmbeddedObjectRef::TryRunningState( xObj ) ) )
            {
                const SdrPageView* pPageView = mpModel->GetPaintingPageView();
                sal_Bool bSuccess = sal_False;

                if ( pPageView )
                {
                    SdrView* pView = (SdrView*)&(pPageView->GetView());
                    if ( pView->ISA( SdrPaintView ) )
                    {
                        SdrPaintView* pPaintView = (SdrPaintView*)pView;
                        pPaintView->DoConnect( pObj );
                        bSuccess = sal_True;
                    }
                }

                if ( !bSuccess )
                    bSuccess = pObj->AddOwnLightClient();

                OSL_ENSURE( bSuccess, "An object without client is provided!" );

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
        Graphic* pGraphic = static_cast<SdrOle2Obj*>( mpObj.get() )->GetGraphic();
        if( pGraphic )
            xGraphic = pGraphic->GetXGraphic();
        rValue <<= xGraphic;
        break;
    }

    case OWN_ATTR_THUMBNAIL:
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
        rValue <<= aURL;
        break;
    }
    case OWN_ATTR_PERSISTNAME:
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

        rValue <<= aPersistName;
        break;
    }
    case OWN_ATTR_OLE_LINKURL:
    {
        OUString    aLinkURL;
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );

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
        return SvxShape::getPropertyValueImpl( pProperty, rValue );
    }

    return true;
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
    if( getPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ) ) >>= aTmpStr )
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
        setPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ), Any( aTmpStr = aPersistName ) );

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

void SvxOle2Shape::resetModifiedState()
{
    ::comphelper::IEmbeddedHelper* pPersist = mpModel ? mpModel->GetPersist() : 0;
    if( pPersist && !pPersist->isEnableSetModified() )
    {
        SdrOle2Obj* pOle = dynamic_cast< SdrOle2Obj* >( mpObj.get() );
        if( pOle && !pOle->IsEmpty() )
        {
            uno::Reference < util::XModifiable > xMod( pOle->GetObjRef(), uno::UNO_QUERY );
            if( xMod.is() )
                // TODO/MBA: what's this?!
                xMod->setModified( sal_False );
        }
    }
}

const SvGlobalName SvxOle2Shape::GetClassName_Impl(rtl::OUString& rHexCLSID)
{
    DBG_TESTSOLARMUTEX();
    SvGlobalName aClassName;
    SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( mpObj.get() );

    if( pOle2Obj )
    {
        rHexCLSID = rtl::OUString();

        if( pOle2Obj->IsEmpty() )
        {
            ::comphelper::IEmbeddedHelper* pPersist = mpModel->GetPersist();
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

        if (!rHexCLSID.getLength())
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

void SAL_CALL SvxAppletShape::setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    SvxShape::setPropertyValue( aPropertyName, rValue );
    resetModifiedState();
}

void SAL_CALL SvxAppletShape::setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    SvxShape::setPropertyValues( aPropertyNames, rValues );
    resetModifiedState();
}

bool SvxAppletShape::setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( (pProperty->nWID >= OWN_ATTR_APPLET_DOCBASE) && (pProperty->nWID <= OWN_ATTR_APPLET_ISSCRIPT) )
    {
        if ( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                // allow exceptions to pass through
                xSet->setPropertyValue( OUString::createFromAscii( pProperty->pName ), rValue );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::setPropertyValueImpl( pProperty, rValue );
    }
}

bool SvxAppletShape::getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( (pProperty->nWID >= OWN_ATTR_APPLET_DOCBASE) && (pProperty->nWID <= OWN_ATTR_APPLET_ISSCRIPT) )
    {
        if ( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                rValue = xSet->getPropertyValue( OUString::createFromAscii( pProperty->pName ) );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::getPropertyValueImpl( pProperty, rValue );
    }
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

void SAL_CALL SvxPluginShape::setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    SvxShape::setPropertyValue( aPropertyName, rValue );
    resetModifiedState();
}

void SAL_CALL SvxPluginShape::setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    SvxOle2Shape::setPropertyValues( aPropertyNames, rValues );
    resetModifiedState();
}

bool SvxPluginShape::setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( (pProperty->nWID >= OWN_ATTR_PLUGIN_MIMETYPE) && (pProperty->nWID <= OWN_ATTR_PLUGIN_COMMANDS) )
    {
        if( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                // allow exceptions to pass through
                xSet->setPropertyValue( OUString::createFromAscii( pProperty->pName ), rValue );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::setPropertyValueImpl( pProperty, rValue );
    }
}

bool SvxPluginShape::getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( (pProperty->nWID >= OWN_ATTR_PLUGIN_MIMETYPE) && (pProperty->nWID <= OWN_ATTR_PLUGIN_COMMANDS) )
    {
        if( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                rValue <<= xSet->getPropertyValue( OUString::createFromAscii( pProperty->pName ) );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::getPropertyValueImpl( pProperty, rValue );
    }
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

void SAL_CALL SvxFrameShape::setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    SvxShape::setPropertyValue( aPropertyName, rValue );
    resetModifiedState();
}

void SAL_CALL SvxFrameShape::setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    SvxOle2Shape::setPropertyValues( aPropertyNames, rValues );
    resetModifiedState();
}

bool SvxFrameShape::setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( (pProperty->nWID >= OWN_ATTR_FRAME_URL) && (pProperty->nWID <= OWN_ATTR_FRAME_MARGIN_HEIGHT) )
    {
        if( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                // allow exceptions to pass through
                xSet->setPropertyValue( OUString::createFromAscii( pProperty->pName ), rValue );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::setPropertyValueImpl( pProperty, rValue );
    }
}

bool SvxFrameShape::getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( (pProperty->nWID >= OWN_ATTR_FRAME_URL) && (pProperty->nWID <= OWN_ATTR_FRAME_MARGIN_HEIGHT) )
    {
        if( svt::EmbeddedObjectRef::TryRunningState( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef() ) )
        {
            uno::Reference < beans::XPropertySet > xSet( static_cast<SdrOle2Obj*>(mpObj.get())->GetObjRef()->getComponent(), uno::UNO_QUERY );
            if( xSet.is() )
            {
                rValue <<= xSet->getPropertyValue( OUString::createFromAscii( pProperty->pName ) );
            }
        }
        return true;
    }
    else
    {
        return SvxOle2Shape::getPropertyValueImpl( pProperty, rValue );
    }
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

bool SvxMediaShape::setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( (pProperty->nWID >= OWN_ATTR_MEDIA_URL) && (pProperty->nWID <= OWN_ATTR_MEDIA_ZOOM) )
    {
        SdrMediaObj* pMedia = static_cast< SdrMediaObj* >( mpObj.get() );
        ::avmedia::MediaItem aItem;
        bool bOk = false;

        switch( pProperty->nWID )
        {
        case OWN_ATTR_MEDIA_URL:
        {
            OUString aURL;
            if( rValue >>= aURL )
            {
                bOk = true;
                aItem.setURL( aURL );
            }
        }
        break;

        case( OWN_ATTR_MEDIA_LOOP ):
        {
            sal_Bool bLoop = sal_Bool();

            if( rValue >>= bLoop )
            {
                bOk = true;
                aItem.setLoop( bLoop );
            }
        }
        break;

        case( OWN_ATTR_MEDIA_MUTE ):
        {
            sal_Bool bMute = sal_Bool();

            if( rValue >>= bMute )
            {
                bOk = true;
                aItem.setMute( bMute );
            }
        }
        break;

        case( OWN_ATTR_MEDIA_VOLUMEDB ):
        {
            sal_Int16 nVolumeDB = sal_Int16();

            if( rValue >>= nVolumeDB )
            {
                bOk = true;
                aItem.setVolumeDB( nVolumeDB );
            }
        }
        break;

        case( OWN_ATTR_MEDIA_ZOOM ):
        {
            ::com::sun::star::media::ZoomLevel eLevel;

            if( rValue >>= eLevel )
            {
                bOk = true;
                aItem.setZoom( eLevel );
            }
        }
        break;

        default:
            DBG_ERROR("SvxMediaShape::setPropertyValueImpl(), unknown argument!");
        }

        if( bOk )
        {
            pMedia->setMediaProperties( aItem );
            return true;
        }
    }
    else
    {
        return SvxShape::setPropertyValueImpl( pProperty, rValue );
    }

    throw IllegalArgumentException();
}

//----------------------------------------------------------------------

bool SvxMediaShape::getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( ( pProperty->nWID >= OWN_ATTR_MEDIA_URL ) && ( pProperty->nWID <= OWN_ATTR_MEDIA_ZOOM ) )
    {
        SdrMediaObj* pMedia = static_cast< SdrMediaObj* >( mpObj.get() );
        const ::avmedia::MediaItem aItem( pMedia->getMediaProperties() );

        switch( pProperty->nWID )
        {
            case OWN_ATTR_MEDIA_URL:
                rValue <<= aItem.getURL();
                break;

            case( OWN_ATTR_MEDIA_LOOP ):
                rValue <<= (sal_Bool) aItem.isLoop();
                break;

            case( OWN_ATTR_MEDIA_MUTE ):
                rValue <<= (sal_Bool) aItem.isMute();
                break;

            case( OWN_ATTR_MEDIA_VOLUMEDB ):
                rValue <<= (sal_Int16) aItem.getVolumeDB();
                break;

            case( OWN_ATTR_MEDIA_ZOOM ):
                rValue <<= aItem.getZoom();
                break;

            default:
                DBG_ERROR("SvxMediaShape::getPropertyValueImpl(), unknown property!");
        }
        return true;
    }
    else
    {
        return SvxShape::getPropertyValueImpl( pProperty, rValue );
    }
}
