/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartprettypainter.hxx,v $
 *
 * $Revision: 1.2 $
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
