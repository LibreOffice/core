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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TextLogger.hxx"

#include "EditWindow.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

namespace sd { namespace notes {

TextLogger* TextLogger::spInstance = NULL;

TextLogger& TextLogger::Instance (void)
{
    if (spInstance == NULL)
    {
        SolarMutexGuard aGuard;
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
    OSL_TRACE("%s", sText);
    if (mpEditWindow != NULL)
        mpEditWindow->InsertText (UniString::CreateFromAscii(sText));
}




void TextLogger::AppendText (const String& sText)
{
    ByteString s(sText, RTL_TEXTENCODING_ISO_8859_1);
    OSL_TRACE("%s", s.GetBuffer());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
