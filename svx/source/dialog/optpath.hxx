/*************************************************************************
 *
 *  $RCSfile: optpath.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:40:51 $
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
#ifndef _SVX_OPTPATH_HXX
#define _SVX_OPTPATH_HXX

// include ---------------------------------------------------------------

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifdef _SVX_OPTPATH_CXX
#ifndef _HEADBAR_HXX //autogen
#include <svtools/headbar.hxx>
#endif
#else
class HeaderBar;
#endif

// forward ---------------------------------------------------------------

class SvTabListBox;
namespace svx
{
    class OptHeaderTabListBox;
}
struct OptPath_Impl;

// define ----------------------------------------------------------------

#define SfxPathTabPage SvxPathTabPage

// class SvxPathControl_Impl ---------------------------------------------

class SvxPathControl_Impl : public Control
{
private:
    Control*        m_pFocusCtrl;

public:
    SvxPathControl_Impl( Window* pParent, const ResId& rId ) :
        Control( pParent, rId ), m_pFocusCtrl( NULL ) {}

    void            SetFocusControl( Control* pCtrl ) { m_pFocusCtrl = pCtrl; }

    virtual long    Notify( NotifyEvent& rNEvt );
};

// class SvxPathTabPage --------------------------------------------------

class SvxPathTabPage : public SfxTabPage
{
private:
    FixedText           aTypeText;
    FixedText           aPathText;
    SvxPathControl_Impl aPathCtrl;
    PushButton          aStandardBtn;
    PushButton          aPathBtn;
    FixedLine           aStdBox;

    HeaderBar*                  pHeaderBar;
    ::svx::OptHeaderTabListBox* pPathBox;
    OptPath_Impl*               pImpl;

#ifdef _SVX_OPTPATH_CXX
    DECL_LINK( PathHdl_Impl, PushButton * );
    DECL_LINK( StandardHdl_Impl, PushButton * );

    DECL_LINK( PathSelect_Impl, OptHeaderTabListBox * );
    DECL_LINK( HeaderSelect_Impl, HeaderBar * );
    DECL_LINK( HeaderEndDrag_Impl, HeaderBar * );
#endif

public:
    SvxPathTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxPathTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static USHORT*      GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        FillUserData();
};


#endif

