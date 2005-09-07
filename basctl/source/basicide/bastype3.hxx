/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bastype3.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:01:01 $
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
#ifndef _BASTYPE3_HXX
#define _BASTYPE3_HXX

#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif

#include <svtools/svmedit.hxx>

#include <iderid.hxx>

class EditorWindow;

#ifndef NO_SPECIALEDIT

class ExtendedEdit : public Edit
{
private:
    Accelerator     aAcc;
    Link            aAccHdl;
    Link            aGotFocusHdl;
    Link            aLoseFocusHdl;

protected:
    DECL_LINK( EditAccHdl, Accelerator * );
    DECL_LINK( ImplGetFocusHdl, Control* );
    DECL_LINK( ImplLoseFocusHdl, Control* );

public:
                    ExtendedEdit( Window* pParent, IDEResId nRes );

    void            SetAccHdl( const Link& rLink )          { aAccHdl = rLink; }
    void            SetLoseFocusHdl( const Link& rLink )    { aLoseFocusHdl = rLink; }
    void            SetGotFocusHdl( const Link& rLink )     { aGotFocusHdl = rLink; }
    Accelerator&    GetAccelerator()                        { return aAcc; }
};

class ExtendedMultiLineEdit : public MultiLineEdit
{
private:
    Accelerator     aAcc;
    Link            aAccHdl;

protected:
    DECL_LINK( EditAccHdl, Accelerator * );
    DECL_LINK( ImplGetFocusHdl, Control* );
    DECL_LINK( ImplLoseFocusHdl, Control* );

public:
                    ExtendedMultiLineEdit( Window* pParent, IDEResId nRes );

    void            SetAccHdl( const Link& rLink )          { aAccHdl = rLink; }
    Accelerator&    GetAccelerator()                        { return aAcc; }
};

#endif  //NO_SPECIALEDIT

#endif  // _BASTYPE3_HXX
