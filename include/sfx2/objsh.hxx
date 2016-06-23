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
#ifndef INCLUDED_SFX2_OBJSH_HXX
#define INCLUDED_SFX2_OBJSH_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/signaturestate.hxx>
#include <sal/types.h>
#include <tools/errcode.hxx>
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
#include <com/sun/star/document/CmisVersion.hpp>

#include <vcl/vclptr.hxx>
#include <svl/poolitem.hxx>
#include <vcl/bitmap.hxx>
#include <sot/formats.hxx>
#include <sot/object.hxx>
#include <rsc/rscsfx.hxx>

#include <sfx2/XmlIdRegistry.hxx>
#include <sfx2/shell.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <memory>
#include <set>
#include <o3tl/typed_flags_set.hxx>
#include <functional>

#include <LibreOfficeKit/LibreOfficeKitTypes.h>

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
enum class SfxModelFlags;

// These values presumably must match exactly the corresponding
// css::embed::Aspects ones (in offapi/com/sun/star/embed/Aspects.idl)
// and the DVASPECT_* ones in the Win32 API.

#define ASPECT_CONTENT      1
#define ASPECT_THUMBNAIL    2 /* 120 * 120, 6 Colors DIB in MetaFile */
#define ASPECT_ICON         4
#define ASPECT_DOCPRINT     8

// must be the same as in OLE2
#define SVOBJ_MISCSTATUS_NOTRESIZEABLE          512
#define SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE  2048

namespace sfx2
{
    class SvLinkSource;
    class StyleManager;
}

namespace com { namespace sun { namespace star { namespace datatransfer { class XTransferable; } } } }

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
    namespace lang {
        class XComponent;
    }
    namespace text {
        class XTextRange;
    }
} } }

enum class SfxObjectShellFlags
{
    HASOPENDOC      = 0x01L,
    HASMENU         = 0x04L,
    DONTLOADFILTERS = 0x08L,
    DONTCLOSE       = 0x10L,
    NODOCINFO       = 0x20L,
    STD_NORMAL      = HASOPENDOC,
    STD_SPECIAL     = DONTLOADFILTERS,
    UNDEFINED       = 0xf000000
};
namespace o3tl
{
    template<> struct typed_flags<SfxObjectShellFlags> : is_typed_flags<SfxObjectShellFlags, 0xf00003d> {};
}

#define SFX_TITLE_TITLE    0
#define SFX_TITLE_FILENAME 1
#define SFX_TITLE_FULLNAME 2
#define SFX_TITLE_APINAME  3
#define SFX_TITLE_DETECT   4
#define SFX_TITLE_CAPTION  5
#define SFX_TITLE_PICKLIST 6
#define SFX_TITLE_HISTORY  7
#define SFX_TITLE_MAXLEN   10   // this gives the limits on length

enum class SfxLoadedFlags
{
    NONE         = 0,
    MAINDOCUMENT = 1,
    IMAGES       = 2,
    ALL          = MAINDOCUMENT | IMAGES
};
namespace o3tl
{
    template<> struct typed_flags<SfxLoadedFlags> : is_typed_flags<SfxLoadedFlags, 0x03> {};
}

enum class HiddenInformation
{
    NONE                  = 0x0000,
    RECORDEDCHANGES       = 0x0001,
    NOTES                 = 0x0002,
    DOCUMENTVERSIONS      = 0x0004
};
namespace o3tl
{
    template<> struct typed_flags<HiddenInformation> : is_typed_flags<HiddenInformation, 0x07> {};
}

enum class HiddenWarningFact
{
    WhenSaving = 0,
    WhenPrinting,
    WhenSigning,
    WhenCreatingPDF
};

enum class SfxObjectCreateMode
{
    EMBEDDED,
    STANDARD,
    PREVIEW,
    ORGANIZER,
    PLUGIN,
    INTERNAL
};

/*
    The class SfxObjectShell is the base class for SFx-objects, ie documents
    and parts of documents that can be integrated as separate objects
    into foreign objects.
*/

class SfxToolBoxConfig;
struct TransferableObjectDescriptor;
template<class T> bool checkSfxObjectShell(const SfxObjectShell* pShell)
{
    return dynamic_cast<const T*>(pShell) != nullptr;
}

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
    SfxObjectCreateMode         eCreateMode;      // Purpose of the object
    bool                        bHasName :1;      // sal_True  := existing object,
                                                  // sal_False := new object
    bool                        bIsInGenerateThumbnail; //optimize thumbnail generate and store procedure to improve odt saving performance, i120030
    bool                        mbAvoidRecentDocs; ///< Avoid adding to the recent documents list, if not necessary.

    bool                        CloseInternal();
private:
    SAL_DLLPRIVATE void UpdateTime_Impl(const css::uno::Reference<
        css::document::XDocumentProperties> & i_xDocProps);

    SAL_DLLPRIVATE bool SaveTo_Impl(SfxMedium &rMedium, const SfxItemSet* pSet );

protected:
                                SfxObjectShell(SfxObjectCreateMode);
                                SfxObjectShell(SfxModelFlags);    // see sfxmodelfactory.hxx
    virtual                     ~SfxObjectShell();

    void                        ModifyChanged();
    virtual bool                Close() override;

    /** declares the document to have capabilities to contain basic/dialog libraries
    */
    void                        SetHasNoBasic();

    /// template method, called by FlushDocInfo; this implementation is empty
    virtual void                DoFlushDocInfo();

    // helper method
    void AddToRecentlyUsedList();

public:
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXDOCSH)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    /* Stampit disable/enable cancel button for print jobs
       default = true = enable! */
    void                        Stamp_SetPrintCancelState(bool bState);
    bool                        Stamp_GetPrintCancelState() const;

    static OUString CreateShellID( const SfxObjectShell* pShell );

    // Document-Shell Iterator
    static SfxObjectShell*      GetFirst( std::function<bool ( const SfxObjectShell* )> isObjectShell = nullptr,
                                          bool bOnlyVisible = true );
    static SfxObjectShell*      GetNext( const SfxObjectShell& rPrev,
                                         std::function<bool ( const SfxObjectShell* )> isObjectShell = nullptr,
                                         bool bOnlyVisible = true );
    static SfxObjectShell*      Current();
    static css::uno::Reference< css::uno::XInterface >
                                GetCurrentComponent();
    static void                 SetCurrentComponent( const css::uno::Reference< css::uno::XInterface >& _rxComponent );

    virtual void                Invalidate(sal_uInt16 nId = 0) override;

    SfxObjectShellFlags         GetFlags( ) const;

    SfxModule*                  GetModule() const;

    virtual SfxObjectFactory&   GetFactory() const=0;
    SfxMedium *                 GetMedium() const { return pMedium; }
    css::uno::Reference< css::document::XDocumentProperties >
                                getDocProperties();
    void                        UpdateDocInfoForSave(  );
    void                        FlushDocInfo();
    bool                        HasName() const { return bHasName; }
    OUString                    GetAPIName() const;
    void                        SetReadOnly();
    bool                        IsReadOnly() const;
    bool                        IsReadOnlyMedium() const;
    bool                        IsOriginallyReadOnlyMedium() const;
    void                        SetReadOnlyUI( bool bReadOnly = true );
    bool                        IsReadOnlyUI() const;
    void                        SetNoName();
    bool                        IsInModalMode() const;
    bool                        IsInPrepareClose() const;
    bool                        AcceptStateUpdate() const;
    bool                        IsHelpDocument() const;

    bool                        IsDocShared() const;
    OUString                    GetSharedFileURL() const;
    bool                        SwitchToShared( bool bShared, bool bSave );
    SAL_DLLPRIVATE void         FreeSharedFile();
    SAL_DLLPRIVATE void         FreeSharedFile( const OUString& aTempFileURL );
    SAL_DLLPRIVATE void         DoNotCleanShareControlFile();
    void                        SetSharedXMLFlag( bool bFlag ) const;
    bool                        HasSharedXMLFlagSet() const;

    SAL_DLLPRIVATE void         SetModalMode_Impl(bool bModal=true);
    SAL_DLLPRIVATE void         SetMacroMode_Impl(bool bModal=true);

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
    bool                        DoInitNew( SfxMedium* pMedium=nullptr );
    bool                        DoLoad( SfxMedium* pMedium );
    bool                        DoLoadExternal( SfxMedium* pMed );
    bool                        DoSave();
    bool                        DoSaveAs( SfxMedium &rNewStor );
    bool                        DoSaveObjectAs( SfxMedium &rNewStor, bool bCommit );

    // TODO/LATER: currently only overridden in Calc, should be made non-virtual
    virtual bool                DoSaveCompleted( SfxMedium* pNewStor=nullptr );

    bool                        LoadOwnFormat( SfxMedium& pMedium );
    virtual bool                SaveAsOwnFormat( SfxMedium& pMedium );
    virtual bool                ConvertFrom( SfxMedium &rMedium );
    virtual bool                ConvertTo( SfxMedium &rMedium );
    virtual bool                InitNew( const css::uno::Reference< css::embed::XStorage >& xStorage );
    virtual bool                Load( SfxMedium &rMedium  );
    virtual bool                LoadFrom( SfxMedium& rMedium );
    virtual bool                Save();
    virtual bool                SaveAs( SfxMedium &rMedium  );
    virtual bool                SaveCompleted( const css::uno::Reference< css::embed::XStorage >& xStorage );
    bool                        SwitchPersistance(
                                    const css::uno::Reference< css::embed::XStorage >& xStorage );
    virtual void                UpdateLinks();
    virtual bool                LoadExternal( SfxMedium& rMedium );
    /**
     * Called when the Options dialog is dismissed with the OK button, to
     * handle potentially conflicting option settings.
     */
    virtual void                CheckConfigOptions();
    bool                        IsConfigOptionsChecked() const;
    void                        SetConfigOptionsChecked( bool bChecked );

    // called for a few slots like SID_SAVE[AS]DOC, SID_PRINTDOC[DIRECT], derived classes may abort the action
    virtual bool                QuerySlotExecutable( sal_uInt16 nSlotId );

    bool                        SaveChildren(bool bObjectsOnly=false);
    bool                        SaveAsChildren( SfxMedium &rMedium );
    bool                        SwitchChildrenPersistance(
                                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    bool bForceNonModified = false );
    bool                        SaveCompletedChildren( bool bSuccess );

    /** a very special case to insert at a position in Writer from UNO,
        via OwnSubFilterService */
    virtual bool                InsertGeneratedStream(SfxMedium& rMedium,
            css::uno::Reference<css::text::XTextRange> const& xInsertPosition);
    virtual bool                ImportFrom( SfxMedium &rMedium,
            css::uno::Reference<css::text::XTextRange> const& xInsertPosition);
    bool                        ExportTo( SfxMedium &rMedium );

    // xmlsec05, check with SFX team
    SignatureState              GetDocumentSignatureState();
    void                        SignDocumentContent();
    SignatureState              GetScriptingSignatureState();
    void                        SignScriptingContent();

    virtual VclPtr<SfxDocumentInfoDialog> CreateDocumentInfoDialog(
                                        vcl::Window *pParent, const SfxItemSet& );

    ErrCode                     CallBasic( const OUString& rMacro, const OUString& rBasicName,
                                    SbxArray* pArgs = nullptr, SbxValue* pRet = nullptr );

    ErrCode     CallXScript(
        const OUString& rScriptURL,
        const css::uno::Sequence< css::uno::Any >& aParams,
        css::uno::Any& aRet,
        css::uno::Sequence< sal_Int16 >& aOutParamIndex,
        css::uno::Sequence< css::uno::Any >& aOutParam,
        bool bRaiseError = true,
        const css::uno::Any* aCaller = nullptr );

    static ErrCode  CallXScript(
        const css::uno::Reference< css::uno::XInterface >& _rxScriptContext,
        const OUString& rScriptURL,
        const css::uno::Sequence< css::uno::Any >& aParams,
        css::uno::Any& aRet,
        css::uno::Sequence< sal_Int16 >& aOutParamIndex,
        css::uno::Sequence< css::uno::Any >& aOutParam,
        bool bRaiseError = true,
        const css::uno::Any* aCaller = nullptr
    );

    /** adjusts the internal macro mode, according to the current security settings

        Finally, the macro mode is either NEVER_EXECUTE or ALWAYS_EXECUTE_NO_WARN.

        @return
            whether macros from this document should be executed
    */
    bool                        AdjustMacroMode( const OUString& rScriptType, bool _bSuppressUI = false );

    SvKeyValueIterator*         GetHeaderAttributes();
    void                        ClearHeaderAttributesForSourceViewHack();
    void                        SetHeaderAttributesForSourceViewHack();

    bool                        IsTemplate() const;

    bool                        IsQueryLoadTemplate() const;
    bool                        IsUseUserData() const;
    bool                        IsUseThumbnailSave() const;
    bool                        IsLoadReadonly() const;
    bool                        IsSaveVersionOnClose() const;
    void                        SetQueryLoadTemplate( bool b );
    void                        SetUseUserData( bool bNew );
    void                        SetUseThumbnailSave( bool _bNew );
    void                        SetLoadReadonly( bool _bReadonly );
    void                        SetSaveVersionOnClose( bool bSet );
    void                        ResetFromTemplate( const OUString& rTemplateName, const OUString& rFileName );

    // TODO/LATER: the following two methods should be replaced by Get/SetModifPasswordInfo in future
    sal_uInt32                  GetModifyPasswordHash() const;
    bool                        SetModifyPasswordHash( sal_uInt32 nHash );

    css::uno::Sequence< css::beans::PropertyValue > GetModifyPasswordInfo() const;
    bool                        SetModifyPasswordInfo( const css::uno::Sequence< css::beans::PropertyValue >& aInfo );

    static sal_uInt32           HandleFilter( SfxMedium* pMedium, SfxObjectShell* pDoc );

    virtual bool                PrepareClose(bool bUI = true);
    virtual bool                IsInformationLost();
    virtual HiddenInformation   GetHiddenInformationState( HiddenInformation nStates );
    sal_Int16                   QueryHiddenInformation( HiddenWarningFact eFact, vcl::Window* pParent );
    bool                        IsSecurityOptOpenReadOnly() const;
    void                        SetSecurityOptOpenReadOnly( bool bOpenReadOnly = true );

    virtual Size                GetFirstPageSize();
    bool                        DoClose();
    virtual void                PrepareReload();
    std::shared_ptr<GDIMetaFile> GetPreviewMetaFile( bool bFullContent = false ) const;
    virtual void                CancelTransfers();

    bool                        GenerateAndStoreThumbnail(
                                    bool bEncrypted,
                                    bool bIsTemplate,
                                    const css::uno::Reference< css::embed::XStorage >& xStor );

    bool                        WriteThumbnail(
                                    bool bEncrypted,
                                    bool bIsTemplate,
                                    const css::uno::Reference< css::io::XStream >& xStream );

    bool                        IsInGenerateAndStoreThumbnail() const {return bIsInGenerateThumbnail;}//optimize thumbnail generate and store procedure to improve odt saving performance, i120030

    /// Don't add to the recent documents - it's an expensive operation, sometimes it is not wanted.
    bool                        IsAvoidRecentDocs() const { return mbAvoidRecentDocs; }

    /// Don't add to the recent documents - it's an expensive operation, sometimes it is not wanted.
    void                        AvoidRecentDocs(bool bAvoid = true) { mbAvoidRecentDocs = bAvoid; }

    // Transfer IFace
    void                        AbortImport();
    bool                        IsAbortingImport() const;
    void                        FinishedLoading( SfxLoadedFlags nWhich = SfxLoadedFlags::ALL );
    void                        TemplateDisconnectionAfterLoad();
    bool                        IsLoading() const;
    bool                        IsLoadingFinished() const;
    void                        SetAutoLoad( const INetURLObject&, sal_uInt32 nTime, bool bReload = true );
    bool                        IsAutoLoadLocked() const;

    // Misc
    bool                        IsPreview() const;
    SfxObjectCreateMode         GetCreateMode() const { return eCreateMode; }
    SfxProgress*                GetProgress() const;
    void                        SetWaitCursor( bool bSet ) const;

    // Naming Interface
    void                        SetTitle( const OUString& rTitle );
    OUString                    GetTitle( sal_uInt16 nMaxLen = 0 ) const;
    void                        InvalidateName();  // Re-set to unnamed

#if defined WNT
    // DDE-Interface
    long                        DdeExecute( const OUString& rCmd );
    virtual bool                DdeGetData( const OUString& rItem,
                                            const OUString& rMimeType,
                                            css::uno::Any & rValue );
    virtual bool                DdeSetData( const OUString& rItem,
                                            const OUString& rMimeType,
                                            const css::uno::Any & rValue );
#endif
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const OUString& rItem );
    virtual void                ReconnectDdeLink(SfxObjectShell& rServer);

    static void                 ReconnectDdeLinks(SfxObjectShell& rServer);

    // Contents
    virtual SfxStyleSheetBasePool*  GetStyleSheetPool();

    virtual void                LoadStyles(SfxObjectShell &rSource);

    virtual sfx2::StyleManager* GetStyleManager();

    // Determine the position of the "Automatic" filter in the stylist
    void                        SetAutoStyleFilterIndex(sal_uInt16 nSet);
    sal_uInt16                  GetAutoStyleFilterIndex();
    bool                        HasBasic() const;
    BasicManager*               GetBasicManager() const;
    css::uno::Reference< css::script::XLibraryContainer >
                                GetBasicContainer();
    css::uno::Reference< css::script::XLibraryContainer >
                                GetDialogContainer();
    StarBASIC*                  GetBasic() const;

    virtual std::set<Color>     GetDocColors();

                                // Documents, for which to format the view size

    virtual SfxObjectShell*     GetObjectShell() override;

    css::uno::Reference< css::frame::XModel >
                                GetModel() const;
    // Only temporarily for the applications!
    void                        SetBaseModel( SfxBaseModel* pModel );
    css::uno::Reference< css::frame::XModel > GetBaseModel() const;
    // Only temporarily for the applications!

    virtual css::uno::Sequence< OUString > GetEventNames();

    vcl::Window*                GetDialogParent( SfxMedium* pMedium=nullptr );
    static SfxObjectShell*      CreateObject( const OUString& rServiceName, SfxObjectCreateMode = SfxObjectCreateMode::STANDARD );
    static SfxObjectShell*      CreateObjectByFactoryName( const OUString& rURL, SfxObjectCreateMode = SfxObjectCreateMode::STANDARD );
    static css::uno::Reference< css::lang::XComponent >
                                CreateAndLoadComponent( const SfxItemSet& rSet, SfxFrame* pFrame = nullptr );
    static SfxObjectShell*      GetShellFromComponent( const css::uno::Reference< css::lang::XComponent >& xComp );
    static OUString             GetServiceNameFromFactory( const OUString& rFact );
    bool                        IsInPlaceActive();
    bool                        IsUIActive();
    virtual void                InPlaceActivate( bool );

    static bool                 CopyStoragesOfUnknownMediaType(
                                    const css::uno::Reference< css::embed::XStorage >& xSource,
                                    const css::uno::Reference< css::embed::XStorage >& xTarget );

    // The functions from SvPersist
    void            EnableSetModified( bool bEnable = true );
    bool            IsEnableSetModified() const;
    virtual void    SetModified( bool bModified = true );
    bool            IsModified();

    /**
     * @param bChart true if the file is a chart doc and FillClass should not be called
     */
    void            SetupStorage(
                        const css::uno::Reference< css::embed::XStorage >& xStorage,
                        sal_Int32 nVersion, bool bTemplate, bool bChart = false ) const;

    css::uno::Reference< css::embed::XStorage > GetStorage();

    SvGlobalName    GetClassName() const;

    // comphelper::IEmbeddedHelper
    virtual css::uno::Reference< css::task::XInteractionHandler > getInteractionHandler() const override;
    virtual css::uno::Reference < css::embed::XStorage > getStorage() const override
    {
        return const_cast<SfxObjectShell*>(this)->GetStorage();
    }
    virtual comphelper::EmbeddedObjectContainer& getEmbeddedObjectContainer() const override
    {
        return GetEmbeddedObjectContainer();
    }
    bool    isEnableSetModified() const override
    {
        return IsEnableSetModified();
    }
    virtual OUString getDocumentBaseURL() const override;

    comphelper::EmbeddedObjectContainer&    GetEmbeddedObjectContainer() const;
    void    ClearEmbeddedObjects();

    // The functions from SvEmbeddedObject
    virtual Printer *       GetDocumentPrinter();
    virtual OutputDevice*   GetDocumentRefDev();
    virtual void            OnDocumentPrinterChanged( Printer * pNewPrinter );
    virtual Rectangle GetVisArea( sal_uInt16 nAspect ) const;
    virtual void      SetVisArea( const Rectangle & rVisArea );
    const Rectangle & GetVisArea() const;
    void              SetVisAreaSize( const Size & rVisSize );
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
                               SotClipboardFormatId * pFormat,
                               OUString * pAppName,
                               OUString * pFullTypeName,
                               OUString * pShortTypeName,
                               sal_Int32 nVersion,
                               bool bTemplate = false) const = 0;

    // change recording and respective passwword protection for Writer and Calc
    // slots available for Writer:  FN_REDLINE_ON, FN_REDLINE_ON
    // slots used for Calc:         FID_CHG_RECORD, SID_CHG_PROTECT
    virtual bool    IsChangeRecording() const;
    virtual bool    HasChangeRecordProtection() const;
    virtual void    SetChangeRecording( bool bActivate );
    virtual bool    SetProtectionPassword( const OUString &rPassword );
    virtual bool    GetProtectionHash( /*out*/ css::uno::Sequence< sal_Int8 > &rPasswordHash );

    SAL_DLLPRIVATE std::shared_ptr<GDIMetaFile> CreatePreviewMetaFile_Impl( bool bFullContent ) const;

    SAL_DLLPRIVATE bool IsOwnStorageFormat_Impl(const SfxMedium &) const;

    SAL_DLLPRIVATE bool IsPackageStorageFormat_Impl(const SfxMedium &) const;

    SAL_DLLPRIVATE bool ConnectTmpStorage_Impl( const css::uno::Reference< css::embed::XStorage >& xStorage, SfxMedium* pMedium );
    SAL_DLLPRIVATE bool DisconnectStorage_Impl( SfxMedium& rSrcMedium, SfxMedium& rTargetMedium );

    SAL_DLLPRIVATE bool PutURLContentsToVersionStream_Impl(
                    const OUString& aURL,
                    const css::uno::Reference< css::embed::XStorage >& xDocStorage,
                    const OUString& aStreamName );

    SAL_DLLPRIVATE OUString CreateTempCopyOfStorage_Impl(
                    const css::uno::Reference< css::embed::XStorage >& xStorage );

    SAL_DLLPRIVATE void InitOwnModel_Impl();
    SAL_DLLPRIVATE void BreakMacroSign_Impl( bool bBreakMacroSing );
    SAL_DLLPRIVATE void CheckSecurityOnLoading_Impl();
    SAL_DLLPRIVATE void CheckForBrokenDocSignatures_Impl( const css::uno::Reference< css::task::XInteractionHandler >& xHandler );
    SAL_DLLPRIVATE SignatureState ImplCheckSignaturesInformation(
                const css::uno::Sequence< css::security::DocumentSignatureInformation >& aInfos );
    SAL_DLLPRIVATE void CheckEncryption_Impl( const css::uno::Reference< css::task::XInteractionHandler >& xHandler );
    SAL_DLLPRIVATE void SetModifyPasswordEntered( bool bEntered = true );
    SAL_DLLPRIVATE bool IsModifyPasswordEntered();

    SAL_DLLPRIVATE void InitBasicManager_Impl();
    SAL_DLLPRIVATE SfxObjectShell_Impl* Get_Impl() { return pImp; }

    SAL_DLLPRIVATE static bool UseInteractionToHandleError(
                    const css::uno::Reference< css::task::XInteractionHandler >& xHandler,
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
    SAL_DLLPRIVATE bool ImportFromGeneratedStream_Impl(
                    const css::uno::Reference< css::io::XStream >& xStream,
                    const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr );
    SAL_DLLPRIVATE void PositionView_Impl();
    SAL_DLLPRIVATE void UpdateFromTemplate_Impl();
    SAL_DLLPRIVATE bool CanReload_Impl();
    SAL_DLLPRIVATE void SetNamedVisibility_Impl();
    SAL_DLLPRIVATE bool DoSave_Impl( const SfxItemSet* pSet=nullptr );
    SAL_DLLPRIVATE bool Save_Impl( const SfxItemSet* pSet=nullptr );
    SAL_DLLPRIVATE bool PreDoSaveAs_Impl(const OUString& rFileName, const OUString& rFiltName, SfxItemSet& rItemSet);
    SAL_DLLPRIVATE bool APISaveAs_Impl(const OUString& aFileName, SfxItemSet& rItemSet);
    SAL_DLLPRIVATE bool CommonSaveAs_Impl(const INetURLObject& aURL, const OUString& aFilterName, SfxItemSet& rItemSet);
    SAL_DLLPRIVATE bool GeneralInit_Impl(
                                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    bool bTypeMustBeSetAlready );
    SAL_DLLPRIVATE void PrepareSecondTryLoad_Impl();
    SAL_DLLPRIVATE void SetInitialized_Impl( const bool i_fromInitNew );

    // public-internals
    SAL_DLLPRIVATE IndexBitSet& GetNoSet_Impl();
    SAL_DLLPRIVATE void SetProgress_Impl( SfxProgress *pProgress );
    SAL_DLLPRIVATE void PostActivateEvent_Impl( SfxViewFrame* );
    SAL_DLLPRIVATE void SetActivateEvent_Impl(sal_uInt16 );
    SAL_DLLPRIVATE SfxObjectShell* GetParentShellByModel_Impl();

    // configuration items
    SAL_DLLPRIVATE SignatureState ImplGetSignatureState( bool bScriptingContent = false );

    SAL_DLLPRIVATE css::uno::Sequence< css::security::DocumentSignatureInformation >
        ImplAnalyzeSignature(
            bool bScriptingContent,
            const css::uno::Reference< css::security::XDocumentDigitalSignatures >& xSigner
                = css::uno::Reference< css::security::XDocumentDigitalSignatures >() );

    SAL_DLLPRIVATE void ImplSign( bool bScriptingContent = false );
    SAL_DLLPRIVATE bool QuerySaveSizeExceededModules_Impl( const css::uno::Reference< css::task::XInteractionHandler >& xHandler );

    SAL_DLLPRIVATE void CheckOut( );
    SAL_DLLPRIVATE void CancelCheckOut( );
    SAL_DLLPRIVATE void CheckIn( );
    SAL_DLLPRIVATE css::uno::Sequence< css::document::CmisVersion > GetCmisVersions();
};

#define SFX_GLOBAL_CLASSID \
    0x9eaba5c3, 0xb232, 0x4309, \
    0x84, 0x5f, 0x5f, 0x15, 0xea, 0x50, 0xd0, 0x74

    struct ModifyBlocker_Impl
    {
        SfxObjectShell* pPersist;
        bool bWasEnabled;
        ModifyBlocker_Impl( SfxObjectShell* pPersistP ) : pPersist( pPersistP )
        {
            bWasEnabled = pPersistP->IsEnableSetModified();
            if ( bWasEnabled )
                pPersistP->EnableSetModified( false );
        }
        ~ModifyBlocker_Impl()
        {
            if ( bWasEnabled )
                pPersist->EnableSetModified( bWasEnabled );
        }
    };


#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
typedef tools::SvRef<SfxObjectShell> SfxObjectShellRef;
#endif

class SfxObjectShellLock
{
protected:
    SfxObjectShell * pObj;
public:
    inline               SfxObjectShellLock() { pObj = nullptr; }
    inline               SfxObjectShellLock( const SfxObjectShellLock & rObj );
    inline               SfxObjectShellLock( SfxObjectShell * pObjP );
    inline void          Clear();
    inline               ~SfxObjectShellLock();
    inline SfxObjectShellLock & operator = ( const SfxObjectShellLock & rObj );
    inline SfxObjectShellLock & operator = ( SfxObjectShell * pObj );
    inline bool        Is() const { return pObj != nullptr; }
    inline SfxObjectShell *     operator &  () const { return pObj; }
    inline SfxObjectShell *     operator -> () const { return pObj; }
    inline SfxObjectShell &     operator *  () const { return *pObj; }
    inline operator SfxObjectShell * () const { return pObj; }
};
inline SfxObjectShellLock::SfxObjectShellLock( const SfxObjectShellLock & rObj )
    { pObj = rObj.pObj; if( pObj ) { pObj->OwnerLock( true ); } }
inline SfxObjectShellLock::SfxObjectShellLock( SfxObjectShell * pObjP )
{ pObj = pObjP; if( pObj ) { pObj->OwnerLock( true ); } }
inline void SfxObjectShellLock::Clear()
{
    if( pObj )
    {
        SfxObjectShell* const pRefObj = pObj;
        pObj = nullptr;
        pRefObj->OwnerLock( false );
    }
}
inline SfxObjectShellLock::~SfxObjectShellLock()
{ if( pObj ) { pObj->OwnerLock( false ); } }
inline SfxObjectShellLock & SfxObjectShellLock::
            operator = ( const SfxObjectShellLock & rObj )
{
    if( rObj.pObj ) rObj.pObj->OwnerLock( true );
    SfxObjectShell* const pRefObj = pObj;
    pObj = rObj.pObj;
    if( pRefObj ) { pRefObj->OwnerLock( false ); }
    return *this;
}
inline SfxObjectShellLock & SfxObjectShellLock::operator = ( SfxObjectShell * pObjP )
{ return *this = SfxObjectShellLock( pObjP ); }

class SFX2_DLLPUBLIC SfxObjectShellItem: public SfxPoolItem
{
    SfxObjectShell*         pObjSh;

public:
                            static SfxPoolItem* CreateDefault();

                            SfxObjectShellItem() :
                                SfxPoolItem( 0 ),
                                pObjSh( nullptr )
                            {}
                            SfxObjectShellItem( sal_uInt16 nWhichId,
                                                SfxObjectShell *pObjShell ):
                                SfxPoolItem( nWhichId ),
                                pObjSh( pObjShell )
                            {}

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
