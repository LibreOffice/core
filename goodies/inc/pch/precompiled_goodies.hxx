/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: precompiled_goodies.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 17:05:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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

// MARKER(update_precomp.py): Generated on 2006-09-01 17:49:44.889563

#ifdef PRECOMPILED_HEADERS

//---MARKER---
#include "sal/types.h"

#include "com/sun/star/awt/FontDescriptor.hpp"
#include "com/sun/star/awt/FontUnderline.hpp"
#include "com/sun/star/awt/FontWeight.hpp"
#include "com/sun/star/awt/Gradient.hpp"
#include "com/sun/star/awt/Rectangle.hpp"
#include "com/sun/star/awt/Size.hpp"
#include "com/sun/star/awt/XBitmap.hpp"
#include "com/sun/star/awt/XDevice.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/drawing/CircleKind.hpp"
#include "com/sun/star/drawing/FillStyle.hpp"
#include "com/sun/star/drawing/FlagSequence.hpp"
#include "com/sun/star/drawing/Hatch.hpp"
#include "com/sun/star/drawing/LineDash.hpp"
#include "com/sun/star/drawing/LineStyle.hpp"
#include "com/sun/star/drawing/PointSequence.hpp"
#include "com/sun/star/drawing/PointSequenceSequence.hpp"
#include "com/sun/star/drawing/PolyPolygonBezierCoords.hpp"
#include "com/sun/star/drawing/TextAdjust.hpp"
#include "com/sun/star/drawing/XDrawPage.hpp"
#include "com/sun/star/drawing/XDrawPages.hpp"
#include "com/sun/star/drawing/XDrawPagesSupplier.hpp"
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/drawing/XShapeGrouper.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/graphic/GraphicType.hpp"
#include "com/sun/star/graphic/XGraphic.hpp"
#include "com/sun/star/graphic/XGraphicProvider.hpp"
#include "com/sun/star/graphic/XGraphicRenderer.hpp"
#include "com/sun/star/io/XStream.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/style/HorizontalAlignment.hpp"
#include "com/sun/star/task/XStatusIndicator.hpp"
#include "com/sun/star/text/XText.hpp"
#include "com/sun/star/text/XTextRange.hpp"

#include "comphelper/processfactory.hxx"
#include "comphelper/propertysethelper.hxx"
#include "comphelper/propertysetinfo.hxx"

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/servicefactory.hxx"


#include "osl/diagnose.h"
#include "osl/endian.h"
#include "osl/file.hxx"
#include "osl/process.h"

#include "rtl/alloc.h"
#include "rtl/math.hxx"
#include "rtl/ustring.hxx"
#include "rtl/uuid.h"

#include "svtools/FilterConfigItem.hxx"
#include "svtools/cacheoptions.hxx"
#include "svtools/fltcall.hxx"
#include "svtools/itemprop.hxx"
#include "svtools/lstner.hxx"
#include "svtools/options3d.hxx"
#include "svtools/solar.hrc"
#include "svtools/stdctrl.hxx"
#include "svtools/svarray.hxx"

#include "toolkit/helper/vclunohelper.hxx"

#include "tools/agapi.hxx"
#include "tools/agitem.hxx"
#include "tools/bigint.hxx"
#include "tools/chapi.hxx"
#include "tools/color.hxx"
#include "tools/datetime.hxx"
#include "tools/debug.hxx"
#include "tools/gen.hxx"
#include "tools/list.hxx"
#include "tools/poly.hxx"
#include "tools/rcid.h"
#include "tools/ref.hxx"
#include "tools/resid.hxx"
#include "tools/resmgr.hxx"
#include "tools/rtti.hxx"
#include "tools/shl.hxx"
#include "tools/solar.h"
#include "tools/string.hxx"
#include "tools/table.hxx"
#include "tools/tempfile.hxx"
#include "tools/time.hxx"
#include "tools/urlobj.hxx"
#include "tools/vcompat.hxx"
#include "tools/vector2d.hxx"

#include "uno/mapping.hxx"

#include "unotools/localfilehelper.hxx"
#include "unotools/processfactory.hxx"

#include "vcl/alpha.hxx"
#include "vcl/animate.hxx"
#include "vcl/bitmap.hxx"
#include "vcl/bitmapex.hxx"
#include "vcl/bmpacc.hxx"
#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/field.hxx"
#include "vcl/fixed.hxx"
#include "vcl/floatwin.hxx"
#include "vcl/font.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/gradient.hxx"
#include "vcl/graph.h"
#include "vcl/hatch.hxx"
#include "vcl/image.hxx"
#include "vcl/imagerepository.hxx"
#include "vcl/lineinfo.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/metric.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/opengl.hxx"
#include "vcl/outdev.hxx"
#include "vcl/outdev3d.hxx"
#include "vcl/region.hxx"
#include "vcl/salbtype.hxx"
#include "vcl/sv.h"
#include "vcl/svapp.hxx"
#include "vcl/timer.hxx"
#include "vcl/virdev.hxx"
#include "vcl/window.hxx"

#include "vos/macros.hxx"
#include "vos/mutex.hxx"
#include "vos/timer.hxx"
//---MARKER---

#endif
