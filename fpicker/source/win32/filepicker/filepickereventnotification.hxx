/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filepickereventnotification.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:45:21 $
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

#ifndef _FILEPICKEREVENTNOTIFICATION_HXX_
#define _FILEPICKEREVENTNOTIFICATION_HXX_

#ifndef _EVENTNOTIFICATION_HXX_
#include "eventnotification.hxx"
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERLISTENER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_FILEPICKEREVENT_HPP_
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>
#endif

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
