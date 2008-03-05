/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BorderHandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:49:04 $
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
#ifndef INCLUDED_BORDERHANDLER_HXX
#define INCLUDED_BORDERHANDLER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <resourcemodel/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
class WRITERFILTER_DLLPRIVATE BorderHandler : public Properties
{
public:
    //todo: order is a guess
    enum BorderPosition
    {
        BORDER_TOP,
        BORDER_LEFT,
        BORDER_BOTTOM,
        BORDER_RIGHT,
        BORDER_HORIZONTAL,
        BORDER_VERTICAL,
        BORDER_COUNT
    };

private:
    sal_Int8        m_nCurrentBorderPosition;
    //values of the current border
    sal_Int32       m_nLineWidth;
    sal_Int32       m_nLineType;
    sal_Int32       m_nLineColor;
    sal_Int32       m_nLineDistance;
    bool            m_bOOXML;

    ::com::sun::star::table::BorderLine         m_aBorderLines[BORDER_COUNT];

public:
    BorderHandler( bool bOOXML );
    virtual ~BorderHandler();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    ::boost::shared_ptr<PropertyMap>            getProperties();
    ::com::sun::star::table::BorderLine         getBorderLine();
    sal_Int32                                   getLineDistance() const { return m_nLineDistance;}
};
typedef boost::shared_ptr< BorderHandler >          BorderHandlerPtr;
}}

#endif //
