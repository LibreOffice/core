/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbatextbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:04:01 $
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
#ifndef SC_VBA_TEXTBOX_HXX
#define SC_VBA_TEXTBOX_HXX
#include <cppuhelper/implbase1.hxx>
#include <org/openoffice/msforms/XTextBox.hpp>
#include "vbacontrol.hxx"
#include "vbahelper.hxx"

typedef cppu::ImplInheritanceHelper1< ScVbaControl, oo::msforms::XTextBox > TextBoxImpl_BASE;

class ScVbaTextBox : public TextBoxImpl_BASE
{
public:
    ScVbaTextBox( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::drawing::XControlShape >& xControlShape );
   // Attributes
    virtual rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const rtl::OUString& _text ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getMaxLength() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMaxLength( sal_Int32 _maxlength ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getMultiline() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMultiline( sal_Bool _multiline ) throw (css::uno::RuntimeException);

};
#endif //SC_VBA_TEXTBOX_HXX
