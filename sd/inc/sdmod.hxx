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

#ifndef _SDMOD_HXX
#define _SDMOD_HXX

#include "glob.hxx"
#include "pres.hxx"

#include <sot/storage.hxx>
#include <tools/shl.hxx>
#include "sddllapi.h"
#include <svl/lstner.hxx>
#include <com/sun/star/text/WritingMode.hpp>
#include <sfx2/module.hxx>
#include <vcl/vclevent.hxx>

#include <memory>

class SdOptions;
class SvxSearchItem;
class EditFieldInfo;
class SdTransferable;
class SvNumberFormatter;
class SfxErrorHandler;
class OutputDevice;
class SdDrawDocument;
class SfxFrame;

namespace sd {
class DrawDocShell;
class SdGlobalResourceContainer;
}

namespace com { namespace sun { namespace star { namespace frame {
    class XFrame;
} } } }


// ----------------------
// - SdOptionStreamMode -
// ----------------------

enum SdOptionStreamMode
{
    SD_OPTION_LOAD = 0,
    SD_OPTION_STORE = 1
};

/*************************************************************************
|*
|* This subclass of <SfxModule> (which is a subclass of <SfxShell>) is
|* linked to the DLL. One instance of this class exists while the DLL is
|* loaded.
|*
|* SdModule is like to be compared with the <SfxApplication>-subclass.
|*
|* Remember: Don`t export this class! It uses DLL-internal symbols.
|*
\************************************************************************/

class SdModule : public SfxModule, public SfxListener
{
public:

                            TYPEINFO();
                            SFX_DECL_INTERFACE(SD_IF_SDAPP)
                            DECL_LINK( CalcFieldValueHdl, EditFieldInfo* );

                            SdModule(SfxObjectFactory* pDrawObjFact, SfxObjectFactory* pGraphicObjFact);
    virtual                 ~SdModule();

    SdTransferable*         pTransferClip;
    SdTransferable*         pTransferDrag;
    SdTransferable*         pTransferSelection;

    void                    Execute(SfxRequest& rReq);
    void                    GetState(SfxItemSet&);

    SdOptions*              GetSdOptions(DocumentType eDocType);
    SD_DLLPUBLIC SvStorageStreamRef     GetOptionStream( const OUString& rOptionName, SdOptionStreamMode eMode );

    sal_Bool                    GetWaterCan() const { return bWaterCan; }
    void                    SetWaterCan( sal_Bool bWC ) { bWaterCan = bWC; }

    SvxSearchItem*          GetSearchItem() { return (pSearchItem); }
    void                    SetSearchItem(SvxSearchItem* pItem) { pSearchItem = pItem; }

    /** Return the virtual device that can be used for printer independent
        layout.
        @return
            The returned pointer is NULL when the device could not be
            created when this modules was instantiated.
    */
    OutputDevice* GetVirtualRefDevice (void);

    /** Deprecated alias to <member>GetVirtualRefDevice</member>.
        @param rDocShell
            Unused dummy parameter.
    */
    OutputDevice* GetRefDevice (::sd::DrawDocShell& rDocShell);

    SD_DLLPUBLIC SvNumberFormatter*     GetNumberFormatter();

    // virtual methods for the option dialog
    virtual SfxItemSet*  CreateItemSet( sal_uInt16 nId );
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet );

protected:

    SdOptions*              pImpressOptions;
    SdOptions*              pDrawOptions;
    SvxSearchItem*          pSearchItem;
    SvNumberFormatter*      pNumberFormatter;
    SvStorageRef            xOptionStorage;
    sal_Bool                    bAutoSave;
    sal_Bool                    bWaterCan;
    SfxErrorHandler*        mpErrorHdl;
    /** This device is used for printer independent layout.  It is virtual
        in the sense that it does not represent a printer.  The pointer may
        be NULL when the virtual device could not be created.
    */
    OutputDevice*           mpVirtualRefDevice;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

private:
    SfxFrame* ExecuteNewDocument( SfxRequest& rReq );

    static void ChangeMedium( ::sd::DrawDocShell* pDocShell, SfxViewFrame* pViewFrame, const sal_Int32 eMedium );
    static SfxFrame* CreateEmptyDocument( DocumentType eDocType, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame );
    static SfxFrame* CreateFromTemplate( const OUString& rTemplatePath, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame );

    /** The resource container controls the lifetime of some singletons.
    */
    ::std::auto_ptr< ::sd::SdGlobalResourceContainer> mpResourceContainer;

    /** Create a new summary page.  When the document has been created in
        the kiosk mode with automatical transitions then this method adds
        this kind of transition to the new summary page.
        @param pViewFrame
            The view frame that is used to execute the slot for creating the
            summary page.
        @param pDocument
            The document which will contain the summary page and from which
            the information about the default transition is retrieved.
    */
    void AddSummaryPage (SfxViewFrame* pViewFrame, SdDrawDocument* pDocument);

    /** Take an outline from a text document and create a new impress
        document according to the structure of the outline.
        @param rRequest
            This typically is the unmodified request from a execute()
            function from where this function is called.
    */
    void OutlineToImpress (SfxRequest& rRequest);

    /** Add an eventlistener as soon as possible in sd, allows to use
        remote devices to start the slideshow elegantly, and respecting
        portability
        @EventListenerHdl
            The event listener handler
        @VclSimpleEvent *
            a poiter to a VCLSimpleEvent (see vcl/vclevent.hxx )
    */
    DECL_LINK( EventListenerHdl, VclSimpleEvent* );

};

#define SD_MOD() ( *(SdModule**) GetAppData(SHL_DRAW) )

#endif                                 // _SDMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
