/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "oox/helper/containerhelper.hxx"
#include "oox/drawingml/textbodyproperties.hxx"
#include "oox/drawingml/textliststyle.hxx"

namespace com { namespace sun { namespace star {
    namespace text { class XText; }
    namespace text { class XTextCursor; }
} } }

namespace oox { namespace core { class XmlFilterBase; } }

namespace oox { namespace drawingml {

class TextParagraph;
typedef RefVector< TextParagraph > TextParagraphVector;

class TextBody
{
public:
    TextBody();
    ~TextBody();

    inline const TextParagraphVector&   getParagraphs() const { return maParagraphs; }
    TextParagraph&                      addParagraph();

    inline const TextListStyle&         getTextListStyle() const { return maTextListStyle; }
    inline TextListStyle&               getTextListStyle() { return maTextListStyle; }

    inline const TextBodyProperties&    getTextProperties() const { return maTextProperties; }
    inline TextBodyProperties&          getTextProperties() { return maTextProperties; }

    /** insert the text body at the text cursor */
    void                insertAt(
                            const ::oox::core::XmlFilterBase& rFilterBase,
                            const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                            const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > & xAt,
                            const TextCharacterProperties& rTextStyleProperties,
                            const TextListStylePtr& pMasterTextListStyle ) const;
protected:
    TextParagraphVector maParagraphs;
    TextBodyProperties  maTextProperties;
    TextListStyle       maTextListStyle;
};

} }

#endif  //  OOX_DRAWINGML_TEXTBODY_HXX
