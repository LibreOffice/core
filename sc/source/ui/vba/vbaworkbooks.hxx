/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaworkbooks.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:13:24 $
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
#ifndef SC_VBA_WORKBOOKS_HXX
#define SC_VBA_WORKBOOKS_HXX


#include "vbacollectionimpl.hxx"
#include <org/openoffice/vba/XGlobals.hpp>
#include <org/openoffice/excel/XWorkbooks.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include "vbahelper.hxx"


class ScModelObj;

typedef ::cppu::ImplInheritanceHelper1< ScVbaCollectionBaseImpl, oo::excel::XWorkbooks > ScVbaWorkbooks_BASE;

class ScVbaWorkbooks : public ScVbaWorkbooks_BASE
{
private:
    css::uno::Reference< oo::vba::XGlobals > getGlobals() throw (css::uno::RuntimeException);
    bool    isTextFile( const rtl::OUString& rString );
    static sal_Int16& getCurrentDelim(){ static sal_Int16 nDelim = 44; return nDelim; }
public:
    ScVbaWorkbooks( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~ScVbaWorkbooks() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // XWorkbooks
    virtual css::uno::Any SAL_CALL Add() throw (css::uno::RuntimeException);
    virtual void SAL_CALL Close(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Open( const ::rtl::OUString& Filename, const css::uno::Any& UpdateLinks, const css::uno::Any& ReadOnly, const css::uno::Any& Format, const css::uno::Any& Password, const css::uno::Any& WriteResPassword, const css::uno::Any& IgnoreReadOnlyRecommended, const css::uno::Any& Origin, const css::uno::Any& Delimiter, const css::uno::Any& Editable, const css::uno::Any& Notify, const css::uno::Any& Converter, const css::uno::Any& AddToMru ) throw (css::uno::RuntimeException);
};

#endif /* SC_VBA_WORKBOOKS_HXX */
