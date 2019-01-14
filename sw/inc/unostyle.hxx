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
#ifndef INCLUDED_SW_INC_UNOSTYLE_HXX
#define INCLUDED_SW_INC_UNOSTYLE_HXX

#include <svl/listener.hxx>
#include <svl/style.hxx>
#include "unocoll.hxx"
#include "tblafmt.hxx"
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <cppuhelper/implbase.hxx>
#include "calbck.hxx"

#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/style/XAutoStyles.hpp>
#include <com/sun/star/style/XAutoStyle.hpp>

#include "istyleaccess.hxx"
#include <memory>
#include <map>

namespace com { namespace sun { namespace star { namespace document { class XEventsSupplier; } } } }

class SwDocShell;
class SwAutoStylesEnumImpl;
class SfxItemSet;

class SwXStyleFamilies :  public cppu::WeakImplHelper
<
    css::container::XIndexAccess,
    css::container::XNameAccess,
    css::lang::XServiceInfo,
    css::style::XStyleLoader
>,
    public SwUnoCollection
{
    SwDocShell*         m_pDocShell;

    std::map<SfxStyleFamily, css::uno::Reference<css::container::XNameContainer>> m_vFamilies;
protected:
    virtual ~SwXStyleFamilies() override;
public:
    SwXStyleFamilies(SwDocShell& rDocShell);

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XStyleLoader
    virtual void SAL_CALL loadStylesFromURL(const OUString& rURL, const css::uno::Sequence< css::beans::PropertyValue >& aOptions) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getStyleLoaderOptions() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static css::uno::Reference<css::style::XStyle> CreateStyle(SfxStyleFamily eFamily, SwDoc& rDoc);
    // FIXME: This is very ugly as is the whole conditional paragraph style
    // hackety. Should be folded into CreateStyle hopefully one day
    static css::uno::Reference<css::style::XStyle> CreateStyleCondParagraph(SwDoc& rDoc);
};

namespace sw
{
    class ICoreFrameStyle
    {
        public:
            virtual void SetItem(sal_uInt16 eAtr, const SfxPoolItem& rItem) =0;
            virtual const SfxPoolItem* GetItem(sal_uInt16 eAtr) =0;
            virtual css::document::XEventsSupplier& GetEventsSupplier() =0;
            virtual ~ICoreFrameStyle() {};
    };
}

// access to all automatic style families
class SwXAutoStyles :
    public cppu::WeakImplHelper< css::style::XAutoStyles >,
    public SwUnoCollection
{
    SwDocShell * const m_pDocShell;
    css::uno::Reference< css::style::XAutoStyleFamily > m_xAutoCharStyles;
    css::uno::Reference< css::style::XAutoStyleFamily > m_xAutoRubyStyles;
    css::uno::Reference< css::style::XAutoStyleFamily > m_xAutoParaStyles;
    virtual ~SwXAutoStyles() override;

public:
    SwXAutoStyles(SwDocShell& rDocShell);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

};

// access to a family of automatic styles (character or paragraph or ...)
class SwXAutoStyleFamily : public cppu::WeakImplHelper< css::style::XAutoStyleFamily >, public SvtListener
{
    SwDocShell *m_pDocShell;
    IStyleAccess::SwAutoStyleFamily const m_eFamily;


public:
    SwXAutoStyleFamily(SwDocShell* pDocShell, IStyleAccess::SwAutoStyleFamily eFamily);
    virtual ~SwXAutoStyleFamily() override;

    //XAutoStyleFamily
    virtual css::uno::Reference< css::style::XAutoStyle > SAL_CALL insertStyle( const css::uno::Sequence< css::beans::PropertyValue >& Values ) override;

    //XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    virtual void Notify( const SfxHint&) override;
};

class SwXAutoStylesEnumerator : public cppu::WeakImplHelper< css::container::XEnumeration >, public SvtListener
{
    std::unique_ptr<SwAutoStylesEnumImpl> m_pImpl;
public:
    SwXAutoStylesEnumerator( SwDoc* pDoc, IStyleAccess::SwAutoStyleFamily eFam );
    virtual ~SwXAutoStylesEnumerator() override;

    //XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual css::uno::Any SAL_CALL nextElement(  ) override;

    virtual void Notify( const SfxHint&) override;
};

// an automatic style
class SwXAutoStyle : public cppu::WeakImplHelper
<
    css::beans::XPropertySet,
    css::beans::XPropertyState,
    css::style::XAutoStyle
>,
    public SvtListener
{
private:
    std::shared_ptr<SfxItemSet>            mpSet;
    IStyleAccess::SwAutoStyleFamily const  meFamily;
    SwDoc&                                 mrDoc;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::uno::Any > GetPropertyValues_Impl( const css::uno::Sequence< OUString >& aPropertyNames );

public:

    SwXAutoStyle( SwDoc* pDoc, std::shared_ptr<SfxItemSet> const & pInitSet, IStyleAccess::SwAutoStyleFamily eFam );
    virtual ~SwXAutoStyle() override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    //XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault(  ) override;
    virtual void SAL_CALL setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    // Special
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getProperties() override;

    virtual void Notify( const SfxHint& ) override;

};

typedef std::map<OUString, sal_Int32> CellStyleNameMap;

/// A text table style is a uno api wrapper for a SwTableAutoFormat
class SwXTextTableStyle : public cppu::WeakImplHelper
<
    css::style::XStyle,
    css::beans::XPropertySet,
    css::container::XNameContainer,
    css::lang::XServiceInfo
>
{
    SwDocShell* m_pDocShell;
    SwTableAutoFormat* m_pTableAutoFormat;
    /// Stores SwTableAutoFormat when this is not a physical style.
    std::unique_ptr<SwTableAutoFormat> m_pTableAutoFormat_Impl;
    /// If true, then it points to a core object, if false, then this is a created, but not-yet-inserted format.
    bool m_bPhysical;

    enum {
        FIRST_ROW_STYLE = 0,
        LAST_ROW_STYLE,
        FIRST_COLUMN_STYLE,
        LAST_COLUMN_STYLE,
        EVEN_ROWS_STYLE,
        ODD_ROWS_STYLE,
        EVEN_COLUMNS_STYLE,
        ODD_COLUMNS_STYLE,
        BODY_STYLE,
        BACKGROUND_STYLE,
        // loext namespace
        FIRST_ROW_START_COLUMN_STYLE,
        FIRST_ROW_END_COLUMN_STYLE,
        LAST_ROW_START_COLUMN_STYLE,
        LAST_ROW_END_COLUMN_STYLE,
        FIRST_ROW_EVEN_COLUMN_STYLE,
        LAST_ROW_EVEN_COLUMN_STYLE,
        STYLE_COUNT
    };

    /// Fills m_aCellStyles with SwXTextCellStyles pointing to children of this style.
    void UpdateCellStylesMapping();
    static const CellStyleNameMap& GetCellStyleNameMap();
    css::uno::Reference<css::style::XStyle> m_aCellStyles[STYLE_COUNT];
public:
    SwXTextTableStyle(SwDocShell* pDocShell, SwTableAutoFormat* pTableAutoFormat);
    /// Create non physical style
    SwXTextTableStyle(SwDocShell* pDocShell, const OUString& rTableAutoFormatName);

    /// This function looks for a SwTableAutoFormat with given name. Returns nullptr if could not be found.
    static SwTableAutoFormat* GetTableAutoFormat(SwDocShell* pDocShell, const OUString& sName);
    /// Returns box format assigned to this style
    SwTableAutoFormat* GetTableFormat();
    void SetPhysical();

    //XStyle
    virtual sal_Bool SAL_CALL isUserDefined() override;
    virtual sal_Bool SAL_CALL isInUse() override;
    virtual OUString SAL_CALL getParentStyle() override;
    virtual void SAL_CALL setParentStyle(const OUString& aParentStyle ) override;

    //XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& rName) override;

    //XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& rName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& rName) override;

    //XNameContainer
    virtual void SAL_CALL insertByName(const OUString& rName, const css::uno::Any& aElement) override;
    virtual void SAL_CALL replaceByName(const OUString& rName, const css::uno::Any& aElement) override;
    virtual void SAL_CALL removeByName(const OUString& rName) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    static css::uno::Reference<css::style::XStyle> CreateXTextTableStyle(SwDocShell* pDocShell, const OUString& rTableAutoFormatName);
};

/// A text cell style is a uno api wrapper for a SwBoxAutoFormat core class
class SwXTextCellStyle : public cppu::WeakImplHelper
<
    css::style::XStyle,
    css::beans::XPropertySet,
    css::beans::XPropertyState,
    css::lang::XServiceInfo
>
{
    SwDocShell* m_pDocShell;
    SwBoxAutoFormat* m_pBoxAutoFormat;
    /// Stores SwBoxAutoFormat when this is not a physical style.
    std::shared_ptr<SwBoxAutoFormat> m_pBoxAutoFormat_Impl;
    OUString m_sParentStyle;
    OUString m_sName;
    /// If true, then it points to a core object, if false, then this is a created, but not-yet-inserted format.
    bool m_bPhysical;

 public:
    SwXTextCellStyle(SwDocShell* pDocShell, SwBoxAutoFormat* pBoxAutoFormat, const OUString& sParentStyle);
    /// Create non physical style
    SwXTextCellStyle(SwDocShell* pDocShell, const OUString& sName);

    /**
    * This function looks for a SwBoxAutoFormat with given name. Parses the name and returns parent name.
    * @param pDocShell pointer to a SwDocShell.
    * @param sName Name of a SwBoxAutoFormat to look for.
    * @param pParentName Optional output. Pointer to a OUString where parsed parent name will be returned.
    * @return Pointer to a SwBoxAutoFormat, nullptr if not found.
    */
    static SwBoxAutoFormat* GetBoxAutoFormat(SwDocShell* pDocShell, const OUString& sName, OUString* pParentName);
    /// returns box format assigned to this style
    SwBoxAutoFormat* GetBoxFormat();
    /// Sets the address of SwBoxAutoFormat this style is bound to. Usable only when style is physical.
    void SetBoxFormat(SwBoxAutoFormat* pBoxFormat);
    void SetPhysical();
    bool IsPhysical();

    //XStyle
    virtual sal_Bool SAL_CALL isUserDefined() override;
    virtual sal_Bool SAL_CALL isInUse() override;
    virtual OUString SAL_CALL getParentStyle() override;
    virtual void SAL_CALL setParentStyle(const OUString& aParentStyle ) override;

    //XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& sName) override;

    //XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState(const OUString& PropertyName) override;
    virtual css::uno::Sequence<css::beans::PropertyState> SAL_CALL getPropertyStates(const css::uno::Sequence< OUString >& aPropertyName) override;
    virtual void SAL_CALL setPropertyToDefault(const OUString& PropertyName) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(const OUString& aPropertyName) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    static css::uno::Reference<css::style::XStyle> CreateXTextCellStyle(SwDocShell* pDocShell, const OUString& sName);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
