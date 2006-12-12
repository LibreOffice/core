/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdmod.hxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:25:32 $
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

#ifndef _SDMOD_HXX
#define _SDMOD_HXX


#ifndef _SDDLL_HXX
#define _SD_DLL             // fuer SD_MOD()
#include "sddll.hxx"        // fuer SdModuleDummy
#endif
#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

#include <sot/storage.hxx>
#include <tools/shl.hxx>

#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif

#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

class SdOptions;
class BasicIDE;
class SvxSearchItem;
class SdAppLinkHdl;
class SvxErrorHandler;
class EditFieldInfo;
class SvFactory;
class SdTransferable;
class SvNumberFormatter;
class SfxErrorHandler;
class OutputDevice;
class SdPage;
class SdDrawDocument;
class SfxFrame;

namespace sd {
class DrawDocShell;
class SdGlobalResourceContainer;
}

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

    virtual void            FillStatusBar(StatusBar& rBar);

    SdOptions*              GetSdOptions(DocumentType eDocType);
    SD_DLLPUBLIC SvStorageStreamRef     GetOptionStream( const String& rOptionName, SdOptionStreamMode eMode );

    BOOL                    GetWaterCan() const { return bWaterCan; }
    void                    SetWaterCan( BOOL bWC ) { bWaterCan = bWC; }

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

    ::com::sun::star::text::WritingMode GetDefaultWritingMode() const;

    //virtuelle Methoden fuer den Optionendialog
    virtual SfxItemSet*  CreateItemSet( USHORT nId );
    virtual void         ApplyItemSet( USHORT nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet );

protected:

    SdOptions*              pImpressOptions;
    SdOptions*              pDrawOptions;
    SvxSearchItem*          pSearchItem;
    SvNumberFormatter*      pNumberFormatter;
    SvStorageRef            xOptionStorage;
    BOOL                    bAutoSave;
    BOOL                    bWaterCan;
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
    static SfxFrame* CreateEmptyDocument( DocumentType eDocType, SfxFrame* pTargetFrame = 0  );
    static SfxFrame* CreateFromTemplate( const String& rTemplatePath, SfxFrame* pTargetFrame = 0 );

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
};




#ifndef SD_MOD
#define SD_MOD() ( *(SdModule**) GetAppData(SHL_DRAW) )
#endif

#endif                                 // _SDMOD_HXX

