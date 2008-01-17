/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textliststyle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:45 $
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

#ifndef OOX_DRAWINGML_TEXTLISTSTYLE_HXX
#define OOX_DRAWINGML_TEXTLISTSTYLE_HXX

#include "oox/drawingml/textparagraphproperties.hxx"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace oox { namespace drawingml {

class TextListStyle;

typedef boost::shared_ptr< TextListStyle > TextListStylePtr;

class TextListStyle
{
public:

    TextListStyle();
    ~TextListStyle();

    void apply( const oox::drawingml::TextListStylePtr& rTextListStylePtr );

    std::vector< ::oox::drawingml::TextParagraphPropertiesPtr >& getListStyle() { return maListStyle; };
    std::vector< ::oox::drawingml::TextParagraphPropertiesPtr >& getAggregationListStyle() { return maAggregationListStyle; };

protected:

    std::vector< ::oox::drawingml::TextParagraphPropertiesPtr > maListStyle;
    std::vector< ::oox::drawingml::TextParagraphPropertiesPtr > maAggregationListStyle;
};

} }

#endif  //  OOX_DRAWINGML_TEXTLISTSTYLE_HXX
