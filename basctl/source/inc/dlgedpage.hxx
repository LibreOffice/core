/*************************************************************************
 *
 *  $RCSfile: dlgedpage.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tbe $ $Date: 2001-02-26 11:08:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BASCTL_DLGEDPAGE_HXX
#define _BASCTL_DLGEDPAGE_HXX

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include "svx/svdundo.hxx"
#endif
#ifndef _SVDPAGE_HXX //autogen
#include "svx/svdpage.hxx"
#endif
#ifndef _SVDOBJ_HXX //autogen
#include "svx/svdobj.hxx"
#endif
#ifndef _TOOLS_SOALR_H
#include <tools/solar.h>
#endif

class SfxPoolItem;


//============================================================================
// DlgPage
//============================================================================

class StarBASIC;
class VCDlgEditor;
class DlgEdModel;
class HelpEvent;  // this probably doesn't work, need include file vcsbx.hxx

class DlgPage : public SdrPage
{

protected:
    StarBASIC*      pBasic;
    String          aPageName;

public:
    TYPEINFO();

    DlgPage( DlgEdModel& rModel, StarBASIC* pBasic, FASTBOOL bMasterPage=FALSE );
    DlgPage( const DlgPage& );
    ~DlgPage();

    void            SetName(const String& rPageName);
    const String&   GetName() const { return aPageName; }

    void            SetBasic( StarBASIC* pBas );
    StarBASIC*      GetBasic() const { return pBasic; }

    BOOL            RequestHelp( Window* pWin, SdrView* pView, const HelpEvent& rEvt );
    virtual void    RequestBasic();

    virtual void    WriteData(SvStream& rOut) const;
    virtual void    ReadData(const SdrIOHeader& rHead, SvStream& rIn);
    virtual void    SetModel(SdrModel* pNewModel);

    virtual SdrPage* Clone() const;
};

//============================================================================
// DlgEdPage
//============================================================================

class DlgEdPage : public DlgPage
{
protected:
    VCDlgEditor*    pEditor;

public:
    DlgEdPage( DlgEdModel& rModel, StarBASIC* pBasic, VCDlgEditor* pEd ) :
    DlgPage( rModel, pBasic ) { pEditor=pEd; }

    VCDlgEditor*    GetDlgEd() const { return pEditor; }
};

#endif //_BASCTL_DLGEDPAGE_HXX
