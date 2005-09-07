/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macropg_impl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:05:39 $
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

#ifndef _MACROPG_IMPL_HXX
#define _MACROPG_IMPL_HXX

class _SfxMacroTabPage_Impl
{
public:
                                    _SfxMacroTabPage_Impl( void );
                                    ~_SfxMacroTabPage_Impl();

    String                          maStaticMacroLBLabel;
    PushButton*                     pAssignPB;
    PushButton*                     pDeletePB;
    String*                         pStrEvent;
    String*                         pAssignedMacro;
    ListBox*                        pScriptTypeLB;
    _HeaderTabListBox*              pEventLB;
    SfxConfigGroupListBox_Impl*     pGroupLB;
    FixedText*                      pFT_MacroLBLabel;
    SfxConfigFunctionListBox_Impl*  pMacroLB;

    FixedText*                      pMacroFT;
    String*                         pMacroStr;
    FNGetRangeHdl                   fnGetRange;
    FNGetMacrosOfRangeHdl           fnGetMacroOfRange;

    BOOL                            bReadOnly;
};


#endif
