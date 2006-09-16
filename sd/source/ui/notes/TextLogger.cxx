/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextLogger.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:01:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TextLogger.hxx"

#include "EditWindow.hxx"

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

namespace sd { namespace notes {

TextLogger* TextLogger::spInstance = NULL;

TextLogger& TextLogger::Instance (void)
{
    if (spInstance == NULL)
    {
        ::vos::OGuard aGuard (::Application::GetSolarMutex());
        if (spInstance == NULL)
            spInstance = new TextLogger ();
    }
    return *spInstance;
}




TextLogger::TextLogger (void)
    : mpEditWindow (NULL)
{
}




void TextLogger::AppendText (const char* sText)
{
    OSL_TRACE (sText);
    if (mpEditWindow != NULL)
        mpEditWindow->InsertText (UniString::CreateFromAscii(sText));
}




void TextLogger::AppendText (const String& sText)
{
    ByteString s(sText, RTL_TEXTENCODING_ISO_8859_1);
    OSL_TRACE (s.GetBuffer());
    if (mpEditWindow != NULL)
        mpEditWindow->InsertText (sText);
}




void TextLogger::AppendNumber (long int nValue)
{
    AppendText (String::CreateFromInt32(nValue));
}




void TextLogger::ConnectToEditWindow (EditWindow* pEditWindow)
{
    if (mpEditWindow != pEditWindow)
    {
        if (pEditWindow != NULL)
            pEditWindow->AddEventListener(
                LINK(this, TextLogger, WindowEventHandler));
        else
            mpEditWindow->RemoveEventListener(
                LINK(this, TextLogger, WindowEventHandler));

        mpEditWindow = pEditWindow;
    }
}




IMPL_LINK(TextLogger, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        DBG_ASSERT(static_cast<VclWindowEvent*>(pEvent)->GetWindow()
            == mpEditWindow,
            "TextLogger: received event from unknown window");
        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_CLOSE:
            case VCLEVENT_OBJECT_DYING:
                mpEditWindow = NULL;
                break;
        }
    }
    return TRUE;
}


} } // end of namespace ::sd::notes
