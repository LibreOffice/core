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



#ifndef _FILEPICKEREVENTNOTIFICATION_HXX_
#define _FILEPICKEREVENTNOTIFICATION_HXX_

#include "eventnotification.hxx"
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>

//-----------------------------------
// A FilePicker event without
// parameter
//-----------------------------------

class CFilePickerEventNotification : public CEventNotification
{
public:
    typedef void (SAL_CALL ::com::sun::star::ui::dialogs::XFilePickerListener::*EventListenerMethod_t)();

public:
    CFilePickerEventNotification(EventListenerMethod_t EventListenerMethod);

    virtual void SAL_CALL notifyEventListener( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xListener );

private:
    EventListenerMethod_t m_EventListenerMethod;
};

//----------------------------------
// A FilePicker event with parameter
//----------------------------------

class CFilePickerParamEventNotification : public CEventNotification
{
public:
    typedef void (SAL_CALL ::com::sun::star::ui::dialogs::XFilePickerListener::*EventListenerMethod_t)(const ::com::sun::star::ui::dialogs::FilePickerEvent&);

    CFilePickerParamEventNotification(EventListenerMethod_t EventListenerMethod, const ::com::sun::star::ui::dialogs::FilePickerEvent& FilePickerEvent);

    virtual void SAL_CALL notifyEventListener( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xListener );

private:
    EventListenerMethod_t                        m_EventListenerMethod;
    com::sun::star::ui::dialogs::FilePickerEvent m_FilePickerEvent;
};


#endif
