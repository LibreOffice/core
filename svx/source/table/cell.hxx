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

#ifndef INCLUDED_SVX_SOURCE_TABLE_CELL_HXX
#define INCLUDED_SVX_SOURCE_TABLE_CELL_HXX

#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <rtl/ref.hxx>
#include <svl/style.hxx>
#include "svx/sdtaitm.hxx"
#include "tablemodel.hxx"
#include "editeng/unotext.hxx"
#include "svx/svdtext.hxx"



class SfxItemSet;
class OutlinerParaObject;
class SdrObject;

namespace sdr { namespace properties {
    class TextProperties;
} }



namespace sdr { namespace table {



class SVX_DLLPUBLIC Cell :  public SdrText,
                public SvxUnoTextBase,
                public css::table::XMergeableCell,
                public css::awt::XLayoutConstrains,
                public css::lang::XEventListener,
                public ::cppu::OWeakObject
{
    friend class CellUndo;

public:
    SVX_DLLPRIVATE static rtl::Reference< Cell > create( SdrTableObj& rTableObj, OutlinerParaObject* pOutlinerParaObject );

    // private
    SVX_DLLPRIVATE void dispose();

    // SdrTextShape proxy
    bool IsTextEditActive();
    SVX_DLLPRIVATE bool hasText() const;

    SVX_DLLPRIVATE void cloneFrom( const CellRef& rCell );

    SVX_DLLPRIVATE void setCellRect( ::Rectangle& rCellRect ) { maCellRect = rCellRect; }
    SVX_DLLPRIVATE const ::Rectangle& getCellRect() const { return maCellRect; }
    SVX_DLLPRIVATE ::Rectangle& getCellRect() { return maCellRect; }

    OutlinerParaObject* GetEditOutlinerParaObject() const;
    SVX_DLLPRIVATE void SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr );
    SVX_DLLPRIVATE virtual SfxStyleSheet* GetStyleSheet() const override;
    SVX_DLLPRIVATE virtual void TakeTextAnchorRect(Rectangle& rAnchorRect) const;

    SVX_DLLPRIVATE virtual const SfxItemSet& GetItemSet() const override;
    SVX_DLLPRIVATE void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems);
    void SetMergedItem(const SfxPoolItem& rItem);

    SVX_DLLPRIVATE sal_Int32 getMinimumWidth();
    SVX_DLLPRIVATE sal_Int32 getMinimumHeight();

    SVX_DLLPRIVATE long GetTextLeftDistance() const;
    SVX_DLLPRIVATE long GetTextRightDistance() const;
    SVX_DLLPRIVATE long GetTextUpperDistance() const;
    SVX_DLLPRIVATE long GetTextLowerDistance() const;

    SVX_DLLPRIVATE SdrTextVertAdjust GetTextVerticalAdjust() const;
    SdrTextHorzAdjust GetTextHorizontalAdjust() const;

    SVX_DLLPRIVATE virtual void SetModel(SdrModel* pNewModel) override;

    SVX_DLLPRIVATE void merge( sal_Int32 nColumnSpan, sal_Int32 nRowSpan );
    SVX_DLLPRIVATE void mergeContent( const CellRef& xSourceCell );
    SVX_DLLPRIVATE void replaceContentAndFormating( const CellRef& xSourceCell );

    SVX_DLLPRIVATE void setMerged();

    SVX_DLLPRIVATE void copyFormatFrom( const CellRef& xSourceCell );

    // XInterface
    SVX_DLLPRIVATE virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& Type ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL acquire() throw () override;
    SVX_DLLPRIVATE virtual void SAL_CALL release() throw () override;

    // XTypeProvider
    SVX_DLLPRIVATE virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XLayoutConstrains
    SVX_DLLPRIVATE virtual css::awt::Size SAL_CALL getMinimumSize()
        throw (css::uno::RuntimeException,
               std::exception) override;
    SVX_DLLPRIVATE virtual css::awt::Size SAL_CALL getPreferredSize()
        throw (css::uno::RuntimeException,
               std::exception) override;
    SVX_DLLPRIVATE virtual css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw (css::uno::RuntimeException, std::exception) override;

    // XMergeableCell
    SVX_DLLPRIVATE virtual ::sal_Int32 SAL_CALL getRowSpan() throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual ::sal_Int32 SAL_CALL getColumnSpan() throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual sal_Bool SAL_CALL isMerged() throw (css::uno::RuntimeException, std::exception) override;

    // XCell
    SVX_DLLPRIVATE virtual OUString SAL_CALL getFormula() throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL setFormula( const OUString& aFormula ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual double SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL setValue( double nValue ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::table::CellContentType SAL_CALL getType() throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual sal_Int32 SAL_CALL getError() throw (css::uno::RuntimeException, std::exception) override;

    // css::beans::XPropertySet
    SVX_DLLPRIVATE virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) throw (css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    // css::beans::XPropertyState
    SVX_DLLPRIVATE virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XMultiPropertyStates
    SVX_DLLPRIVATE virtual void SAL_CALL setAllPropertiesToDefault()
        throw (css::uno::RuntimeException,
               std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XText
    SVX_DLLPRIVATE virtual void SAL_CALL insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange, const css::uno::Reference< css::text::XTextContent >& xContent, sal_Bool bAbsorb ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent ) throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;

    // XSimpleText
    SVX_DLLPRIVATE virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL createTextCursor(  ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL createTextCursorByRange( const css::uno::Reference< css::text::XTextRange >& aTextPosition ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL insertString( const css::uno::Reference< css::text::XTextRange >& xRange, const OUString& aString, sal_Bool bAbsorb ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange, ::sal_Int16 nControlCharacter, sal_Bool bAbsorb ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XTextRange
    SVX_DLLPRIVATE virtual css::uno::Reference< css::text::XText > SAL_CALL getText(  ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart(  ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd(  ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual OUString SAL_CALL getString(  ) throw (css::uno::RuntimeException, std::exception) override;
    SVX_DLLPRIVATE virtual void SAL_CALL setString( const OUString& aString ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    SVX_DLLPRIVATE virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    SVX_DLLPRIVATE virtual void SetOutlinerParaObject( OutlinerParaObject* pTextObject ) override;

    SVX_DLLPRIVATE void AddUndo();

    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    SVX_DLLPRIVATE sdr::properties::TextProperties* CloneProperties( SdrObject& rNewObj, Cell& rNewCell );

    SVX_DLLPRIVATE static sdr::properties::TextProperties* CloneProperties( sdr::properties::TextProperties* pProperties, SdrObject& rNewObj, Cell& rNewCell );

    SVX_DLLPRIVATE void notifyModified();

protected:
    SVX_DLLPRIVATE virtual const SfxItemSet& GetObjectItemSet() override;
    SVX_DLLPRIVATE virtual void SetObjectItem(const SfxPoolItem& rItem) override;

    SVX_DLLPRIVATE css::uno::Any GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertySimpleEntry* pMap );

private:
    SVX_DLLPRIVATE Cell( SdrTableObj& rTableObj, OutlinerParaObject* pOutlinerParaObject ) throw(css::uno::RuntimeException);
    SVX_DLLPRIVATE virtual ~Cell() throw();

    const SvxItemPropertySet* mpPropSet;

    sdr::properties::TextProperties*    mpProperties;

    css::table::CellContentType mnCellContentType;

    OUString msFormula;
    double          mfValue;
    ::sal_Int32     mnError;
    bool      mbMerged;
    ::sal_Int32     mnRowSpan;
    ::sal_Int32     mnColSpan;

    Rectangle maCellRect;

    css::uno::Reference< css::table::XTable > mxTable;
};



} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
