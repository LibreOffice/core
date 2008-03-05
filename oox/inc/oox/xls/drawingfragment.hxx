/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawingfragment.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:03:08 $
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

#ifndef OOX_XLS_DRAWINGFRAGMENT_HXX
#define OOX_XLS_DRAWINGFRAGMENT_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "oox/drawingml/shape.hxx"
#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct OoxAnchorPosition
{
    sal_Int64           mnX;
    sal_Int64           mnY;

    explicit            OoxAnchorPosition();
    inline bool         isValid() const { return (mnX >= 0) && (mnY >= 0); }
};

// ----------------------------------------------------------------------------

struct OoxAnchorSize
{
    sal_Int64           mnWidth;
    sal_Int64           mnHeight;

    explicit            OoxAnchorSize();
    inline bool         isValid() const { return (mnWidth >= 0) && (mnHeight >= 0); }
};

// ----------------------------------------------------------------------------

struct OoxAnchorCell
{
    sal_Int32           mnCol;
    sal_Int32           mnRow;
    sal_Int64           mnColOffset;
    sal_Int64           mnRowOffset;

    explicit            OoxAnchorCell();
    inline bool         isValid() const { return (mnCol >= 0) && (mnRow >= 0); }
};

// ----------------------------------------------------------------------------

struct OoxAnchorClientData
{
    bool                mbLocksWithSheet;
    bool                mbPrintsWithSheet;

    explicit            OoxAnchorClientData();
};

// ============================================================================

class ShapeAnchor : public WorksheetHelper
{
public:
    explicit            ShapeAnchor( const WorksheetHelper& rHelper );

    void                importAbsoluteAnchor( const AttributeList& rAttribs );
    void                importOneCellAnchor( const AttributeList& rAttribs );
    void                importTwoCellAnchor( const AttributeList& rAttribs );
    void                importPos( const AttributeList& rAttribs );
    void                importExt( const AttributeList& rAttribs );
    void                importClientData( const AttributeList& rAttribs );
    void                setCellPos( sal_Int32 nElement, sal_Int32 nParentContext, const ::rtl::OUString& rValue );

    ::com::sun::star::awt::Rectangle
                        calcApiLocation(
                            const ::com::sun::star::awt::Size& rApiSheetSize,
                            const OoxAnchorSize& rEmuSheetSize ) const;

    ::com::sun::star::awt::Rectangle
                        calcEmuLocation( const OoxAnchorSize& rEmuSheetSize ) const;

private:
    enum AnchorType { ANCHOR_ABSOLUTE, ANCHOR_ONECELL, ANCHOR_TWOCELL, ANCHOR_INVALID };

    AnchorType          meType;
    OoxAnchorPosition   maPos;
    OoxAnchorSize       maSize;
    OoxAnchorCell       maFrom;
    OoxAnchorCell       maTo;
    OoxAnchorClientData maClientData;
    sal_Int32           mnEditAs;
};

typedef ::boost::shared_ptr< ShapeAnchor > ShapeAnchorRef;

// ============================================================================

class OoxDrawingFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxDrawingFragment(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                        mxDrawPage;             /// Drawing page of this sheet.
    ::com::sun::star::awt::Size maApiSheetSize; /// Sheet size in 1/100 mm.
    OoxAnchorSize       maEmuSheetSize;         /// Sheet size in EMU.
    ::oox::drawingml::ShapePtr mxShape;         /// Current top-level shape.
    ShapeAnchorRef      mxAnchor;               /// Current anchor of top-level shape.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

