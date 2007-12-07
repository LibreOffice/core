/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbanames.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:56:16 $
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
#ifndef SC_VBA_NAMES_HXX
#define SC_VBA_NAMES_HXX

#include <org/openoffice/excel/XNames.hpp>
#include <org/openoffice/vba/XCollection.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include "vbacollectionimpl.hxx"

typedef CollTestImplHelper< oo::excel::XNames > ScVbaNames_BASE;

class ScVbaNames : public ScVbaNames_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XNamedRanges > mxNames;

protected:
    virtual css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }

public:
    ScVbaNames( const css::uno::Reference< oo::vba::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XNamedRanges >& xNames , const css::uno::Reference< css::frame::XModel >& xModel );

    ScDocument* getScDocument();
    ScDocShell* getScDocShell();

    virtual ~ScVbaNames();

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& aName ,
                    const css::uno::Any& aRefersTo,
                    const css::uno::Any& aVisible,
                    const css::uno::Any& aMacroType,
                    const css::uno::Any& aShoutcutKey,
                    const css::uno::Any& aCategory,
                    const css::uno::Any& aNameLocal,
                    const css::uno::Any& aRefersToLocal,
                    const css::uno::Any& aCategoryLocal,
                    const css::uno::Any& aRefersToR1C1,
                    const css::uno::Any& aRefersToR1C1Local ) throw (css::uno::RuntimeException);

    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // ScVbaNames_BASE
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};
#endif /* SC_VBA_NAMES_HXX */

