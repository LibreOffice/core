/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVX_BKWND_HYPERLINK_HXX
#define _SVX_BKWND_HYPERLINK_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <vcl/dialog.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <svtools/svtreebx.hxx>

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

    sal_Bool            mbUserMoved;
    sal_Bool            mbFirst;

    SvxHyperlinkTabPageBase* mpParent;

    String          maStrLastURL;

    sal_uInt16          mnError;

protected:
    sal_Bool RefreshFromDoc( ::rtl::OUString aURL );

    SvLBoxEntry* FindEntry ( String aStrName );
    void ClearTree();
    int FillTree( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > xLinks, SvLBoxEntry* pParentEntry =NULL );

    virtual void Move ();

    DECL_LINK (ClickApplyHdl_Impl, void * );
    DECL_LINK (ClickCloseHdl_Impl, void * );

public:
    SvxHlinkDlgMarkWnd (SvxHyperlinkTabPageBase *pParent);
    ~SvxHlinkDlgMarkWnd();

    sal_Bool MoveTo ( Point aNewPos );
    void RefreshTree ( String aStrURL );
    void SelectEntry ( String aStrMark );

    sal_Bool ConnectToDialog( sal_Bool bDoit = sal_True );

    sal_uInt16 SetError( sal_uInt16 nError);
};


#endif  // _SVX_BKWND_HYPERLINK_HXX
