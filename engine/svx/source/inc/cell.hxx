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
    SVX_DLLPRIVATE virtual cpo::uno::Any queryInterface( const cpo::uno::Type& Type ) override;
    virtual void acquire() noexcept override;
    virtual void release() noexcept override;

    // XTypeProvider
    SVX_DLLPRIVATE virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;
    SVX_DLLPRIVATE virtual cpo::uno::Sequence< ::sal_Int8 > getImplementationId(  ) override;

    // XLayoutConstrains
    SVX_DLLPRIVATE virtual css::awt::Size getMinimumSize() override;
    SVX_DLLPRIVATE virtual css::awt::Size getPreferredSize() override;
    SVX_DLLPRIVATE virtual css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // XMergeableCell
    virtual ::sal_Int32 getRowSpan() override;
    virtual ::sal_Int32 getColumnSpan() override;
    virtual bool isMerged() override;

    // XCell
    SVX_DLLPRIVATE virtual OUString getFormula() override;
    SVX_DLLPRIVATE virtual void setFormula( const OUString& aFormula ) override;
    SVX_DLLPRIVATE virtual double getValue() override;
    SVX_DLLPRIVATE virtual void setValue( double nValue ) override;
    SVX_DLLPRIVATE virtual css::table::CellContentType getType() override;
    SVX_DLLPRIVATE virtual sal_Int32 getError() override;

    // css::beans::XPropertySet
    SVX_DLLPRIVATE virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;
    SVX_DLLPRIVATE virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    SVX_DLLPRIVATE virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    SVX_DLLPRIVATE virtual void addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    SVX_DLLPRIVATE virtual void removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    SVX_DLLPRIVATE virtual void addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    SVX_DLLPRIVATE virtual void removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
    SVX_DLLPRIVATE virtual void setPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames, const cpo::uno::Sequence< cpo::uno::Any >& aValues ) override;
    SVX_DLLPRIVATE virtual cpo::uno::Sequence< cpo::uno::Any > getPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
    SVX_DLLPRIVATE virtual void addPropertiesChangeListener( const cpo::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    SVX_DLLPRIVATE virtual void removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    SVX_DLLPRIVATE virtual void firePropertiesChangeEvent( const cpo::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // css::beans::XPropertyState
    SVX_DLLPRIVATE virtual css::beans::PropertyState getPropertyState( const OUString& PropertyName ) override;
    SVX_DLLPRIVATE virtual cpo::uno::Sequence< css::beans::PropertyState > getPropertyStates( const cpo::uno::Sequence< OUString >& aPropertyName ) override;
    SVX_DLLPRIVATE virtual void setPropertyToDefault( const OUString& PropertyName ) override;
    SVX_DLLPRIVATE virtual cpo::uno::Any getPropertyDefault( const OUString& aPropertyName ) override;

    // XMultiPropertyStates
    SVX_DLLPRIVATE virtual void setAllPropertiesToDefault() override;
    SVX_DLLPRIVATE virtual void setPropertiesToDefault( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
    SVX_DLLPRIVATE virtual cpo::uno::Sequence< cpo::uno::Any > getPropertyDefaults( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;

    // XText
    SVX_DLLPRIVATE virtual void insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange, const css::uno::Reference< css::text::XTextContent >& xContent, bool bAbsorb ) override;
    SVX_DLLPRIVATE virtual void removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent ) override;

    // XSimpleText
    SVX_DLLPRIVATE virtual void insertString( const css::uno::Reference< css::text::XTextRange >& xRange, const OUString& aString, bool bAbsorb ) override;
    SVX_DLLPRIVATE virtual void insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange, ::sal_Int16 nControlCharacter, bool bAbsorb ) override;

    // XTextRange
    SVX_DLLPRIVATE virtual OUString getString(  ) override;
    SVX_DLLPRIVATE virtual void setString( const OUString& aString ) override;

    // XEventListener
    SVX_DLLPRIVATE virtual void disposing( const css::lang::EventObject& Source ) override;

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

    SVX_DLLPRIVATE static cpo::uno::Any GetAnyForItem( SfxItemSet const & aSet, const SfxItemPropertyMapEntry* pMap );
    SVX_DLLPRIVATE void setPropertyValueImpl( const OUString& aPropertyName, const cpo::uno::Any& aValue, SfxItemSet& rSet );

    /// @throws cpo::uno::RuntimeException
    SVX_DLLPRIVATE Cell( SdrTableObj& rTableObj );
    SVX_DLLPRIVATE virtual ~Cell() override;

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
