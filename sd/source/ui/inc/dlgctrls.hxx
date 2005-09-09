/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgctrls.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:25:51 $
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


#ifndef SD_DLGCTRLS_HXX
#define SD_DLGCTRLS_HXX

#ifndef _SD_TRANSITIONPRESET_HXX
#include "TransitionPreset.hxx"
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif
#ifndef _SD_FADEDEF_H
#include "fadedef.h"
#endif

#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

/*************************************************************************
|*
|* FadeEffectLB
|*
\************************************************************************/

struct FadeEffectLBImpl;

class SD_DLLPUBLIC FadeEffectLB : public ListBox
{
public:
                                FadeEffectLB( Window* pParent, SdResId Id );
                                FadeEffectLB( Window* pParent, WinBits aWB );
                                ~FadeEffectLB();
    virtual void                Fill();

/*  void                        selectEffectFromPage( SdPage* pPage ); */
    void                        applySelected( SdPage* pSlide ) const;

private:
    USHORT                      GetSelectEntryPos() const { return ListBox::GetSelectEntryPos(); }
    void                        SelectEntryPos( USHORT nPos ) { ListBox::SelectEntryPos( nPos ); }

    FadeEffectLBImpl*           mpImpl;
};

#endif // SD_DLGCTRLS_HXX
