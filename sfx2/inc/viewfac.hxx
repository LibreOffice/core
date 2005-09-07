/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewfac.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:23:37 $
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
#ifndef _VIEWFAC_HXX
#define _VIEWFAC_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

class SfxViewFrame;
class SfxViewShell;
class Window;

typedef SfxViewShell* (*SfxViewCtor)(SfxViewFrame*, SfxViewShell*);
typedef void (*SfxViewInit)();

// CLASS -----------------------------------------------------------------
class SFX2_DLLPUBLIC SfxViewFactory
{
public:
    SfxViewFactory( SfxViewCtor fnC, SfxViewInit fnI,
                    USHORT nOrdinal, const ResId& aDescrResId );
    ~SfxViewFactory();

    SfxViewShell *CreateInstance(SfxViewFrame *pViewFrame, SfxViewShell *pOldSh);
    void          InitFactory();

    String        GetDescription() const
                  { return String( aDescription ); }
    USHORT        GetOrdinal() const { return nOrd; }

private:
    SfxViewCtor fnCreate;
    SfxViewInit fnInit;
    USHORT      nOrd;
    ResId       aDescription;
};

#endif

