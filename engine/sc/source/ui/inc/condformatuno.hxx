/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <types.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XConditionalFormats.hpp>
#include <com/sun/star/sheet/XConditionalFormat.hpp>
#include <com/sun/star/sheet/XConditionEntry.hpp>
#include <com/sun/star/sheet/XColorScaleEntry.hpp>
#include <com/sun/star/sheet/XDataBarEntry.hpp>
#include <com/sun/star/sheet/XIconSetEntry.hpp>

#include <cppuhelper/implbase.hxx>
#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>
#include <rtl/ref.hxx>

class ScDocShell;
class ScConditionalFormatList;
class ScConditionalFormat;
class ScIconSetFormat;
class ScDataBarFormat;
class ScColorScaleFormat;
class ScCondFormatEntry;
class ScColorScaleEntry;
class ScCondDateFormatEntry;

class ScCondFormatsObj : public cppu::WeakImplHelper<css::sheet::XConditionalFormats>,
                            public SfxListener
{
public:
    ScCondFormatsObj(ScDocShell* pDocShell, SCTAB nTab);

    virtual ~ScCondFormatsObj() override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XConditionalFormats
    virtual sal_Int32 createByRange(const cpo::uno::Reference<css::sheet::XSheetCellRanges>& xRanges) override;

    virtual void removeByID( const sal_Int32 nID ) override;

    virtual cpo::uno::Sequence< cpo::uno::Reference< css::sheet::XConditionalFormat > > getConditionalFormats() override;

    virtual sal_Int32 getLength() override;

    ScConditionalFormatList* getCoreObject();

private:
    SCTAB mnTab;
    ScDocShell* mpDocShell;
};

class ScCondFormatObj : public cppu::WeakImplHelper<css::sheet::XConditionalFormat,
                            css::beans::XPropertySet>
{
public:
    ScCondFormatObj(ScDocShell* pDocShell, rtl::Reference<ScCondFormatsObj> xCondFormats, sal_Int32 nKey);

    virtual ~ScCondFormatObj() override;

    ScDocShell* getDocShell();

    // XConditionalFormat
    virtual void createEntry(const sal_Int32 nType, const sal_Int32 nPos) override;

    virtual void removeByIndex(const sal_Int32 nIndex) override;

    // XIndexAccess

    virtual cpo::uno::Type getElementType() override;

    virtual bool hasElements() override;

    virtual sal_Int32 getCount() override;

    virtual cpo::uno::Any getByIndex(sal_Int32 nIndex) override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    ScConditionalFormat* getCoreObject();

private:
    rtl::Reference<ScCondFormatsObj> mxCondFormatList;
    ScDocShell* mpDocShell;
    SfxItemPropertySet maPropSet;
    sal_Int32 mnKey;
};

class ScConditionEntryObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:

    ScConditionEntryObj(rtl::Reference<ScCondFormatObj> const & xParent,
            const ScCondFormatEntry* pFormat);
    virtual ~ScConditionEntryObj() override;

    ScCondFormatEntry* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 getType() override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    ScDocShell* mpDocShell;
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScCondFormatEntry* mpFormat;
};

class ScColorScaleFormatObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:

    ScColorScaleFormatObj(rtl::Reference<ScCondFormatObj> xParent, const ScColorScaleFormat* pFormat);
    virtual ~ScColorScaleFormatObj() override;

    // XConditionEntry
    virtual sal_Int32 getType() override;


    ScColorScaleFormat* getCoreObject();

                            // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScColorScaleFormat* mpFormat;
};

class ScColorScaleEntryObj : public cppu::WeakImplHelper<css::sheet::XColorScaleEntry>
{
public:
    ScColorScaleEntryObj(rtl::Reference<ScColorScaleFormatObj> xParent, size_t nPos);

    virtual ~ScColorScaleEntryObj() override;

    virtual sal_Int32 getColor() override;

    virtual void setColor(sal_Int32 aColor) override;

    virtual sal_Int32 getType() override;

    virtual void setType(sal_Int32 nType) override;

    virtual OUString getFormula() override;

    virtual void setFormula(const OUString& rString) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScColorScaleFormatObj> mxParent;
    size_t mnPos;
};

class ScDataBarFormatObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:
    ScDataBarFormatObj(rtl::Reference<ScCondFormatObj> xParent,
            const ScDataBarFormat* pFormat);
    virtual ~ScDataBarFormatObj() override;

    ScDataBarFormat* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 getType() override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScDataBarFormat* mpFormat;
};

class ScDataBarEntryObj : public cppu::WeakImplHelper<css::sheet::XDataBarEntry>
{
public:
    ScDataBarEntryObj(rtl::Reference<ScDataBarFormatObj> xParent, size_t nPos);

    virtual ~ScDataBarEntryObj() override;

    virtual sal_Int32 getType() override;

    virtual void setType(sal_Int32 nType) override;

    virtual OUString getFormula() override;

    virtual void setFormula(const OUString& rString) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScDataBarFormatObj> mxParent;
    size_t mnPos;
};

class ScIconSetFormatObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:
    ScIconSetFormatObj(rtl::Reference<ScCondFormatObj> xParent,
            const ScIconSetFormat* pFormat);
    virtual ~ScIconSetFormatObj() override;

    ScIconSetFormat* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 getType() override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScIconSetFormat* mpFormat;
};

class ScIconSetEntryObj : public cppu::WeakImplHelper<css::sheet::XIconSetEntry>
{
public:
    ScIconSetEntryObj(rtl::Reference<ScIconSetFormatObj> xParent, size_t nPos);

    virtual ~ScIconSetEntryObj() override;

    virtual sal_Int32 getType() override;

    virtual void setType(sal_Int32 nType) override;

    virtual OUString getFormula() override;

    virtual void setFormula(const OUString& rString) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScIconSetFormatObj> mxParent;
    size_t mnPos;
};

class ScCondDateFormatObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:
    ScCondDateFormatObj(rtl::Reference<ScCondFormatObj> xParent,
            const ScCondDateFormatEntry* pFormat);

    virtual ~ScCondDateFormatObj() override;

    ScCondDateFormatEntry* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 getType() override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScCondDateFormatEntry* mpFormat;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
