/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textbody.hxx,v $
 * $Revision: 1.5 $
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

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

#include "oox/helper/containerhelper.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/textliststyle.hxx"

namespace oox { namespace drawingml {

typedef RefVector< TextParagraph > TextParagraphVector;

class TextBody
{
public:
    TextBody();
    ~TextBody();

    inline const TextParagraphVector&   getParagraphs() const { return maParagraphs; }
    TextParagraph&                      addParagraph();

    inline const TextListStyle& getTextListStyle() const { return maTextListStyle; }
    inline TextListStyle&       getTextListStyle() { return maTextListStyle; }

    inline const PropertyMap&   getTextProperties() const { return maTextProperties; }
    inline PropertyMap&         getTextProperties() { return maTextProperties; }

    /** insert the text body at the text cursor */
    void                insertAt(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                            const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > & xAt,
                            const TextListStylePtr& pMasterTextListStyle );
protected:
    TextParagraphVector maParagraphs;
    PropertyMap         maTextProperties;
    TextListStyle       maTextListStyle;
};

typedef boost::shared_ptr< TextBody > TextBodyPtr;

} }

#endif  //  OOX_DRAWINGML_TEXTBODY_HXX
