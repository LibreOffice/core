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

// MARKER(update_precomp.py): Generated on 2006-09-01 17:50:16.045215

#ifdef PRECOMPILED_HEADERS
//---MARKER---
#include "sal/types.h"

#include "com/sun/star/beans/IllegalTypeException.hpp"
#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyChangeEvent.hpp"
#include "com/sun/star/beans/PropertySetInfoChange.hpp"
#include "com/sun/star/beans/PropertySetInfoChangeEvent.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XPropertiesChangeListener.hpp"
#include "com/sun/star/beans/XPropertiesChangeNotifier.hpp"
#include "com/sun/star/beans/XPropertyAccess.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertyContainer.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/beans/XPropertySetInfoChangeListener.hpp"
#include "com/sun/star/beans/XPropertySetInfoChangeNotifier.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "com/sun/star/container/XChild.hpp"
#include "com/sun/star/container/XEnumeration.hpp"
#include "com/sun/star/container/XEnumerationAccess.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/container/XNameReplace.hpp"
#include "com/sun/star/container/XNamed.hpp"
#include "com/sun/star/document/XEventBroadcaster.hpp"
#include "com/sun/star/document/XEventListener.hpp"
#include "com/sun/star/document/XStorageBasedDocument.hpp"
#include "com/sun/star/embed/ElementModes.hpp"
#include "com/sun/star/embed/XStorage.hpp"
#include "com/sun/star/embed/XTransactedObject.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/XModuleManager.hpp"
#include "com/sun/star/frame/XStorable.hpp"
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/io/XActiveDataSink.hpp"
#include "com/sun/star/io/XActiveDataStreamer.hpp"
#include "com/sun/star/io/XAsyncOutputMonitor.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/io/XOutputStream.hpp"
#include "com/sun/star/io/XSeekable.hpp"
#include "com/sun/star/io/XStream.hpp"
#include "com/sun/star/io/XTruncate.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/IllegalAccessException.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/lang/XTypeProvider.hpp"
#include "com/sun/star/packages/WrongPasswordException.hpp"
#include "com/sun/star/reflection/XProxyFactory.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/script/XTypeConverter.hpp"
#include "com/sun/star/sdbc/DataType.hpp"
#include "com/sun/star/sdbc/FetchDirection.hpp"
#include "com/sun/star/sdbc/ResultSetType.hpp"
#include "com/sun/star/sdbc/XCloseable.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XResultSetMetaData.hpp"
#include "com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/task/DocumentPasswordRequest.hpp"
#include "com/sun/star/task/NoMasterException.hpp"
#include "com/sun/star/task/PasswordRequestMode.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionDisapprove.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/task/XInteractionPassword.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/task/XPasswordContainer.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/XReference.hpp"
#include "com/sun/star/uno/XWeak.hpp"
#include "com/sun/star/util/Date.hpp"
#include "com/sun/star/util/DateTime.hpp"
#include "com/sun/star/util/Time.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "com/sun/star/util/XChangesNotifier.hpp"
#include "com/sun/star/util/XMacroExpander.hpp"
#include "com/sun/star/util/XOfficeInstallationDirectories.hpp"

#include "comphelper/processfactory.hxx"

#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implbase5.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/interfacecontainer.h"
#include "cppuhelper/interfacecontainer.hxx"
#include "cppuhelper/queryinterface.hxx"
#include "cppuhelper/typeprovider.hxx"
#include "cppuhelper/weak.hxx"
#include "cppuhelper/weakref.hxx"

#include "libxml/parser.h"

#include "osl/conditn.hxx"
#include "osl/diagnose.h"
#include "osl/doublecheckedlocking.h"
#include "osl/file.hxx"
#include "osl/interlck.h"
#include "osl/module.h"
#include "osl/mutex.hxx"
#include "osl/process.h"
#include "osl/security.hxx"
#include "osl/socket.h"
#include "osl/socket.hxx"
#include "osl/thread.h"
#include "osl/time.h"

#include "rtl/alloc.h"
#include "rtl/memory.h"
#include "rtl/ref.hxx"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "salhelper/simplereferenceobject.hxx"

#include "sys/types.h"

#include "ucbhelper/configurationkeys.hxx"
#include "ucbhelper/macros.hxx"
#include "ucbhelper/proxydecider.hxx"

//---MARKER---

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
