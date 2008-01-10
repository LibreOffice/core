/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TDefTableHandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:42:19 $
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
#ifndef INCLUDED_TDEFTABLEHANDLER_HXX
#define INCLUDED_TDEFTABLEHANDLER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <resourcemodel/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#include <vector>
namespace com{ namespace sun{ namespace star{namespace table {
    struct BorderLine;
}}}}

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
class WRITERFILTER_DLLPRIVATE TDefTableHandler : public Properties
{
public:

private:
    ::std::vector<sal_Int32>                                m_aCellBorderPositions;
    ::std::vector<sal_Int32>                                m_aCellVertAlign;

    ::std::vector< ::com::sun::star::table::BorderLine >    m_aLeftBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aRightBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aTopBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine >    m_aBottomBorderLines;

    //values of the current border
    sal_Int32                                           m_nLineWidth;
    sal_Int32                                           m_nLineType;
    sal_Int32                                           m_nLineColor;
    sal_Int32                                           m_nLineDistance;

    bool                                                m_bOOXML;

    void localResolve(Id Name, writerfilter::Reference<Properties>::Pointer_t pProperties);
public:
    TDefTableHandler( bool bOOXML );
    virtual ~TDefTableHandler();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    size_t                                      getCellCount() const;
    ::boost::shared_ptr<PropertyMap>            getCellProperties( size_t nCell ) const;
    ::boost::shared_ptr<PropertyMap>            getRowProperties() const;
    sal_Int32                                   getTableWidth() const;
};
typedef boost::shared_ptr< TDefTableHandler >          TDefTableHandlerPtr;
}}

#endif //
