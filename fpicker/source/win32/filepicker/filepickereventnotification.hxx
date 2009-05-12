/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filepickereventnotification.hxx,v $
 * $Revision: 1.4 $
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
