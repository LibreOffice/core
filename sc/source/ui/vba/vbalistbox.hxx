/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbalistbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:55:31 $
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
#ifndef SC_VBA_LISTBOX_HXX
#define SC_VBA_LISTBOX_HXX
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <org/openoffice/msforms/XListBox.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "vbacontrol.hxx"
#include "vbapropvalue.hxx"
#include "vbahelper.hxx"

typedef cppu::ImplInheritanceHelper2<ScVbaControl, oo::msforms::XListBox, css::script::XDefaultProperty > ListBoxImpl_BASE;
class ScVbaListBox : public ListBoxImpl_BASE
    ,public PropListener
{
    rtl::OUString sSourceName;
    rtl::OUString msDftPropName;

    sal_Int16 m_nIndex;

public:
    ScVbaListBox( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::drawing::XControlShape >& xControlShape );
    ScVbaListBox( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::beans::XPropertySet >& xProps,
                    const css::uno::Reference< css::drawing::XControlShape> xControlShape );


    // Attributes
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const ::rtl::OUString& _text ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getMultiSelect() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMultiSelect( sal_Bool _multiselect ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Selected( ::sal_Int32 index ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL AddItem( const css::uno::Any& pvargItem, const css::uno::Any& pvargIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Clear(  ) throw (css::uno::RuntimeException);


    // XDefaultProperty
    rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return ::rtl::OUString::createFromAscii("Value"); }

    //PropListener
    virtual void setValueEvent( const css::uno::Any& value );
    virtual css::uno::Any getValueEvent();


};

#endif //
