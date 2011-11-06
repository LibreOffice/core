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
#include "precompiled_automation.hxx"

#include "editwin.hxx"
#include <tools/stream.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif


sal_Bool GHEditWindow::Close()
{
    if (aInhalt.IsModified())
    {
    }
    delete(this);
    return sal_True;
}

void GHEditWindow::Resize()
{
    aInhalt.SetPosSizePixel(Point(1,1),GetOutputSizePixel());
}

GHEditWindow::GHEditWindow(Window * pParent, String aName, WinBits iWstyle)
: FloatingWindow(pParent)
, aInhalt(this,iWstyle)
{
    SetOutputSizePixel( Size( 120,80 ) );
    Show();
    Resize();
    aInhalt.Show();
    SetText(aName);
}

void GHEditWindow::Clear()
{
    aInhalt.SetText(String());
}

void GHEditWindow::AddText( String aNew, sal_Bool bMoveToEnd)
{
    String aOld = aInhalt.GetText();

    aOld += aNew;
    aOld.ConvertLineEnd();
    aInhalt.SetText(aOld);
    if (bMoveToEnd)
        aInhalt.SetSelection(Selection(SELECTION_MAX,SELECTION_MAX));
}


EditFileWindow::EditFileWindow(Window * pParent, String aName, WinBits iWstyle)
: GHEditWindow(pParent, aName, iWstyle)
, aFileName(aName)
{
    LoadFile();
}

void EditFileWindow::LoadFile()
{

    SvFileStream Stream;
    String       All,Line;

    Stream.Open(aFileName, STREAM_STD_READ);

    if (!Stream.IsOpen())
    {
        AddText(CUniString("could not open ").Append(aFileName).AppendAscii("\n"));
        aFileName.Erase();
        return;
    }

    while (!Stream.IsEof())
    {

        Stream.ReadByteStringLine( Line, RTL_TEXTENCODING_UTF8 );

        All += Line;
        All += '\n';

    }

    All.ConvertLineEnd();

    AddText(All,sal_False);

}

sal_Bool EditFileWindow::Close()
{

    if (aInhalt.IsModified() && QueryBox(this,WB_DEF_YES | WB_YES_NO_CANCEL, String(aFileName).AppendAscii("\nhas been changed.\n\nSave file?")).Execute())
    {

    }
    return GHEditWindow::Close();
}

