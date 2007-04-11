/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontworkbar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:53:23 $
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

#ifndef _SVX_FONTWORK_BAR_HXX
#define _SVX_FONTWORK_BAR_HXX

#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif

#ifndef _SVX_IFACEIDS_HXX
#include <svx/ifaceids.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SfxViewShell;
class SdrView;

/************************************************************************/

namespace svx
{

bool SVX_DLLPUBLIC checkForSelectedFontWork( SdrView* pSdrView, sal_uInt32& nCheckStatus );

class SVX_DLLPUBLIC FontworkBar : public SfxShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SVX_INTERFACE_FONTWORK_BAR)

     FontworkBar(SfxViewShell* pViewShell );
    ~FontworkBar();

    static void execute( SdrView* pSdrView, SfxRequest& rReq, SfxBindings& rBindings );
    static void getState( SdrView* pSdrView, SfxItemSet& rSet );
};

}

#endif          // _SVX_FONTWORK_BAR_HXX
