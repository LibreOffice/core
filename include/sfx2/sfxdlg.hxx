/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
namespace rtl {
   class OUString;
};
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

class SfxAbstractDialog : virtual public VclAbstractDialog
{
public:
    /** Get a set of items changed in the dialog.
      */
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
    virtual void                SetText( const OUString& rStr ) = 0;
    virtual OUString            GetText() const = 0;
};

class SfxAbstractTabDialog : virtual public SfxAbstractDialog
{
public:
    virtual void                SetCurPageId( sal_uInt16 nId ) = 0;
    virtual void                SetCurPageId( const OString &rName ) = 0;
    virtual const sal_uInt16*   GetInputRanges( const SfxItemPool& ) = 0;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) = 0;
};

class SfxAbstractApplyTabDialog : virtual public SfxAbstractTabDialog
{
public:
    virtual void                SetApplyHdl( const Link& rLink ) = 0;
};

class SfxAbstractInsertObjectDialog : virtual public VclAbstractDialog
{
public:
    virtual com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject()=0;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType )=0;
    virtual sal_Bool IsCreateNew()=0;
};

class SfxAbstractPasteDialog : virtual public VclAbstractDialog
{
public:
    virtual void Insert( SotFormatStringId nFormat, const rtl::OUString & rFormatName ) = 0;
    virtual void SetObjName( const SvGlobalName & rClass, const rtl::OUString & rObjName ) = 0;
    virtual sal_uIntPtr GetFormat( const TransferableDataHelper& aHelper,
                        const DataFlavorExVector* pFormats=0,
                        const TransferableObjectDescriptor* pDesc=0 ) = 0;
};

class SfxAbstractLinksDialog : virtual public VclAbstractDialog
{
};

class AbstractScriptSelectorDialog : virtual public VclAbstractDialog
{
public:
    virtual rtl::OUString       GetScriptURL() const = 0;
    virtual void                SetRunLabel() = 0;
};

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } }

class SFX2_DLLPUBLIC SfxAbstractDialogFactory : virtual public VclAbstractDialogFactory
{
public:
                                        virtual ~SfxAbstractDialogFactory();    // needed for export of vtable
    static SfxAbstractDialogFactory*    Create();
    virtual VclAbstractDialog*          CreateSfxDialog( Window* pParent, const SfxBindings& rBindings, sal_uInt32 nResId ) = 0;
    virtual VclAbstractDialog*          CreateFrameDialog( Window* pParent, const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame, sal_uInt32 nResId, const rtl::OUString& rParameter ) = 0;
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SfxViewFrame* pViewFrame,
                                            bool bEditFmt=false,
                                            const rtl::OUString *pUserButtonText=0 ) = 0;
    virtual SfxAbstractTabDialog*       CreateTabDialog( sal_uInt32 nResId,
                                            Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xViewFrame,
                                            bool bEditFmt=false,
                                            const rtl::OUString *pUserButtonText=0 ) = 0;
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) = 0;
    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId ) = 0;
    virtual SfxAbstractInsertObjectDialog* CreateInsertObjectDialog( Window* pParent, const OUString& rCommand,
            const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStor,
            const SvObjectServerList* pList = 0 )=0;
    virtual VclAbstractDialog*          CreateEditObjectDialog( Window* pParent, const OUString& rCommand,
            const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObj )=0;
    virtual  SfxAbstractPasteDialog*         CreatePasteDialog( Window* pParent )=0;
    virtual  SfxAbstractLinksDialog*         CreateLinksDialog( Window* pParent, sfx2::LinkManager* pMgr, sal_Bool bHTML=sal_False, sfx2::SvBaseLink* p=0 )=0;
    virtual VclAbstractDialog *         CreateSvxScriptOrgDialog( Window* pParent,  const rtl::OUString& rLanguage ) = 0;

    virtual AbstractScriptSelectorDialog*
        CreateScriptSelectorDialog(
            Window* pParent,
            sal_Bool bShowSlots,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
        ) = 0;

    virtual VclAbstractDialog* CreateScriptErrorDialog(
        Window* pParent, com::sun::star::uno::Any aException ) = 0;

    virtual VclAbstractDialog*  CreateOptionsDialog(
        Window* pParent, const OUString& rExtensionId, const OUString& rApplicationContext ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
