/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbashape.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:02:09 $
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
#ifndef SC_VBA_SHAPE_HXX
#define SC_VBA_SHAPE_HXX

#include <org/openoffice/office/MsoShapeType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <org/openoffice/msforms/XShape.hpp>
#include <org/openoffice/msforms/XLineFormat.hpp>
#include <cppuhelper/implbase2.hxx>

#include "vbahelperinterface.hxx"

typedef  ::cppu::WeakImplHelper2< oo::msforms::XShape, css::lang::XEventListener > ListeningShape;

typedef InheritedHelperInterfaceImpl< ListeningShape > ScVbaShape_BASE;

class ScVbaShape : public ScVbaShape_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 m_nType;
    css::uno::Any m_aRange;
protected:
    virtual void addListeners();
    virtual void removeShapeListener() throw( css::uno::RuntimeException );
    virtual void removeShapesListener() throw( css::uno::RuntimeException );
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
public:
    ScVbaShape( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape > xShape, const css::uno::Reference< css::drawing::XShapes > xShapes, sal_Int32 nType ) throw ( css::lang::IllegalArgumentException );
    ScVbaShape( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape > xShape ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaShape();
    css::uno::Any getRange() { return m_aRange; };
    void setRange( css::uno::Any aRange ) { m_aRange = aRange; };

    static sal_Int32 getType( const css::uno::Reference< css::drawing::XShape > xShape ) throw (css::uno::RuntimeException);

    // Attributes
    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const rtl::OUString& _name ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeight( double _height ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWidth( double _width ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeft( double _left ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTop( double _top ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getZOrderPosition() throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException);
    virtual double SAL_CALL getRotation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRotation( double _rotation ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::msforms::XLineFormat > SAL_CALL getLine() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::msforms::XFillFormat > SAL_CALL getFill() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::msforms::XPictureFormat > SAL_CALL getPictureFormat() throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Reference< oo::excel::XTextFrame > SAL_CALL TextFrame(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException);
    virtual void SAL_CALL ZOrder( sal_Int32 ZOrderCmd ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementRotation( double Increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementLeft( double Increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementTop( double Increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ScaleHeight( double Factor, sal_Bool RelativeToOriginalSize, sal_Int32 Scale ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ScaleWidth( double Factor, sal_Bool RelativeToOriginalSize, sal_Int32 Scale ) throw (css::uno::RuntimeException);
    // Replace??
    virtual void SAL_CALL Select( const css::uno::Any& Replace ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL ShapeRange( const css::uno::Any& index ) throw ( css::uno::RuntimeException );
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEventObject ) throw( css::uno::RuntimeException );
};
#endif//SC_VBA_SHAPE_HXX
