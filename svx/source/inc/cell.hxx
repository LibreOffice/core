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

#ifndef INCLUDED_SVX_SOURCE_INC_CELL_HXX
#define INCLUDED_SVX_SOURCE_INC_CELL_HXX

#include <config_options.h>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <rtl/ref.hxx>
#include <svl/style.hxx>
#include <svl/grabbagitem.hxx>
#include <svx/sdtaitm.hxx>
#include "tablemodel.hxx"
#include <editeng/unotext.hxx>
#include <svx/svdtext.hxx>


class SfxItemSet;
class SdrObject;
namespace sdr::properties { class CellProperties; }


namespace sdr::table {


class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) Cell final : public SdrText,
                public SvxUnoTextBase,
                public css::table::XMergeableCell,
                public css::awt::XLayoutConstrains,
                public css::lang::XEventListener
{
    friend class CellUndo;

public:
    SVX_DLLPRIVATE static rtl::Reference< Cell > create( SdrTableObj& rTableObj );

    // private
    SVX_DLLPRIVATE void dispose();

    // SdrTextShape proxy
    bool IsActiveCell() const;
    bool IsTextEditActive() const;
    SVX_DLLPRIVATE bool hasText() const;

    SVX_DLLPRIVATE void cloneFrom( const CellRef& rCell );

    SVX_DLLPRIVATE void setCellRect( ::tools::Rectangle const & rCellRect ) { maCellRect = rCellRect; }
    SVX_DLLPRIVATE const ::tools::Rectangle& getCellRect() const { return maCellRect; }
    SVX_DLLPRIVATE ::tools::Rectangle& getCellRect() { return maCellRect; }

    bool CanCreateEditOutlinerParaObject() const;
    std::optional<OutlinerParaObject> CreateEditOutlinerParaObject() const;
    SVX_DLLPRIVATE void SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr );
    SVX_DLLPRIVATE virtual SfxStyleSheet* GetStyleSheet() const override;
    SVX_DLLPRIVATE void TakeTextAnchorRect(tools::Rectangle& rAnchorRect) const;

    SVX_DLLPRIVATE void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems);
    void SetMergedItem(const SfxPoolItem& rItem);

    SVX_DLLPRIVATE sal_Int32 calcPreferredWidth( const Size aSize );
    SVX_DLLPRIVATE sal_Int32 getMinimumWidth() const;
    SVX_DLLPRIVATE sal_Int32 getMinimumHeight();

    SVX_DLLPRIVATE tools::Long GetTextLeftDistance() const;
    SVX_DLLPRIVATE tools::Long GetTextRightDistance() const;
    SVX_DLLPRIVATE tools::Long GetTextUpperDistance() const;
    SVX_DLLPRIVATE tools::Long GetTextLowerDistance() const;

    SVX_DLLPRIVATE SdrTextVertAdjust GetTextVerticalAdjust() const;
    SdrTextHorzAdjust GetTextHorizontalAdjust() const;

    SVX_DLLPRIVATE void merge( sal_Int32 nColumnSpan, sal_Int32 nRowSpan );
    SVX_DLLPRIVATE void mergeContent( const CellRef& xSourceCell );
    SVX_DLLPRIVATE void replaceContentAndFormatting( const CellRef& xSourceCell );

    SVX_DLLPRIVATE void setMerged();

    SVX_DLLPRIVATE void copyFormatFrom( const CellRef& xSourceCell );

    // XInterface
    SVX_DLLPRIVATE virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& Type ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XTypeProvider
    SVX_DLLPRIVATE virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    SVX_DLLPRIVATE virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XLayoutConstrains
    SVX_DLLPRIVATE virtual css::awt::Size SAL_CALL getMinimumSize() override;
    SVX_DLLPRIVATE virtual css::awt::Size SAL_CALL getPreferredSize() override;
    SVX_DLLPRIVATE virtual css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // XMergeableCell
    virtual ::sal_Int32 SAL_CALL getRowSpan() override;
    virtual ::sal_Int32 SAL_CALL getColumnSpan() override;
    virtual sal_Bool SAL_CALL isMerged() override;

    // XCell
    SVX_DLLPRIVATE virtual OUString SAL_CALL getFormula() override;
    SVX_DLLPRIVATE virtual void SAL_CALL setFormula( const OUString& aFormula ) override;
    SVX_DLLPRIVATE virtual double SAL_CALL getValue() override;
    SVX_DLLPRIVATE virtual void SAL_CALL setValue( double nValue ) override;
    SVX_DLLPRIVATE virtual css::table::CellContentType SAL_CALL getType() override;
    SVX_DLLPRIVATE virtual sal_Int32 SAL_CALL getError() override;

    // css::beans::XPropertySet
    SVX_DLLPRIVATE virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    SVX_DLLPRIVATE virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    SVX_DLLPRIVATE virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // css::beans::XPropertyState
    SVX_DLLPRIVATE virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    SVX_DLLPRIVATE virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    SVX_DLLPRIVATE virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    // XMultiPropertyStates
    SVX_DLLPRIVATE virtual void SAL_CALL setAllPropertiesToDefault() override;
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    SVX_DLLPRIVATE virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    // XText
    SVX_DLLPRIVATE virtual void SAL_CALL insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange, const css::uno::Reference< css::text::XTextContent >& xContent, sal_Bool bAbsorb ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent ) override;

    // XSimpleText
    SVX_DLLPRIVATE virtual void SAL_CALL insertString( const css::uno::Reference< css::text::XTextRange >& xRange, const OUString& aString, sal_Bool bAbsorb ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange, ::sal_Int16 nControlCharacter, sal_Bool bAbsorb ) override;

    // XTextRange
    SVX_DLLPRIVATE virtual OUString SAL_CALL getString(  ) override;
    SVX_DLLPRIVATE virtual void SAL_CALL setString( const OUString& aString ) override;

    // XEventListener
    SVX_DLLPRIVATE virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    SVX_DLLPRIVATE virtual void SetOutlinerParaObject( std::optional<OutlinerParaObject> pTextObject ) override;

    SVX_DLLPRIVATE void AddUndo();

    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    SVX_DLLPRIVATE sdr::properties::CellProperties* CloneProperties( SdrObject& rNewObj, Cell& rNewCell );

    SVX_DLLPRIVATE void notifyModified();

    void dumpAsXml(xmlTextWriterPtr pWriter, sal_Int32 nRow, sal_Int32 nCol) const;

private:
    SVX_DLLPRIVATE virtual const SfxItemSet& GetObjectItemSet() override;
    SVX_DLLPRIVATE void SetObjectItem(const SfxPoolItem& rItem);

    SVX_DLLPRIVATE static css::uno::Any GetAnyForItem( SfxItemSet const & aSet, const SfxItemPropertyMapEntry* pMap );

    /// @throws css::uno::RuntimeException
    SVX_DLLPRIVATE Cell( SdrTableObj& rTableObj );
    SVX_DLLPRIVATE virtual ~Cell() COVERITY_NOEXCEPT_FALSE override;

    Cell(Cell const &) = delete;
    void operator =(Cell const &) = delete;

    const SvxItemPropertySet* mpPropSet;

    std::unique_ptr<sdr::properties::CellProperties> mpProperties;

    css::table::CellContentType mnCellContentType;

    OUString        msFormula;
    double          mfValue;
    ::sal_Int32     mnError;
    bool            mbMerged;
    ::sal_Int32     mnRowSpan;
    ::sal_Int32     mnColSpan;

    tools::Rectangle       maCellRect;

    css::uno::Reference< css::table::XTable > mxTable;

    std::unique_ptr<SfxGrabBagItem> mpGrabBagItem = {};
};


}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
