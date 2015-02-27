/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "condformatuno.hxx"

#include "document.hxx"
#include "conditio.hxx"
#include "colorscale.hxx"

#include <vcl/svapp.hxx>
#include <rt/ustring.hxx>

namespace {

const SfxItemPropertyMapEntry* getCondFormatPropset()
{
    static const SfxItemPropertyMapEntry aCondFormatPropertyMap_Impl[] =
    {
        {OUString("ID"), 0, cppu::UnoType<sal_Int32>::get(), 0, 0},
        {OUString("Range"), 0, cppu::UnoType<sheet::XSheetCellRanges>::get(), 0, 0},
        {OUString(), 0, css::uno::Type(), 0, 0}
    };
    return aCondFormatPropertyMap_Impl;
}

}

ScCondFormatsObj::ScCondFormatsObj(ScDocument* pDoc, SCTAB nTab):
    mpFormatList(pDoc->GetCondFormList(nTab))
{
}

sal_Int32 ScCondFormatsObj::addByRange(const uno::Reference< sheet::XConditionalFormat >& xCondFormat,
        const uno::Reference< sheet::XSheetCellRanges >& xRanges)
    throw(uno::RuntimeException, std::exception)
{

    return 0;
}

void ScCondFormatsObj::removeByID(const sal_Int32 nID)
    throw(uno::RuntimeException, std::exception)
{
}

uno::Sequence<uno::Reference<sheet::XConditionalFormat> > ScCondFormatsObj::getConditionalFormats()
    throw(uno::RuntimeException, std::exception)
{
    return uno::Sequence<uno::Reference<sheet::XConditionalFormat> >();
}

sal_Int32 ScCondFormatsObj::getLength()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return mpFormatList->size();;
}

ScCondFormatObj::ScCondFormatObj(ScDocument* pDoc, ScConditionalFormat* pFormat):
    mpFormat(pFormat),
    maPropSet(getCondFormatPropset())
{
}

void ScCondFormatObj::addEntry(const uno::Reference<sheet::XConditionEntry>& xEntry)
    throw(uno::RuntimeException, std::exception)
{
}

void ScCondFormatObj::removeByIndex(const sal_Int32 nIndex)
    throw(uno::RuntimeException, std::exception)
{
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCondFormatObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap()));
    return aRef;
}

void SAL_CALL ScCondFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScCondFormatObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    // GetOnePropertyValue( pEntry, aAny );
    return aAny;
}

void SAL_CALL ScCondFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScConditionEntryObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScConditionEntryObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScConditionEntryObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    // GetOnePropertyValue( pEntry, aAny );
    return aAny;
}

void SAL_CALL ScConditionEntryObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScColorScaleFormatObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScColorScaleFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScColorScaleFormatObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    // GetOnePropertyValue( pEntry, aAny );
    return aAny;
}

void SAL_CALL ScColorScaleFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDataBarFormatObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScDataBarFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScDataBarFormatObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    // GetOnePropertyValue( pEntry, aAny );
    return aAny;
}

void SAL_CALL ScDataBarFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScIconSetFormatObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScIconSetFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScIconSetFormatObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    // GetOnePropertyValue( pEntry, aAny );
    return aAny;
}

void SAL_CALL ScIconSetFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
