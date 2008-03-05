/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawingmltypes.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:39:46 $
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

#ifndef OOX_DRAWINGML_TYPES_HXX
#define OOX_DRAWINGML_TYPES_HXX

#include <com/sun/star/style/TabAlign.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "oox/drawingml/shape.hxx"
#include "oox/core/contexthandler.hxx"

namespace oox { namespace core {
    class PropertyMap;
} }

namespace oox { namespace drawingml {

/** converts the attributes from an CT_Point2D into an awt Point with 1/100th mm */
com::sun::star::awt::Point GetPoint2D( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

/** converts the attributes from an CT_TLPoint into an awt Point with 1/1000% */
com::sun::star::awt::Point GetPointPercent( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs );


/** converts the attributes from an CT_Size2D into an awt Size with 1/100th mm */
com::sun::star::awt::Size GetSize2D( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

/** converts the attributes from a CT_RelativeRect to an IntegerRectangle2D */
com::sun::star::geometry::IntegerRectangle2D GetRelativeRect( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

/** converts an emu string into 1/100th mmm but constrain as per ST_TextMargin
 * see 5.1.12.73
 */
sal_Int32 GetTextMargin( const ::rtl::OUString& sValue );

/** converts an emu string into 1/100th mmm */
sal_Int32 GetCoordinate( const ::rtl::OUString& sValue );

/** converts a ST_Percentage % string into 1/1000th of % */
sal_Int32 GetPercent( const ::rtl::OUString& sValue );

/** Converts a ST_PositiveFixedPercentage to a float. 1.0 == 100% */
double GetPositiveFixedPercentage( const ::rtl::OUString& sValue );

/** converts the ST_TextFontSize to point */
float GetTextSize( const ::rtl::OUString& rValue );

/** converts the ST_TextSpacingPoint to 1/100mm */
sal_Int32 GetTextSpacingPoint(  const ::rtl::OUString& sValue );

/** */
::com::sun::star::style::TabAlign GetTabAlign( ::sal_Int32 aToken );

sal_Int16 GetFontUnderline( ::sal_Int32 nToken );

sal_Int16 GetFontStrikeout( sal_Int32 nToken );

sal_Int16 GetCaseMap( sal_Int32 nToken );

/** converts pitchFamily from CT_TextFont */
void GetFontPitch( sal_Int32 nOoxValue, sal_Int16 & nPitch, sal_Int16 & nFamily );

/** converts a paragraph align to a ParaAdjust */
sal_Int16 GetParaAdjust( sal_Int32 nAlign );

// CT_IndexRange
struct IndexRange {
    sal_Int32 start;
    sal_Int32 end;
};

/** retrieve the content of CT_IndexRange */
IndexRange GetIndexRange( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );


/** context to import a CT_Transform2D */
class Transform2DContext : public ::oox::core::ContextHandler
{
public:
    Transform2DContext( ::oox::core::ContextHandler& rParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, Shape& rShape ) throw();
    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    Shape&              mrShape;
};

} }

#endif // OOX_DRAWINGML_TYPES_HXX
