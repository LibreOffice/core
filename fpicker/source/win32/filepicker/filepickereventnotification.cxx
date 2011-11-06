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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"
#include "filepickereventnotification.hxx"
#include <sal/types.h>

//-----------------------------------
// namespace directives
//-----------------------------------

using namespace com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;

//-----------------------------------
// A FilePicker event without
// parameter
//-----------------------------------

CFilePickerEventNotification::CFilePickerEventNotification(EventListenerMethod_t EventListenerMethod) :
    m_EventListenerMethod(EventListenerMethod)
{
}

//----------------------------------
//
//----------------------------------

void SAL_CALL CFilePickerEventNotification::notifyEventListener( Reference< XInterface > xListener )
{
    Reference<XFilePickerListener> xFilePickerListener(xListener,UNO_QUERY);
    if (xFilePickerListener.is())
        (xFilePickerListener.get()->*m_EventListenerMethod)();
}


//##################################################


//----------------------------------
// A FilePicker event with parameter
//----------------------------------

CFilePickerParamEventNotification::CFilePickerParamEventNotification(EventListenerMethod_t EventListenerMethod, const FilePickerEvent& FilePickerEvent) :
    m_EventListenerMethod(EventListenerMethod),
    m_FilePickerEvent(FilePickerEvent)
{
}

//----------------------------------
// A FilePicker event with parameter
//----------------------------------

void SAL_CALL CFilePickerParamEventNotification::notifyEventListener( Reference< XInterface > xListener )
{
    Reference<XFilePickerListener> xFilePickerListener(xListener,UNO_QUERY);
    if (xFilePickerListener.is())
        (xFilePickerListener.get()->*m_EventListenerMethod)(m_FilePickerEvent);
}
