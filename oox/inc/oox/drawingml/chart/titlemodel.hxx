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



#ifndef OOX_DRAWINGML_CHART_TITLEMODEL_HXX
#define OOX_DRAWINGML_CHART_TITLEMODEL_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/chart/datasourcemodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct TextModel
{
    typedef ModelRef< DataSequenceModel >   DataSequenceRef;
    typedef ModelRef< TextBody >            TextBodyRef;

    DataSequenceRef     mxDataSeq;          /// The string data or formula link of this text.
    TextBodyRef         mxTextBody;         /// Rich-formatted literal text (for title objects only).

    explicit            TextModel();
                        ~TextModel();
};

// ============================================================================

struct TitleModel
{
    typedef ModelRef< Shape >       ShapeRef;
    typedef ModelRef< TextBody >    TextBodyRef;
    typedef ModelRef< LayoutModel > LayoutRef;
    typedef ModelRef< TextModel >   TextRef;

    ShapeRef            mxShapeProp;        /// Title shape formatting.
    TextBodyRef         mxTextProp;         /// Title text formatting.
    LayoutRef           mxLayout;           /// Layout/position of the frame.
    TextRef             mxText;             /// Text source of the title.
    bool                mbOverlay;          /// True = title may overlay other objects.

    explicit            TitleModel();
                        ~TitleModel();
};

// ============================================================================

struct LegendModel
{
    typedef ModelRef< Shape >       ShapeRef;
    typedef ModelRef< TextBody >    TextBodyRef;
    typedef ModelRef< LayoutModel > LayoutRef;

    ShapeRef            mxShapeProp;        /// Legend shape formatting.
    TextBodyRef         mxTextProp;         /// Legend text formatting.
    LayoutRef           mxLayout;           /// Layout/position of the legend.
    sal_Int32           mnPosition;         /// Legend position.
    bool                mbOverlay;          /// True = legend may overlay other objects.

    explicit            LegendModel();
                        ~LegendModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
