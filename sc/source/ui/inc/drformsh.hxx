/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drformsh.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:22:00 $
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

#ifndef DRFORMSH_HXX
#define DRFORMSH_HXX

#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#include "shellids.hxx"
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif

#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif

class ScViewData;


#include "drawsh.hxx"

class ScDrawFormShell: public ScDrawShell
{
public:

    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_FORM_SHELL)

                ScDrawFormShell(ScViewData* pData);
    virtual     ~ScDrawFormShell();

//  void        Execute(SfxRequest &);
//  void        GetState(SfxItemSet &);
};

#endif
