/*************************************************************************
 *
 *  $RCSfile: selector.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:14:51 $
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLECOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#endif
#include <drafts/com/sun/star/script/browse/XBrowseNode.hpp>

#define _SVSTDARR_USHORTS
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>     // SvUShorts
#include <sfx2/minarray.hxx>

class SfxMacroInfoArr_Impl;
class SfxMacroInfoItem;
class SfxMacroInfo;

#define SFX_CFGGROUP_FUNCTION 1
#define SFX_CFGGROUP_BASICMGR 2
#define SFX_CFGGROUP_DOCBASICMGR 3
#define SFX_CFGGROUP_BASICLIB 4
#define SFX_CFGGROUP_BASICMOD 5
#define SFX_CFGFUNCTION_MACRO 6
#define SFX_CFGFUNCTION_SLOT  7
#define SFX_CFGGROUP_SCRIPTCONTAINER  8
#define SFX_CFGFUNCTION_SCRIPT 9

struct SvxGroupInfo_Impl
{
    USHORT      nKind;
    USHORT      nOrd;
    void*       pObject;
    BOOL        bWasOpened;

    SvxGroupInfo_Impl( USHORT n, USHORT nr, void* pObj = 0 ) :
        nKind( n ), nOrd( nr ), pObject( pObj ), bWasOpened(FALSE) {}
};

typedef SvxGroupInfo_Impl* SvxGroupInfoPtr;
SV_DECL_PTRARR_DEL(SvxGroupInfoArr_Impl, SvxGroupInfoPtr, 5, 5);
DECL_2BYTEARRAY(USHORTArr, USHORT, 10, 10);

class ImageProvider
{
public:
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
    USHORT                          GetCurId()
                                    { return GetId( FirstSelected() ); }
    SvLBoxEntry*                    GetLastSelectedEntry();
    SfxMacroInfo*                   GetMacroInfo();
    void                            FunctionSelected();

    // drag n drop methods
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvLBoxEntry* );

    virtual void        DragFinished( sal_Int8 );
};

class SfxSlotPool;
class SvxConfigGroupListBox_Impl : public SvTreeListBox
{
    SvxGroupInfoArr_Impl            aArr;
    ULONG                           nMode;
    SfxSlotPool*                    pSlotPool;

    SvxConfigFunctionListBox_Impl*  pFunctionListBox;
    ImageProvider*                  m_pImageProvider;

    // show Scripting Framework scripts?
    BOOL    bShowSF;

    // show Basic scripts?
    BOOL    bShowBasic;

    ImageList m_aImagesNormal;
    ImageList m_aImagesHighContrast;
    Image m_hdImage;
    Image m_hdImage_hc;
    ::rtl::OUString m_sMyMacros;
    ::rtl::OUString m_sProdMacros;
    Image GetImage( ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode > node, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xCtx, bool bIsRootNode, bool bHighContrast );
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx, ::rtl::OUString& docName );
    ::rtl::OUString xModelToDocTitle( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    ::rtl::OUString parseLocationName( const ::rtl::OUString& location );
protected:
    virtual void    RequestingChilds( SvLBoxEntry *pEntry);
    virtual BOOL    Expand( SvLBoxEntry* pParent );

public:
            SvxConfigGroupListBox_Impl (
                Window* pParent, const ResId&, ULONG nConfigMode = 0 );

            ~SvxConfigGroupListBox_Impl();

    void    Init( SvStringsDtor *pArr = 0, SfxSlotPool* pSlotPool = 0 );
    void    Open( SvLBoxEntry*, BOOL );
    void    ClearAll();
    void    SelectMacro( const SfxMacroInfoItem* );
    void    SelectMacro( const String&, const String& );
    void    GroupSelected();
    String  GetGroup();

    void    SetFunctionListBox( SvxConfigFunctionListBox_Impl *pBox )
        { pFunctionListBox = pBox; }

    void    SetImageProvider( ImageProvider* provider )
        { m_pImageProvider = provider; }
};

class SvxScriptSelectorDialog :
    public ModelessDialog
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
        Window* pParent = NULL, BOOL bShowSlots = FALSE );

    ~SvxScriptSelectorDialog ( );

    void        SetAddHdl( const Link& rLink ) { m_aAddHdl = rLink; }
    const Link& GetAddHdl() const { return m_aAddHdl; }

    void        SetImageProvider( ImageProvider* provider )
        { aCategories.SetImageProvider( provider ); }

    USHORT      GetSelectedId();
    String      GetScriptURL();
    String      GetSelectedDisplayName();
    String      GetSelectedHelpText();
    void        SetRunLabel();
};

#endif
