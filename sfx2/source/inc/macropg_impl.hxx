/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macropg_impl.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
    Timer                           maFillGroupTimer;
};

#endif

