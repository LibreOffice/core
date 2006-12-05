/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: precompiled_sc.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 14:26:12 $
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

// MARKER(update_precomp.py): Generated on 2006-07-11 15:52:42.937361

#ifdef PRECOMPILED_HEADERS
#include "scitems.hxx"

#include <algorithm>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <iosfwd>
#include <limits.h>
#include <limits>
#include <list>
#include <math.h>
#include <memory>
#include <new>
#include <cfloat>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <config/_epilog.h>
#include <config/_msvc_warnings_off.h>
#include <config/_prolog.h>
#include <cppu/macros.hxx>
#include <cppuhelper/weakref.hxx>
#include <cstddef>
#include <cwchar>
#include <float.h>
#include <functional>
#include <goodies/b3dcolor.hxx>
#include <goodies/b3dcompo.hxx>
#include <goodies/b3dentty.hxx>
#include <goodies/b3dlight.hxx>
#include <goodies/bucket.hxx>
#include <goodies/matril3d.hxx>
#include <offuh/com/sun/star/awt/Point.hdl>
#include <offuh/com/sun/star/awt/Point.hpp>
#include <offuh/com/sun/star/awt/Size.hdl>
#include <offuh/com/sun/star/awt/Size.hpp>
#include <offuh/com/sun/star/beans/PropertyVetoException.hdl>
#include <offuh/com/sun/star/beans/PropertyVetoException.hpp>
#include <offuh/com/sun/star/container/ElementExistException.hdl>
#include <offuh/com/sun/star/container/ElementExistException.hpp>
#include <offuh/com/sun/star/container/NoSuchElementException.hpp>
#include <offuh/com/sun/star/container/xElementAccess.hdl>
#include <offuh/com/sun/star/container/xElementAccess.hpp>
#include <offuh/com/sun/star/container/xNameAccess.hpp>
#include <offuh/com/sun/star/datatransfer/dataflavor.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/draggestureevent.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/dragsourcedragevent.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/dragsourcedragevent.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/dragsourcedropevent.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/dragsourceevent.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/droptargetdragenterevent.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/droptargetdragenterevent.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/droptargetdragevent.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/droptargetdragevent.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/droptargetdropevent.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/droptargetevent.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/droptargetevent.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/xdraggesturelistener.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/xdraggesturelistener.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/xdragsource.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/xdragsource.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/xdragsourcecontext.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/xdragsourcecontext.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/xdragsourcelistener.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/xdragsourcelistener.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/xdroptargetdragcontext.hpp>
#include <offuh/com/sun/star/datatransfer/dnd/xdroptargetlistener.hdl>
#include <offuh/com/sun/star/datatransfer/dnd/xdroptargetlistener.hpp>
#include <offuh/com/sun/star/datatransfer/unsupportedflavorexception.hdl>
#include <offuh/com/sun/star/datatransfer/xtransferable.hpp>
#include <offuh/com/sun/star/drawing/xshape.hpp>
#include <offuh/com/sun/star/embed/invalidstorageexception.hpp>
#include <offuh/com/sun/star/embed/storagewrappedtargetexception.hdl>
#include <offuh/com/sun/star/embed/storagewrappedtargetexception.hpp>
#include <offuh/com/sun/star/embed/xstorage.hdl>
#include <offuh/com/sun/star/embed/xstorage.hpp>
#include <offuh/com/sun/star/io/buffersizeexceededexception.hpp>
#include <offuh/com/sun/star/io/ioexception.hdl>
#include <offuh/com/sun/star/io/notconnectedexception.hdl>
#include <offuh/com/sun/star/io/notconnectedexception.hpp>
#include <offuh/com/sun/star/io/xinputstream.hdl>
#include <offuh/com/sun/star/io/xinputstream.hpp>
#include <offuh/com/sun/star/io/xoutputstream.hdl>
#include <offuh/com/sun/star/io/xoutputstream.hpp>
#include <offuh/com/sun/star/io/xstream.hdl>
#include <offuh/com/sun/star/lang/eventobject.hdl>
#include <offuh/com/sun/star/lang/illegalargumentexception.hpp>
#include <offuh/com/sun/star/lang/wrappedtargetexception.hdl>
#include <offuh/com/sun/star/lang/wrappedtargetexception.hpp>
#include <offuh/com/sun/star/lang/xcomponent.hpp>
#include <offuh/com/sun/star/lang/xeventlistener.hpp>
#include <offuh/com/sun/star/packages/noencryptionexception.hdl>
#include <offuh/com/sun/star/packages/noencryptionexception.hpp>
#include <offuh/com/sun/star/packages/wrongpasswordexception.hdl>
#include <offuh/com/sun/star/packages/wrongpasswordexception.hpp>
#include <offuh/com/sun/star/uno/exception.hdl>
#include <offuh/com/sun/star/uno/exception.hpp>
#include <offuh/com/sun/star/uno/runtimeexception.hdl>
#include <offuh/com/sun/star/uno/runtimeexception.hpp>
#include <offuh/com/sun/star/uno/xadapter.hdl>
#include <offuh/com/sun/star/uno/xadapter.hpp>
#include <offuh/com/sun/star/uno/xinterface.hdl>
#include <offuh/com/sun/star/uno/xreference.hdl>
#include <offuh/com/sun/star/uno/xreference.hpp>
#include <offuh/com/sun/star/uno/xweak.hpp>
#include <osl/endian.h>
#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <rtl/alloc.h>
#include <rtl/string.h>
#include <rtl/ustrbuf.h>
#include <rtl/ustring.h>
#include <sal/mathconf.h>
#include <sal/types.h>
#include <sot/exchange.hxx>
#include <sot/factory.hxx>
#include <sot/storage.hxx>
#include <svtools/brdcst.hxx>
#include <svtools/cenumitm.hxx>
#include <svtools/cintitem.hxx>
#include <svtools/fltrcfg.hxx>
#include <svtools/intitem.hxx>
#include <svtools/listener.hxx>
#include <svtools/lstner.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/solar.hrc>
#include <svtools/useroptions.hxx>
#include <svx/editobj.hxx>
#include <svx/eeitem.hxx>
#include <svx/fmglob.hxx>
#include <svx/outlobj.hxx>
#include <svx/sdangitm.hxx>
#include <svx/sderitm.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/sdrmasterpagedescriptor.hxx>
#include <svx/sdrpageuser.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdoattr.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdtypes.hxx>
#include <svx/unoapi.hxx>
#include <svx/volume3d.hxx>
#include <svx/xcolit.hxx>
#include <svx/xenum.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnasit.hxx>
#include <svx/xtextit0.hxx>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <tools/errcode.hxx>
#include <tools/errinf.hxx>
#include <tools/gen.hxx>
#include <tools/globname.hxx>
#include <tools/list.hxx>
#include <tools/rc.hxx>
#include <tools/rtti.hxx>
#include <tools/solar.h>
#include <tools/string.hxx>
#include <tools/toolsdllapi.h>
#include <tools/weakbase.h>
#include <tools/weakbase.hxx>
#include <typeinfo.h>
#include <typeinfo>
#include <typelib/typeclass.h>
#include <typelib/typedescription.h>
#include <typelib/uik.h>
#include <uno/any2.h>
#include <uno/lbnames.h>
#include <uno/sequence2.h>
#include <unotools/ucbstreamhelper.hxx>

#include <vcl/apptypes.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/dllapi.h>
#include <vcl/dndhelp.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fldunit.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/jobset.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/menu.hxx>
#include <vcl/pointr.hxx>
#include <vcl/print.hxx>
#include <vcl/prntypes.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/region.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/sv.h>
#include <vcl/svapp.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/window.hxx>
#include <vcl/wintypes.hxx>
#include <vos/macros.hxx>
#include <vos/object.hxx>
#include <vos/types.hxx>
#include <wchar.h>

#endif

