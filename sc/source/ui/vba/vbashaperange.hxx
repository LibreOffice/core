/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbashaperange.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:02:37 $
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
#ifndef SC_VBA_SHAPERANGE_HXX
#define SC_VBA_SHAPERANGE_HXX

#include <com/sun/star/drawing/XShapes.hpp>
#include <org/openoffice/msforms/XShapeRange.hpp>

#include "vbacollectionimpl.hxx"

typedef CollTestImplHelper< oo::msforms::XShapeRange > ScVbaShapeRange_BASE;

class ScVbaShapeRange : public ScVbaShapeRange_BASE
{
private:
    css::uno::Reference< css::drawing::XDrawPage > m_xDrawPage;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    sal_Int32 m_nShapeGroupCount;
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    css::uno::Reference< css::drawing::XShapes > getShapes() throw (css::uno::RuntimeException) ;
public:
    ScVbaShapeRange( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xShapes, const css::uno::Reference< css::drawing::XDrawPage>& xDrawShape );

    // Methods
    virtual void SAL_CALL Select(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ::org::openoffice::msforms::XShape > SAL_CALL Group() throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementRotation( double Increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementLeft( double Increment ) throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL IncrementTop( double Increment ) throw (css::uno::RuntimeException);
    //XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
};

#endif//SC_VBA_SHAPERANGE_HXX
