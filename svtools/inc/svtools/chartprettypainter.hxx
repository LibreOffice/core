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



#ifndef CHARTPRETTYPAINTER_HXX
#define CHARTPRETTYPAINTER_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

//#include <svtools/embedhlp.hxx>
#include <vcl/outdev.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>

// predeclarations
namespace svt { class EmbeddedObjectRef; }

/** use this class for a correct view representation of charts
see issues #i82893#, #i75867#: charts must be painted resolution dependent!!

example usage when painting ole objects:
if( ChartPrettyPainter::IsChart(xObjRef) && ChartPrettyPainter::ShouldPrettyPaintChartOnThisDevice( pOutDev )
    && ChartPrettyPainter::DoPrettyPaintChart( get/loadChartModel(), pOutDev, rLogicObjectRect ) )
    return;
else
    use any other painting method for charts or paint other ole objects

this way of usage ensures that ole objects are only loaded if necessary
*/

class SVT_DLLPUBLIC ChartPrettyPainter
{
public:
    static bool IsChart( const svt::EmbeddedObjectRef& xObjRef );
    static bool ShouldPrettyPaintChartOnThisDevice( OutputDevice* pOutDev );
    static bool DoPrettyPaintChart( ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel > xChartModel,
                        OutputDevice* pOutDev, const Rectangle& rLogicObjectRect );

    ChartPrettyPainter();
    virtual ~ChartPrettyPainter();
    virtual bool DoPaint( OutputDevice* pOutDev, const Rectangle& rLogicObjectRect ) const;
    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
};

#endif
