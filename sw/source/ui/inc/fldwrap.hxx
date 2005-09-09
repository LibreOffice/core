/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldwrap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:14:17 $
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
#ifndef _FLDWRAP_HXX
#define _FLDWRAP_HXX

#include "chldwrap.hxx"
class AbstractSwFldDlg;

class SwFldDlgWrapper : public SwChildWinWrapper
{
public:
    AbstractSwFldDlg * pDlgInterface; // CHINA001 this member hold the pointer to the interface AbstractSwFldDlg
    SwFldDlgWrapper( Window* pParent, USHORT nId,
                        SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwFldDlgWrapper);

    virtual BOOL    ReInitDlg(SwDocShell *pDocSh);
    void            ShowPage(USHORT nPage = 0);
};
/* -----------------04.02.2003 14:14-----------------
 * field dialog only showing database page to support
 * mail merge
 * --------------------------------------------------*/
class SwFldDataOnlyDlgWrapper : public SwChildWinWrapper
{
public:
    AbstractSwFldDlg * pDlgInterface; // CHINA001 this member hold the pointer to the interface AbstractSwFldDlg
    SwFldDataOnlyDlgWrapper( Window* pParent, USHORT nId,
                        SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwFldDataOnlyDlgWrapper);

    virtual BOOL    ReInitDlg(SwDocShell *pDocSh);
};


#endif

