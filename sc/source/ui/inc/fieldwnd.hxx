/*************************************************************************
 *
 *  $RCSfile: fieldwnd.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_FIELDWND_HXX
#define SC_FIELDWND_HXX


#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif


#define MAX_LABELS  256
#define PAGE_SIZE   16
#define LINE_SIZE   8
#define MAX_FIELDS  8
#define EMPTY_SLOT  0xffff

#define OWIDTH  PivotGlobal::nObjWidth
#define OHEIGHT PivotGlobal::nObjHeight
#define SSPACE  PivotGlobal::nSelSpace

//===================================================================

enum FieldType
{
    TYPE_ROW, TYPE_COL, TYPE_DATA, TYPE_SELECT
};

//-------------------------------------------------------------------
class ScPivotLayoutDlg;

class FieldWindow : public Window
{
public:
    FieldWindow( ScPivotLayoutDlg* pDialog, const ResId& rResId,
                 FieldType eType );
    ~FieldWindow();

    BOOL    AddField        ( const String& rStr, const Point& rAt, USHORT& rAddedAt );
    void    AddField        ( const String& rStr, USHORT nIndex );
    void    DelField        ( USHORT nFieldIndex );
    void    ClearFields     ();
    void    Redraw          ();
    void    SetText     ( const String& rStr, USHORT nIndex );
    BOOL    GetInsertIndex  ( const Point& rInsertPos, USHORT& rIndex );

protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    MouseButtonDown ( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp   ( const MouseEvent& rMEvt );
    virtual void    MouseMove       ( const MouseEvent& rMEvt );

private:
    ScPivotLayoutDlg*   pDlg;
    Rectangle           aWndRect;
    FieldType           eFieldType;
    Point               aCenterPos;
    String**            aFieldArr;
    USHORT              nFieldSize;
    USHORT              nFieldCount;

private:
    void    DoPaint             ( const Rectangle& rRect );
    void    DrawField           ( OutputDevice& rDev,
                                  const Rectangle& rRect,
                                  const String& rStr );
    BOOL    GetFieldIndex       ( const Point& rPos, USHORT& rIndex );
    BOOL    GetFirstEmptySlot   ( USHORT& rIndex );
};


#endif // SC_FIELDWND_HXX
