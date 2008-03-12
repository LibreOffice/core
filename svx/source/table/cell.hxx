/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cell.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:59:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_CELL_HXX_
#define _SVX_CELL_HXX_

#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>

#include <rtl/ref.hxx>

#include "svx/sdtaitm.hxx"
#include "tablemodel.hxx"
#include "svx/unotext.hxx"
#include "svx/svdtext.hxx"

// -----------------------------------------------------------------------------

class SfxItemSet;
class OutlinerParaObject;
struct SfxItemPropertyMap;
namespace sdr { namespace properties {
    class TextProperties;
} }

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

// -----------------------------------------------------------------------------

class Cell :    public SdrText,
                public SvxUnoTextBase,
                public ::com::sun::star::table::XMergeableCell,
                public ::com::sun::star::awt::XLayoutConstrains,
                public ::com::sun::star::beans::XMultiPropertyStates,
                public ::cppu::OWeakObject
{
    friend class CellUndo;

public:
    Cell( SdrTableObj& rTableObj, OutlinerParaObject* pOutlinerParaObject ) throw();
    virtual ~Cell() throw();

    // private
    void dispose();

    // SdrTextShape proxy
    bool IsTextEditActive();
    bool hasText() const;

    void cloneFrom( const CellRef& rCell );

    void setCellRect( ::Rectangle& rCellRect ) { maCellRect = rCellRect; }
    const ::Rectangle& getCellRect() const { return maCellRect; }
    ::Rectangle& getCellRect() { return maCellRect; }

    OutlinerParaObject* GetEditOutlinerParaObject() const;
    void SetStyleSheet( SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr );
    virtual SfxStyleSheet* GetStyleSheet() const;
    SfxStyleSheetPool* GetStyleSheetPool() const;
    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual void TakeTextAnchorRect(Rectangle& rAnchorRect) const;

    const SfxItemSet& GetItemSet() const;
    void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, sal_Bool bClearAllItems);
    void SetMergedItem(const SfxPoolItem& rItem);

    sal_Int32 getMinimumWidth();
    sal_Int32 getMinimumHeight();

    long GetTextLeftDistance() const;
    long GetTextRightDistance() const;
    long GetTextUpperDistance() const;
    long GetTextLowerDistance() const;

    SdrTextVertAdjust GetTextVerticalAdjust() const;
    SdrTextHorzAdjust GetTextHorizontalAdjust() const;

    virtual void SetModel(SdrModel* pNewModel);

    void merge( sal_Int32 nColumnSpan, sal_Int32 nRowSpan );
    void mergeContent( const CellRef& xSourceCell );
    void replaceContentAndFormating( const CellRef& xSourceCell );

    void setMerged();

    sal_Bool hasLevels();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& Type ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XLayoutConstrains
    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw (::com::sun::star::uno::RuntimeException);

    // XMergeableCell
    virtual ::sal_Int32 SAL_CALL getRowSpan() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getColumnSpan() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isMerged() throw (::com::sun::star::uno::RuntimeException);

    // XCell
    virtual ::rtl::OUString SAL_CALL getFormula() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFormula( const ::rtl::OUString& aFormula ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setValue( double nValue ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellContentType SAL_CALL getType() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getError() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertiesToDefault( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyDefaults( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue( ::sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XText
    virtual void SAL_CALL insertTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent, ::sal_Bool bAbsorb ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursor(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursorByRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& aTextPosition ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertString( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const ::rtl::OUString& aString, ::sal_Bool bAbsorb ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertControlCharacter( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, ::sal_Int16 nControlCharacter, ::sal_Bool bAbsorb ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const ::rtl::OUString& aString ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SetOutlinerParaObject( OutlinerParaObject* pTextObject );

    void    AddUndo();

    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    sdr::properties::TextProperties* CloneProperties( SdrObject& rNewObj, Cell& rNewCell );

    void notifyModified();

protected:
    virtual const SfxItemSet& GetObjectItemSet();
    virtual void SetObjectItem(const SfxPoolItem& rItem);

    ::com::sun::star::uno::Any GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertyMap* pMap );

private:
    SvxItemPropertySet maPropSet;

    sdr::properties::TextProperties*    mpProperties;

    ::com::sun::star::table::CellContentType mnCellContentType;

    ::rtl::OUString msFormula;
    double          mfValue;
    ::sal_Int32     mnError;
    ::sal_Bool      mbMerged;
    ::sal_Int32     mnRowSpan;
    ::sal_Int32     mnColSpan;

    Rectangle maCellRect;

    ::com::sun::star::uno::Reference< ::com::sun::star::table::XTable > mxTable;
};

// -----------------------------------------------------------------------------

} }

#endif
