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
                public ::com::sun::star::table::XMergeableCell,
                public ::com::sun::star::awt::XLayoutConstrains,
                public ::com::sun::star::lang::XEventListener,
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
    SVX_DLLPRIVATE void SetStyleSheet( SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr );
    SVX_DLLPRIVATE virtual SfxStyleSheet* GetStyleSheet() const;
    SVX_DLLPRIVATE virtual const Rectangle& GetCurrentBoundRect() const;
    SVX_DLLPRIVATE virtual void TakeTextAnchorRect(Rectangle& rAnchorRect) const;

    SVX_DLLPRIVATE virtual const SfxItemSet& GetItemSet() const;
    SVX_DLLPRIVATE void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, sal_Bool bClearAllItems);
    void SetMergedItem(const SfxPoolItem& rItem);

    SVX_DLLPRIVATE sal_Int32 getMinimumWidth();
    SVX_DLLPRIVATE sal_Int32 getMinimumHeight();

    SVX_DLLPRIVATE long GetTextLeftDistance() const;
    SVX_DLLPRIVATE long GetTextRightDistance() const;
    SVX_DLLPRIVATE long GetTextUpperDistance() const;
    SVX_DLLPRIVATE long GetTextLowerDistance() const;

    SVX_DLLPRIVATE SdrTextVertAdjust GetTextVerticalAdjust() const;
    SdrTextHorzAdjust GetTextHorizontalAdjust() const;

    SVX_DLLPRIVATE virtual void SetModel(SdrModel* pNewModel);

    SVX_DLLPRIVATE void merge( sal_Int32 nColumnSpan, sal_Int32 nRowSpan );
    SVX_DLLPRIVATE void mergeContent( const CellRef& xSourceCell );
    SVX_DLLPRIVATE void replaceContentAndFormating( const CellRef& xSourceCell );

    SVX_DLLPRIVATE void setMerged();

    // XInterface
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& Type ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL acquire() throw ();
    SVX_DLLPRIVATE virtual void SAL_CALL release() throw ();

    // XTypeProvider
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    SVX_DLLPRIVATE virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XLayoutConstrains
    SVX_DLLPRIVATE virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::awt::Size SAL_CALL getPreferredSize()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XMergeableCell
    SVX_DLLPRIVATE virtual ::sal_Int32 SAL_CALL getRowSpan() throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::sal_Int32 SAL_CALL getColumnSpan() throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::sal_Bool SAL_CALL isMerged() throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XCell
    SVX_DLLPRIVATE virtual OUString SAL_CALL getFormula() throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL setFormula( const OUString& aFormula ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual double SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL setValue( double nValue ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::table::CellContentType SAL_CALL getType() throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual sal_Int32 SAL_CALL getError() throw (::com::sun::star::uno::RuntimeException, std::exception);

    // ::com::sun::star::beans::XPropertySet
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XMultiPropertySet
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // ::com::sun::star::beans::XPropertyState
    SVX_DLLPRIVATE virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XMultiPropertyStates
    SVX_DLLPRIVATE virtual void SAL_CALL setAllPropertiesToDefault()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL setPropertiesToDefault( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyDefaults( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XFastPropertySet
    SVX_DLLPRIVATE virtual void SAL_CALL setFastPropertyValue( ::sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XText
    SVX_DLLPRIVATE virtual void SAL_CALL insertTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent, ::sal_Bool bAbsorb ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL removeTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XSimpleText
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursor(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursorByRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& aTextPosition ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL insertString( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const OUString& aString, ::sal_Bool bAbsorb ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL insertControlCharacter( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, ::sal_Int16 nControlCharacter, ::sal_Bool bAbsorb ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XTextRange
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual OUString SAL_CALL getString(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    SVX_DLLPRIVATE virtual void SAL_CALL setString( const OUString& aString ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XEventListener
    SVX_DLLPRIVATE virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    SVX_DLLPRIVATE virtual void SetOutlinerParaObject( OutlinerParaObject* pTextObject );

    SVX_DLLPRIVATE void AddUndo();

    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    SVX_DLLPRIVATE sdr::properties::TextProperties* CloneProperties( SdrObject& rNewObj, Cell& rNewCell );

    SVX_DLLPRIVATE static sdr::properties::TextProperties* CloneProperties( sdr::properties::TextProperties* pProperties, SdrObject& rNewObj, Cell& rNewCell );

    SVX_DLLPRIVATE void notifyModified();

    OUString getName();

protected:
    SVX_DLLPRIVATE virtual const SfxItemSet& GetObjectItemSet();
    SVX_DLLPRIVATE virtual void SetObjectItem(const SfxPoolItem& rItem);

    SVX_DLLPRIVATE ::com::sun::star::uno::Any GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertySimpleEntry* pMap );

private:
    SVX_DLLPRIVATE Cell( SdrTableObj& rTableObj, OutlinerParaObject* pOutlinerParaObject ) throw();
    SVX_DLLPRIVATE virtual ~Cell() throw();

    const SvxItemPropertySet* mpPropSet;

    sdr::properties::TextProperties*    mpProperties;

    ::com::sun::star::table::CellContentType mnCellContentType;

    OUString msFormula;
    double          mfValue;
    ::sal_Int32     mnError;
    ::sal_Bool      mbMerged;
    ::sal_Int32     mnRowSpan;
    ::sal_Int32     mnColSpan;

    Rectangle maCellRect;

    ::com::sun::star::uno::Reference< ::com::sun::star::table::XTable > mxTable;
};



} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
