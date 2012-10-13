/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <toolkit/unohlp.hxx>
#include <svl/itemprop.hxx>
#include <svl/smplhint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <com/sun/star/awt/XBitmap.hpp>

#include "targuno.hxx"
#include "miscuno.hxx"
#include "docuno.hxx"
#include "datauno.hxx"
#include "nameuno.hxx"
#include "docsh.hxx"
#include "content.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "unonames.hxx"

using  namespace ::com::sun::star;

//------------------------------------------------------------------------

sal_uInt16 nTypeResIds[SC_LINKTARGETTYPE_COUNT] =
{
    SCSTR_CONTENT_TABLE,        // SC_LINKTARGETTYPE_SHEET
    SCSTR_CONTENT_RANGENAME,    // SC_LINKTARGETTYPE_RANGENAME
    SCSTR_CONTENT_DBAREA        // SC_LINKTARGETTYPE_DBAREA
};

static const SfxItemPropertyMapEntry* lcl_GetLinkTargetMap()
{
    static SfxItemPropertyMapEntry aLinkTargetMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_LINKDISPBIT),  0,  &getCppuType((const uno::Reference<awt::XBitmap>*)0),   beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPNAME), 0,  &getCppuType((const ::rtl::OUString*)0),                beans::PropertyAttribute::READONLY, 0 },
        {0,0,0,0,0,0}
    };
    return aLinkTargetMap_Impl;
}

//------------------------------------------------------------------------

// service for ScLinkTargetTypeObj is not defined
//  must not support document::LinkTarget because the target type cannot be used as a target

SC_SIMPLE_SERVICE_INFO( ScLinkTargetTypesObj, "ScLinkTargetTypesObj", "com.sun.star.document.LinkTargets" )
SC_SIMPLE_SERVICE_INFO( ScLinkTargetTypeObj,  "ScLinkTargetTypeObj",  "com.sun.star.document.LinkTargetSupplier" )
SC_SIMPLE_SERVICE_INFO( ScLinkTargetsObj,     "ScLinkTargetsObj",     "com.sun.star.document.LinkTargets" )

//------------------------------------------------------------------------

ScLinkTargetTypesObj::ScLinkTargetTypesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        aNames[i] = String( ScResId( nTypeResIds[i] ) );
}

ScLinkTargetTypesObj::~ScLinkTargetTypesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScLinkTargetTypesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pDocShell = NULL;       // document gone
}

// container::XNameAccess

uno::Any SAL_CALL ScLinkTargetTypesObj::getByName(const rtl::OUString& aName)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    if (pDocShell)
    {
        String aNameStr(aName);
        for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
            if ( aNames[i] == aNameStr )
                return uno::makeAny(uno::Reference< beans::XPropertySet >(new ScLinkTargetTypeObj( pDocShell, i )));
    }

    throw container::NoSuchElementException();
}

uno::Sequence<rtl::OUString> SAL_CALL ScLinkTargetTypesObj::getElementNames(void) throw( uno::RuntimeException )
{
    uno::Sequence<rtl::OUString> aRet(SC_LINKTARGETTYPE_COUNT);
    rtl::OUString* pArray = aRet.getArray();
    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        pArray[i] = aNames[i];
    return aRet;
}

sal_Bool SAL_CALL ScLinkTargetTypesObj::hasByName(const rtl::OUString& aName) throw( uno::RuntimeException )
{
    String aNameStr = aName;
    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        if ( aNames[i] == aNameStr )
            return sal_True;
    return false;
}

// container::XElementAccess

uno::Type SAL_CALL ScLinkTargetTypesObj::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
}

sal_Bool SAL_CALL ScLinkTargetTypesObj::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

//------------------------------------------------------------------------

ScLinkTargetTypeObj::ScLinkTargetTypeObj(ScDocShell* pDocSh, sal_uInt16 nT) :
    pDocShell( pDocSh ),
    nType( nT )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
    aName = String( ScResId( nTypeResIds[nType] ) );    //! on demand?
}

ScLinkTargetTypeObj::~ScLinkTargetTypeObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScLinkTargetTypeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pDocShell = NULL;       // document gone
}

// document::XLinkTargetSupplier

uno::Reference< container::XNameAccess > SAL_CALL  ScLinkTargetTypeObj::getLinks(void) throw( uno::RuntimeException )
{
    uno::Reference< container::XNameAccess >  xCollection;

    if ( pDocShell )
    {
        switch ( nType )
        {
            case SC_LINKTARGETTYPE_SHEET:
                xCollection.set(new ScTableSheetsObj(pDocShell));
                break;
            case SC_LINKTARGETTYPE_RANGENAME:
                xCollection.set(new ScGlobalNamedRangesObj(pDocShell));
                break;
            case SC_LINKTARGETTYPE_DBAREA:
                xCollection.set(new ScDatabaseRangesObj(pDocShell));
                break;
            default:
                OSL_FAIL("invalid type");
        }
    }

    //  wrap collection in ScLinkTargetsObj because service document::LinkTargets requires
    //  beans::XPropertySet as ElementType in container::XNameAccess.
    if ( xCollection.is() )
        return new ScLinkTargetsObj( xCollection );
    return NULL;
}

// beans::XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL  ScLinkTargetTypeObj::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo >  aRef(new SfxItemPropertySetInfo( lcl_GetLinkTargetMap() ));
    return aRef;
}

void SAL_CALL ScLinkTargetTypeObj::setPropertyValue(const rtl::OUString& /* aPropertyName */,
            const uno::Any& /* aValue */)
        throw(  beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                 uno::RuntimeException )
{
    //  everything is read-only
    //! exception?
}

void ScLinkTargetTypeObj::SetLinkTargetBitmap( uno::Any& rRet, sal_uInt16 nType )
{
    sal_uInt16 nImgId = 0;
    switch ( nType )
    {
        case SC_LINKTARGETTYPE_SHEET:
            nImgId = SC_CONTENT_TABLE;
            break;
        case SC_LINKTARGETTYPE_RANGENAME:
            nImgId = SC_CONTENT_RANGENAME;
            break;
        case SC_LINKTARGETTYPE_DBAREA:
            nImgId = SC_CONTENT_DBAREA;
            break;
    }
    if (nImgId)
    {
        ImageList aEntryImages( ScResId( RID_IMAGELIST_NAVCONT ) );
        const Image& rImage = aEntryImages.GetImage( nImgId );
        rRet <<= uno::Reference< awt::XBitmap > (VCLUnoHelper::CreateBitmap( rImage.GetBitmapEx() ));
    }
}

uno::Any SAL_CALL ScLinkTargetTypeObj::getPropertyValue(const rtl::OUString& PropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    String aNameStr(PropertyName);
    if ( aNameStr.EqualsAscii( SC_UNO_LINKDISPBIT ) )
        SetLinkTargetBitmap( aRet, nType );
    else if ( aNameStr.EqualsAscii( SC_UNO_LINKDISPNAME ) )
        aRet <<= rtl::OUString( aName );

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScLinkTargetTypeObj )

//------------------------------------------------------------------------

ScLinkTargetsObj::ScLinkTargetsObj( const uno::Reference< container::XNameAccess > & rColl ) :
    xCollection( rColl )
{
    OSL_ENSURE( xCollection.is(), "ScLinkTargetsObj: NULL" );
}

ScLinkTargetsObj::~ScLinkTargetsObj()
{
}

// container::XNameAccess

uno::Any SAL_CALL ScLinkTargetsObj::getByName(const rtl::OUString& aName)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet >  xProp( ScUnoHelpFunctions::AnyToInterface( xCollection->getByName(aName) ), uno::UNO_QUERY );
    if (xProp.is())
        return uno::makeAny(xProp);

    throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScLinkTargetsObj::getElementNames(void) throw( uno::RuntimeException )
{
    return xCollection->getElementNames();
}

sal_Bool SAL_CALL ScLinkTargetsObj::hasByName(const rtl::OUString& aName) throw( uno::RuntimeException )
{
    return xCollection->hasByName(aName);
}

// container::XElementAccess

uno::Type SAL_CALL ScLinkTargetsObj::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
}

sal_Bool SAL_CALL ScLinkTargetsObj::hasElements(void) throw( uno::RuntimeException )
{
    return xCollection->hasElements();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
