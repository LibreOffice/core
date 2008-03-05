/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textbody.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:43:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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


#ifndef OOX_DRAWINGML_TEXTBODY_HXX
#define OOX_DRAWINGML_TEXTBODY_HXX

#include <vector>
#include <boost/shared_ptr.hpp>

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/textliststyle.hxx"

namespace oox { namespace drawingml {

class TextBody
{
public:

    TextBody();
    ~TextBody();

    const std::vector< TextParagraphPtr > & getParagraphs() const { return maParagraphs; }
    ::oox::drawingml::TextListStylePtr      getTextListStyle() const { return mpTextListStyle; }

    void                              addParagraph( const TextParagraphPtr & pPara ) { maParagraphs.push_back( pPara ); }
    /** insert the text body at the text cursor */
    void                insertAt(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                            const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > & xAt,
                            const TextListStylePtr& pMasterTextListStyle );
protected:
    std::vector< TextParagraphPtr > maParagraphs;
    TextListStylePtr    mpTextListStyle;
};

typedef boost::shared_ptr< TextBody > TextBodyPtr;

} }

#endif  //  OOX_DRAWINGML_TEXTBODY_HXX
