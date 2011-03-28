/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFX2_DIALOG_HXX
#define _SFX2_DIALOG_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <vcl/abstdlg.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <sot/exchange.hxx>
#include <sfx2/lnkbase.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.h>

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

#include <sfx2/tabdlg.hxx>

namespace sfx2
{
    class LinkManager;
}

namespace com { namespace sun { namespace star { namespace frame {
    class XModel;
} } } }

class SfxAbstractDialog : public VclAbstractDialog
{
public:
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
    virtual void                SetText( const XubString& rStr ) = 0;
    virtual String              GetText() const = 0;
};

class SfxAbstractTabDialog : public SfxAbstractDialog
{
public:
    virtual void                SetCurPageId( sal_uInt16 nId ) = 0;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& ) = 0;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) = 0;
};

class SfxAbstractInsertObjectDialog : public VclAbstractDialog
{
public:
    virtual com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject()=0;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( ::rtl::OUString* pGraphicMediaType )=0;
    virtual sal_Bool IsCreateNew()=0;
};

class SfxAbstractPasteDialog : public VclAbstractDialog
{
public:
    virtual void Insert( SotFormatStringId nFormat, const String & rFormatName ) = 0;
    virtual void SetObjName( const SvGlobalName & rClass, const String & rObjName ) = 0;
    virtual sal_uIntPtr GetFormat( const TransferableDataHelper& aHelper,
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

class SFX2_DLLPUBLIC SfxAbstractDialogFactory : virtual public VclAbstractDialogFactory
{
public:
                                        virtual ~SfxAbstractDialogFactory();    // needed for export of vtable
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
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) = 0;
    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId ) = 0;
    virtual SfxAbstractInsertObjectDialog* CreateInsertObjectDialog( Window* pParent, const rtl::OUString& rCommand,
            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStor,
            const SvObjectServerList* pList = 0 )=0;
    virtual VclAbstractDialog*          CreateEditObjectDialog( Window* pParent, const rtl::OUString& rCommand,
            const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj )=0;
    virtual  SfxAbstractPasteDialog*         CreatePasteDialog( Window* pParent )=0;
    virtual  SfxAbstractLinksDialog*         CreateLinksDialog( Window* pParent, sfx2::LinkManager* pMgr, sal_Bool bHTML=sal_False, sfx2::SvBaseLink* p=0 )=0;
    virtual VclAbstractDialog *         CreateSvxScriptOrgDialog( Window* pParent,  const String& rLanguage ) = 0;

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            Window* pParent,
            sal_Bool bShowSlots,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
        ) = 0;

    virtual VclAbstractDialog* CreateScriptErrorDialog(
        Window* pParent, com::sun::star::uno::Any aException ) = 0;

    virtual VclAbstractDialog*  CreateOptionsDialog(
        Window* pParent, const rtl::OUString& rExtensionId, const rtl::OUString& rApplicationContext ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
