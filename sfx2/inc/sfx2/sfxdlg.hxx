/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfxdlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:45:48 $
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
#ifndef _SFX2_DIALOG_HXX
#define _SFX2_DIALOG_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#include <vcl/abstdlg.hxx>

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#include <sot/exchange.hxx>
#include <sfx2/lnkbase.hxx>

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

class SfxTabPage;
class SfxViewFrame;
class SfxBindings;
class SfxItemSet;
class ResId;
class Window;
class String;
class SfxItemPool;
class SvObjectServerList;
class TransferableDataHelper;
struct TransferableObjectDescriptor;

typedef SfxTabPage* (*CreateTabPage)(Window *pParent, const SfxItemSet &rAttrSet);
typedef USHORT*     (*GetTabPageRanges)();

namespace sfx2
{
    class SvLinkManager;
}

namespace com { namespace sun { namespace star { namespace frame {
    class XModel;
} } } }

class SfxAbstractTabDialog : public VclAbstractDialog
{
public:
    virtual void                SetCurPageId( USHORT nId ) = 0;
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& ) = 0;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) = 0;
    virtual void                SetText( const XubString& rStr ) = 0;
    virtual String              GetText() const = 0;
};

class AbstractSfxSingleTabDialog : public VclAbstractDialog //add for SvxMeasureDialog & SvxConnectionDialog
{
public:
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;

};

class SfxAbstractInsertObjectDialog : public VclAbstractDialog
{
public:
    virtual com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject()=0;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType )=0;
    virtual BOOL IsCreateNew()=0;
};

class SfxAbstractPasteDialog : public VclAbstractDialog
{
public:
    virtual void Insert( SotFormatStringId nFormat, const String & rFormatName ) = 0;
    virtual void SetObjName( const SvGlobalName & rClass, const String & rObjName ) = 0;
    virtual ULONG GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats=0,
                        const TransferableObjectDescriptor* pDesc=0 ) = 0;
};

class SfxAbstractLinksDialog : public VclAbstractDialog
{
};

class AbstractScriptSelectorDialog : public VclAbstractDialog
{
public:
    virtual String              GetScriptURL() const = 0;
    virtual void                SetRunLabel() = 0;
};

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } }

class SFX2_DLLPUBLIC SfxAbstractDialogFactory : public VclAbstractDialogFactory
{
public:
    static SfxAbstractDialogFactory*    Create();
    virtual VclAbstractDialog*          CreateSfxDialog( Window* pParent, const SfxBindings& rBindings, sal_uInt32 nResId ) = 0;
    virtual VclAbstractDialog*          CreateFrameDialog( Window* pParent, const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame, sal_uInt32 nResId, const String& rParameter ) = 0;
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SfxViewFrame* pViewFrame,
                                            bool bEditFmt=false,
                                            const String *pUserButtonText=0 ) = 0;
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xViewFrame,
                                            bool bEditFmt=false,
                                            const String *pUserButtonText=0 ) = 0;
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId ) = 0;
    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId ) = 0;
    virtual SfxAbstractInsertObjectDialog* CreateInsertObjectDialog( Window* pParent, USHORT nSlotId,
            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStor,
            const SvObjectServerList* pList = 0 )=0;
    virtual VclAbstractDialog*          CreateEditObjectDialog( Window* pParent, USHORT nSlotId,
            const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj )=0;
    virtual  SfxAbstractPasteDialog*         CreatePasteDialog( Window* pParent )=0;
    virtual  SfxAbstractLinksDialog*         CreateLinksDialog( Window* pParent, sfx2::SvLinkManager* pMgr, BOOL bHTML=FALSE, sfx2::SvBaseLink* p=0 )=0;
    virtual VclAbstractDialog *         CreateSvxScriptOrgDialog( Window* pParent,  const String& rLanguage ) = 0;

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            Window* pParent,
            BOOL bShowSlots,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
        ) = 0;

    virtual VclAbstractDialog* CreateScriptErrorDialog(
        Window* pParent, com::sun::star::uno::Any aException ) = 0;

    static SfxTabPage *CreateSfxAcceleratorConfigPage(
        Window *pParent, const SfxItemSet& rSet );

    virtual VclAbstractDialog*  CreateOptionsDialog(
        Window* pParent, const rtl::OUString& rExtensionId, const rtl::OUString& rApplicationContext ) = 0;
};

#endif

