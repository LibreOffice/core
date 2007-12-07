/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaname.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:55:55 $
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
#ifndef SC_VBA_NAME_HXX
#define SC_VBA_NAME_HXX

#include <org/openoffice/excel/XName.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

#include "vbahelperinterface.hxx"

typedef InheritedHelperInterfaceImpl1< oo::excel::XName > NameImpl_BASE;

class ScVbaName : public NameImpl_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XNamedRange > mxNamedRange;
    css::uno::Reference< css::sheet::XNamedRanges > mxNames;

    ScDocument * m_pDoc;

protected:
    virtual css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }
    virtual css::uno::Reference< oo::excel::XWorksheet > getWorkSheet() throw (css::uno::RuntimeException);

public:
    ScVbaName( const css::uno::Reference< oo::vba::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XNamedRange >& xName , const css::uno::Reference< css::sheet::XNamedRanges >& xNames , const css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~ScVbaName();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString &rName ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameLocal() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setNameLocal( const ::rtl::OUString &rName ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const ::rtl::OUString &rValue ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRefersTo() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersTo( const ::rtl::OUString &rRefersTo ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRefersToLocal() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersToLocal( const ::rtl::OUString &rRefersTo ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRefersToR1C1() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersToR1C1( const ::rtl::OUString &rRefersTo ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRefersToR1C1Local() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersToR1C1Local( const ::rtl::OUString &rRefersTo ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL getRefersToRange() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersToRange( const css::uno::Reference< oo::excel::XRange > xRange ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SC_VBA_NAME_HXX */

