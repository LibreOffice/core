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
#include "precompiled_sd.hxx"

#include "TextLogger.hxx"

#include "EditWindow.hxx"
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

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
    return true;
}


} } // end of namespace ::sd::notes
