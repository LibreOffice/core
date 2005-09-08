/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: redcom.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:46:33 $
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

#ifndef SC_REDCOM_HXX
#define SC_REDCOM_HXX

//CHINA001 #ifndef _SVX_POSTDLG_HXX //autogen
//CHINA001 #include <svx/postdlg.hxx>
//CHINA001 #endif

#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif

class ScDocShell;
class AbstractSvxPostItDialog; //CHINA001

class ScRedComDialog
{
private:

    ScChangeAction  *pChangeAction;
    ScDocShell      *pDocShell;
    String          aComment;
    AbstractSvxPostItDialog* pDlg;

    DECL_LINK( PrevHdl, AbstractSvxPostItDialog* );
    DECL_LINK( NextHdl, AbstractSvxPostItDialog* );

protected:

    void    ReInit(ScChangeAction *);
    void    SelectCell();

    ScChangeAction *FindPrev(ScChangeAction *pAction);
    ScChangeAction *FindNext(ScChangeAction *pAction);

public:

    ScRedComDialog( Window* pParent, const SfxItemSet& rCoreSet,
                    ScDocShell *,ScChangeAction *,BOOL bPrevNext = FALSE);
    ~ScRedComDialog();

    short Execute();
};

#endif

