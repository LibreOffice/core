/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "filepickereventnotification.hxx"
#include <sal/types.h>





using namespace com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;






CFilePickerEventNotification::CFilePickerEventNotification(EventListenerMethod_t EventListenerMethod) :
    m_EventListenerMethod(EventListenerMethod)
{
}


//


void SAL_CALL CFilePickerEventNotification::notifyEventListener( Reference< XInterface > xListener )
{
    Reference<XFilePickerListener> xFilePickerListener(xListener,UNO_QUERY);
    if (xFilePickerListener.is())
        (xFilePickerListener.get()->*m_EventListenerMethod)();
}









CFilePickerParamEventNotification::CFilePickerParamEventNotification(EventListenerMethod_t EventListenerMethod, const FilePickerEvent& FilePickerEvent) :
    m_EventListenerMethod(EventListenerMethod),
    m_FilePickerEvent(FilePickerEvent)
{
}





void SAL_CALL CFilePickerParamEventNotification::notifyEventListener( Reference< XInterface > xListener )
{
    Reference<XFilePickerListener> xFilePickerListener(xListener,UNO_QUERY);
    if (xFilePickerListener.is())
        (xFilePickerListener.get()->*m_EventListenerMethod)(m_FilePickerEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
