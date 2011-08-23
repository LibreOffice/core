/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef OOX_DRAWINGML_TEXTPARAGRAPH_HXX
#define OOX_DRAWINGML_TEXTPARAGRAPH_HXX

#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XText.hpp>

#include "oox/helper/containerhelper.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/textrun.hxx"
#include "oox/drawingml/textliststyle.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"

namespace oox { namespace drawingml {

typedef RefVector< TextRun > TextRunVector;

class TextParagraph
{
public:
    TextParagraph();
    ~TextParagraph();

    inline TextRunVector&       getRuns() { return maRuns; }
    inline const TextRunVector& getRuns() const { return maRuns; }
    inline void                 addRun( const TextRunPtr & pRun ) { maRuns.push_back( pRun ); }

    inline TextParagraphProperties&         getProperties() { return maProperties; }
    inline const TextParagraphProperties&   getProperties() const { return maProperties; }

    inline TextCharacterProperties&         getEndProperties() { return maEndProperties; }
    inline const TextCharacterProperties&   getEndProperties() const { return maEndProperties; }

    //inline void                        setProperties( TextParagraphPropertiesPtr pProps ) { mpProperties = pProps; }

    void                        insertAt(
                                    const ::oox::core::XmlFilterBase& rFilterBase,
                                    const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                                    const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > &xAt,
                                    const TextCharacterProperties& rTextStyleProperties,
                                    const TextListStyle& rTextListStyle,
                                    bool bFirst = false ) const;

private:
    TextParagraphProperties     maProperties;
    TextCharacterProperties     maEndProperties;
    TextRunVector               maRuns;
};

typedef boost::shared_ptr< TextParagraph > TextParagraphPtr;

} }

#endif  //  OOX_DRAWINGML_TEXTPARAGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
