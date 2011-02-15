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

//  ItemID-Defines etc. muessen immer ganz vorne stehen

#include "scitems.hxx"


#define _ZFORLIST_DECLARE_TABLE

#define SC_PROGRESS_CXX

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
#include <rtl/locale.hxx>
#include <rtl/locale.h>
#include <tools/contnr.hxx>
#include <i18npool/lang.h>
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
#include <svl/svarray.hxx>
#include <vcl/sv.h>
#include <vcl/timer.hxx>
#include <tools/gen.hxx>
#include <tools/color.hxx>
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
#include <svl/hint.hxx>
#include <svl/poolitem.hxx>
#include <document.hxx>
#include <vcl/prntypes.hxx>
#include <table.hxx>
#include <column.hxx>
#include <markarr.hxx>
#include <root.hxx>
#include <flttypes.hxx>
#include <svl/solar.hrc>
#include <sfx2/sfxsids.hrc>
#include <svl/cntwids.hrc>
#include <sfx2/cntids.hrc>
#include <tools/mempool.hxx>
#include <compiler.hxx>
#include <formula/compiler.hrc>
#include <sfx2/sfx.hrc>
#include <scitems.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>
#include <svl/memberid.hrc>
#include <tools/table.hxx>
#include <flttools.hxx>
#include <vcl/vclenum.hxx>
#include <tools/resid.hxx>
#include <tools/rc.hxx>
#include <tools/resmgr.hxx>
#include <tools/fract.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/mapunit.hxx>
#include <vcl/region.hxx>
#include <svl/lstner.hxx>
#include <patattr.hxx>
#include <vcl/font.hxx>
#include <svl/cenumitm.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/cintitem.hxx>
#include <svl/brdcst.hxx>
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
#include <editeng/fhgtitem.hxx>
#include <tools/unqid.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <uno/sequence2.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <attrib.hxx>
#include <svl/zforlist.hxx>
#include <editeng/fontitem.hxx>
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
#include <tools/ownlist.hxx>
#include <vcl/cmdevt.hxx>
#include <vcl/vclenum.hxx>
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
#include <tools/datetime.hxx>
#include <osl/thread.h>
#include <imp_op.hxx>
#include <otlnbuff.hxx>
#include <tokstack.hxx>
#include <com/sun/star/container/NoSuchElementException.hdl>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <vcl/svapp.hxx>
#include <osl/thread.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <vcl/apptypes.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <com/sun/star/lang/WrappedTargetException.hdl>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <sfx2/shell.hxx>
#include <tools/stack.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XComponent.hdl>
#include <editeng/svxenum.hxx>
#include <formel.hxx>
#include <com/sun/star/container/XElementAccess.hdl>
#include <com/sun/star/container/XElementAccess.hpp>
#include <svl/itempool.hxx>
#include <editeng/eeitem.hxx>
#include <rangenam.hxx>
#include <vcl/syswin.hxx>
#include <svl/smplhint.hxx>
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
#include <com/sun/star/container/XIndexAccess.hdl>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <excrecds.hxx>
#include <scerrors.hxx>
#include <docpool.hxx>
#include <svx/msdffimp.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hdl>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <tools/urlobj.hxx>
#include <editeng/colritem.hxx>
#include <vcl/wrkwin.hxx>
#include <excimp8.hxx>
#include <excscen.hxx>
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




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
