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
#ifndef _SFX_OBJSH_HXX
#define _SFX_OBJSH_HXX

#include <config_features.h>

#include "sal/config.h"

#include "tools/solar.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <boost/shared_ptr.hpp>

//________________________________________________________________________________________________________________
//  include something else
//________________________________________________________________________________________________________________

#include <vcl/timer.hxx>
#include <svl/poolitem.hxx>
#include <vcl/bitmap.hxx>
#include <sot/storage.hxx>
#include <rsc/rscsfx.hxx>

#include <sfx2/XmlIdRegistry.hxx>
#include <sfx2/shell.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <com/sun/star/frame/XModel.hpp>

class SbxValue;
class SvxMacro;
class SbxArray;
class BasicManager;
class SfxMedium;
class SfxObjectFactory;
class SfxDocumentInfoDialog;
class SfxStyleSheetBasePool;
class INote;
class SfxStyleSheetPool;
class SfxFrame;
class SbMethod;
class StarBASIC;
class Printer;
class SvKeyValueIterator;
class SfxBaseModel;
class SfxModule;
class SvData;
class SfxProgress;
class GDIMetaFile;
class Bitmap;
class INetURLObject;
class IndexBitSet;
class JobSetup;
class Size;
class Point;

// From embobj.hxx
#define ASPECT_CONTENT      1
#define ASPECT_THUMBNAIL    2 /* 120 * 120, 6 Colors DIB in MetaFile */
#define ASPECT_ICON         4
#define ASPECT_DOCPRINT     8
#define ASPECT_ALL      (ASPECT_CONTENT | ASPECT_THUMBNAIL | ASPECT_ICON | ASPECT_DOCPRINT)

// from pseudo.hxx; must be the same as in OLE2
#define SVOBJ_MISCSTATUS_SERVERRESIZE           1
#define SVOBJ_MISCSTATUS_NOTREPLACE             4
#define SVOBJ_MISCSTATUS_CANTLINKINSIDE         16
#define SVOBJ_MISCSTATUS_LINK                   64
#define SVOBJ_MISCSTATUS_INSIDEOUT              128
#define SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE    256
#define SVOBJ_MISCSTATUS_NOTRESIZEABLE          512
#define SVOBJ_MISCSTATUS_ALWAYSACTIVATE         1024
#define SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE  2048
#define SVOBJ_MISCSTATUS_SPECIALOBJECT          4096


namespace sfx2
{
    class SvLinkSource;
}

namespace com { namespace sun { namespace star { namespace datatransfer { class XTransferable; } } } }

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
    namespace lang {
        class XComponent;
    }
} } }

typedef sal_uInt32 SfxObjectShellFlags;
#define SFXOBJECTSHELL_HASOPENDOC      0x01L
#define SFXOBJECTSHELL_DONTREPLACE     0x02L
#define SFXOBJECTSHELL_HASMENU         0x04L
#define SFXOBJECTSHELL_DONTLOADFILTERS 0x08L
#define SFXOBJECTSHELL_DONTCLOSE       0x10L
#define SFXOBJECTSHELL_NODOCINFO       0x20L
#define SFXOBJECTSHELL_STD_NORMAL      ( SFXOBJECTSHELL_HASOPENDOC )
#define SFXOBJECTSHELL_STD_SPECIAL     ( SFXOBJECTSHELL_DONTLOADFILTERS )
#define SFXOBJECTSHELL_UNDEFINED       0xf000000

//--------------------------------------------------------------------

#define SFX_TITLE_TITLE    0
#define SFX_TITLE_FILENAME 1
#define SFX_TITLE_FULLNAME 2
#define SFX_TITLE_APINAME  3
#define SFX_TITLE_DETECT   4
#define SFX_TITLE_CAPTION  5
#define SFX_TITLE_PICKLIST 6
#define SFX_TITLE_HISTORY  7
#define SFX_TITLE_MAXLEN   10   // this gives the limits on length

#define SFX_LOADED_MAINDOCUMENT 1
#define SFX_LOADED_IMAGES       2
#define SFX_LOADED_ALL          3

//--------------------------------------------------------------------

#define HIDDENINFORMATION_RECORDEDCHANGES       0x0001
#define HIDDENINFORMATION_NOTES                 0x0002
#define HIDDENINFORMATION_DOCUMENTVERSIONS      0x0004

enum HiddenWarningFact
{
    WhenSaving = 0,
    WhenPrinting,
    WhenSigning,
    WhenCreatingPDF
};

//--------------------------------------------------------------------

enum SfxObjectCreateMode
{
    SFX_CREATE_MODE_EMBEDDED,
    SFX_CREATE_MODE_STANDARD,
    SFX_CREATE_MODE_PREVIEW,
    SFX_CREATE_MODE_ORGANIZER,
    SFX_CREATE_MODE_PLUGIN,
    SFX_CREATE_MODE_INTERNAL
};

/*====================================================================

The class SfxObjectShell is the base class for SFx-objects, ie documents
and parts of documents that can be integrated as separate objects
into foreign objects.

----------------------------------------------------------------------*/

class SfxToolBoxConfig;
struct TransferableObjectDescriptor;

class SFX2_DLLPUBLIC SfxObjectShell :
    public SfxShell, virtual public SotObject,
    public ::comphelper::IEmbeddedHelper, public ::sfx2::IXmlIdRegistrySupplier
{
friend struct ModifyBlocker_Impl;
friend class SfxObjectShellLock;

private:
    struct SfxObjectShell_Impl* pImp;     // internal data

    SfxMedium *                 pMedium;  // Description of the file for example
                                          // storage that contains the object
    SfxStyleSheetBasePool*      pStyleSheetPool;    // StyleSheets
    SfxObjectCreateMode         eCreateMode;      // Purpose of the object
    sal_Bool                    bHasName :1,      // sal_True  := existing object,
                                                  // sal_False := new object
                                bIsTmp :1;        // temporary Storage
    sal_Bool                    bIsInGenerateThumbnail; //optimize thumbnail generate and store procedure to improve odt saving performance, i120030

private:
    SAL_DLLPRIVATE void UpdateTime_Impl(const ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> & i_xDocProps);

    SAL_DLLPRIVATE sal_Bool SaveTo_Impl(SfxMedium &rMedium, const SfxItemSet* pSet );

protected:
                                SfxObjectShell(SfxObjectCreateMode);
                                SfxObjectShell( const sal_uInt64 i_nCreationFlags );    // see sfxmodelfactory.hxx
    virtual                     ~SfxObjectShell();

    virtual void                ModifyChanged();
    virtual bool                Close();

    /** declares the document to have capabilities to contain basic/dialog libraries
    */
    void                        SetHasNoBasic();

    /// template method, called by FlushDocInfo; this implementation is empty
    virtual void                DoFlushDocInfo();

    // helper method
    void AddToRecentlyUsedList();

public:
                                TYPEINFO();
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXDOCSH)

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    /* Stampit disable/enable cancel button for print jobs
       default = true = enable! */
    void                        Stamp_SetPrintCancelState(sal_Bool bState);
    sal_Bool                    Stamp_GetPrintCancelState() const;

    using SotObject::GetInterface;

    // Document-Shell Iterator
    static SfxObjectShell*      GetFirst( const TypeId* pType = 0,
                                          sal_Bool bOnlyVisible = sal_True );
    static SfxObjectShell*      GetNext( const SfxObjectShell& rPrev,
                                         const TypeId* pType = 0,
                                         sal_Bool bOnlyVisible = sal_True );
    static SfxObjectShell*      Current();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                                GetCurrentComponent();
    static void                 SetCurrentComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent );

    virtual void                Invalidate(sal_uInt16 nId = 0);

    SfxObjectShellFlags         GetFlags( ) const ;

    SfxModule*                  GetModule() const;

    virtual SfxObjectFactory&   GetFactory() const=0;
    SfxMedium *                 GetMedium() const { return pMedium; }
    void                        ForgetMedium() { pMedium = 0; }
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties > getDocProperties();
    void                        UpdateDocInfoForSave(  );
    void                        FlushDocInfo();
    sal_Bool                    HasName() const { return bHasName; }
    virtual String              GetAPIName() const;
    void                        SetReadOnly();
    sal_Bool                    IsReadOnly() const;
    sal_Bool                    IsReadOnlyMedium() const;
    bool                        IsOriginallyReadOnlyMedium() const;
    void                        SetReadOnlyUI( sal_Bool bReadOnly = sal_True );
    sal_Bool                    IsReadOnlyUI() const;
    void                        SetNoName();
    sal_Bool                    IsInModalMode() const;
    sal_Bool                    IsInPrepareClose() const;
    virtual sal_Bool            AcceptStateUpdate() const;
    sal_Bool                    IsHelpDocument() const;

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
    sal_Bool                    IsDocShared() const;
    OUString             GetSharedFileURL() const;
    sal_Bool                    SwitchToShared( sal_Bool bShared, sal_Bool bSave );
    SAL_DLLPRIVATE void         FreeSharedFile();
    SAL_DLLPRIVATE void         FreeSharedFile( const OUString& aTempFileURL );
    SAL_DLLPRIVATE void         DoNotCleanShareControlFile();
    void                        SetSharedXMLFlag( sal_Bool bFlag ) const;
    sal_Bool                    HasSharedXMLFlagSet() const;
#else
    sal_Bool                    IsDocShared() const
        { return sal_False; };
    OUString             GetSharedFileURL() const
        { return OUString(); };
#endif

    SAL_DLLPRIVATE void SetModalMode_Impl(sal_Bool bModal=sal_True);
    SAL_DLLPRIVATE void SetMacroMode_Impl(sal_Bool bModal=sal_True);

    void                        ResetError();
    sal_uInt32                  GetError() const;
    sal_uInt32                  GetErrorCode() const;
    void                        SetError( sal_uInt32 rErr, const OUString& aLogMessage );

    void                        AddLog( const OUString& aMessage );
    void                        StoreLog();

    /**
     * Initialize bare minimum just enough for unit test runs.
     *
     * @return true if the initialization is successful, false otherwise.
     */
    bool                        DoInitUnitTest();
    sal_Bool                    DoInitNew( SfxMedium* pMedium=0 );
    sal_Bool                    DoLoad( SfxMedium* pMedium );
    bool                        DoLoadExternal( SfxMedium* pMed );
    sal_Bool                    DoSave();
    sal_Bool                    DoSaveAs( SfxMedium &rNewStor );
    sal_Bool                    DoSaveObjectAs( SfxMedium &rNewStor, sal_Bool bCommit );

    // TODO/LATER: currently only overloaded in Calc, should be made non-virtual
    virtual sal_Bool            DoSaveCompleted( SfxMedium* pNewStor=0 );

    virtual sal_Bool            LoadOwnFormat( SfxMedium& pMedium );
    virtual sal_Bool            SaveAsOwnFormat( SfxMedium& pMedium );
    virtual sal_Bool            ConvertFrom( SfxMedium &rMedium );
    virtual sal_Bool            ConvertTo( SfxMedium &rMedium );
    virtual sal_Bool            InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    virtual sal_Bool            Load( SfxMedium &rMedium  );
    virtual sal_Bool            LoadFrom( SfxMedium& rMedium );
    virtual sal_Bool            Save();
    virtual sal_Bool            SaveAs( SfxMedium &rMedium  );
    virtual sal_Bool            SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    virtual sal_Bool            SwitchPersistance(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    virtual void                UpdateLinks();
    virtual bool                LoadExternal( SfxMedium& rMedium );
    /**
     * Called when the Options dialog is dismissed with the OK button, to
     * handle potentially conflicting option settings.
     */
    virtual void                CheckConfigOptions();
    sal_Bool                    IsConfigOptionsChecked() const;
    void                        SetConfigOptionsChecked( sal_Bool bChecked );

    // called for a few slots like SID_SAVE[AS]DOC, SID_PRINTDOC[DIRECT], derived classes may abort the action
    virtual sal_Bool            QuerySlotExecutable( sal_uInt16 nSlotId );

    sal_Bool                    SaveChildren(sal_Bool bObjectsOnly=sal_False);
    sal_Bool                    SaveAsChildren( SfxMedium &rMedium );
    sal_Bool                    SwitchChildrenPersistance(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    sal_Bool bForceNonModified = sal_False );
    sal_Bool                    SaveCompletedChildren( sal_Bool bSuccess );

    virtual sal_Bool            ImportFrom( SfxMedium &rMedium, bool bInsert );
    sal_Bool                    ExportTo( SfxMedium &rMedium );

    // xmlsec05, check with SFX team
    sal_uInt16                  GetDocumentSignatureState();
    void                        SignDocumentContent();
    sal_uInt16                  GetScriptingSignatureState();
    void                        SignScriptingContent();

    virtual SfxDocumentInfoDialog* CreateDocumentInfoDialog(
                                        Window *pParent, const SfxItemSet& );

    ErrCode                     CallBasic( const String& rMacro, const String& rBasicName,
                                    SbxArray* pArgs = 0, SbxValue* pRet = 0 );

    ErrCode     CallXScript(
        const String& rScriptURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams,
        ::com::sun::star::uno::Any& aRet,
        ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam,
        bool bRaiseError = true,
        const ::com::sun::star::uno::Any* aCaller = 0 );

    static ErrCode  CallXScript(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxScriptContext,
        const OUString& rScriptURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams,
        ::com::sun::star::uno::Any& aRet,
        ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam,
        bool bRaiseError = true,
        const ::com::sun::star::uno::Any* aCaller = 0
    );

    /** adjusts the internal macro mode, according to the current security settings

        Finally, the macro mode is either NEVER_EXECUTE or ALWAYS_EXECUTE_NO_WARN.

        @return
            whether macros from this document should be executed
    */
    bool                        AdjustMacroMode( const String& rScriptType, bool _bSuppressUI = false );

    SvKeyValueIterator*         GetHeaderAttributes();
    void                        ClearHeaderAttributesForSourceViewHack();
    void                        SetHeaderAttributesForSourceViewHack();

    sal_Bool                    IsTemplate() const;

    sal_Bool                                  IsQueryLoadTemplate() const;
    sal_Bool                                        IsUseUserData() const;
    sal_Bool                                        IsLoadReadonly() const;
    sal_Bool                                        IsSaveVersionOnClose() const;
    void                                              SetQueryLoadTemplate( sal_Bool b );
    void                                              SetUseUserData( sal_Bool bNew );
    void                                            SetLoadReadonly( sal_Bool _bReadonly );
    void                                            SetSaveVersionOnClose( sal_Bool bSet );
    void                                              ResetFromTemplate( const String& rTemplateName, const String& rFileName );

    // TODO/LATER: the following two methods should be replaced by Get/SetModifPasswordInfo in future
    sal_uInt32                  GetModifyPasswordHash() const;
    sal_Bool                    SetModifyPasswordHash( sal_uInt32 nHash );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetModifyPasswordInfo() const;
    sal_Bool                    SetModifyPasswordInfo( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aInfo );

    static sal_uInt32           HandleFilter( SfxMedium* pMedium, SfxObjectShell* pDoc );

    virtual void                ViewAssigned();
    virtual sal_uInt16          PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False );
    virtual sal_Bool            IsInformationLost();
    virtual sal_uInt16          GetHiddenInformationState( sal_uInt16 nStates );
    sal_Int16                   QueryHiddenInformation( HiddenWarningFact eFact, Window* pParent );
    virtual sal_Bool            HasSecurityOptOpenReadOnly() const;
    sal_Bool                    IsSecurityOptOpenReadOnly() const;
    void                        SetSecurityOptOpenReadOnly( sal_Bool bOpenReadOnly = sal_True );

    virtual Size                GetFirstPageSize();
    virtual sal_Bool            DoClose();
    virtual void                PrepareReload();
    virtual ::boost::shared_ptr<GDIMetaFile> GetPreviewMetaFile( sal_Bool bFullContent = sal_False ) const;
    virtual void                CancelTransfers();

    sal_Bool                    GenerateAndStoreThumbnail(
                                    sal_Bool bEncrypted,
                                    sal_Bool bSigned,
                                    sal_Bool bIsTemplate,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor );

    sal_Bool                    WriteThumbnail(
                                    sal_Bool bEncrypted,
                                    sal_Bool bSigned,
                                    sal_Bool bIsTemplate,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream );

    sal_Bool                    IsInGenerateAndStoreThumbnail() const {return bIsInGenerateThumbnail;}//optimize thumbnail generate and store procedure to improve odt saving performance, i120030

    // Transfer IFace
    void                        AbortImport();
    sal_Bool                    IsAbortingImport() const;
    sal_Bool                    IsReloading() const;
    void                        FinishedLoading( sal_uInt16 nWhich = SFX_LOADED_ALL );
    void                        TemplateDisconnectionAfterLoad();
    sal_Bool                    IsLoading() const;
    sal_Bool                    IsLoadingFinished() const;
    void                        SetAutoLoad( const INetURLObject&, sal_uInt32 nTime, sal_Bool bReload = sal_True );
    sal_Bool                    IsAutoLoadLocked() const;
    void                        NotifyReloadAvailable();

    // Misc
    sal_Bool                    IsPreview() const;
    SfxObjectCreateMode         GetCreateMode() const { return eCreateMode; }
    SfxProgress*                GetProgress() const;
    void                        SetWaitCursor( sal_Bool bSet ) const;

    // Naming Interface
    void                        SetTitle( const String& rTitle );
    String                      GetTitle( sal_uInt16 nMaxLen = 0 ) const;
    void                        InvalidateName();  // Re-set to unnamed

    // DDE-Interface
    virtual long                DdeExecute( const OUString& rCmd );
    virtual long                DdeGetData( const OUString& rItem,
                                            const OUString& rMimeType,
                                            ::com::sun::star::uno::Any & rValue );
    virtual long                DdeSetData( const OUString& rItem,
                                            const OUString& rMimeType,
                                            const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const OUString& rItem );
    virtual void                ReconnectDdeLink(SfxObjectShell& rServer);

    static void                 ReconnectDdeLinks(SfxObjectShell& rServer);

    // Contents
    virtual SfxStyleSheetBasePool*  GetStyleSheetPool();
    void                     SetStyleSheetPool(SfxStyleSheetBasePool *pBasePool ) {
                                        pStyleSheetPool = pBasePool; }

    // Determine the position of the "Automatic" filter in the stylist
    void                        SetAutoStyleFilterIndex(sal_uInt16 nSet);
    sal_uInt16                  GetAutoStyleFilterIndex();
    virtual sal_Bool            HasBasic() const;
    BasicManager*               GetBasicManager() const;
    com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >
                                GetBasicContainer();
    com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >
                                GetDialogContainer();
    StarBASIC*                  GetBasic() const;

    // Interface Document content, Organizer
#define INDEX_IGNORE USHRT_MAX

#define CONTENT_STYLE 0
#define CONTENT_CONFIG 1
#define CONTENT_MACRO 2
#define DEF_CONTENT_COUNT 1


    virtual void                SetOrganizerSearchMask(
                                    SfxStyleSheetBasePool* ) const;

    virtual sal_Bool            CanHaveChildren(
                                        sal_uInt16 nIdx1,
                                        sal_uInt16 nIdx2 = INDEX_IGNORE
                                );

    virtual void                GetContent( String &,
                                        Bitmap &rClosedBitmap,
                                        Bitmap &rOpenedBitmap,
                                        sal_Bool   &bCanDelete,
                                        sal_uInt16 nPos,
                                        sal_uInt16 nIdx1
                                );

    virtual Bitmap              GetStyleFamilyBitmap(SfxStyleFamily eFamily);

    virtual sal_Bool            InsertStuffWithIndexes( SfxObjectShell &rSource,
                                        sal_uInt16 nSourceIdx1,
                                        sal_uInt16 nSourceIdx2,
                                        sal_uInt16 nSourceIdx3,
                                        sal_uInt16 &nIdx1,
                                        sal_uInt16 &nIdx2,
                                        sal_uInt16 &nIdx3,
                                        sal_uInt16 &rIdxDeleted);

    virtual sal_Bool            RemoveStuffWithIndexes( sal_uInt16 nIdx1,
                                        sal_uInt16 nIdx2 = INDEX_IGNORE,
                                        sal_uInt16 nIdx3 = INDEX_IGNORE);

    virtual void                LoadStyles( SfxObjectShell &rSource );
    void                        ReadNote( INote * );
    void                        UpdateNote( INote * );

                                // Documents, for which to format the view size

    virtual SfxObjectShell*     GetObjectShell();

    virtual SfxFrame*           GetSmartSelf( SfxFrame* pSelf, SfxMedium& rMedium );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                                GetModel() const;
    // Only temporarily for the applications!
    void                        SetBaseModel( SfxBaseModel* pModel );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetBaseModel() const;
    // Only temporarily for the applications!

    virtual ::com::sun::star::uno::Sequence< OUString > GetEventNames();

    Window*                     GetDialogParent( SfxMedium* pMedium=0 );
    static SfxObjectShell*      CreateObject( const String& rServiceName, SfxObjectCreateMode = SFX_CREATE_MODE_STANDARD );
    static SfxObjectShell*      CreateObjectByFactoryName( const String& rURL, SfxObjectCreateMode = SFX_CREATE_MODE_STANDARD );
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                                CreateAndLoadComponent( const SfxItemSet& rSet, SfxFrame* pFrame = NULL );
    static SfxObjectShell*      GetShellFromComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xComp );
    static String               GetServiceNameFromFactory( const String& rFact );
    sal_Bool                        IsInPlaceActive();
    sal_Bool                        IsUIActive();
    virtual void                InPlaceActivate( sal_Bool );
    virtual void                UIActivate( sal_Bool );

    static sal_Bool             CopyStoragesOfUnknownMediaType(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xSource,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xTarget );

    // ==== The functions from SvPersist
    void            EnableSetModified( sal_Bool bEnable = sal_True );
    sal_Bool        IsEnableSetModified() const;
    virtual void    SetModified( sal_Bool bModified = sal_True );
    sal_Bool        IsModified();

    /**
     * @param bChart true if the file is a chart doc and FillClass should not be called
     */
    void            SetupStorage(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                        sal_Int32 nVersion, sal_Bool bTemplate, bool bChart = false ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetStorage();

    SvGlobalName    GetClassName() const;

    // comphelper::IEmbeddedHelper
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > getInteractionHandler() const;
    virtual com::sun::star::uno::Reference < com::sun::star::embed::XStorage > getStorage() const
    {
        return const_cast<SfxObjectShell*>(this)->GetStorage();
    }
    virtual comphelper::EmbeddedObjectContainer& getEmbeddedObjectContainer() const
    {
        return GetEmbeddedObjectContainer();
    }
    bool    isEnableSetModified() const
    {
        return IsEnableSetModified();
    }
    comphelper::EmbeddedObjectContainer&    GetEmbeddedObjectContainer() const;
    void    ClearEmbeddedObjects();

    // ==== The functions from SvEmbeddedObject
    virtual Printer *       GetDocumentPrinter();
    virtual OutputDevice*    GetDocumentRefDev();
    virtual void            OnDocumentPrinterChanged( Printer * pNewPrinter );
    virtual Rectangle GetVisArea( sal_uInt16 nAspect ) const;
    virtual void    SetVisArea( const Rectangle & rVisArea );
    const Rectangle & GetVisArea() const;
    void            SetVisAreaSize( const Size & rVisSize );
    virtual sal_uIntPtr GetMiscStatus() const;

    MapUnit         GetMapUnit() const;
    void            SetMapUnit( MapUnit nMUnit );

    void            FillTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc ) const;
    void            DoDraw( OutputDevice *, const Point & rObjPos,
                            const Size & rSize,
                            const JobSetup & rSetup,
                            sal_uInt16 nAspect = ASPECT_CONTENT );
    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                          sal_uInt16 nAspect = ASPECT_CONTENT ) = 0;


    virtual void    FillClass( SvGlobalName * pClassName,
                               sal_uInt32 * pFormat,
                               OUString * pAppName,
                               OUString * pFullTypeName,
                               OUString * pShortTypeName,
                               sal_Int32 nVersion,
                               sal_Bool bTemplate = sal_False) const = 0;

    // change recording and respective passwword protection for Writer and Calc
    // slots available for Writer:  FN_REDLINE_ON, FN_REDLINE_ON
    // slots used for Calc:         FID_CHG_RECORD, SID_CHG_PROTECT
    virtual bool    IsChangeRecording() const;
    virtual bool    HasChangeRecordProtection() const;
    virtual void    SetChangeRecording( bool bActivate );
    virtual bool    SetProtectionPassword( const String &rPassword );
    virtual bool    GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash );

    // =================================

    SAL_DLLPRIVATE ::boost::shared_ptr<GDIMetaFile> CreatePreviewMetaFile_Impl( sal_Bool bFullContent ) const;

    SAL_DLLPRIVATE sal_Bool IsOwnStorageFormat_Impl(const SfxMedium &) const;

    SAL_DLLPRIVATE sal_Bool IsPackageStorageFormat_Impl(const SfxMedium &) const;

    SAL_DLLPRIVATE sal_Bool ConnectTmpStorage_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, SfxMedium* pMedium );
    SAL_DLLPRIVATE sal_Bool DisconnectStorage_Impl( SfxMedium& rSrcMedium, SfxMedium& rTargetMedium );

    SAL_DLLPRIVATE sal_Bool PutURLContentsToVersionStream_Impl(
                    OUString aURL,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDocStorage,
                    OUString aStreamName );

    SAL_DLLPRIVATE OUString CreateTempCopyOfStorage_Impl(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    SAL_DLLPRIVATE void InitOwnModel_Impl();
    SAL_DLLPRIVATE void BreakMacroSign_Impl( sal_Bool bBreakMacroSing );
    SAL_DLLPRIVATE void CheckSecurityOnLoading_Impl();
    SAL_DLLPRIVATE void CheckForBrokenDocSignatures_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );
    SAL_DLLPRIVATE sal_uInt16 ImplCheckSignaturesInformation(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignatureInformation >& aInfos );
    SAL_DLLPRIVATE void CheckEncryption_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );
    SAL_DLLPRIVATE void SetModifyPasswordEntered( sal_Bool bEntered = sal_True );
    SAL_DLLPRIVATE sal_Bool IsModifyPasswordEntered();

    SAL_DLLPRIVATE void InitBasicManager_Impl();
    SAL_DLLPRIVATE SfxObjectShell_Impl* Get_Impl() { return pImp; }

    SAL_DLLPRIVATE static sal_Bool UseInteractionToHandleError(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler,
                    sal_uInt32 nError );
    SAL_DLLPRIVATE const SfxObjectShell_Impl* Get_Impl() const { return pImp; }

    SAL_DLLPRIVATE void SetCreateMode_Impl( SfxObjectCreateMode nMode );

    SAL_DLLPRIVATE void DoDraw_Impl( OutputDevice* pDev,
                                            const Point & rViewPos,
                                            const Fraction & rScaleX,
                                            const Fraction & rScaleY,
                                            const JobSetup & rSetup,
                                            sal_uInt16 nAspect );

    // Shell Interface
    SAL_DLLPRIVATE void ExecFile_Impl(SfxRequest &);
    SAL_DLLPRIVATE void GetState_Impl(SfxItemSet&);
    SAL_DLLPRIVATE void PrintExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void PrintState_Impl(SfxItemSet&);
    SAL_DLLPRIVATE void ExecProps_Impl(SfxRequest &);
    SAL_DLLPRIVATE void StateProps_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void ExecView_Impl(SfxRequest &);
    SAL_DLLPRIVATE void StateView_Impl(SfxItemSet &);

    // Load/Save public internals
    SAL_DLLPRIVATE sal_Bool ImportFromGeneratedStream_Impl(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr );
    SAL_DLLPRIVATE void PositionView_Impl();
    SAL_DLLPRIVATE void UpdateFromTemplate_Impl();
    SAL_DLLPRIVATE sal_Bool CanReload_Impl();
    SAL_DLLPRIVATE void SetNamedVisibility_Impl();
    SAL_DLLPRIVATE sal_Bool DoSave_Impl( const SfxItemSet* pSet=0 );
    SAL_DLLPRIVATE sal_Bool Save_Impl( const SfxItemSet* pSet=0 );
    SAL_DLLPRIVATE sal_Bool PreDoSaveAs_Impl(const String &rFileName, const String &rFiltName, SfxItemSet *);
    SAL_DLLPRIVATE sal_Bool APISaveAs_Impl ( const String& aFileName, SfxItemSet* aParams );
    SAL_DLLPRIVATE sal_Bool CommonSaveAs_Impl ( const INetURLObject& aURL, const String& aFilterName, SfxItemSet* aParams );
    SAL_DLLPRIVATE sal_Bool GeneralInit_Impl(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    sal_Bool bTypeMustBeSetAlready );
    SAL_DLLPRIVATE void PrepareSecondTryLoad_Impl();
    SAL_DLLPRIVATE void SetInitialized_Impl( const bool i_fromInitNew );

    // public-internals
    SAL_DLLPRIVATE IndexBitSet& GetNoSet_Impl();
    SAL_DLLPRIVATE void SetProgress_Impl( SfxProgress *pProgress );
    SAL_DLLPRIVATE void PostActivateEvent_Impl( SfxViewFrame* );
    SAL_DLLPRIVATE void SetActivateEvent_Impl(sal_uInt16 );
    SAL_DLLPRIVATE SfxObjectShell* GetParentShellByModel_Impl();

    // configuration items
    SAL_DLLPRIVATE SfxToolBoxConfig* GetToolBoxConfig_Impl();
    SAL_DLLPRIVATE sal_uInt16 ImplGetSignatureState( sal_Bool bScriptingContent = sal_False );

    SAL_DLLPRIVATE ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignatureInformation >
        ImplAnalyzeSignature(
            sal_Bool bScriptingContent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures >& xSigner
                = ::com::sun::star::uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures >() );

    SAL_DLLPRIVATE void ImplSign( sal_Bool bScriptingContent = sal_False );
    SAL_DLLPRIVATE sal_Bool QuerySaveSizeExceededModules_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );

    SAL_DLLPRIVATE void CheckOut( );
    SAL_DLLPRIVATE void CancelCheckOut( );
    SAL_DLLPRIVATE void CheckIn( );
};

#define SFX_GLOBAL_CLASSID \
    0x9eaba5c3, 0xb232, 0x4309, \
    0x84, 0x5f, 0x5f, 0x15, 0xea, 0x50, 0xd0, 0x74

    struct ModifyBlocker_Impl
    {
        SfxObjectShell* pPersist;
        sal_Bool bWasEnabled;
        ModifyBlocker_Impl( SfxObjectShell* pPersistP ) : pPersist( pPersistP )
        {
            bWasEnabled = pPersistP->IsEnableSetModified();
            if ( bWasEnabled )
                pPersistP->EnableSetModified( sal_False );
        }
        ~ModifyBlocker_Impl()
        {
            if ( bWasEnabled )
                pPersist->EnableSetModified( bWasEnabled );
        }
    };


//--------------------------------------------------------------------
#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
SV_DECL_REF(SfxObjectShell)
#endif
SV_DECL_LOCK(SfxObjectShell)
SV_IMPL_LOCK(SfxObjectShell)
SV_IMPL_REF(SfxObjectShell)

//--------------------------------------------------------------------
class AutoReloadTimer_Impl : public Timer
{
    String          aUrl;
    SfxObjectShell*   pObjSh;

public:
    AutoReloadTimer_Impl( const String& rURL, sal_uInt32 nTime,
                          SfxObjectShell* pSh );
    virtual void Timeout();
};

//-------------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxObjectShellItem: public SfxPoolItem
{
    SfxObjectShell*         pObjSh;

public:
                            TYPEINFO();
                            SfxObjectShellItem() :
                                SfxPoolItem( 0 ),
                                pObjSh( 0 )
                            {}
                            SfxObjectShellItem( SfxObjectShell *pObjShell ):
                                SfxPoolItem( 0 ),
                                pObjSh( pObjShell )
                            {}
                            SfxObjectShellItem( sal_uInt16 nWhichId,
                                                SfxObjectShell *pObjShell ):
                                SfxPoolItem( nWhichId ),
                                pObjSh( pObjShell )
                            {}

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual OUString        GetValueText() const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    SfxObjectShell*         GetObjectShell() const
                            { return pObjSh; }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
