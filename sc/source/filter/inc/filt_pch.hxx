/*************************************************************************
 *
 *  $RCSfile: filt_pch.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-16 14:20:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//  ItemID-Defines etc. muessen immer ganz vorne stehen

#include "scitems.hxx"
#include "eetext.hxx"

#define ITEMID_FIELD EE_FEATURE_FIELD

#define _ZFORLIST_DECLARE_TABLE

#define SC_PROGRESS_CXX

//  ab hier automatisch per makepch generiert
//  folgende duerfen nicht aufgenommen werden:
//      ...

#include <tools/solar.h>
#include <tools/string.hxx>
#include <rtl/textenc.h>
#include <sal/types.h>
#include <sal/config.h>
#include <rtl/textcvt.h>
#include <rtl/string.hxx>
#include <rtl/string.h>
#include <rtl/ustring.h>
#include <rtl/memory.h>
#include <rtl/ustring.hxx>
#include <rtl/ustring>
#include <rtl/locale.hxx>
#include <rtl/locale.h>
#include <tools/contnr.hxx>
#include <tools/lang.hxx>
#include <tools/list.hxx>
#include <global.hxx>
#include <tools/stream.hxx>
#include <tools/errinf.hxx>
#include <tools/rtti.hxx>
#include <tools/errcode.hxx>
#include <tools/ref.hxx>
#include <tools/link.hxx>
#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <tools/date.hxx>
#include <svtools/svarray.hxx>
#include <vcl/sv.h>
#include <vcl/timer.hxx>
#include <tools/gen.hxx>
#include <vcl/color.hxx>
#include <tools/color.hxx>
#include <filter.hxx>
#include <rangelst.hxx>
#include <osl/mutex.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Any.h>
#include <cppu/macros.hxx>
#include <uno/lbnames.h>
#include <uno/any2.h>
#include <uno/data.h>
#include <typelib/typedescription.h>
#include <typelib/uik.h>
#include <typelib/typeclass.h>
#include <com/sun/star/uno/Type.h>
#include <com/sun/star/uno/TypeClass.hdl>
#include <com/sun/star/uno/Type.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/genfunc.hxx>
#include <com/sun/star/uno/genfunc.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XInterface.hdl>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hdl>
#include <com/sun/star/uno/Exception.hdl>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <svtools/hint.hxx>
#include <svtools/poolitem.hxx>
#include <document.hxx>
#include <vcl/prntypes.hxx>
#include <table.hxx>
#include <column.hxx>
#include <markarr.hxx>
#include <root.hxx>
#include <flttypes.hxx>
#include <svtools/solar.hrc>
#include <sfx2/sfxsids.hrc>
#include <svtools/cntwids.hrc>
#include <sfx2/cntids.hrc>
#include <tools/mempool.hxx>
#include <compiler.hxx>
#include <compiler.hrc>
#include <sfx2/sfx.hrc>
#include <scitems.hxx>
#include <svx/svxids.hrc>
#include <svtools/itemset.hxx>
#include <svtools/memberid.hrc>
#include <tools/table.hxx>
#include <flttools.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/resid.hxx>
#include <tools/resid.hxx>
#include <vcl/rc.hxx>
#include <tools/rc.hxx>
#include <tools/resmgr.hxx>
#include <tools/fract.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/mapunit.hxx>
#include <vcl/region.hxx>
#include <svtools/lstner.hxx>
#include <patattr.hxx>
#include <vcl/font.hxx>
#include <vcl/gdiobj.hxx>
#include <svtools/cenumitm.hxx>
#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/cintitem.hxx>
#include <svtools/brdcst.hxx>
#include <sot/sotref.hxx>
#include <tools/globname.hxx>
#include <sot/factory.hxx>
#include <sot/object.hxx>
#include <sot/sotdata.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/alpha.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/unqidx.hxx>
#include <vcl/graph.hxx>
#include <vcl/animate.hxx>
#include <vcl/graph.h>
#include <vcl/gfxlink.hxx>
#include <rsc/rscsfx.hxx>
#include <vcl/wall.hxx>
#include <vcl/settings.hxx>
#include <vcl/accel.hxx>
#include <vcl/keycod.hxx>
#include <vcl/keycodes.hxx>
#include <namebuff.hxx>
#include <tools/shl.hxx>
#include <tools/pstm.hxx>
#include <so3/iface.hxx>
#include <so3/factory.hxx>
#include <so3/so2ref.hxx>
#include <so3/so2dll.hxx>
#include <svx/fhgtitem.hxx>
#include <xfbuff.hxx>
#include <vos/types.hxx>
#include <vos/object.hxx>
#include <vos/macros.hxx>
#include <tools/unqid.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <uno/sequence2.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <attrib.hxx>
#include <svtools/zforlist.hxx>
#include <svx/fontitem.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XEventListener.hdl>
#include <com/sun/star/lang/EventObject.hdl>
#include <com/sun/star/lang/EventObject.hpp>
#include <vcl/outdev.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValue.hdl>
#include <com/sun/star/beans/PropertyState.hdl>
#include <com/sun/star/beans/PropertyState.hpp>
#include <collect.hxx>
#include <vcl/window.hxx>
#include <vcl/pointr.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/event.hxx>
#include <sot/dtrans.hxx>
#include <tools/ownlist.hxx>
#include <vcl/cmdevt.hxx>
#include <vcl/fonttype.hxx>
#include <cell.hxx>
#include <osl/interlck.h>
#include <sfx2/sfxuno.hxx>
#include <colrowst.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URL.hdl>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hdl>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hdl>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XTypeProvider.hdl>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/factory.hxx>
#include <uno/dispatcher.h>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XRegistryKey.hdl>
#include <com/sun/star/registry/InvalidRegistryException.hdl>
#include <com/sun/star/registry/InvalidValueException.hdl>
#include <com/sun/star/registry/RegistryKeyType.hdl>
#include <com/sun/star/registry/RegistryValueType.hdl>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/registry/InvalidValueException.hpp>
#include <com/sun/star/registry/RegistryKeyType.hpp>
#include <com/sun/star/registry/RegistryValueType.hpp>
#include <sot/storage.hxx>
#include <so3/so2defs.hxx>
#include <tools/datetime.hxx>
#include <osl/thread.h>
#include <imp_op.hxx>
#include <otlnbuff.hxx>
#include <tokstack.hxx>
#include <com/sun/star/container/NoSuchElementException.hdl>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <vcl/svapp.hxx>
#include <vos/thread.hxx>
#include <vos/runnable.hxx>
#include <vos/refernce.hxx>
#include <vcl/apptypes.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <com/sun/star/lang/WrappedTargetException.hdl>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <sfx2/shell.hxx>
#include <tools/stack.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XComponent.hdl>
#include <svx/svxenum.hxx>
#include <formel.hxx>
#include <so3/svstor.hxx>
#include <com/sun/star/container/XElementAccess.hdl>
#include <com/sun/star/container/XElementAccess.hpp>
#include <svtools/itempool.hxx>
#include <svx/eeitem.hxx>
#include <rangenam.hxx>
#include <vcl/syswin.hxx>
#include <svtools/smplhint.hxx>
#include <fontbuff.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/edit.hxx>
#include <vcl/menu.hxx>
#include <vcl/combobox.hxx>
#include <vcl/combobox.h>
#include <vcl/fldunit.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrame.hdl>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindow.hdl>
#include <com/sun/star/awt/Rectangle.hdl>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XFocusListener.hdl>
#include <com/sun/star/awt/FocusEvent.hdl>
#include <com/sun/star/awt/FocusEvent.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XKeyListener.hdl>
#include <com/sun/star/awt/KeyEvent.hdl>
#include <com/sun/star/awt/InputEvent.hdl>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/InputEvent.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseListener.hdl>
#include <com/sun/star/awt/MouseEvent.hdl>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hdl>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XPaintListener.hdl>
#include <com/sun/star/awt/PaintEvent.hdl>
#include <com/sun/star/awt/PaintEvent.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XWindowListener.hdl>
#include <com/sun/star/awt/WindowEvent.hdl>
#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XController.hdl>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModel.hdl>
#include <com/sun/star/lang/IndexOutOfBoundsException.hdl>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <so3/persist.hxx>
#include <com/sun/star/container/XIndexAccess.hdl>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <excrecds.hxx>
#include <scerrors.hxx>
#include <docpool.hxx>
#include <svx/msdffimp.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hdl>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <tools/urlobj.hxx>
#include <svx/colritem.hxx>
#include <vcl/wrkwin.hxx>
#include <fltprgrs.hxx>
#include <excsst.hxx>
#include <excimp8.hxx>
#include <excscen.hxx>
#include <excpivot.hxx>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hdl>
#include <com/sun/star/frame/FrameActionEvent.hdl>
#include <com/sun/star/frame/FrameAction.hdl>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XFramesSupplier.hdl>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XFrames.hdl>
#include <svx/msdffdef.hxx>
#include <vcl/image.hxx>




