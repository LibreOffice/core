/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mailmergechildwindow.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:04:04 $
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
#ifndef _SWMAILMERGECHILDWINDOW_HXX
#define _SWMAILMERGECHILDWINDOW_HXX
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

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
                    USHORT nId,
                    SfxBindings*,
                    SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW( SwMailMergeChildWindow );
};

#endif

