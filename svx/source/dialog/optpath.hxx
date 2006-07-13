/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optpath.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 12:01:24 $
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
#ifndef SVX_CONTROLL_FUCUS_HELPER_HXX
#include "ControlFocusHelper.hxx"
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

// class SvxPathTabPage --------------------------------------------------

class SvxPathTabPage : public SfxTabPage
{
private:
    FixedText           aTypeText;
    FixedText           aPathText;
    SvxControlFocusHelper aPathCtrl;
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

    void        GetPathList( USHORT _nPathHandle,
                             String& _rInternalPath, String& _rUserPath, String& _rWritablePath );
    void        SetPathList( USHORT _nPathHandle,
                             const String& _rUserPath, const String& _rWritablePath );
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

