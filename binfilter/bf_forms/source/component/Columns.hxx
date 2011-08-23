/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FRM_COLUMNS_HXX
#define _FRM_COLUMNS_HXX

#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>
#include "strings.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>
#include "cloneable.hxx"
namespace binfilter {

    using namespace ::comphelper;

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OGridColumn
//==================================================================
typedef ::cppu::WeakAggComponentImplHelper2<	::com::sun::star::container::XChild,
                                                ::com::sun::star::lang::XUnoTunnel > OGridColumn_BASE;
class OGridColumn	:public ::comphelper::OBaseMutex
                    ,public OGridColumn_BASE
                    ,public OPropertySetAggregationHelper
                    ,public OCloneableAggregation
{
protected:
// [properties]
    ::com::sun::star::uno::Any	m_aWidth;					// column width
    ::com::sun::star::uno::Any	m_aAlign;					// column alignment
    ::com::sun::star::uno::Any	m_aHidden;					// column hidden?
// [properties]

    InterfaceRef 				m_xParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation> 
                                m_xAggregate;
    ::rtl::OUString				m_aModelName;

// [properties]
    ::rtl::OUString				m_aLabel;					// Name der Spalte
// [properties]

public:
    OGridColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _sModelName = ::rtl::OUString());
    virtual ~OGridColumn();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OGridControlModel, OGridColumn_BASE);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelImplementationId();
    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<sal_Int8>& _rIdentifier) throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence<sal_Int8>			SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XChild
    virtual InterfaceRef SAL_CALL getParent() throw(::com::sun::star::uno::RuntimeException){return m_xParent;}
    virtual void SAL_CALL setParent(const InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::io::XPersistObject
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream);

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);

// ::com::sun::star::beans::XPropertyState
    virtual	::com::sun::star::beans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual	void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

    const ::rtl::OUString& getModelName() const { return m_aModelName; }

protected:
    static void clearAggregateProperties(::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property>& seqProps, sal_Bool bAllowDropDown);
    static void setOwnProperties(::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property>& seqProps);

    //DECLARE_XCLONEABLE();
};

#define DECL_COLUMN(ClassName)										\
class ClassName														\
    :public OGridColumn												\
    ,public OAggregationArrayUsageHelper< ClassName >				\
{																					\
public:																				\
    ClassName(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);\
                                                                                    \
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);	\
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();				\
    \
    virtual void fillProperties(	\
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,	\
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps	\
        ) const;	\
};


#define IMPL_COLUMN(ClassName, Model, bAllowDropDown)								\
ClassName::ClassName(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)	\
    :OGridColumn(_rxFactory, Model){}											\
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  ClassName::getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException)\
{																					\
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );	\
    return xInfo;																   	\
}																					\
::cppu::IPropertyArrayHelper& ClassName::getInfoHelper()				\
{																					\
    return *const_cast<ClassName*>(this)->getArrayHelper();	\
}	\
void ClassName::fillProperties(	\
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,	\
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps	\
    ) const	\
{	\
    if (m_xAggregateSet.is())	\
    {	\
        _rAggregateProps = m_xAggregateSet->getPropertySetInfo()->getProperties();	\
        clearAggregateProperties(_rAggregateProps, bAllowDropDown);					\
        setOwnProperties(_rProps);												\
    }	\
}	\

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
#define TYPE_CHECKBOX		0
#define TYPE_COMBOBOX		1
#define TYPE_CURRENCYFIELD  2
#define TYPE_DATEFIELD		3
#define TYPE_FORMATTEDFIELD	4
#define TYPE_LISTBOX		5
#define TYPE_NUMERICFIELD	6
#define TYPE_PATTERNFIELD	7
#define TYPE_TEXTFIELD		8
#define TYPE_TIMEFIELD		9

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


}	// namespace frm
}//end of namespace binfilter
#endif // _FRM_COLUMNS_HXX

//.........................................................................

//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
