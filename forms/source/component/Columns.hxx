/*************************************************************************
 *
 *  $RCSfile: Columns.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-19 11:52:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FRM_COLUMNS_HXX
#define _FRM_COLUMNS_HXX

#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#include <comphelper/propagg.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#include "strings.hxx"

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

using namespace comphelper;

//.........................................................................
namespace frm
{
//.........................................................................

    namespace starcontainer = ::com::sun::star::container;
    namespace stario        = ::com::sun::star::io;

//==================================================================
// OGridColumn
//==================================================================
class OGridColumn
            :public ::cppu::OComponentHelper
            ,public OPropertySetAggregationHelper
            ,public starcontainer::XChild
            ,public starlang::XUnoTunnel
{
protected:
// [properties]
    staruno::Any                    m_aWidth;                   // Spaltenbreite
    staruno::Any                    m_aAlign;
    staruno::Any                    m_aHidden;                  // Spalte ist versteckt ?
// [properties]

    ::osl::Mutex                                    m_aMutex;
    InterfaceRef                                    m_xParent;
    staruno::Reference<staruno::XAggregation>       m_xAggregate;
    ::rtl::OUString                                 m_aModelName;

// [properties]
    ::rtl::OUString                 m_aLabel;                   // Name der Spalte

// [properties]

public:
    OGridColumn(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _sModelName = ::rtl::OUString());
    virtual ~OGridColumn();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OGridControlModel, OComponentHelper);
    virtual staruno::Any SAL_CALL queryAggregation( const staruno::Type& _rType ) throw (staruno::RuntimeException);

    static const staruno::Sequence<sal_Int8>& getUnoTunnelImplementationId();
    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const staruno::Sequence<sal_Int8>& _rIdentifier) throw(staruno::RuntimeException);

// XTypeProvider
    virtual staruno::Sequence<sal_Int8>         SAL_CALL getImplementationId() throw(staruno::RuntimeException);
    virtual staruno::Sequence<staruno::Type>    SAL_CALL getTypes() throw(staruno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// starlang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource) throw(staruno::RuntimeException);

// starcontainer::XChild
    virtual InterfaceRef SAL_CALL getParent() throw(staruno::RuntimeException){return m_xParent;}
    virtual void SAL_CALL setParent(const InterfaceRef& Parent) throw(starlang::NoSupportException, staruno::RuntimeException);

// stario::XPersistObject
    virtual void SAL_CALL write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream);

// starbeans::XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(staruno::Any& rConvertedValue, staruno::Any& rOldValue,
                                          sal_Int32 nHandle, const staruno::Any& rValue )
                                        throw(starlang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const staruno::Any& rValue);

// starbeans::XPropertyState
    virtual starbeans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual staruno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

    const ::rtl::OUString& getModelName() const { return m_aModelName; }

protected:
    static void clearAggregateProperties(staruno::Sequence<starbeans::Property>& seqProps, sal_Bool bAllowDropDown);
    static void setOwnProperties(staruno::Sequence<starbeans::Property>& seqProps);
};

#define DECL_COLUMN(ClassName)                                      \
class ClassName                                                     \
    :public OGridColumn                                             \
    ,public OAggregationArrayUsageHelper< ClassName >               \
{                                                                                   \
public:                                                                             \
    ClassName(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);\
                                                                                    \
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException); \
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();             \
    \
    virtual void fillProperties(    \
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,  \
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps  \
        ) const;    \
};


#define IMPL_COLUMN(ClassName, Model, bAllowDropDown)                               \
ClassName::ClassName(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)  \
    :OGridColumn(_rxFactory, Model){}                                           \
staruno::Reference<starbeans::XPropertySetInfo>  ClassName::getPropertySetInfo() throw(staruno::RuntimeException)\
{                                                                                   \
    staruno::Reference<starbeans::XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) ); \
    return xInfo;                                                                   \
}                                                                                   \
::cppu::IPropertyArrayHelper& ClassName::getInfoHelper()                \
{                                                                                   \
    return *const_cast<ClassName*>(this)->getArrayHelper(); \
}   \
void ClassName::fillProperties( \
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,  \
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps  \
    ) const \
{   \
    if (m_xAggregateSet.is())   \
    {   \
        _rAggregateProps = m_xAggregateSet->getPropertySetInfo()->getProperties();  \
        clearAggregateProperties(_rAggregateProps, bAllowDropDown);                 \
        setOwnProperties(_rProps);                                              \
    }   \
}   \

// liste der moeglichen Controls
DECLARE_CONSTASCII_USTRING(FRM_COL_TEXTFIELD);
DECLARE_CONSTASCII_USTRING(FRM_COL_CHECKBOX);
DECLARE_CONSTASCII_USTRING(FRM_COL_COMBOBOX);
DECLARE_CONSTASCII_USTRING(FRM_COL_LISTBOX);
DECLARE_CONSTASCII_USTRING(FRM_COL_NUMERICFIELD);
DECLARE_CONSTASCII_USTRING(FRM_COL_DATEFIELD);
DECLARE_CONSTASCII_USTRING(FRM_COL_TIMEFIELD);
DECLARE_CONSTASCII_USTRING(FRM_COL_CURRENCYFIELD);
DECLARE_CONSTASCII_USTRING(FRM_COL_PATTERNFIELD);
DECLARE_CONSTASCII_USTRING(FRM_COL_FORMATTEDFIELD);


// column type ids
#define TYPE_CHECKBOX       0
#define TYPE_COMBOBOX       1
#define TYPE_CURRENCYFIELD  2
#define TYPE_DATEFIELD      3
#define TYPE_FORMATTEDFIELD 4
#define TYPE_LISTBOX        5
#define TYPE_NUMERICFIELD   6
#define TYPE_PATTERNFIELD   7
#define TYPE_TEXTFIELD      8
#define TYPE_TIMEFIELD      9

// liste aller bekannten columns
const StringSequence& getColumnTypes();
sal_Int32 getColumnTypeByModelName(const ::rtl::OUString& aModelName);

// Columns
DECL_COLUMN(TextFieldColumn)
DECL_COLUMN(PatternFieldColumn)
DECL_COLUMN(DateFieldColumn)
DECL_COLUMN(TimeFieldColumn)
DECL_COLUMN(NumericFieldColumn)
DECL_COLUMN(CurrencyFieldColumn)
DECL_COLUMN(CheckBoxColumn)
DECL_COLUMN(ComboBoxColumn)
DECL_COLUMN(ListBoxColumn)
DECL_COLUMN(FormattedFieldColumn)

#endif // _FRM_COLUMNS_HXX

//.........................................................................
}   // namespace frm
//.........................................................................

