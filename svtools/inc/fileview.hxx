/*************************************************************************
 *
 *  $RCSfile: fileview.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:32:20 $
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
#ifndef _SVT_FILEVIEW_HXX
#define _SVT_FILEVIEW_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif

#include <vcl/ctrl.hxx>
#include <vcl/image.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>

// class SvtFileView -----------------------------------------------------

#define FILEVIEW_ONLYFOLDER         0x0001
#define FILEVIEW_MULTISELECTION     0x0002

#define FILEVIEW_SHOW_TITLE         0x0010
#define FILEVIEW_SHOW_SIZE          0x0020
#define FILEVIEW_SHOW_DATE          0x0040
#define FILEVIEW_SHOW_ALL           0x0070

class ViewTabListBox_Impl;
class SvtFileView_Impl;
class SvLBoxEntry;
class HeaderBar;

class IUrlFilter;
class SvtFileView : public Control
{
private:
    SvtFileView_Impl*       mpImp;

    void                    OpenFolder( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aContents );

    DECL_LINK(              HeaderSelect_Impl, HeaderBar * );
    DECL_LINK(              HeaderEndDrag_Impl, HeaderBar * );

protected:
    virtual void GetFocus();

public:
    SvtFileView( Window* pParent, const ResId& rResId, sal_Bool bOnlyFolder, sal_Bool bMultiSelection );
    SvtFileView( Window* pParent, const ResId& rResId, sal_Int8 nFlags );
    ~SvtFileView();

    const String&           GetViewURL() const;
    String                  GetURL( SvLBoxEntry* pEntry ) const;
    String                  GetCurrentURL() const;

    sal_Bool                CreateNewFolder( const String& rNewFolder );
    sal_Bool                HasPreviousLevel( String& rParentURL ) const;
    sal_Bool                PreviousLevel( String& rNewURL );

    void                    SetHelpId( sal_uInt32 nHelpId );
    sal_uInt32              GetHelpId( ) const;
    void                    SetSizePixel( const Size& rNewSize );
    void                    SetPosSizePixel( const Point& rNewPos, const Size& rNewSize );
    sal_Bool                Initialize( const String& rURL, const String& rFilter );
    sal_Bool                Initialize( const String& rURL,
                                        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aContents );

    sal_Bool                Initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent, const String& rFilter );

    sal_Bool                ExecuteFilter( const String& rFilter );
    void                    SetNoSelection();
    void                    ResetCursor();

    void                    SetSelectHdl( const Link& rHdl );
    void                    SetDoubleClickHdl( const Link& rHdl );
    void                    SetOpenDoneHdl( const Link& rHdl );

    ULONG                   GetSelectionCount() const;
    SvLBoxEntry*            FirstSelected() const;
    SvLBoxEntry*            NextSelected( SvLBoxEntry* pEntry ) const;
    void                    EnableAutoResize();
    void                    SetFocus();

    void                    EnableContextMenu( sal_Bool bEnable );
    void                    EnableDelete( sal_Bool bEnable );
    void                    EnableNameReplacing( sal_Bool bEnable = sal_True );
                                // translate folder names or display doc-title instead of file name
                                // EnableContextMenu( TRUE )/EnableDelete(TRUE) disable name replacing!

                            // save and load column size and sort order
    String                  GetConfigString() const;
    void                    SetConfigString( const String& rCfgStr );

    void                    SetUrlFilter( const IUrlFilter* _pFilter );
    const IUrlFilter*       GetUrlFilter( ) const;
};

// struct SvtContentEntry ------------------------------------------------

struct SvtContentEntry
{
    sal_Bool    mbIsFolder;
    UniString   maURL;

    SvtContentEntry( const UniString& rURL, sal_Bool bIsFolder ) :
        mbIsFolder( bIsFolder ), maURL( rURL ) {}
};

namespace svtools {

// -----------------------------------------------------------------------
// QueryDeleteDlg_Impl
// -----------------------------------------------------------------------

enum QueryDeleteResult_Impl
{
    QUERYDELETE_YES = 0,
    QUERYDELETE_NO,
    QUERYDELETE_ALL,
    QUERYDELETE_CANCEL
};

class QueryDeleteDlg_Impl : public ModalDialog
{
    FixedText               _aEntryLabel;
    FixedText               _aEntry;
    FixedText               _aQueryMsg;

    PushButton              _aYesButton;
    PushButton              _aAllButton;
    PushButton              _aNoButton;
    CancelButton            _aCancelButton;

    QueryDeleteResult_Impl  _eResult;

private:

    DECL_STATIC_LINK( QueryDeleteDlg_Impl, ClickLink, PushButton* );

public:

                            QueryDeleteDlg_Impl( Window* pParent,
                                                 const String& rName );

    void                    EnableAllButton() { _aAllButton.Enable( sal_True ); }
    QueryDeleteResult_Impl  GetResult() const { return _eResult; }
};

}

#endif // _SVT_FILEVIEW_HXX

