/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SWMAILMERGECHILDWINDOW_HXX
#define _SWMAILMERGECHILDWINDOW_HXX
#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/toolbox.hxx>
#include "swdllapi.h"

/*-- 25.05.2004 15:25:56---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeChildWin : public SfxFloatingWindow
{
    ToolBox                 m_aBackTB;

    DECL_LINK( BackHdl,      ToolBox* );

public:
    SwMailMergeChildWin(SfxBindings*, SfxChildWindow*, Window *pParent);
    ~SwMailMergeChildWin();

    virtual void    FillInfo(SfxChildWinInfo&) const;
};

/*-- 25.05.2004 15:25:56---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwSendMailDialog;
class SwMailMergeChildWindow : public SfxChildWindow
{
public:
    SwMailMergeChildWindow( Window* ,
                    sal_uInt16 nId,
                    SfxBindings*,
                    SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW( SwMailMergeChildWindow );
};

#endif

