/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <toolkit/helper/vclunohelper.hxx>
#include <svl/itemprop.hxx>
#include <svl/hint.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>

#include <targuno.hxx>
#include <miscuno.hxx>
#include <docuno.hxx>
#include <datauno.hxx>
#include <nameuno.hxx>
#include <docsh.hxx>
#include <content.hxx>
#include <drwlayer.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <unonames.hxx>

#include <algorithm>

using  namespace ::com::sun::star;

const TranslateId aTypeResIds[SC_LINKTARGETTYPE_COUNT] =
{
    SCSTR_CONTENT_TABLE,        // SC_LINKTARGETTYPE_SHEET
    SCSTR_CONTENT_RANGENAME,    // SC_LINKTARGETTYPE_RANGENAME
    SCSTR_CONTENT_DBAREA,       // SC_LINKTARGETTYPE_DBAREA
    SCSTR_CONTENT_OLEOBJECT     // SC_LINKTARGETTYPE_OLEOBJECT
};

static std::span<const SfxItemPropertyMapEntry> lcl_GetLinkTargetMap()
{
    static const SfxItemPropertyMapEntry aLinkTargetMap_Impl[] =
    {
        { SC_UNO_LINKDISPBIT,  0,  cppu::UnoType<awt::XBitmap>::get(),   beans::PropertyAttribute::READONLY, 0 },
        { SC_UNO_LINKDISPNAME, 0,  cppu::UnoType<OUString>::get(),                beans::PropertyAttribute::READONLY, 0 },
    };
    return aLinkTargetMap_Impl;
}

// service for ScLinkTargetTypeObj is not defined
//  must not support document::LinkTarget because the target type cannot be used as a target

SC_SIMPLE_SERVICE_INFO( ScLinkTargetTypesObj, u"ScLinkTargetTypesObj"_ustr, u"com.sun.star.document.LinkTargets"_ustr )
SC_SIMPLE_SERVICE_INFO( ScLinkTargetTypeObj,  u"ScLinkTargetTypeObj"_ustr,  u"com.sun.star.document.LinkTargetSupplier"_ustr )
SC_SIMPLE_SERVICE_INFO( ScLinkTargetsObj,     u"ScLinkTargetsObj"_ustr,     u"com.sun.star.document.LinkTargets"_ustr )
SC_SIMPLE_SERVICE_INFO( ScOleObjectsObj,      u"ScOleObjectsObj"_ustr,      u"com.sun.star.document.LinkTargets"_ustr )
SC_SIMPLE_SERVICE_INFO( ScOleObjectLinkTargetObj, u"ScOleObjectLinkTargetObj"_ustr, SCLINKTARGET_SERVICE )

ScLinkTargetTypesObj::ScLinkTargetTypesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument().AddUnoObject(*this);

    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        aNames[i] = ScResId(aTypeResIds[i]);
}

ScLinkTargetTypesObj::~ScLinkTargetTypesObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScLinkTargetTypesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
        pDocShell = nullptr;       // document gone
}

// container::XNameAccess

cpo::uno::Any SAL_CALL ScLinkTargetTypesObj::getByName(const OUString& aName)
{
    if (pDocShell)
    {
        for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
            if ( aNames[i] == aName )
                return cpo::uno::Any(uno::Reference< beans::XPropertySet >(new ScLinkTargetTypeObj( pDocShell, i )));
    }

    throw container::NoSuchElementException();
}

cpo::uno::Sequence<OUString> SAL_CALL ScLinkTargetTypesObj::getElementNames()
{
    cpo::uno::Sequence<OUString> aRet(SC_LINKTARGETTYPE_COUNT);
    OUString* pArray = aRet.getArray();
    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        pArray[i] = aNames[i];
    return aRet;
}

bool SAL_CALL ScLinkTargetTypesObj::hasByName(const OUString& aName)
{
    return std::find(std::begin(aNames), std::end(aNames), aName) != std::end(aNames);
}

// container::XElementAccess

cpo::uno::Type SAL_CALL ScLinkTargetTypesObj::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

bool SAL_CALL ScLinkTargetTypesObj::hasElements()
{
    return true;
}

ScLinkTargetTypeObj::ScLinkTargetTypeObj(ScDocShell* pDocSh, sal_uInt16 nT) :
    pDocShell( pDocSh ),
    nType( nT )
{
    pDocShell->GetDocument().AddUnoObject(*this);
    aName = ScResId(aTypeResIds[nType]);    //! on demand?
}

ScLinkTargetTypeObj::~ScLinkTargetTypeObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScLinkTargetTypeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
        pDocShell = nullptr;       // document gone
}

// document::XLinkTargetSupplier

uno::Reference< container::XNameAccess > SAL_CALL  ScLinkTargetTypeObj::getLinks()
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
            case SC_LINKTARGETTYPE_OLEOBJECT:
                xCollection.set(new ScOleObjectsObj(pDocShell));
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

uno::Reference< beans::XPropertySetInfo > SAL_CALL  ScLinkTargetTypeObj::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  aRef(new SfxItemPropertySetInfo( lcl_GetLinkTargetMap() ));
    return aRef;
}

void SAL_CALL ScLinkTargetTypeObj::setPropertyValue(const OUString& /* aPropertyName */,
            const cpo::uno::Any& /* aValue */)
{
    //  everything is read-only
    //! exception?
}

constexpr OUString aContentBmps[]=
{
    RID_BMP_CONTENT_TABLE,
    RID_BMP_CONTENT_RANGENAME,
    RID_BMP_CONTENT_DBAREA,
    RID_BMP_CONTENT_GRAPHIC,
    RID_BMP_CONTENT_OLEOBJECT,
    RID_BMP_CONTENT_NOTE,
    RID_BMP_CONTENT_AREALINK,
    RID_BMP_CONTENT_DRAWING
};

void ScLinkTargetTypeObj::SetLinkTargetBitmap( cpo::uno::Any& rRet, sal_uInt16 nType )
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
        case SC_LINKTARGETTYPE_OLEOBJECT:
            nImgId = ScContentId::OLEOBJECT;
            break;
    }
    if (nImgId != ScContentId::ROOT)
    {
        Bitmap aBitmap { aContentBmps[static_cast<int>(nImgId) -1 ] };
        rRet <<= VCLUnoHelper::CreateBitmap(aBitmap);
    }
}

cpo::uno::Any SAL_CALL ScLinkTargetTypeObj::getPropertyValue(const OUString& PropertyName)
{
    cpo::uno::Any aRet;
    if ( PropertyName == SC_UNO_LINKDISPBIT )
        SetLinkTargetBitmap( aRet, nType );
    else if ( PropertyName == SC_UNO_LINKDISPNAME )
        aRet <<= aName;

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScLinkTargetTypeObj )

ScLinkTargetsObj::ScLinkTargetsObj( uno::Reference< container::XNameAccess > xColl ) :
    xCollection(std::move( xColl ))
{
    OSL_ENSURE( xCollection.is(), "ScLinkTargetsObj: NULL" );
}

ScLinkTargetsObj::~ScLinkTargetsObj()
{
}

// container::XNameAccess

cpo::uno::Any SAL_CALL ScLinkTargetsObj::getByName(const OUString& aName)
{
    uno::Reference<beans::XPropertySet> xProp(xCollection->getByName(aName), uno::UNO_QUERY);
    if (xProp.is())
        return cpo::uno::Any(xProp);

    throw container::NoSuchElementException();
}

cpo::uno::Sequence<OUString> SAL_CALL ScLinkTargetsObj::getElementNames()
{
    return xCollection->getElementNames();
}

bool SAL_CALL ScLinkTargetsObj::hasByName(const OUString& aName)
{
    return xCollection->hasByName(aName);
}

// container::XElementAccess

cpo::uno::Type SAL_CALL ScLinkTargetsObj::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

bool SAL_CALL ScLinkTargetsObj::hasElements()
{
    return xCollection->hasElements();
}

// Collects the visible names of every OLE object (for example a chart) drawn on any
// sheet of the document, in the same "flat, skip groups" order the Navigator's OLE
// objects category uses.
static std::vector<OUString> lcl_GetOleObjectNames(ScDocShell* pDocShell)
{
    std::vector<OUString> aNames;
    if (!pDocShell)
        return aNames;

    ScDocument& rDoc = pDocShell->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    if (!pDrawLayer)
        return aNames;

    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        if (!pPage)
            continue;

        SdrObjListIter aIter(pPage, SdrIterMode::DeepNoGroups);
        for (SdrObject* pObject = aIter.Next(); pObject; pObject = aIter.Next())
        {
            if (pObject->GetObjIdentifier() == SdrObjKind::OLE2)
            {
                OUString aName = ScDrawLayer::GetVisibleName(pObject);
                if (!aName.isEmpty())
                    aNames.push_back(aName);
            }
        }
    }
    return aNames;
}

ScOleObjectsObj::ScOleObjectsObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScOleObjectsObj::~ScOleObjectsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScOleObjectsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
        pDocShell = nullptr;       // document gone
}

// container::XNameAccess

cpo::uno::Any SAL_CALL ScOleObjectsObj::getByName(const OUString& aName)
{
    if (hasByName(aName))
        return cpo::uno::Any(uno::Reference<beans::XPropertySet>(new ScOleObjectLinkTargetObj(aName)));

    throw container::NoSuchElementException();
}

cpo::uno::Sequence<OUString> SAL_CALL ScOleObjectsObj::getElementNames()
{
    const std::vector<OUString> aNames = lcl_GetOleObjectNames(pDocShell);
    return cpo::uno::Sequence<OUString>(aNames.data(), aNames.size());
}

bool SAL_CALL ScOleObjectsObj::hasByName(const OUString& aName)
{
    const std::vector<OUString> aNames = lcl_GetOleObjectNames(pDocShell);
    return std::find(aNames.begin(), aNames.end(), aName) != aNames.end();
}

// container::XElementAccess

cpo::uno::Type SAL_CALL ScOleObjectsObj::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

bool SAL_CALL ScOleObjectsObj::hasElements()
{
    return !lcl_GetOleObjectNames(pDocShell).empty();
}

ScOleObjectLinkTargetObj::ScOleObjectLinkTargetObj( OUString aObjectName ) :
    aName(std::move(aObjectName))
{
}

ScOleObjectLinkTargetObj::~ScOleObjectLinkTargetObj()
{
}

// beans::XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL ScOleObjectLinkTargetObj::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  aRef(new SfxItemPropertySetInfo( lcl_GetLinkTargetMap() ));
    return aRef;
}

void SAL_CALL ScOleObjectLinkTargetObj::setPropertyValue(const OUString&,
            const cpo::uno::Any&)
{
    throw beans::PropertyVetoException(u"LinkDisplayName and LinkDisplayBitmap are read-only."_ustr);
}

cpo::uno::Any SAL_CALL ScOleObjectLinkTargetObj::getPropertyValue(const OUString& PropertyName)
{
    cpo::uno::Any aRet;
    if ( PropertyName == SC_UNO_LINKDISPBIT )
        ScLinkTargetTypeObj::SetLinkTargetBitmap( aRet, SC_LINKTARGETTYPE_OLEOBJECT );
    else if ( PropertyName == SC_UNO_LINKDISPNAME )
        aRet <<= aName;

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScOleObjectLinkTargetObj )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
