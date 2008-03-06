/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selector.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:30:58 $
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
#ifndef _SVXSELECTOR_HXX
#define _SVXSELECTOR_HXX

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#define _SVSTDARR_USHORTS
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>     // SvUShorts
#include <sfx2/minarray.hxx>

#define SVX_CFGGROUP_FUNCTION 1
#define SVX_CFGFUNCTION_SLOT  2
#define SVX_CFGGROUP_SCRIPTCONTAINER  3
#define SVX_CFGFUNCTION_SCRIPT 4

struct SvxGroupInfo_Impl
{
    USHORT          nKind;
    USHORT          nOrd;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >
                    xBrowseNode;
    ::rtl::OUString sURL;
    ::rtl::OUString sHelpText;
    BOOL            bWasOpened;

    SvxGroupInfo_Impl( USHORT n, USHORT nr )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode()
        ,sURL()
        ,sHelpText()
        ,bWasOpened(FALSE)
    {
    }

    SvxGroupInfo_Impl( USHORT n, USHORT nr, const ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& _rxNode )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode( _rxNode )
        ,sURL()
        ,sHelpText()
        ,bWasOpened(FALSE)
    {
    }

    SvxGroupInfo_Impl( USHORT n, USHORT nr, const ::rtl::OUString& _rURL, const ::rtl::OUString& _rHelpText )
        :nKind( n )
        ,nOrd( nr )
        ,xBrowseNode()
        ,sURL( _rURL )
        ,sHelpText( _rHelpText )
        ,bWasOpened(FALSE)
    {
    }
};

typedef SvxGroupInfo_Impl* SvxGroupInfoPtr;
SV_DECL_PTRARR_DEL(SvxGroupInfoArr_Impl, SvxGroupInfoPtr, 5, 5)
DECL_2BYTEARRAY(USHORTArr, USHORT, 10, 10)

class ImageProvider
{
public:
    virtual ~ImageProvider() {}

    virtual Image GetImage( const rtl::OUString& rCommandURL ) = 0;
};

class SvxConfigFunctionListBox_Impl : public SvTreeListBox
{
friend class SvxConfigGroupListBox_Impl;
    Timer                           aTimer;
    SvLBoxEntry*                    pCurEntry;
    SvxGroupInfoArr_Impl            aArr;
    SvLBoxEntry*                    m_pDraggingEntry;

    DECL_LINK(                      TimerHdl, Timer* );
    virtual void                    MouseMove( const MouseEvent& rMEvt );

public:
                                    SvxConfigFunctionListBox_Impl( Window*, const ResId& );
                                    ~SvxConfigFunctionListBox_Impl();
    void                            ClearAll();
    SvLBoxEntry*                    GetEntry_Impl( USHORT nId );
    SvLBoxEntry*                    GetEntry_Impl( const String& );
    USHORT                          GetId( SvLBoxEntry *pEntry );
    String                          GetHelpText( SvLBoxEntry *pEntry );
    using Window::GetHelpText;
    USHORT                          GetCurId()
                                    { return GetId( FirstSelected() ); }
    SvLBoxEntry*                    GetLastSelectedEntry();
    void                            FunctionSelected();

    // drag n drop methods
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvLBoxEntry* );

    virtual void        DragFinished( sal_Int8 );
};

class SvxConfigGroupListBox_Impl : public SvTreeListBox
{
    SvxGroupInfoArr_Impl            aArr;
    bool                            m_bShowSlots;

    SvxConfigFunctionListBox_Impl*  pFunctionListBox;
    ImageProvider*                  m_pImageProvider;

    ::com::sun::star::uno::Reference
        < ::com::sun::star::frame::XFrame > m_xFrame;

    ::com::sun::star::uno::Reference
        < ::com::sun::star::container::XNameAccess > m_xModuleCommands;

    Image m_hdImage;
    Image m_hdImage_hc;
    Image m_libImage;
    Image m_libImage_hc;
    Image m_macImage;
    Image m_macImage_hc;
    Image m_docImage;
    Image m_docImage_hc;
    ::rtl::OUString m_sMyMacros;
    ::rtl::OUString m_sProdMacros;
    Image GetImage( ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > node, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xCtx, bool bIsRootNode, bool bHighContrast );
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx, ::rtl::OUString& docName );

private:
    void    fillScriptList(
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& _rxRootNode,
        SvLBoxEntry* _pParentEntry,
        bool _bCheapChildsOnDemand
    );

protected:
    virtual void    RequestingChilds( SvLBoxEntry *pEntry);
    virtual BOOL    Expand( SvLBoxEntry* pParent );
    using SvListView::Expand;

public:
            SvxConfigGroupListBox_Impl (
                Window* pParent, const ResId&,
                bool _bShowSlots,
                const ::com::sun::star::uno::Reference
                    < ::com::sun::star::frame::XFrame >& xFrame
            );

            ~SvxConfigGroupListBox_Impl();

    void    Init();
    void    Open( SvLBoxEntry*, BOOL );
    void    ClearAll();
    void    GroupSelected();

    void    SetFunctionListBox( SvxConfigFunctionListBox_Impl *pBox )
        { pFunctionListBox = pBox; }

    void    SetImageProvider( ImageProvider* provider )
        { m_pImageProvider = provider; }
};

class SVX_DLLPUBLIC SvxScriptSelectorDialog : public ModelessDialog
{
    FixedText                       aDialogDescription;
    FixedText                       aGroupText;
    SvxConfigGroupListBox_Impl      aCategories;
    FixedText                       aFunctionText;
    SvxConfigFunctionListBox_Impl   aCommands;
    OKButton                        aOKButton;
    CancelButton                    aCancelButton;
    HelpButton                      aHelpButton;
    FixedLine                       aDescription;
    FixedText                       aDescriptionText;

    BOOL                            m_bShowSlots;
    Link                            m_aAddHdl;

    DECL_LINK( ClickHdl, Button * );
    DECL_LINK( SelectHdl, Control* );

    void                                UpdateUI();
    void                                ResizeControls();

public:

    SvxScriptSelectorDialog (
        Window* pParent = NULL,
        BOOL bShowSlots = FALSE,
        const ::com::sun::star::uno::Reference
            < ::com::sun::star::frame::XFrame >& xFrame = 0
    );

    ~SvxScriptSelectorDialog ( );

    void        SetAddHdl( const Link& rLink ) { m_aAddHdl = rLink; }
    const Link& GetAddHdl() const { return m_aAddHdl; }

    void        SetImageProvider( ImageProvider* provider )
        { aCategories.SetImageProvider( provider ); }

    USHORT      GetSelectedId();
    String      GetScriptURL() const;
    String      GetSelectedDisplayName();
    String      GetSelectedHelpText();
    void        SetRunLabel();
    void        SetDialogDescription(const String& rDescription);
};

#endif
