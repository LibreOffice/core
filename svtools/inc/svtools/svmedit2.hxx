/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svmedit2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:37:16 $
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
#ifndef _SVTOOLS_SVMEDIT2_HXX
#define _SVTOOLS_SVMEDIT2_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

class TextAttrib;

class SVT_DLLPUBLIC ExtMultiLineEdit : public MultiLineEdit
{
public:
                    ExtMultiLineEdit( Window* pParent, WinBits nWinStyle = WB_LEFT | WB_BORDER );
                    ExtMultiLineEdit( Window* pParent, const ResId& rResId );
                    ~ExtMultiLineEdit();

                    // methods of TextView
    void            InsertText( const String& rNew, BOOL bSelect = FALSE );
    void            SetAutoScroll( BOOL bAutoScroll );
    void            EnableCursor( BOOL bEnable );

                    // methods of TextEngine
    void            SetAttrib( const TextAttrib& rAttr, ULONG nPara, USHORT nStart, USHORT nEnd );
    void            SetLeftMargin( USHORT nLeftMargin );
    ULONG           GetParagraphCount() const;
};

#endif

