/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagram.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:52:17 $
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



#ifndef OOX_DRAWINGML_DIAGRAM_HXX
#define OOX_DRAWINGML_DIAGRAM_HXX

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <rtl/ustring.hxx>

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/diagram/diagramlayoutatoms.hxx"
#include "oox/drawingml/fillproperties.hxx"

namespace oox { namespace drawingml {

namespace dgm {

/** A Connection
 */
class Connection
{
public:
    Connection()
        : mnType( 0 )
        , mnSourceOrder( 0 )
        , mnDestOrder( 0 )
        {
        }

    void dump();

    sal_Int32 mnType;
    ::rtl::OUString msModelId;
    ::rtl::OUString msSourceId;
    ::rtl::OUString msDestId;
    ::rtl::OUString msParTransId;
    ::rtl::OUString msPresId;
    ::rtl::OUString msSibTransId;
    sal_Int32 mnSourceOrder;
    sal_Int32 mnDestOrder;

};

typedef boost::shared_ptr< Connection > ConnectionPtr;
typedef std::vector< ConnectionPtr > Connections;

class Point;

typedef boost::shared_ptr< Point > PointPtr;
typedef std::vector< PointPtr >      Points;
/** A point
 */
class Point
{
public:
    Point();
    ShapePtr & getShape( )
        { return mpShape; }

    void setCnxId( const ::rtl::OUString & sCnxId )
        { msCnxId = sCnxId; }
    void setModelId( const ::rtl::OUString & sModelId );
    const ::rtl::OUString & getModelId() const
        { return msModelId; }
    void setType( const sal_Int32 nType )
        { mnType = nType; }
    sal_Int32 getType() const
        { return mnType; }

    void dump();
private:
    ShapePtr        mpShape;
    ::rtl::OUString msCnxId;
    ::rtl::OUString msModelId;
    sal_Int32       mnType;
};


class PointsTree;
typedef boost::shared_ptr< PointsTree > PointsTreePtr;

/** a points tree node */
class PointsTree
    : public boost::enable_shared_from_this< PointsTree >
{
public:
    typedef std::vector< PointsTreePtr > Childrens;
    PointsTree()
        {};
    PointsTree( const PointPtr & pPoint )
        : mpNode( pPoint )
        { }
    bool addChild( const PointsTreePtr & pChild );
    const PointPtr & getPoint() const
        { return mpNode; }
    PointsTreePtr getParent() const;
    Childrens::const_iterator beginChild() const
        { return maChildrens.begin(); }
    Childrens::const_iterator endChild() const
        { return maChildrens.end(); }
private:
    PointPtr                           mpNode;
    boost::weak_ptr< PointsTree >      mpParent;
    Childrens       maChildrens;
};

}

////////////////////

class DiagramData
{
public:

    DiagramData();
    FillPropertiesPtr & getFillProperties()
        { return mpFillProperties; }
    dgm::Connections & getConnections()
        { return maConnections; }
    dgm::Points & getPoints()
        { return maPoints; }
    void dump();
private:
    FillPropertiesPtr   mpFillProperties;
    dgm::Connections    maConnections;
    dgm::Points         maPoints;
};

typedef boost::shared_ptr< DiagramData > DiagramDataPtr;



////////////////////

class DiagramLayout
{
public:
    void setDefStyle( const ::rtl::OUString & sDefStyle )
        { msDefStyle = sDefStyle; }
    void setMinVer( const ::rtl::OUString & sMinVer )
        { msMinVer = sMinVer; }
    void setUniqueId( const ::rtl::OUString & sUniqueId )
        { msUniqueId = sUniqueId; }
    const ::rtl::OUString & getUniqueId()
        { return msUniqueId; }
    void setTitle( const ::rtl::OUString & sTitle )
        { msTitle = sTitle; }
    void setDesc( const ::rtl::OUString & sDesc )
        { msDesc = sDesc; }

    LayoutNodePtr & getNode()
        { return mpNode; }
    const LayoutNodePtr & getNode() const
        { return mpNode; }
    DiagramDataPtr & getSampData()
        { return mpSampData; }
    const DiagramDataPtr & getSampData() const
        { return mpSampData; }
    DiagramDataPtr & getStyleData()
        { return mpStyleData; }
    const DiagramDataPtr & getStyleData() const
        { return mpStyleData; }

    void layout( const dgm::PointsTreePtr & pTree, const com::sun::star::awt::Point & pt );
private:
    ::rtl::OUString msDefStyle;
    ::rtl::OUString msMinVer;
    ::rtl::OUString msUniqueId;

    ::rtl::OUString msTitle;
    ::rtl::OUString msDesc;
    LayoutNodePtr  mpNode;
    DiagramDataPtr mpSampData;
    DiagramDataPtr mpStyleData;
    // TODO
    // catLst
    // clrData
};

typedef boost::shared_ptr< DiagramLayout > DiagramLayoutPtr;

///////////////////////

class DiagramQStyles
{

};

typedef boost::shared_ptr< DiagramQStyles > DiagramQStylesPtr;

///////////////////////

class DiagramColors
{

};

typedef boost::shared_ptr< DiagramColors > DiagramColorsPtr;

///////////////////////

class Diagram
{
public:
    void setData( const DiagramDataPtr & );
    void setLayout( const DiagramLayoutPtr & );
    DiagramLayoutPtr getLayout() const
        {
            return mpLayout;
        }
    void setQStyles( const DiagramQStylesPtr & );
    void setColors( const DiagramColorsPtr & );

    void addTo( const ShapePtr & pShape );
    ::rtl::OUString getLayoutId() const;
private:
    void build( );
    DiagramDataPtr    mpData;
    DiagramLayoutPtr  mpLayout;
    DiagramQStylesPtr mpQStyles;
    DiagramColorsPtr  mpColors;
    std::map< ::rtl::OUString, ShapePtr > maShapeMap;
    dgm::PointsTreePtr  mpRoot;
};


typedef boost::shared_ptr< Diagram > DiagramPtr;

} }

#endif
