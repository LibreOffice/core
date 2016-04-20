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

#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <toolkit/helper/vclunohelper.hxx>
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

sal_uInt16 nTypeResIds[SC_LINKTARGETTYPE_COUNT] =
{
    SCSTR_CONTENT_TABLE,        // SC_LINKTARGETTYPE_SHEET
    SCSTR_CONTENT_RANGENAME,    // SC_LINKTARGETTYPE_RANGENAME
    SCSTR_CONTENT_DBAREA        // SC_LINKTARGETTYPE_DBAREA
};

static const SfxItemPropertyMapEntry* lcl_GetLinkTargetMap()
{
    static const SfxItemPropertyMapEntry aLinkTargetMap_Impl[] =
    {
        {OUString(SC_UNO_LINKDISPBIT),  0,  cppu::UnoType<awt::XBitmap>::get(),   beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNO_LINKDISPNAME), 0,  cppu::UnoType<OUString>::get(),                beans::PropertyAttribute::READONLY, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aLinkTargetMap_Impl;
}

// service for ScLinkTargetTypeObj is not defined
//  must not support document::LinkTarget because the target type cannot be used as a target

SC_SIMPLE_SERVICE_INFO( ScLinkTargetTypesObj, "ScLinkTargetTypesObj", "com.sun.star.document.LinkTargets" )
SC_SIMPLE_SERVICE_INFO( ScLinkTargetTypeObj,  "ScLinkTargetTypeObj",  "com.sun.star.document.LinkTargetSupplier" )
SC_SIMPLE_SERVICE_INFO( ScLinkTargetsObj,     "ScLinkTargetsObj",     "com.sun.star.document.LinkTargets" )

ScLinkTargetTypesObj::ScLinkTargetTypesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument().AddUnoObject(*this);

    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        aNames[i] = ScResId( nTypeResIds[i] );
}

ScLinkTargetTypesObj::~ScLinkTargetTypesObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScLinkTargetTypesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
        pDocShell = nullptr;       // document gone
}

// container::XNameAccess

uno::Any SAL_CALL ScLinkTargetTypesObj::getByName(const OUString& aName)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    if (pDocShell)
    {
        for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
            if ( aNames[i] == aName )
                return uno::makeAny(uno::Reference< beans::XPropertySet >(new ScLinkTargetTypeObj( pDocShell, i )));
    }

    throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScLinkTargetTypesObj::getElementNames() throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aRet(SC_LINKTARGETTYPE_COUNT);
    OUString* pArray = aRet.getArray();
    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        pArray[i] = aNames[i];
    return aRet;
}

sal_Bool SAL_CALL ScLinkTargetTypesObj::hasByName(const OUString& aName) throw( uno::RuntimeException, std::exception )
{
    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        if ( aNames[i] == aName )
            return true;
    return false;
}

// container::XElementAccess

uno::Type SAL_CALL ScLinkTargetTypesObj::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SAL_CALL ScLinkTargetTypesObj::hasElements() throw( uno::RuntimeException, std::exception )
{
    return true;
}

ScLinkTargetTypeObj::ScLinkTargetTypeObj(ScDocShell* pDocSh, sal_uInt16 nT) :
    pDocShell( pDocSh ),
    nType( nT )
{
    pDocShell->GetDocument().AddUnoObject(*this);
    aName = ScResId( nTypeResIds[nType] );    //! on demand?
}

ScLinkTargetTypeObj::~ScLinkTargetTypeObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScLinkTargetTypeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
        pDocShell = nullptr;       // document gone
}

// document::XLinkTargetSupplier

uno::Reference< container::XNameAccess > SAL_CALL  ScLinkTargetTypeObj::getLinks() throw( uno::RuntimeException, std::exception )
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
    return nullptr;
}

// beans::XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL  ScLinkTargetTypeObj::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo >  aRef(new SfxItemPropertySetInfo( lcl_GetLinkTargetMap() ));
    return aRef;
}

void SAL_CALL ScLinkTargetTypeObj::setPropertyValue(const OUString& /* aPropertyName */,
            const uno::Any& /* aValue */)
        throw(  beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                 uno::RuntimeException, std::exception )
{
    //  everything is read-only
    //! exception?
}

void ScLinkTargetTypeObj::SetLinkTargetBitmap( uno::Any& rRet, sal_uInt16 nType )
{
    ScContentId nImgId = ScContentId::ROOT;
    switch ( nType )
    {
        case SC_LINKTARGETTYPE_SHEET:
            nImgId = ScContentId::TABLE;
            break;
        case SC_LINKTARGETTYPE_RANGENAME:
            nImgId = ScContentId::RANGENAME;
            break;
        case SC_LINKTARGETTYPE_DBAREA:
            nImgId = ScContentId::DBAREA;
            break;
    }
    if (nImgId != ScContentId::ROOT)
    {
        ImageList aEntryImages( ScResId( RID_IMAGELIST_NAVCONT ) );
        const Image& rImage = aEntryImages.GetImage( (sal_uInt16)nImgId );
        rRet <<= uno::Reference< awt::XBitmap > (VCLUnoHelper::CreateBitmap( rImage.GetBitmapEx() ));
    }
}

uno::Any SAL_CALL ScLinkTargetTypeObj::getPropertyValue(const OUString& PropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    uno::Any aRet;
    OUString aNameStr(PropertyName);
    if ( aNameStr == SC_UNO_LINKDISPBIT )
        SetLinkTargetBitmap( aRet, nType );
    else if ( aNameStr == SC_UNO_LINKDISPNAME )
        aRet <<= OUString( aName );

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScLinkTargetTypeObj )

ScLinkTargetsObj::ScLinkTargetsObj( const uno::Reference< container::XNameAccess > & rColl ) :
    xCollection( rColl )
{
    OSL_ENSURE( xCollection.is(), "ScLinkTargetsObj: NULL" );
}

ScLinkTargetsObj::~ScLinkTargetsObj()
{
}

// container::XNameAccess

uno::Any SAL_CALL ScLinkTargetsObj::getByName(const OUString& aName)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    uno::Reference< beans::XPropertySet >  xProp( ScUnoHelpFunctions::AnyToInterface( xCollection->getByName(aName) ), uno::UNO_QUERY );
    if (xProp.is())
        return uno::makeAny(xProp);

    throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<OUString> SAL_CALL ScLinkTargetsObj::getElementNames() throw( uno::RuntimeException, std::exception )
{
    return xCollection->getElementNames();
}

sal_Bool SAL_CALL ScLinkTargetsObj::hasByName(const OUString& aName) throw( uno::RuntimeException, std::exception )
{
    return xCollection->hasByName(aName);
}

// container::XElementAccess

uno::Type SAL_CALL ScLinkTargetsObj::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SAL_CALL ScLinkTargetsObj::hasElements() throw( uno::RuntimeException, std::exception )
{
    return xCollection->hasElements();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
