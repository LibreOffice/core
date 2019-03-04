/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONDFORMATUNO_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONDFORMATUNO_HXX

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

using namespace com::sun::star;

namespace com { namespace sun { namespace star {

namespace sheet {

class XSheetCellRanges;

}

} } }

class ScCondFormatsObj : public cppu::WeakImplHelper<css::sheet::XConditionalFormats>,
                            public SfxListener
{
public:
    ScCondFormatsObj(ScDocShell* pDocShell, SCTAB nTab);

    virtual ~ScCondFormatsObj() override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XConditionalFormats
    virtual sal_Int32 SAL_CALL createByRange(const uno::Reference<sheet::XSheetCellRanges>& xRanges) override;

    virtual void SAL_CALL removeByID( const sal_Int32 nID ) override;

    virtual uno::Sequence< uno::Reference< sheet::XConditionalFormat > > SAL_CALL getConditionalFormats() override;

    virtual sal_Int32 SAL_CALL getLength() override;

    ScConditionalFormatList* getCoreObject();

private:
    SCTAB const mnTab;
    ScDocShell* mpDocShell;
};

class ScCondFormatObj : public cppu::WeakImplHelper<css::sheet::XConditionalFormat,
                            css::beans::XPropertySet>
{
public:
    ScCondFormatObj(ScDocShell* pDocShell, rtl::Reference<ScCondFormatsObj> const & xCondFormats, sal_Int32 nKey);

    virtual ~ScCondFormatObj() override;

    ScDocShell* getDocShell();

    // XConditionalFormat
    virtual void SAL_CALL createEntry(const sal_Int32 nType, const sal_Int32 nPos) override;

    virtual void SAL_CALL removeByIndex(const sal_Int32 nIndex) override;

    // XIndexAccess

    virtual uno::Type SAL_CALL getElementType() override;

    virtual sal_Bool SAL_CALL hasElements() override;

    virtual sal_Int32 SAL_CALL getCount() override;

    virtual uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    ScConditionalFormat* getCoreObject();

private:
    rtl::Reference<ScCondFormatsObj> mxCondFormatList;
    ScDocShell* mpDocShell;
    SfxItemPropertySet const maPropSet;
    sal_Int32 const mnKey;
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
    virtual sal_Int32 SAL_CALL getType() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    ScDocShell* mpDocShell;
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet const maPropSet;
    const ScCondFormatEntry* mpFormat;
};

class ScColorScaleFormatObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:

    ScColorScaleFormatObj(rtl::Reference<ScCondFormatObj> const & xParent, const ScColorScaleFormat* pFormat);
    virtual ~ScColorScaleFormatObj() override;

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType() override;


    ScColorScaleFormat* getCoreObject();

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet const maPropSet;
    const ScColorScaleFormat* mpFormat;
};

class ScColorScaleEntryObj : public cppu::WeakImplHelper<css::sheet::XColorScaleEntry>
{
public:
    ScColorScaleEntryObj(rtl::Reference<ScColorScaleFormatObj> const & xParent, size_t nPos);

    virtual ~ScColorScaleEntryObj() override;

    virtual sal_Int32 SAL_CALL getColor() override;

    virtual void SAL_CALL setColor(sal_Int32 aColor) override;

    virtual sal_Int32 SAL_CALL getType() override;

    virtual void SAL_CALL setType(sal_Int32 nType) override;

    virtual OUString SAL_CALL getFormula() override;

    virtual void SAL_CALL setFormula(const OUString& rString) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScColorScaleFormatObj> mxParent;
    size_t const mnPos;
};

class ScDataBarFormatObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:
    ScDataBarFormatObj(rtl::Reference<ScCondFormatObj> const & xParent,
            const ScDataBarFormat* pFormat);
    virtual ~ScDataBarFormatObj() override;

    ScDataBarFormat* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet const maPropSet;
    const ScDataBarFormat* mpFormat;
};

class ScDataBarEntryObj : public cppu::WeakImplHelper<css::sheet::XDataBarEntry>
{
public:
    ScDataBarEntryObj(rtl::Reference<ScDataBarFormatObj> const & xParent, size_t nPos);

    virtual ~ScDataBarEntryObj() override;

    virtual sal_Int32 SAL_CALL getType() override;

    virtual void SAL_CALL setType(sal_Int32 nType) override;

    virtual OUString SAL_CALL getFormula() override;

    virtual void SAL_CALL setFormula(const OUString& rString) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScDataBarFormatObj> mxParent;
    size_t const mnPos;
};

class ScIconSetFormatObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:
    ScIconSetFormatObj(rtl::Reference<ScCondFormatObj> const & xParent,
            const ScIconSetFormat* pFormat);
    virtual ~ScIconSetFormatObj() override;

    ScIconSetFormat* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet const maPropSet;
    const ScIconSetFormat* mpFormat;
};

class ScIconSetEntryObj : public cppu::WeakImplHelper<css::sheet::XIconSetEntry>
{
public:
    ScIconSetEntryObj(rtl::Reference<ScIconSetFormatObj> const & xParent, size_t nPos);

    virtual ~ScIconSetEntryObj() override;

    virtual sal_Int32 SAL_CALL getType() override;

    virtual void SAL_CALL setType(sal_Int32 nType) override;

    virtual OUString SAL_CALL getFormula() override;

    virtual void SAL_CALL setFormula(const OUString& rString) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScIconSetFormatObj> mxParent;
    size_t const mnPos;
};

class ScCondDateFormatObj : public cppu::WeakImplHelper<css::beans::XPropertySet,
                                css::sheet::XConditionEntry>
{
public:
    ScCondDateFormatObj(rtl::Reference<ScCondFormatObj> const & xParent,
            const ScCondDateFormatEntry* pFormat);

    virtual ~ScCondDateFormatObj() override;

    ScCondDateFormatEntry* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet const maPropSet;
    const ScCondDateFormatEntry* mpFormat;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
