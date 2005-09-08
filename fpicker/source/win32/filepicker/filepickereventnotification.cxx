/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filepickereventnotification.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:45:07 $
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
#include "filepickereventnotification.hxx"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
