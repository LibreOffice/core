/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdotxfl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:57:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#include "svdfield.hxx"
#include "svdotext.hxx"

static BOOL bInit = FALSE;

// Do not remove this, it is still used in src536a!
void SdrRegisterFieldClasses()
{
    if ( !bInit )
    {
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SdrMeasureField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxHeaderField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxFooterField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxDateTimeField);
        bInit = TRUE;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////// */

FASTBOOL SdrTextObj::CalcFieldValue(const SvxFieldItem& /*rField*/, USHORT /*nPara*/, USHORT /*nPos*/,
    FASTBOOL /*bEdit*/, Color*& /*rpTxtColor*/, Color*& /*rpFldColor*/, XubString& /*rRet*/) const
{
    return FALSE;
}

