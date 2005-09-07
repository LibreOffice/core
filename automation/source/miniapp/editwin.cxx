/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editwin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:18:55 $
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

#include "editwin.hxx"
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif


BOOL GHEditWindow::Close()
{
    if (aInhalt.IsModified())
    {
    }
    delete(this);
    return TRUE;
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

void GHEditWindow::AddText( String aNew, BOOL bMoveToEnd)
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

    AddText(All,FALSE);

}

BOOL EditFileWindow::Close()
{

    if (aInhalt.IsModified() && QueryBox(this,WB_DEF_YES | WB_YES_NO_CANCEL, String(aFileName).AppendAscii("\nhas been changed.\n\nSave file?")).Execute())
    {

    }
    return GHEditWindow::Close();
}

