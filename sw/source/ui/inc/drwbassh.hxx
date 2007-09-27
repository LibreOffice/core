/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drwbassh.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:58:38 $
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
#ifndef _SWDRWBASSH_HXX
#define _SWDRWBASSH_HXX
#include "basesh.hxx"

class SwView;
class SfxItemSet;
class SwDrawBase;
class AbstractSvxNameDialog;
struct SvxSwFrameValidation;

class SwDrawBaseShell: public SwBaseShell
{
    SwDrawBase* pDrawActual;

    UINT16      eDrawMode;
    BOOL        bRotate : 1;
    BOOL        bSelMove: 1;

    DECL_LINK( CheckGroupShapeNameHdl, AbstractSvxNameDialog* );
    DECL_LINK(ValidatePosition, SvxSwFrameValidation* );
public:
                SwDrawBaseShell(SwView &rShell);
    virtual     ~SwDrawBaseShell();

    SFX_DECL_INTERFACE(SW_DRAWBASESHELL)
    TYPEINFO();

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        DisableState(SfxItemSet &rSet)               { Disable(rSet);}
    BOOL        Disable(SfxItemSet& rSet, USHORT nWhich = 0);

    void        StateStatusline(SfxItemSet &rSet);

};


#endif
