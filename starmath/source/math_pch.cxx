/*************************************************************************
 *
 *  $RCSfile: math_pch.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:26 $
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

#define SMDLL 1

#include "starmath.hrc"

#define ITEMID_FONT         1
#define ITEMID_FONTHEIGHT   2
#define ITEMID_LRSPACE      3
#define ITEMID_WEIGHT       4

//--------- ab hier die "generierten"
#include <tools/string.hxx>
#include <tools/solar.h>
#include <tools/list.hxx>
#include <tools/contnr.hxx>
#include <tools/rtti.hxx>
#include <tools/ref.hxx>
#include <tools/link.hxx>
#include <svtools/brdcst.hxx>
#include <svtools/svarray.hxx>
#include <tools/debug.hxx>
#include <svtools/hint.hxx>
#include <svtools/smplhint.hxx>
#include <sot/sotref.hxx>
#include <tools/globname.hxx>
#include <sot/factory.hxx>
#include <vcl/sv.h>
#include <svtools/sbxdef.hxx>
#include <tools/time.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <tools/errinf.hxx>
#include <tools/errcode.hxx>
#include <sot/object.hxx>
#include <sot/sotdata.hxx>
#include <sfx2/shell.hxx>
#include <sal/types.h>
#include <sal/config.h>
#include <tools/date.hxx>
#include <sot/dtrans.hxx>
#include <tools/ownlist.hxx>
#include <vcl/accel.hxx>
#include <vcl/resid.hxx>
#include <tools/resid.hxx>
#include <vcl/rc.hxx>
#include <tools/rc.hxx>
#include <tools/lang.hxx>
#include <tools/resmgr.hxx>
#include <vcl/keycod.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/timer.hxx>
#include <vcl/cmdevt.hxx>
#include <vcl/font.hxx>
#include <vcl/color.hxx>
#include <vcl/outdev.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>
#include <tools/fract.hxx>
#include <vcl/wall.hxx>
#include <vcl/settings.hxx>
#include <tools/intn.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/pointr.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/event.hxx>
#include <format.hxx>
#include <utility.hxx>
#include <vcl/fixed.hxx>
#include <vcl/image.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/window.hxx>
#include <vcl/icon.hxx>
#include <sfx2/minarray.hxx>
#include <vcl/combobox.hxx>
#include <vcl/combobox.h>
#include <vcl/edit.hxx>
#include <vcl/menu.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/lstbox.h>
#include <usr/guid.hxx>
#include <usr/sequ.hxx>
#include <uno/types.h>
#include <uno/macros.h>
#include <vos/xception.hxx>
#include <osl/mutex.h>
#include <tools/shl.hxx>
#include <sfx2/module.hxx>
#include <sfx2/imgdef.hxx>
#include <usr/uno.hxx>
#include <usr/xiface.hxx>
#include <usr/ustring.hxx>
#include <vos/refernce.hxx>
#include <vos/types.hxx>
#include <osl/types.h>
#include <osl/interlck.h>
#include <vos/object.hxx>
#include <vos/macros.hxx>
#include <smdll.hxx>
#include <smdll0.hxx>
#include <sfx2/sfxsids.hrc>
#include <chaos/cntids.hrc>
#include <svtools/cntwids.hrc>
#include <svtools/solar.hrc>
#include <svtools/lstner.hxx>
#include <starmath.hrc>
#include <svtools/sbx.hxx>
#include <svtools/sbxform.hxx>
#include <svtools/sbxobj.hxx>
#include <svtools/sbxvar.hxx>
#include <svtools/sbxcore.hxx>
#include <svtools/sbxprop.hxx>
#include <svtools/sbxmeth.hxx>
#include <tools/unqid.hxx>
#include <tools/unqidx.hxx>
#include <svtools/poolitem.hxx>
#include <svtools/args.hxx>
#include <smmod.hxx>
#include <vos/thread.hxx>
#include <osl/thread.h>
#include <vos/runnable.hxx>
#include <vcl/apptypes.hxx>
#include <tools/dynary.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <svtools/iniman.hxx>
#include <vcl/timer.hxx>
#include <sfx2/app.hxx>
#include <svtools/plugapp.hxx>
#include <sfx2/sfx.hrc>
#include <svtools/memberid.hrc>
#include <vcl/syswin.hxx>
#include <vcl/virdev.hxx>
#include <tools/datetime.hxx>
#include <tools/wldcrd.hxx>
#include <parse.hxx>
#include <tools/stack.hxx>
#include <types.hxx>
#include <config.hxx>
#include <sfx2/cfgitem.hxx>
#include <svtools/confitem.hxx>
#include <xchar.hxx>
#include <vcl/poly.hxx>
#include <svx/xpoly.hxx>
#include <rect.hxx>
#include <vcl/outdev.hxx>
#include <so3/so2ref.hxx>
#ifndef _SMART_COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HXX_
#include <smart/com/sun/star/frame/XDispatchProviderInterceptor.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_XDISPATCH_HXX_
#include <smart/com/sun/star/frame/XDispatch.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HXX_
#include <smart/com/sun/star/frame/XDispatchProvider.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_XSTATUSLISTENER_HXX_
#include <smart/com/sun/star/frame/XStatusListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HXX_
#include <smart/com/sun/star/frame/FrameSearchFlag.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HXX_
#include <smart/com/sun/star/frame/XDispatchProviderInterception.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HXX_
#include <smart/com/sun/star/frame/FeatureStateEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HXX_
#include <smart/com/sun/star/frame/DispatchDescriptor.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HXX_
#include <smart/com/sun/star/frame/XFrameActionListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HXX_
#include <smart/com/sun/star/frame/XComponentLoader.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_XFRAME_HXX_
#include <smart/com/sun/star/frame/XFrame.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HXX_
#include <smart/com/sun/star/frame/FrameActionEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_FRAMEACTION_HXX_
#include <smart/com/sun/star/frame/FrameAction.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_UTIL_XURLTRANSFORMER_HXX_
#include <smart/com/sun/star/util/XURLTransformer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HXX_
#include <smart/com/sun/star/task/XStatusIndicatorFactory.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HXX_
#include <smart/com/sun/star/task/XStatusIndicatorSupplier.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_TASK_XSTATUSINDICATOR_HXX_
#include <smart/com/sun/star/task/XStatusIndicator.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_FRAME_XBROWSEHISTORYREGISTRY_HXX_
#include <smart/com/sun/star/frame/XBrowseHistoryRegistry.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_IO_BUFFERSIZEEXCEEDEDEXCEPTION_HXX_
#include <smart/com/sun/star/io/BufferSizeExceededException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_IO_NOTCONNECTEDEXCEPTION_HXX_
#include <smart/com/sun/star/io/NotConnectedException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_IO_IOEXCEPTION_HXX_
#include <smart/com/sun/star/io/IOException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_IO_UNEXPECTEDEOFEXCEPTION_HXX_
#include <smart/com/sun/star/io/UnexpectedEOFException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_IO_WRONGFORMATEXCEPTION_HXX_
#include <smart/com/sun/star/io/WrongFormatException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HXX_
#include <smart/com/sun/star/lang/ServiceNotRegisteredException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HXX_
#include <smart/com/sun/star/lang/NullPointerException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_CLASSNOTFOUNDEXCEPTION_HXX_
#include <smart/com/sun/star/lang/ClassNotFoundException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_NOSUCHMETHODEXCEPTION_HXX_
#include <smart/com/sun/star/lang/NoSuchMethodException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_SECURITYEXCEPTION_HXX_
#include <smart/com/sun/star/lang/SecurityException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_NOSUCHFIELDEXCEPTION_HXX_
#include <smart/com/sun/star/lang/NoSuchFieldException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HXX_
#include <smart/com/sun/star/lang/DisposedException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_ARRAYINDEXOUTOFBOUNDSEXCEPTION_HXX_
#include <smart/com/sun/star/lang/ArrayIndexOutOfBoundsException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_ILLEGALACCESSEXCEPTION_HXX_
#include <smart/com/sun/star/lang/IllegalAccessException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HXX_
#include <smart/com/sun/star/lang/IndexOutOfBoundsException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HXX_
#include <smart/com/sun/star/lang/IllegalArgumentException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_NOSUPPORTEXCEPTION_HXX_
#include <smart/com/sun/star/lang/NoSupportException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HXX_
#include <smart/com/sun/star/lang/WrappedTargetException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_PARAMINFO_HXX_
#include <smart/com/sun/star/reflection/ParamInfo.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_XIDLARRAY_HXX_
#include <smart/com/sun/star/reflection/XIdlArray.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_XIDLCLASSPROVIDER_HXX_
#include <smart/com/sun/star/reflection/XIdlClassProvider.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_FIELDACCESSMODE_HXX_
#include <smart/com/sun/star/reflection/FieldAccessMode.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_XIDLCLASS_HXX_
#include <smart/com/sun/star/reflection/XIdlClass.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_XIDLFIELD_HXX_
#include <smart/com/sun/star/reflection/XIdlField.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_PARAMMODE_HXX_
#include <smart/com/sun/star/reflection/ParamMode.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_METHODMODE_HXX_
#include <smart/com/sun/star/reflection/MethodMode.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_XIDLMEMBER_HXX_
#include <smart/com/sun/star/reflection/XIdlMember.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_XIDLREFLECTION_HXX_
#include <smart/com/sun/star/reflection/XIdlReflection.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_XIDLMETHOD_HXX_
#include <smart/com/sun/star/reflection/XIdlMethod.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_REFLECTION_INVOCATIONTARGETEXCEPTION_HXX_
#include <smart/com/sun/star/reflection/InvocationTargetException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_PROPERTYVALUES_HXX_
#include <smart/com/sun/star/beans/PropertyValues.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XPROPERTYSET_HXX_
#include <smart/com/sun/star/beans/XPropertySet.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_PROPERTYVALUE_HXX_
#include <smart/com/sun/star/beans/PropertyValue.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_PROPERTYSTATE_HXX_
#include <smart/com/sun/star/beans/PropertyState.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HXX_
#include <smart/com/sun/star/beans/XPropertySetInfo.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HXX_
#include <smart/com/sun/star/beans/XMultiPropertySet.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HXX_
#include <smart/com/sun/star/beans/XFastPropertySet.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HXX_
#include <smart/com/sun/star/beans/XVetoableChangeListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XPROPERTYSTATE_HXX_
#include <smart/com/sun/star/beans/XPropertyState.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XPROPERTYSTATECHANGELISTENER_HXX_
#include <smart/com/sun/star/beans/XPropertyStateChangeListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <smart/com/sun/star/beans/PropertyAttribute.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HXX_
#include <smart/com/sun/star/beans/XPropertiesChangeListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HXX_
#include <smart/com/sun/star/beans/XPropertyChangeListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XPROPERTYACCESS_HXX_
#include <smart/com/sun/star/beans/XPropertyAccess.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HXX_
#include <smart/com/sun/star/beans/XPropertyContainer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_PROPERTYSTATECHANGEEVENT_HXX_
#include <smart/com/sun/star/beans/PropertyStateChangeEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HXX_
#include <smart/com/sun/star/beans/PropertyChangeEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HXX_
#include <smart/com/sun/star/beans/UnknownPropertyException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_INTROSPECTIONEXCEPTION_HXX_
#include <smart/com/sun/star/beans/IntrospectionException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_PROPERTYEXISTEXCEPTION_HXX_
#include <smart/com/sun/star/beans/PropertyExistException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_ILLEGALTYPEEXCEPTION_HXX_
#include <smart/com/sun/star/beans/IllegalTypeException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_BEANS_PROPERTYVETOEXCEPTION_HXX_
#include <smart/com/sun/star/beans/PropertyVetoException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HXX_
#include <smart/com/sun/star/container/XEnumerationAccess.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HXX_
#include <smart/com/sun/star/container/XHierarchicalNameAccess.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XNAMEACCESS_HXX_
#include <smart/com/sun/star/container/XNameAccess.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HXX_
#include <smart/com/sun/star/container/XContentEnumerationAccess.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XENUMERATION_HXX_
#include <smart/com/sun/star/container/XEnumeration.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XELEMENTACCESS_HXX_
#include <smart/com/sun/star/container/XElementAccess.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XINDEXACCESS_HXX_
#include <smart/com/sun/star/container/XIndexAccess.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_XEVENTLISTENER_HXX_
#include <smart/com/sun/star/lang/XEventListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_EVENTOBJECT_HXX_
#include <smart/com/sun/star/lang/EventObject.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_SCRIPT_XALLLISTENERADAPTERSERVICE_HXX_
#include <smart/com/sun/star/script/XAllListenerAdapterService.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_SCRIPT_XALLLISTENER_HXX_
#include <smart/com/sun/star/script/XAllListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_SCRIPT_ALLEVENTOBJECT_HXX_
#include <smart/com/sun/star/script/AllEventObject.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XCOMPONENTENUMERATION_HXX_
#include <smart/com/sun/star/container/XComponentEnumeration.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_XCOMPONENT_HXX_
#include <smart/com/sun/star/lang/XComponent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XCOMPONENTENUMERATIONACCESS_HXX_
#include <smart/com/sun/star/container/XComponentEnumerationAccess.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_LISTENEREXISTEXCEPTION_HXX_
#include <smart/com/sun/star/lang/ListenerExistException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_ELEMENTEXISTEXCEPTION_HXX_
#include <smart/com/sun/star/container/ElementExistException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_LANG_INVALIDLISTENEREXCEPTION_HXX_
#include <smart/com/sun/star/lang/InvalidListenerException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HXX_
#include <smart/com/sun/star/container/NoSuchElementException.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XKEYLISTENER_HXX_
#include <smart/com/sun/star/awt/XKeyListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XPAINTLISTENER_HXX_
#include <smart/com/sun/star/awt/XPaintListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_KEYEVENT_HXX_
#include <smart/com/sun/star/awt/KeyEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_KEYMODIFIER_HXX_
#include <smart/com/sun/star/awt/KeyModifier.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XMOUSEMOTIONLISTENER_HXX_
#include <smart/com/sun/star/awt/XMouseMotionListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FOCUSEVENT_HXX_
#include <smart/com/sun/star/awt/FocusEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XWINDOWLISTENER_HXX_
#include <smart/com/sun/star/awt/XWindowListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XACTIVATELISTENER_HXX_
#include <smart/com/sun/star/awt/XActivateListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_MOUSEEVENT_HXX_
#include <smart/com/sun/star/awt/MouseEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTOPWINDOWLISTENER_HXX_
#include <smart/com/sun/star/awt/XTopWindowListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_PAINTEVENT_HXX_
#include <smart/com/sun/star/awt/PaintEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_INPUTEVENT_HXX_
#include <smart/com/sun/star/awt/InputEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_KEYGROUP_HXX_
#include <smart/com/sun/star/awt/KeyGroup.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_KEY_HXX_
#include <smart/com/sun/star/awt/Key.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_WINDOWEVENT_HXX_
#include <smart/com/sun/star/awt/WindowEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XMOUSELISTENER_HXX_
#include <smart/com/sun/star/awt/XMouseListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_KEYFUNCTION_HXX_
#include <smart/com/sun/star/awt/KeyFunction.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FOCUSCHANGEREASON_HXX_
#include <smart/com/sun/star/awt/FocusChangeReason.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_MOUSEBUTTON_HXX_
#include <smart/com/sun/star/awt/MouseButton.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XFOCUSLISTENER_HXX_
#include <smart/com/sun/star/awt/XFocusListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XADJUSTMENTLISTENER_HXX_
#include <smart/com/sun/star/awt/XAdjustmentListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XACTIONLISTENER_HXX_
#include <smart/com/sun/star/awt/XActionListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTEXTLISTENER_HXX_
#include <smart/com/sun/star/awt/XTextListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XSPINLISTENER_HXX_
#include <smart/com/sun/star/awt/XSpinListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XITEMLISTENER_HXX_
#include <smart/com/sun/star/awt/XItemListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XVCLCONTAINERLISTENER_HXX_
#include <smart/com/sun/star/awt/XVclContainerListener.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XFILEDIALOG_HXX_
#include <smart/com/sun/star/awt/XFileDialog.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTEXTCOMPONENT_HXX_
#include <smart/com/sun/star/awt/XTextComponent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XLISTBOX_HXX_
#include <smart/com/sun/star/awt/XListBox.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XPROGRESSMONITOR_HXX_
#include <smart/com/sun/star/awt/XProgressMonitor.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_TEXTALIGN_HXX_
#include <smart/com/sun/star/awt/TextAlign.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XSCROLLBAR_HXX_
#include <smart/com/sun/star/awt/XScrollBar.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XVCLCONTAINERPEER_HXX_
#include <smart/com/sun/star/awt/XVclContainerPeer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HXX_
#include <smart/com/sun/star/awt/XTabControllerModel.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XMESSAGEBOX_HXX_
#include <smart/com/sun/star/awt/XMessageBox.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTEXTEDITFIELD_HXX_
#include <smart/com/sun/star/awt/XTextEditField.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_STYLE_HXX_
#include <smart/com/sun/star/awt/Style.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTIMEFIELD_HXX_
#include <smart/com/sun/star/awt/XTimeField.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XVCLWINDOWPEER_HXX_
#include <smart/com/sun/star/awt/XVclWindowPeer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XCONTROLMODEL_HXX_
#include <smart/com/sun/star/awt/XControlModel.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XSPINFIELD_HXX_
#include <smart/com/sun/star/awt/XSpinField.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XUNOCONTROLCONTAINER_HXX_
#include <smart/com/sun/star/awt/XUnoControlContainer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTEXTLAYOUTCONSTRAINS_HXX_
#include <smart/com/sun/star/awt/XTextLayoutConstrains.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XNUMERICFIELD_HXX_
#include <smart/com/sun/star/awt/XNumericField.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XBUTTON_HXX_
#include <smart/com/sun/star/awt/XButton.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTEXTAREA_HXX_
#include <smart/com/sun/star/awt/XTextArea.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XIMAGEBUTTON_HXX_
#include <smart/com/sun/star/awt/XImageButton.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XFIXEDTEXT_HXX_
#include <smart/com/sun/star/awt/XFixedText.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XCONTROLCONTAINER_HXX_
#include <smart/com/sun/star/awt/XControlContainer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XDIALOG_HXX_
#include <smart/com/sun/star/awt/XDialog.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HXX_
#include <smart/com/sun/star/awt/ScrollBarOrientation.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XRADIOBUTTON_HXX_
#include <smart/com/sun/star/awt/XRadioButton.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XCURRENCYFIELD_HXX_
#include <smart/com/sun/star/awt/XCurrencyField.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XPATTERNFIELD_HXX_
#include <smart/com/sun/star/awt/XPatternField.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HXX_
#include <smart/com/sun/star/awt/VclWindowPeerAttribute.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTABCONTROLLER_HXX_
#include <smart/com/sun/star/awt/XTabController.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XVCLCONTAINER_HXX_
#include <smart/com/sun/star/awt/XVclContainer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XDATEFIELD_HXX_
#include <smart/com/sun/star/awt/XDateField.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XCOMBOBOX_HXX_
#include <smart/com/sun/star/awt/XComboBox.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XCONTROL_HXX_
#include <smart/com/sun/star/awt/XControl.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XCHECKBOX_HXX_
#include <smart/com/sun/star/awt/XCheckBox.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_MESSAGEBOXCOMMAND_HXX_
#include <smart/com/sun/star/awt/MessageBoxCommand.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HXX_
#include <smart/com/sun/star/awt/XLayoutConstrains.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XPROGRESSBAR_HXX_
#include <smart/com/sun/star/awt/XProgressBar.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_SIMPLEFONTMETRIC_HXX_
#include <smart/com/sun/star/awt/SimpleFontMetric.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTWEIGHT_HXX_
#include <smart/com/sun/star/awt/FontWeight.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTSLANT_HXX_
#include <smart/com/sun/star/awt/FontSlant.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_CHARSET_HXX_
#include <smart/com/sun/star/awt/CharSet.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTDESCRIPTOR_HXX_
#include <smart/com/sun/star/awt/FontDescriptor.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTWIDTH_HXX_
#include <smart/com/sun/star/awt/FontWidth.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XFONT_HXX_
#include <smart/com/sun/star/awt/XFont.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTTYPE_HXX_
#include <smart/com/sun/star/awt/FontType.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTUNDERLINE_HXX_
#include <smart/com/sun/star/awt/FontUnderline.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTSTRIKEOUT_HXX_
#include <smart/com/sun/star/awt/FontStrikeout.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTFAMILY_HXX_
#include <smart/com/sun/star/awt/FontFamily.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_FONTPITCH_HXX_
#include <smart/com/sun/star/awt/FontPitch.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTOPWINDOW_HXX_
#include <smart/com/sun/star/awt/XTopWindow.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XWINDOW_HXX_
#include <smart/com/sun/star/awt/XWindow.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_POSSIZE_HXX_
#include <smart/com/sun/star/awt/PosSize.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_VCLCONTAINEREVENT_HXX_
#include <smart/com/sun/star/awt/VclContainerEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_ITEMEVENT_HXX_
#include <smart/com/sun/star/awt/ItemEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_SPINEVENT_HXX_
#include <smart/com/sun/star/awt/SpinEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_TEXTEVENT_HXX_
#include <smart/com/sun/star/awt/TextEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_ADJUSTMENTTYPE_HXX_
#include <smart/com/sun/star/awt/AdjustmentType.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_ACTIONEVENT_HXX_
#include <smart/com/sun/star/awt/ActionEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_ADJUSTMENTEVENT_HXX_
#include <smart/com/sun/star/awt/AdjustmentEvent.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_RECTANGLE_HXX_
#include <smart/com/sun/star/awt/Rectangle.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_SELECTION_HXX_
#include <smart/com/sun/star/awt/Selection.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_SIZE_HXX_
#include <smart/com/sun/star/awt/Size.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_WINDOWDESCRIPTOR_HXX_
#include <smart/com/sun/star/awt/WindowDescriptor.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_INVALIDATESTYLE_HXX_
#include <smart/com/sun/star/awt/InvalidateStyle.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XTOOLKIT_HXX_
#include <smart/com/sun/star/awt/XToolkit.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XWINDOWPEER_HXX_
#include <smart/com/sun/star/awt/XWindowPeer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_WINDOWCLASS_HXX_
#include <smart/com/sun/star/awt/WindowClass.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XSYSTEMDEPENDENTWINDOWPEER_HXX_
#include <smart/com/sun/star/awt/XSystemDependentWindowPeer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HXX_
#include <smart/com/sun/star/awt/WindowAttribute.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XPOINTER_HXX_
#include <smart/com/sun/star/awt/XPointer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_SYSTEMPOINTER_HXX_
#include <smart/com/sun/star/awt/SystemPointer.hxx>
#endif
#ifndef _SMART_COM_SUN_STAR_AWT_XVIEW_HXX_
#include <smart/com/sun/star/awt/XView.hxx>
#endif
#include <usr/refl.hxx>
#include <vcl/resid.hxx>
#include <sfx2/msg.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/window.hxx>
#include <so3/protocol.hxx>
#include <sfx2/viewfrm.hxx>
#include <so3/factory.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/objface.hxx>
#include <so3/advise.hxx>
#include <so3/dtrans.hxx>
#include <vcl/event.hxx>
#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>
#include <symbol.hxx>
#include <svtools/itempool.hxx>
#include <vcl/image.hxx>
#include <vcl/metric.hxx>
#include <so3/iface.hxx>
#include <sfx2/inimgr.hxx>
#include <node.hxx>
#include <so3/so2dll.hxx>
#include <tools/pstm.hxx>
#include <tools/table.hxx>
#include <so3/ipobj.hxx>
#include <so3/embobj.hxx>
#include <sfx2/docfac.hxx>
#include <so3/pseudo.hxx>
#include <svtools/ownlist.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/stritem.hxx>
#include <vcl/mapmod.hxx>
#include <sfx2/ipfrm.hxx>
#include <vcl/dialog.hxx>
#include <so3/svstor.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/svstdarr.hxx>
#include <sfx2/bindings.hxx>
#include <so3/persist.hxx>
#include <dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/symbol.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <so3/so2defs.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/group.hxx>
#include <svx/optgenrl.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <document.hxx>
#include <so3/binding.hxx>
#include <vcl/field.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/scrbar.hxx>
#include <svtools/ctrlbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/interno.hxx>
#include <sfx2/sfxdefs.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/chalign.hxx>
#include <vcl/floatwin.hxx>
#include <sot/storage.hxx>
#include <rsc/rscsfx.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/btndlg.hxx>
#include <uno/uno.h>
#include <uno/string.h>
#include <uno/sequence.h>
#include <uno/any.h>
#include <uno/exceptn.h>
#include <uno/intrface.h>
#include <uno/factory.h>
#include <uno/api.h>
#include <svx/svxids.hrc>
#include <view.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/dockwin.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/icon.hxx>
#include <sfx2/clientsh.hxx>
#include <so3/ipclient.hxx>
#include <so3/client.hxx>
#include <svtools/scrwin.hxx>
#include <vcl/scrbar.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/viewfac.hxx>
#include <edit.hxx>
#include <svx/editdata.hxx>
#include <toolbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/dockwin.hxx>
#include <smslots.hxx>
#include <svtools/undo.hxx>
#include <sfx2/request.hxx>
#include <svtools/whiter.hxx>
#include <vcl/prntypes.hxx>
#include <vcl/jobset.hxx>
#include <vcl/gdimtf.hxx>
#include <sot/exchange.hxx>
#include <vcl/exchange.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/sstring.hxx>
#include <action.hxx>
#include <sfx2/docinf.hxx>
#include <sfx2/filedlg.hxx>
#include <sfx2/iodlg.hxx>
