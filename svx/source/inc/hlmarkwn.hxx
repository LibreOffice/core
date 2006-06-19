/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hlmarkwn.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 16:08:22 $
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

#ifndef _SVX_BKWND_HYPERLINK_HXX
#define _SVX_BKWND_HYPERLINK_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#include "hlmarkwn_def.hxx" //ADD CHINA001
class SvxHyperlinkTabPageBase;

//########################################################################
//#                                                                      #
//# Tree-Window                                                          #
//#                                                                      #
//########################################################################

class SvxHlinkDlgMarkWnd;

class SvxHlmarkTreeLBox : public SvTreeListBox
{
private:
    SvxHlinkDlgMarkWnd* mpParentWnd;

public:
    SvxHlmarkTreeLBox( Window* pParent, const ResId& rResId );

    virtual void Paint( const Rectangle& rRect );
};

//########################################################################
//#                                                                      #
//# Window-Class                                                         #
//#                                                                      #
//########################################################################

class SvxHlinkDlgMarkWnd : public ModalDialog //FloatingWindow
{
private:
    friend class SvxHlmarkTreeLBox;

    PushButton      maBtApply;
    PushButton      maBtClose;
    //SvTreeListBox maLbTree;
    SvxHlmarkTreeLBox maLbTree;

    BOOL            mbUserMoved;
    BOOL            mbFirst;

    SvxHyperlinkTabPageBase* mpParent;

    String          maStrLastURL;

    USHORT          mnError;

protected:
    BOOL RefreshFromDoc( ::rtl::OUString aURL );

    SvLBoxEntry* FindEntry ( String aStrName );
    void ClearTree();
    int FillTree( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > xLinks, SvLBoxEntry* pParentEntry =NULL );

    virtual void Move ();

    DECL_LINK (ClickApplyHdl_Impl, void * );
    DECL_LINK (ClickCloseHdl_Impl, void * );

public:
    SvxHlinkDlgMarkWnd (SvxHyperlinkTabPageBase *pParent);
    ~SvxHlinkDlgMarkWnd();

    const BOOL MoveTo ( Point aNewPos );
    void RefreshTree ( String aStrURL );
    void SelectEntry ( String aStrMark );

    const BOOL ConnectToDialog( BOOL bDoit = TRUE );

    USHORT SetError( USHORT nError);
};


#endif  // _SVX_BKWND_HYPERLINK_HXX
