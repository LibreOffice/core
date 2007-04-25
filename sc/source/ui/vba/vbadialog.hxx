/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbadialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:05:33 $
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
#ifndef SC_VBA_DIALOG_HXX
#define SC_VBA_DIALOG_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <org/openoffice/vba/XGlobals.hpp>
#include <org/openoffice/excel/XApplication.hpp>
#include <org/openoffice/excel/XDialog.hpp>

#include "vbahelper.hxx"
#include "vbadialog.hxx"

typedef ::cppu::WeakImplHelper1< oo::excel::XDialog > ScVbaDialog_BASE;

class ScVbaDialog : public ScVbaDialog_BASE
{
    sal_Int32 mnIndex;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
public:
    ScVbaDialog( sal_Int32 nIndex, css::uno::Reference< css::uno::XComponentContext > xContext ):
        mnIndex( nIndex ),
        m_xContext( xContext ) {}
    virtual ~ScVbaDialog() {}

    // Attributes
    virtual css::uno::Reference< oo::excel::XApplication > SAL_CALL getApplication() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getParent() throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getCreator() throw(css::uno::RuntimeException);
    // Methods
    virtual void SAL_CALL Show() throw (css::uno::RuntimeException);
    rtl::OUString mapIndexToName( sal_Int32 nIndex );
};

#endif /* SC_VBA_DIALOG_HXX */
