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



#ifndef OOX_DRAWINGML_TEXTLISTSTYLE_HXX
#define OOX_DRAWINGML_TEXTLISTSTYLE_HXX

#include "oox/drawingml/textparagraphproperties.hxx"
#include "oox/helper/refvector.hxx"

namespace oox { namespace drawingml {

typedef RefVector< TextParagraphProperties > TextParagraphPropertiesVector;

class TextListStyle
{
public:

    TextListStyle();
    ~TextListStyle();

    void apply( const TextListStyle& rTextListStyle );

    inline const TextParagraphPropertiesVector& getListStyle() const { return maListStyle; };
    inline TextParagraphPropertiesVector&       getListStyle() { return maListStyle; };

    inline const TextParagraphPropertiesVector& getAggregationListStyle() const { return maAggregationListStyle; };
    inline TextParagraphPropertiesVector&       getAggregationListStyle() { return maAggregationListStyle; };

protected:

    TextParagraphPropertiesVector maListStyle;
    TextParagraphPropertiesVector maAggregationListStyle;
};

typedef boost::shared_ptr< TextListStyle > TextListStylePtr;

} }

#endif  //  OOX_DRAWINGML_TEXTLISTSTYLE_HXX
