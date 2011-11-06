/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OOX_DRAWINGML_TEXTPARAGRAPH_HXX
#define OOX_DRAWINGML_TEXTPARAGRAPH_HXX

#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XText.hpp>

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
