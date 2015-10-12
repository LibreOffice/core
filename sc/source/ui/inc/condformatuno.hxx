/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _SC_CONDFORMATUNO_HXX_
#define _SC_CONDFORMATUNO_HXX_

#include "address.hxx"

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

class ScDocument;
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

class ScCondFormatsObj : public cppu::WeakImplHelper<com::sun::star::sheet::XConditionalFormats>,
                            public SfxListener
{
public:
    ScCondFormatsObj(ScDocShell* pDocShell, SCTAB nTab);

    virtual ~ScCondFormatsObj();

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XConditionalFormats
    virtual sal_Int32 SAL_CALL createByRange(const uno::Reference<sheet::XSheetCellRanges>& xRanges)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeByID( const sal_Int32 nID )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

    virtual uno::Sequence< uno::Reference< sheet::XConditionalFormat > > SAL_CALL getConditionalFormats()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

    virtual sal_Int32 SAL_CALL getLength()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

    ScConditionalFormatList* getCoreObject();

private:
    SCTAB mnTab;
    ScDocShell* mpDocShell;
};

class ScCondFormatObj : public cppu::WeakImplHelper<com::sun::star::sheet::XConditionalFormat,
                            com::sun::star::beans::XPropertySet>
{
public:
    ScCondFormatObj(ScDocShell* pDocShell, rtl::Reference<ScCondFormatsObj> xCondFormats, sal_Int32 nKey);

    virtual ~ScCondFormatObj();

    ScDocShell* getDocShell();

    // XConditionalFormat
    virtual void SAL_CALL createEntry(const sal_Int32 nType, const sal_Int32 nPos)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeByIndex(const sal_Int32 nIndex)
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

    // XIndexAccess

    virtual uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

    virtual sal_Bool SAL_CALL hasElements()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

    virtual sal_Int32 SAL_CALL getCount()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

    virtual uno::Any SAL_CALL getByIndex(sal_Int32 nIndex)
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

    ScConditionalFormat* getCoreObject();

private:
    rtl::Reference<ScCondFormatsObj> mxCondFormatList;
    ScDocShell* mpDocShell;
    SfxItemPropertySet maPropSet;
    sal_Int32 mnKey;
};

class ScConditionEntryObj : public cppu::WeakImplHelper<com::sun::star::beans::XPropertySet,
                                com::sun::star::sheet::XConditionEntry>
{
public:

    ScConditionEntryObj(rtl::Reference<ScCondFormatObj> xParent,
            const ScCondFormatEntry* pFormat);
    virtual ~ScConditionEntryObj();

    ScCondFormatEntry* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ScDocShell* mpDocShell;
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScCondFormatEntry* mpFormat;
};

class ScColorScaleFormatObj : public cppu::WeakImplHelper<com::sun::star::beans::XPropertySet,
                                com::sun::star::sheet::XConditionEntry>
{
public:

    ScColorScaleFormatObj(rtl::Reference<ScCondFormatObj> xParent, const ScColorScaleFormat* pFormat);
    virtual ~ScColorScaleFormatObj();

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;


    ScColorScaleFormat* getCoreObject();

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScColorScaleFormat* mpFormat;
};

class ScColorScaleEntryObj : public cppu::WeakImplHelper<com::sun::star::sheet::XColorScaleEntry>
{
public:
    ScColorScaleEntryObj(rtl::Reference<ScColorScaleFormatObj> xParent, size_t nPos);

    virtual ~ScColorScaleEntryObj();

    virtual com::sun::star::util::Color SAL_CALL getColor()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setColor(com::sun::star::util::Color aColor)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setType(sal_Int32 nType)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getFormula()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setFormula(const OUString& rString)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScColorScaleFormatObj> mxParent;
    size_t mnPos;
};

class ScDataBarFormatObj : public cppu::WeakImplHelper<com::sun::star::beans::XPropertySet,
                                com::sun::star::sheet::XConditionEntry>
{
public:
    ScDataBarFormatObj(rtl::Reference<ScCondFormatObj> xParent,
            const ScDataBarFormat* pFormat);
    virtual ~ScDataBarFormatObj();

    ScDataBarFormat* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScDataBarFormat* mpFormat;
};

class ScDataBarEntryObj : public cppu::WeakImplHelper<com::sun::star::sheet::XDataBarEntry>
{
public:
    ScDataBarEntryObj(rtl::Reference<ScDataBarFormatObj> xParent, size_t nPos);

    virtual ~ScDataBarEntryObj();

    virtual sal_Int32 SAL_CALL getType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setType(sal_Int32 nType)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getFormula()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setFormula(const OUString& rString)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScDataBarFormatObj> mxParent;
    size_t mnPos;
};

class ScIconSetFormatObj : public cppu::WeakImplHelper<com::sun::star::beans::XPropertySet,
                                com::sun::star::sheet::XConditionEntry>
{
public:
    ScIconSetFormatObj(rtl::Reference<ScCondFormatObj> xParent,
            const ScIconSetFormat* pFormat);
    virtual ~ScIconSetFormatObj();

    ScIconSetFormat* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScIconSetFormat* mpFormat;
};

class ScIconSetEntryObj : public cppu::WeakImplHelper<com::sun::star::sheet::XIconSetEntry>
{
public:
    ScIconSetEntryObj(rtl::Reference<ScIconSetFormatObj> xParent, size_t nPos);

    virtual ~ScIconSetEntryObj();

    virtual sal_Int32 SAL_CALL getType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setType(sal_Int32 nType)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getFormula()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setFormula(const OUString& rString)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ScColorScaleEntry* getCoreObject();

    rtl::Reference<ScIconSetFormatObj> mxParent;
    size_t mnPos;
};

class ScCondDateFormatObj : public cppu::WeakImplHelper<com::sun::star::beans::XPropertySet,
                                com::sun::star::sheet::XConditionEntry>
{
public:
    ScCondDateFormatObj(rtl::Reference<ScCondFormatObj> xParent,
            const ScCondDateFormatEntry* pFormat);

    virtual ~ScCondDateFormatObj();

    ScCondDateFormatEntry* getCoreObject();

    // XConditionEntry
    virtual sal_Int32 SAL_CALL getType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    rtl::Reference<ScCondFormatObj> mxParent;
    SfxItemPropertySet maPropSet;
    const ScCondDateFormatEntry* mpFormat;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
