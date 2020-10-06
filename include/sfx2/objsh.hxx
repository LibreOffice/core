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
#include <vcl/errcode.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>

#include <svl/poolitem.hxx>
#include <sot/formats.hxx>
#include <sot/object.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>

#include <sfx2/shell.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <memory>
#include <set>
#include <o3tl/typed_flags_set.hxx>
#include <functional>
#include <sfx2/AccessibilityIssue.hxx>

namespace weld {class Button; }
class SbxValue;
class SbxArray;
class BasicManager;
class SfxMedium;
class SfxObjectFactory;
class SfxDocumentInfoDialog;
class SfxStyleSheetBasePool;
class StarBASIC;
class Printer;
class SvKeyValueIterator;
class SfxBaseModel;
class SfxModule;
class SfxProgress;
class GDIMetaFile;
class INetURLObject;
class IndexBitSet;
class JobSetup;
class OutputDevice;
class Color;
class Fraction;
class SvGlobalName;
class InfobarData;

enum class SfxModelFlags;
enum class SfxEventHintId;
enum class InfobarType;

// These values presumably must match exactly the corresponding
// css::embed::Aspects ones (in offapi/com/sun/star/embed/Aspects.idl)
// and the DVASPECT_* ones in the Win32 API.

#define ASPECT_CONTENT      1
#define ASPECT_THUMBNAIL    2 /* 120 * 120, 6 Colors DIB in MetaFile */
#define ASPECT_ICON         4
#define ASPECT_DOCPRINT     8

namespace sfx2
{
    class SvLinkSource;
    class StyleManager;
}

namespace vcl { class Window; }
namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::document { struct CmisVersion; }
namespace com::sun::star::embed { class XStorage; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::graphic { class XGraphic; }
namespace com::sun::star::io { class XStream; }
namespace com::sun::star::script { class XLibraryContainer; }
namespace com::sun::star::security { class XCertificate; }
namespace com::sun::star::security { class XDocumentDigitalSignatures; }
namespace com::sun::star::security { struct DocumentSignatureInformation; }
namespace com::sun::star::task { class XInteractionHandler; }

namespace com::sun::star {
    namespace document {
        class XDocumentProperties;
    }
    namespace lang {
        class XComponent;
    }
    namespace text {
        class XTextRange;
    }
}

namespace sfx2 { class IXmlIdRegistry; }

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

namespace weld { class Window; }

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
    ORGANIZER,
    INTERNAL
};

/*
    The class SfxObjectShell is the base class for SFx-objects, ie documents
    and parts of documents that can be integrated as separate objects
    into foreign objects.
    There can be multiple edit windows (SfxViewShell) for one SfxObjectShell.
*/

struct TransferableObjectDescriptor;
template<class T> bool checkSfxObjectShell(const SfxObjectShell* pShell)
{
    return dynamic_cast<const T*>(pShell) != nullptr;
}

class SFX2_DLLPUBLIC SfxObjectShell :
    public SfxShell, virtual public SotObject,
    public ::comphelper::IEmbeddedHelper
{
friend struct ModifyBlocker_Impl;
friend class SfxObjectShellLock;

private:
    std::unique_ptr<struct SfxObjectShell_Impl> pImpl;     // internal data

    SfxMedium *                 pMedium;  // Description of the file for example
                                          // storage that contains the object
    SfxObjectCreateMode         eCreateMode;      // Purpose of the object
    bool                        bHasName :1;      // sal_True  := existing object,
                                                  // sal_False := new object
    bool                        bIsInGenerateThumbnail; //optimize thumbnail generate and store procedure to improve odt saving performance, i120030
    bool                        mbAvoidRecentDocs; ///< Avoid adding to the recent documents list, if not necessary.

    enum TriState               {undefined, yes, no};
    TriState                    mbContinueImportOnFilterExceptions = undefined; // try to import as much as possible

    bool                        CloseInternal();

    SAL_DLLPRIVATE void UpdateTime_Impl(const css::uno::Reference<
        css::document::XDocumentProperties> & i_xDocProps);

    SAL_DLLPRIVATE bool SaveTo_Impl(SfxMedium &rMedium, const SfxItemSet* pSet );

protected:
                                SfxObjectShell(SfxObjectCreateMode);
                                SfxObjectShell(SfxModelFlags);    // see sfxmodelfactory.hxx
    virtual                     ~SfxObjectShell() override;

    void                        ModifyChanged();
    virtual bool                Close() override;

    /// template method, called by FlushDocInfo; this implementation is empty
    virtual void                DoFlushDocInfo();

    // helper method
    void AddToRecentlyUsedList();

    // Parent class for actual guard objects that would do useful work
    class LockAllViewsGuard
    {
    public:
        virtual ~LockAllViewsGuard() {}
    };

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
    static SfxObjectShell*      GetFirst( const std::function<bool ( const SfxObjectShell* )>& isObjectShell = nullptr,
                                          bool bOnlyVisible = true );
    static SfxObjectShell*      GetNext( const SfxObjectShell& rPrev,
                                         const std::function<bool ( const SfxObjectShell* )>& isObjectShell = nullptr,
                                         bool bOnlyVisible = true );
    static SfxObjectShell*      Current();
    static css::uno::Reference< css::uno::XInterface >
                                GetCurrentComponent();
    static void                 SetCurrentComponent( const css::uno::Reference< css::uno::XInterface >& _rxComponent );

    virtual void                Invalidate(sal_uInt16 nId = 0) override;

    SfxModule*                  GetModule() const;

    virtual SfxObjectFactory&   GetFactory() const=0;
    SfxMedium *                 GetMedium() const { return pMedium; }
    css::uno::Reference< css::document::XDocumentProperties >
                                getDocProperties() const;
    void                        UpdateDocInfoForSave(  );
    void                        FlushDocInfo();
    bool                        HasName() const { return bHasName; }
    OUString                    GetAPIName() const;
    void                        SetReadOnly();
    bool                        IsReadOnly() const;
    bool                        IsReadOnlyMedium() const;
    bool                        IsOriginallyReadOnlyMedium() const;
    bool                        IsOriginallyLoadedReadOnlyMedium() const;
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
    SAL_DLLPRIVATE void         FreeSharedFile( const OUString& aTempFileURL );
    SAL_DLLPRIVATE void         DoNotCleanShareControlFile();
    void                        SetSharedXMLFlag( bool bFlag ) const;
    bool                        HasSharedXMLFlagSet() const;

    SAL_DLLPRIVATE void         SetModalMode_Impl(bool bModal);
    SAL_DLLPRIVATE void         SetMacroMode_Impl(bool bModal=true);

    void                        ResetError();
    ErrCode                     GetError() const;
    ErrCode                     GetErrorCode() const;
    void                        SetError(ErrCode rErr);

    /**
     * Initialize bare minimum just enough for unit test runs.
     *
     * @return true if the initialization is successful, false otherwise.
     */
    void                        DoInitUnitTest();
    bool                        DoInitNew( SfxMedium* pMedium=nullptr );
    bool                        DoLoad( SfxMedium* pMedium );
    bool                        DoLoadExternal( SfxMedium* pMed );
    bool                        DoSave();
    bool                        DoSaveAs( SfxMedium &rNewStor );
    bool                        DoSaveObjectAs( SfxMedium &rNewStor, bool bCommit );

    // TODO/LATER: currently only overridden in Calc, should be made non-virtual
    virtual bool                DoSaveCompleted( SfxMedium* pNewStor=nullptr, bool bRegisterRecent=true );
    /// Terminate any in-flight editing. Used before saving, primarily by Calc to commit cell changes.
    virtual void                TerminateEditing() {}

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
    bool                        IsConfigOptionsChecked() const;
    void                        SetConfigOptionsChecked( bool bChecked );

    // called for a few slots like SID_SAVE[AS]DOC, SID_PRINTDOC[DIRECT], derived classes may abort the action
    virtual bool                QuerySlotExecutable( sal_uInt16 nSlotId );

    void                        SaveChildren(bool bObjectsOnly=false);
    bool                        SaveAsChildren( SfxMedium &rMedium );
    bool                        SwitchChildrenPersistance(
                                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    bool bForceNonModified = false );
    bool                        SaveCompletedChildren();

    /** a very special case to insert at a position in Writer from UNO,
        via OwnSubFilterService */
    virtual bool                InsertGeneratedStream(SfxMedium& rMedium,
            css::uno::Reference<css::text::XTextRange> const& xInsertPosition);
    virtual bool                ImportFrom( SfxMedium &rMedium,
            css::uno::Reference<css::text::XTextRange> const& xInsertPosition);
    bool                        ExportTo( SfxMedium &rMedium );

    /** Returns to if preparing was successful, else false. */
    bool PrepareForSigning(weld::Window* pDialogParent);
    bool CheckIsReadonly(bool bSignScriptingContent);
    void RecheckSignature(bool bAlsoRecheckScriptingSignature);
    void AfterSigning(bool bSignSuccess, bool bSignScriptingContent);
    bool HasValidSignatures() const;
    SignatureState              GetDocumentSignatureState();
    void                        SignDocumentContent(weld::Window* pDialogParent);
    css::uno::Sequence<css::security::DocumentSignatureInformation> GetDocumentSignatureInformation(
        bool bScriptingContent,
        const css::uno::Reference<css::security::XDocumentDigitalSignatures>& xSigner
        = css::uno::Reference<css::security::XDocumentDigitalSignatures>());

    bool SignDocumentContentUsingCertificate(const css::uno::Reference<css::security::XCertificate>& xCertificate);

    void SignSignatureLine(weld::Window* pDialogParent, const OUString& aSignatureLineId,
                           const css::uno::Reference<css::security::XCertificate>& xCert,
                           const css::uno::Reference<css::graphic::XGraphic>& xValidGraphic,
                           const css::uno::Reference<css::graphic::XGraphic>& xInvalidGraphic,
                           const OUString& aComment);
    SignatureState              GetScriptingSignatureState();
    void                        SignScriptingContent(weld::Window* pDialogParent);
    DECL_LINK(SignDocumentHandler, weld::Button&, void);

    virtual std::shared_ptr<SfxDocumentInfoDialog> CreateDocumentInfoDialog(weld::Window* pParent, const SfxItemSet& rItemSet);

    ErrCode                     CallBasic( const OUString& rMacro, const OUString& rBasicName,
                                    SbxArray* pArgs, SbxValue* pRet = nullptr );

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
    bool                        AdjustMacroMode();

    static bool                 UnTrustedScript(const OUString& rScriptURL);

    static bool                 isScriptAccessAllowed(const css::uno::Reference<css::uno::XInterface>& rScriptContext);

    SvKeyValueIterator*         GetHeaderAttributes();
    void                        ClearHeaderAttributesForSourceViewHack();
    void                        SetHeaderAttributesForSourceViewHack();

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

    void                        SetMacroCallsSeenWhileLoading();
    bool                        GetMacroCallsSeenWhileLoading() const;

    const css::uno::Sequence< css::beans::PropertyValue >& GetModifyPasswordInfo() const;
    bool                        SetModifyPasswordInfo( const css::uno::Sequence< css::beans::PropertyValue >& aInfo );

    static ErrCode              HandleFilter( SfxMedium* pMedium, SfxObjectShell const * pDoc );

    virtual bool                PrepareClose(bool bUI = true);
    virtual HiddenInformation   GetHiddenInformationState( HiddenInformation nStates );
    sal_Int16                   QueryHiddenInformation( HiddenWarningFact eFact, weld::Window* pParent );
    bool                        IsSecurityOptOpenReadOnly() const;
    void                        SetSecurityOptOpenReadOnly( bool bOpenReadOnly );

    Size                        GetFirstPageSize() const;
    bool                        DoClose();
    std::shared_ptr<GDIMetaFile> GetPreviewMetaFile( bool bFullContent = false ) const;
    virtual void                CancelTransfers();

    bool                        GenerateAndStoreThumbnail(
                                    bool bEncrypted,
                                    const css::uno::Reference< css::embed::XStorage >& xStor );

    bool                        WriteThumbnail(
                                    bool bEncrypted,
                                    const css::uno::Reference< css::io::XStream >& xStream );

    bool                        IsInGenerateAndStoreThumbnail() const {return bIsInGenerateThumbnail;}//optimize thumbnail generate and store procedure to improve odt saving performance, i120030

    /// Don't add to the recent documents - it's an expensive operation, sometimes it is not wanted.
    bool                        IsAvoidRecentDocs() const { return mbAvoidRecentDocs; }

    /// Don't add to the recent documents - it's an expensive operation, sometimes it is not wanted.
    void                        AvoidRecentDocs(bool bAvoid) { mbAvoidRecentDocs = bAvoid; }

    /// On first error ask user if import should continue; return saved answer.
    bool                        IsContinueImportOnFilterExceptions(const OUString& aErrMessage);

    // Transfer IFace
    bool                        IsAbortingImport() const;
    void                        FinishedLoading( SfxLoadedFlags nWhich = SfxLoadedFlags::ALL );

    void                        TemplateDisconnectionAfterLoad();
    void                        SetLoading(SfxLoadedFlags nFlags);
    bool                        IsLoading() const;
    bool                        IsLoadingFinished() const;
    void                        SetAutoLoad( const INetURLObject&, sal_uInt32 nTime, bool bReload );
    bool                        IsAutoLoadLocked() const;

    // Misc
    bool                        IsPreview() const;
    SfxObjectCreateMode         GetCreateMode() const { return eCreateMode; }
    SfxProgress*                GetProgress() const;
    void                        SetWaitCursor( bool bSet ) const;

    // Naming Interface
    void                        SetTitle( const OUString& rTitle );
    /* Small non-zero values of nMaxLen don't mean length, but have a magic meaning:
       0 (default)
       the title itself, as it is

       1 (==SFX_TITLE_FILENAME)
       provides the logical file name without path
       (under WNT depending on the system settings
       without extension)

       2 (==SFX_TITLE_FULLNAME)
       provides the logical file names with full path
       (remote =>:: com:: sun:: star:: util:: URL)

       3 (==SFX_TITLE_APINAME)
       provides the logical filename without path
       and extension

       4 (==SFX_TITLE_DETECT)
       provides the complete title, if not set yet
       it will be created from DocInfo or the name of
       the medium.

       5 (==SFX_TITLE_CAPTION)
       provides the Title just like MB now in the
       CaptionBar view

       6 (==SFX_TITLE_PICKLIST)
       returns the Title, just like MB now would
       display it in the PickList

       7 (==SFX_TITLE_HISTORY)
       returns the Title just like MB now would
       display it in the History

       10 bis USHRT_MAX
       provides the 'nMaxLength' of the logical
       file name including the path
       (remote => css::util::URL)
    */
    OUString                    GetTitle( sal_uInt16 nMaxLen = 0 ) const;
    void                        InvalidateName();  // Re-set to unnamed

#if defined(_WIN32)
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
    sal_uInt16                  GetAutoStyleFilterIndex() const;
    bool                        HasBasic() const;
    BasicManager*               GetBasicManager() const;
    css::uno::Reference< css::script::XLibraryContainer >
                                GetBasicContainer();
    css::uno::Reference< css::script::XLibraryContainer >
                                GetDialogContainer();
    StarBASIC*                  GetBasic() const;

    virtual std::set<Color>     GetDocColors();

    // Accessibility Check
    virtual sfx::AccessibilityIssueCollection runAccessibilityCheck();

                                // Documents, for which to format the view size

    virtual SfxObjectShell*     GetObjectShell() override;

    css::uno::Reference< css::frame::XModel >
                                GetModel() const;
    // Only temporarily for the applications!
    void                        SetBaseModel( SfxBaseModel* pModel );
    css::uno::Reference< css::frame::XModel > GetBaseModel() const;
    // Only temporarily for the applications!

    virtual css::uno::Sequence< OUString > GetEventNames();

    vcl::Window*                GetDialogParent( SfxMedium const * pMedium=nullptr );
    static SfxObjectShell*      CreateObject( const OUString& rServiceName, SfxObjectCreateMode = SfxObjectCreateMode::STANDARD );
    static SfxObjectShell*      CreateObjectByFactoryName( const OUString& rURL, SfxObjectCreateMode = SfxObjectCreateMode::STANDARD );
    static css::uno::Reference< css::lang::XComponent >
                                CreateAndLoadComponent( const SfxItemSet& rSet );
    static SfxObjectShell* GetShellFromComponent(const css::uno::Reference< css::uno::XInterface >& xComp);
    static SfxObjectShell* GetParentShell(const css::uno::Reference<css::uno::XInterface>& xChild);
    static OUString             GetServiceNameFromFactory( const OUString& rFact );
    bool                        IsInPlaceActive() const;
    bool                        IsUIActive() const;

    static bool                 CopyStoragesOfUnknownMediaType(
                                    const css::uno::Reference< css::embed::XStorage >& xSource,
                                    const css::uno::Reference<css::embed::XStorage>& xTarget,
                                    const css::uno::Sequence<OUString>& rExceptions = css::uno::Sequence<OUString>());

    bool isEditDocLocked() const;
    bool isContentExtractionLocked() const;
    bool isExportLocked() const;
    bool isPrintLocked() const;
    bool isSaveLocked() const;

    // The functions from SvPersist
    void            EnableSetModified( bool bEnable = true );
    bool            IsEnableSetModified() const;
    virtual void    SetModified( bool bModified = true );
    bool            IsModified() const;

    /**
     * @param bChart true if the file is a chart doc and FillClass should not be called
     */
    void            SetupStorage(
                        const css::uno::Reference< css::embed::XStorage >& xStorage,
                        sal_Int32 nVersion, bool bTemplate ) const;

    css::uno::Reference< css::embed::XStorage > const & GetStorage();

    SvGlobalName const & GetClassName() const;

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
    virtual tools::Rectangle GetVisArea( sal_uInt16 nAspect ) const;
    virtual void      SetVisArea( const tools::Rectangle & rVisArea );
    const tools::Rectangle & GetVisArea() const;
    void              SetVisAreaSize( const Size & rVisSize );

    MapUnit         GetMapUnit() const;
    void            SetMapUnit( MapUnit nMUnit );

    void            FillTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc ) const;
    void            DoDraw( OutputDevice *, const Point & rObjPos,
                            const Size & rSize,
                            const JobSetup & rSetup,
                            sal_uInt16 nAspect = ASPECT_CONTENT );
    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                          sal_uInt16 nAspect ) = 0;


    virtual void    FillClass( SvGlobalName * pClassName,
                               SotClipboardFormatId * pFormat,
                               OUString * pFullTypeName,
                               sal_Int32 nVersion,
                               bool bTemplate = false) const = 0;

    // change recording and respective passwword protection for Writer and Calc
    // slots available for Writer:  FN_REDLINE_ON, FN_REDLINE_ON
    // slots used for Calc:         FID_CHG_RECORD, SID_CHG_PROTECT
    virtual bool    IsChangeRecording() const;
    virtual bool    HasChangeRecordProtection() const;
    virtual void    SetChangeRecording( bool bActivate );
    virtual void    SetProtectionPassword( const OUString &rPassword );
    virtual bool    GetProtectionHash( /*out*/ css::uno::Sequence< sal_Int8 > &rPasswordHash );

    static bool IsOwnStorageFormat(const SfxMedium &);

    /** Append Infobar once the frame is ready.
        Useful when you want to register an Infobar before the doc/frame is fully loaded. */
    void AppendInfoBarWhenReady(const OUString& sId, const OUString& sPrimaryMessage,
                                const OUString& sSecondaryMessage, InfobarType aInfobarType,
                                bool bShowCloseButton = true);
    std::vector<InfobarData>& getPendingInfobars();

    SAL_DLLPRIVATE std::shared_ptr<GDIMetaFile> CreatePreviewMetaFile_Impl(bool bFullContent) const;

    SAL_DLLPRIVATE static bool IsPackageStorageFormat_Impl(const SfxMedium &);

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
    SAL_DLLPRIVATE void CheckForBrokenDocSignatures_Impl();
    SAL_DLLPRIVATE void CheckEncryption_Impl( const css::uno::Reference< css::task::XInteractionHandler >& xHandler );
    SAL_DLLPRIVATE void SetModifyPasswordEntered( bool bEntered = true );
    SAL_DLLPRIVATE bool IsModifyPasswordEntered() const;

    SAL_DLLPRIVATE void InitBasicManager_Impl();
    SAL_DLLPRIVATE SfxObjectShell_Impl* Get_Impl() { return pImpl.get(); }

    SAL_DLLPRIVATE static bool UseInteractionToHandleError(
                    const css::uno::Reference< css::task::XInteractionHandler >& xHandler,
                    ErrCode nError );
    SAL_DLLPRIVATE const SfxObjectShell_Impl* Get_Impl() const { return pImpl.get(); }

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
    SAL_DLLPRIVATE static void StateView_Impl(SfxItemSet &);

    // Load/Save public internals
    SAL_DLLPRIVATE bool ImportFromGeneratedStream_Impl(
                    const css::uno::Reference< css::io::XStream >& xStream,
                    const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr );
    SAL_DLLPRIVATE void UpdateFromTemplate_Impl();
    SAL_DLLPRIVATE bool CanReload_Impl();
    SAL_DLLPRIVATE void SetNamedVisibility_Impl();
    SAL_DLLPRIVATE bool DoSave_Impl( const SfxItemSet* pSet );
    SAL_DLLPRIVATE bool Save_Impl( const SfxItemSet* pSet );
    SAL_DLLPRIVATE bool
    PreDoSaveAs_Impl(const OUString& rFileName, const OUString& rFiltName,
                     SfxItemSet const& rItemSet,
                     const css::uno::Sequence<css::beans::PropertyValue>& rArgs);
    SAL_DLLPRIVATE bool APISaveAs_Impl(const OUString& aFileName, SfxItemSet& rItemSet,
                                       const css::uno::Sequence<css::beans::PropertyValue>& rArgs);
    SAL_DLLPRIVATE bool
    CommonSaveAs_Impl(const INetURLObject& aURL, const OUString& aFilterName, SfxItemSet& rItemSet,
                      const css::uno::Sequence<css::beans::PropertyValue>& rArgs);
    SAL_DLLPRIVATE bool GeneralInit_Impl(
                                    const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    bool bTypeMustBeSetAlready );
    SAL_DLLPRIVATE void PrepareSecondTryLoad_Impl();
    SAL_DLLPRIVATE void SetInitialized_Impl( const bool i_fromInitNew );

    // public-internals
    SAL_DLLPRIVATE IndexBitSet& GetNoSet_Impl();
    SAL_DLLPRIVATE void SetProgress_Impl( SfxProgress *pProgress );
    SAL_DLLPRIVATE void PostActivateEvent_Impl( SfxViewFrame const * );
    SAL_DLLPRIVATE void SetActivateEvent_Impl(SfxEventHintId );

    // configuration items
    SAL_DLLPRIVATE SignatureState ImplGetSignatureState( bool bScriptingContent = false );

    SAL_DLLPRIVATE bool QuerySaveSizeExceededModules_Impl( const css::uno::Reference< css::task::XInteractionHandler >& xHandler );
    SAL_DLLPRIVATE static bool QueryAllowExoticFormat_Impl( const css::uno::Reference< css::task::XInteractionHandler >& xHandler,
                                                     const OUString& rURL,
                                                     const OUString& rFilterUIName);

    SAL_DLLPRIVATE void CheckOut( );
    SAL_DLLPRIVATE void CancelCheckOut( );
    SAL_DLLPRIVATE void CheckIn( );
    SAL_DLLPRIVATE css::uno::Sequence< css::document::CmisVersion > GetCmisVersions() const;

    /** override this if you have a XmlIdRegistry. */
    virtual const sfx2::IXmlIdRegistry* GetXmlIdRegistry() const { return nullptr; }

    /// Is this read-only object shell opened via .uno:SignPDF?
    bool IsSignPDF() const;

    /// Gets the certificate that is already picked by the user but not yet used for signing.
    css::uno::Reference<css::security::XCertificate> GetSignPDFCertificate() const;

    // Lock all unlocked views, and returns a guard object which unlocks those views when destructed
    virtual std::unique_ptr<LockAllViewsGuard> LockAllViews()
    {
        return std::make_unique<LockAllViewsGuard>();
    }

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


typedef tools::SvRef<SfxObjectShell> SfxObjectShellRef;

class SfxObjectShellLock
{
protected:
    SfxObjectShell * pObj;
public:
    SfxObjectShellLock() { pObj = nullptr; }
    inline               SfxObjectShellLock( const SfxObjectShellLock & rObj );
    inline               SfxObjectShellLock( SfxObjectShellLock && rObj ) noexcept;
    inline               SfxObjectShellLock( SfxObjectShell * pObjP );
    inline void          Clear();
    inline               ~SfxObjectShellLock();
    inline SfxObjectShellLock & operator = ( const SfxObjectShellLock & rObj );
    inline SfxObjectShellLock & operator = ( SfxObjectShellLock && rObj );
    inline SfxObjectShellLock & operator = ( SfxObjectShell * pObj );
    bool                 Is() const { return pObj != nullptr; }
    SfxObjectShell *     operator &  () const { return pObj; }
    SfxObjectShell *     operator -> () const { return pObj; }
    SfxObjectShell &     operator *  () const { return *pObj; }
    operator SfxObjectShell * () const { return pObj; }
};
inline SfxObjectShellLock::SfxObjectShellLock( const SfxObjectShellLock & rObj )
{
    pObj = rObj.pObj;
    if( pObj )
        pObj->OwnerLock( true );
}
inline SfxObjectShellLock::SfxObjectShellLock( SfxObjectShellLock && rObj ) noexcept
{
    pObj = rObj.pObj;
    rObj.pObj = nullptr;
}
inline SfxObjectShellLock::SfxObjectShellLock( SfxObjectShell * pObjP )
{
    pObj = pObjP;
    if( pObj )
        pObj->OwnerLock( true );
}
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
{
    if( pObj )
        pObj->OwnerLock( false );
}
inline SfxObjectShellLock & SfxObjectShellLock::operator=( const SfxObjectShellLock & rObj )
{
    if( rObj.pObj )
        rObj.pObj->OwnerLock( true );
    SfxObjectShell* const pRefObj = pObj;
    pObj = rObj.pObj;
    if( pRefObj )
        pRefObj->OwnerLock( false );
    return *this;
}
inline SfxObjectShellLock & SfxObjectShellLock::operator=( SfxObjectShellLock && rObj )
{
    if (pObj)
        pObj->OwnerLock( false );
    pObj = rObj.pObj;
    rObj.pObj = nullptr;
    return *this;
}
inline SfxObjectShellLock & SfxObjectShellLock::operator=( SfxObjectShell * pObjP )
{
    *this = SfxObjectShellLock( pObjP );
    return *this;
}

class SFX2_DLLPUBLIC SfxObjectShellItem final : public SfxPoolItem
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
    virtual SfxObjectShellItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
