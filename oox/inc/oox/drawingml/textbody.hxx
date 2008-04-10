/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textbody.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
