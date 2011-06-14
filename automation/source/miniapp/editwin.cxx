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
#include "precompiled_automation.hxx"

#include "editwin.hxx"
#include <tools/stream.hxx>
#include <vcl/msgbox.hxx>


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
