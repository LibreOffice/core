#ifndef SwDoc_H
#define SwDoc_H


#include <IInterface.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentLineNumberAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentState.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentTimerAccess.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentOutlineNodes.hxx>
#include <IDocumentListItems.hxx>

#include <IDocumentListsAccess.hxx>
class SwList;

#include <IDocumentExternalData.hxx>
#include <svl/svstdarr.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <vcl/timer.hxx>
#include <sal/macros.h>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <swatrset.hxx>
#include <toxe.hxx>             // enums
#include <flyenum.hxx>
#include <itabenum.hxx>
#include <swdbdata.hxx>
#include <chcmprse.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <rtl/ref.hxx>
#include <svx/svdtypes.hxx>
#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <editeng/numitem.hxx>
#include "comphelper/implementationreference.hxx"
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>

#include <boost/unordered_map.hpp>
#include <stringhash.hxx>

#include <svtools/embedhlp.hxx>
#include <vector>
#include <set>
#include <map>
#include <memory>

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


namespace editeng { class SvxBorderLine; }

class SvxForbiddenCharactersTable;
class SwExtTextInput;
class DateTime;
class EditFieldInfo;
class JobSetup;
class Color;
class KeyCode;
class Outliner;
class OutputDevice;
class Point;
class SbxArray;
class SdrModel;
class SdrObject;
class SdrUndoAction;
class VirtualDevice;
class SfxPrinter;
class SvNumberFormatter;
class SvxMacro;
class SvxMacroTableDtor;
class SwAutoCompleteWord;
class SwAutoCorrExceptWord;
class SwCalc;
class SwCellFrm;
class SwCharFmt;
class SwCharFmts;
class SwConditionTxtFmtColl;
class SwCrsrShell;
class SwCursor;
class SwDBNameInfField;
class SwDocShell;
class SwDocUpdtFld;
class SwDrawFrmFmt;
class SwDrawView;
class SwEditShell;
class SwFieldType;
class SwField;
class SwTxtFld;
class SwFldTypes;
class SwFlyFrmFmt;
class SwFmt;
class SwFmtINetFmt;
class SwFmtRefMark;
class SwFrmFmt;
class SwFrmFmts;
class SwFtnIdxs;
class SwFtnInfo;
class SwEndNoteInfo;
class GraphicObject;
class SwGrfFmtColl;
class SwGrfFmtColls;
class SwLineNumberInfo;
class SwNewDBMgr;
class SwNoTxtNode;
class SwNodeIndex;
class SwNodeRange;
class SwNodes;
class SwNumRule;
class SwNumRuleTbl;
class SwPageDesc;
class SwPosFlyFrms;
class SwPagePreViewPrtData;
class SwRedline;
class SwRedlineTbl;
class SwRootFrm;
class SwRubyList;
class SwRubyListEntry;
class SwSectionFmt;
class SwSectionFmts;
class SwSectionData;
class SwSelBoxes;
class SwSpzFrmFmts;
class SwTOXBase;
class SwTOXBaseSection;
class SwTOXMark;
class SwTOXMarks;
class SwTOXType;
class SwTOXTypes;
class SwTabCols;
class SwTable;
class SwTableAutoFmt;
class SwTableBox;
class SwTableBoxFmt;
class SwTableFmt;
class SwTableLineFmt;
class SwTableNode;
class SwTextBlocks;
class SwTxtFmtColl;
class SwTxtFmtColls;
class SwURLStateChanged;
class SwUnoCrsr;
class SwUnoCrsrTbl;
class ViewShell;
class _SetGetExpFld;
class SwDrawContact;
class SwLayouter;
class SdrView;
class SdrMarkList;
class SwAuthEntry;
class SwLayoutCache;
class IStyleAccess;
struct SwCallMouseEvent;
struct SwDocStat;
struct SwHash;
struct SwSortOptions;
struct SwDefTOXBase_Impl;
class SwPrintData;
class SwPrintUIOptions;
class SdrPageView;
struct SwConversionArgs;
class SwRewriter;
class SwMsgPoolItem;
class SwChartDataProvider;
class SwChartLockController_Helper;
class IGrammarContact;
class SwPrintData;
class SwRenderData;
class SwPageFrm;
class SwViewOption;
class IDocumentUndoRedo;

namespace sw { namespace mark {
    class MarkManager;
}}
namespace sw {
    class MetaFieldManager;
    class UndoManager;
    class IShellCursorSupplier;
}

namespace com { namespace sun { namespace star {
namespace i18n {
    struct ForbiddenCharacters;    // comes from the I18N UNO interface
}
namespace uno {
    template < class > class Sequence;
}
namespace container {
    class XNameContainer; // for getXForms()/isXForms()/initXForms() methods
}
}}}

namespace sfx2 {
    class SvLinkSource;
    class IXmlIdRegistry;
    class LinkManager;
}

// PageDescriptor-interface, Array because of inlines.
typedef SwPageDesc* SwPageDescPtr;
SV_DECL_PTRARR_DEL( SwPageDescs, SwPageDescPtr, 4 )

// forward declaration
void SetAllScriptItem( SfxItemSet& rSet, const SfxPoolItem& rItem );

// global function to start grammar checking in the document
void StartGrammarChecking( SwDoc &rDoc );

class SW_DLLPUBLIC SwDoc : public IInterface ,
	 public IDocumentSettingAccess ,
	 public IDocumentDeviceAccess ,
	 public IDocumentRedlineAccess ,
	 public IDocumentLinksAdministration ,
	 public IDocumentFieldsAccess ,
	 public IDocumentContentOperations ,
	 public IDocumentStylePoolAccess ,
	 public IDocumentLineNumberAccess ,
	 public IDocumentStatistics ,
	 public IDocumentState ,
	 public IDocumentDrawModelAccess ,
	 public IDocumentLayoutAccess ,
	 public IDocumentTimerAccess ,
	 public IDocumentChartDataProviderAccess ,
	 public IDocumentListItems ,
	 public IDocumentOutlineNodes ,
	 public IDocumentListsAccess ,
	 public IDocumentExternalData {
private:
	 SwDoc(const SwDoc & arg1);
         sal_Int32 mReferenceCount;
public:
	 SwDoc();
	 virtual ~SwDoc();

	 static SwAutoCompleteWord *pACmpltWords;

         virtual bool InsertString(const SwPaM &rRg, const String&,
                 const enum InsertFlags nInsertMode = INS_EMPTYEXPAND );
	virtual bool IsInDtor() const;
	virtual SwNodes & GetNodes();
	virtual SwNodes const & GetNodes() const;
	virtual sal_Int32 acquire();
	virtual sal_Int32 release();
	virtual sal_Int32 getReferenceCount() const;
	virtual bool get(DocumentSettingId id) const;
	virtual void set(DocumentSettingId id, bool value);
	virtual const com::sun::star::i18n::ForbiddenCharacters * getForbiddenCharacters(sal_uInt16 nLang, bool bLocaleData) const;
	virtual void setForbiddenCharacters(sal_uInt16 nLang, const com::sun::star::i18n::ForbiddenCharacters & rForbiddenCharacters);
	virtual rtl::Reference<SvxForbiddenCharactersTable> & getForbiddenCharacterTable();
	virtual const rtl::Reference<SvxForbiddenCharactersTable> & getForbiddenCharacterTable() const;
	virtual sal_uInt16 getLinkUpdateMode(bool bGlobalSettings) const;
	virtual void setLinkUpdateMode(sal_uInt16 nMode);
	virtual SwFldUpdateFlags getFieldUpdateFlags(bool bGlobalSettings) const;
	virtual void setFieldUpdateFlags(SwFldUpdateFlags eMode);
	virtual SwCharCompressType getCharacterCompressionType() const;
	virtual void setCharacterCompressionType(SwCharCompressType nType);
	virtual sal_uInt32 getRsid() const;
	virtual void setRsid(sal_uInt32 nVal);
	virtual sal_uInt32 getRsidRoot() const;
	virtual void setRsidRoot(sal_uInt32 nVal);
	virtual SfxPrinter * getPrinter(bool bCreate) const;
	virtual void setPrinter(SfxPrinter * pP, bool bDeleteOld, bool bCallPrtDataChanged);
	virtual VirtualDevice * getVirtualDevice(bool bCreate) const;
	virtual void setVirtualDevice(VirtualDevice * pVd, bool bDeleteOld, bool bCallVirDevDataChanged);
	virtual OutputDevice * getReferenceDevice(bool bCreate) const;
	virtual void setReferenceDeviceType(bool bNewVirtual, bool bNewHiRes);
	virtual const JobSetup * getJobsetup() const;
	virtual void setJobsetup(const JobSetup & rJobSetup);
	virtual const SwPrintData & getPrintData() const;
	virtual void setPrintData(const SwPrintData & rPrtData);
	virtual IDocumentMarkAccess * getIDocumentMarkAccess();
	virtual const IDocumentMarkAccess * getIDocumentMarkAccess() const;
	virtual RedlineMode_t GetRedlineMode() const;
	virtual void SetRedlineMode_intern(RedlineMode_t eMode);
	virtual void SetRedlineMode(RedlineMode_t eMode);
	virtual bool IsRedlineOn() const;
	virtual bool IsIgnoreRedline() const;
	virtual bool IsInRedlines(const SwNode & rNode) const;
	virtual const SwRedlineTbl & GetRedlineTbl() const;
	virtual bool AppendRedline(SwRedline * pPtr, bool bCallDelete);
	virtual bool SplitRedline(const SwPaM & rPam);
	virtual bool DeleteRedline(const SwPaM & rPam, bool bSaveInUndo, sal_uInt16 nDelType);
	virtual bool DeleteRedline(const SwStartNode & rSection, bool bSaveInUndo, sal_uInt16 nDelType);
	virtual sal_uInt16 GetRedlinePos(const SwNode & rNode, sal_uInt16 nType) const;
	virtual void CompressRedlines();
	virtual const SwRedline * GetRedline(const SwPosition & rPos, sal_uInt16 * pFndPos) const;
	virtual bool IsRedlineMove() const;
	virtual void SetRedlineMove(bool bFlag);
	virtual bool AcceptRedline(sal_uInt16 nPos, bool bCallDelete);
	virtual bool AcceptRedline(const SwPaM & rPam, bool bCallDelete);
	virtual bool RejectRedline(sal_uInt16 nPos, bool bCallDelete);
	virtual bool RejectRedline(const SwPaM & rPam, bool bCallDelete);
	virtual const SwRedline * SelNextRedline(SwPaM & rPam) const;
	virtual const SwRedline * SelPrevRedline(SwPaM & rPam) const;
	virtual void UpdateRedlineAttr();
	virtual sal_uInt16 GetRedlineAuthor();
	virtual sal_uInt16 InsertRedlineAuthor(const String & rAuthor);
	virtual bool SetRedlineComment(const SwPaM & rPam, const String & rComment);
	virtual const ::com::sun::star::uno::Sequence<sal_Int8> & GetRedlinePassword() const;
	virtual void SetRedlinePassword(const ::com::sun::star::uno::Sequence<sal_Int8> & rNewPassword);
	virtual IDocumentUndoRedo & GetIDocumentUndoRedo();
	virtual IDocumentUndoRedo const & GetIDocumentUndoRedo() const;
	virtual bool IsVisibleLinks() const;
	virtual void SetVisibleLinks(bool bFlag);
	virtual sfx2::LinkManager & GetLinkManager();
	virtual const sfx2::LinkManager & GetLinkManager() const;
	virtual void UpdateLinks(sal_Bool bUI);
	virtual bool GetData(const rtl::OUString & rItem, const String & rMimeType, ::com::sun::star::uno::Any & rValue) const;
	virtual bool SetData(const rtl::OUString & rItem, const String & rMimeType, const ::com::sun::star::uno::Any & rValue);
	virtual ::sfx2::SvLinkSource * CreateLinkSource(const rtl::OUString & rItem);
	virtual bool EmbedAllLinks();
	virtual void SetLinksUpdated(const bool bNewLinksUpdated);
	virtual bool LinksUpdated() const;
	virtual const SwFldTypes * GetFldTypes() const;
	virtual SwFieldType * InsertFldType(const SwFieldType & arg1);
	virtual SwFieldType * GetSysFldType(const sal_uInt16 eWhich) const;
	virtual SwFieldType * GetFldType(sal_uInt16 nResId, const String & rName, bool bDbFieldMatching) const;
	virtual void RemoveFldType(sal_uInt16 nFld);
	virtual void UpdateFlds(SfxPoolItem * pNewHt, bool bCloseDB);
	virtual void InsDeletedFldType(SwFieldType & arg1);
	virtual bool PutValueToField(const SwPosition & rPos, const com::sun::star::uno::Any & rVal, sal_uInt16 nWhich);
	virtual bool UpdateFld(SwTxtFld * rDstFmtFld, SwField & rSrcFld, SwMsgPoolItem * pMsgHnt, bool bUpdateTblFlds);
	virtual void UpdateRefFlds(SfxPoolItem * pHt);
	virtual void UpdateTblFlds(SfxPoolItem * pHt);
	virtual void UpdateExpFlds(SwTxtFld * pFld, bool bUpdateRefFlds);
	virtual void UpdateUsrFlds();
	virtual void UpdatePageFlds(SfxPoolItem * arg1);
	virtual void LockExpFlds();
	virtual void UnlockExpFlds();
	virtual bool IsExpFldsLocked() const;
	virtual SwDocUpdtFld & GetUpdtFlds() const;
	virtual bool SetFieldsDirty(bool b, const SwNode * pChk, sal_uLong nLen);
	virtual void SetFixFields(bool bOnlyTimeDate, const DateTime * pNewDateTime);
	virtual void FldsToCalc(SwCalc & rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt);
	virtual void FldsToCalc(SwCalc & rCalc, const _SetGetExpFld & rToThisFld);
	virtual void FldsToExpand(SwHash * * & ppTbl, sal_uInt16 & rTblSize, const _SetGetExpFld & rToThisFld);
	virtual bool IsNewFldLst() const;
	virtual void SetNewFldLst(bool bFlag);
	virtual void InsDelFldInFldLst(bool bIns, const SwTxtFld & rFld);
	static SwField * GetField(const SwPosition & rPos);
	static SwTxtFld * GetTxtFld(const SwPosition & rPos);
	virtual bool CopyRange(SwPaM & arg1, SwPosition & arg2, const bool bCopyAll) const;
	virtual void DeleteSection(SwNode * pNode);
	virtual bool DeleteRange(SwPaM & arg1);
	virtual bool DelFullPara(SwPaM & arg1);
	virtual bool DeleteAndJoin(SwPaM & arg1, const bool bForceJoinNext= false);
	virtual bool MoveRange(SwPaM & arg1, SwPosition & arg2, SwMoveFlags arg3);
	virtual bool MoveNodeRange(SwNodeRange & arg1, SwNodeIndex & arg2, SwMoveFlags arg3);
	virtual bool MoveAndJoin(SwPaM & arg1, SwPosition & arg2, SwMoveFlags arg3);
	virtual bool Overwrite(const SwPaM & rRg, const String & rStr);
	virtual bool UpdateRsid(SwTxtNode * pTxtNode, xub_StrLen nStt, xub_StrLen nEnd);
	virtual bool UpdateParRsid(SwTxtNode * pTxtNode, sal_uInt32 nVal= 0);
	virtual bool UpdateRsid(const SwPaM & rRg, xub_StrLen nLen);
	virtual SwFlyFrmFmt * Insert(const SwPaM & rRg, const String & rGrfName, const String & rFltName, const Graphic * pGraphic, const SfxItemSet * pFlyAttrSet, const SfxItemSet * pGrfAttrSet, SwFrmFmt * arg1);
	virtual SwFlyFrmFmt * Insert(const SwPaM & rRg, const GraphicObject & rGrfObj, const SfxItemSet * pFlyAttrSet, const SfxItemSet * pGrfAttrSet, SwFrmFmt * arg1);
	virtual SwDrawFrmFmt * Insert(const SwPaM & rRg, SdrObject & rDrawObj, const SfxItemSet * pFlyAttrSet, SwFrmFmt * arg1);
	virtual SwFlyFrmFmt * Insert(const SwPaM & rRg, const svt::EmbeddedObjectRef & xObj, const SfxItemSet * pFlyAttrSet, const SfxItemSet * pGrfAttrSet, SwFrmFmt * arg1);
	virtual bool InsertPoolItem(const SwPaM & rRg, const SfxPoolItem & arg1, const SetAttrMode nFlags);
	virtual bool InsertItemSet(const SwPaM & rRg, const SfxItemSet & arg1, const SetAttrMode nFlags);
	virtual void ReRead(SwPaM & arg1, const String & rGrfName, const String & rFltName, const Graphic * pGraphic, const GraphicObject * pGrfObj);
	virtual void TransliterateText(const SwPaM & rPaM, utl::TransliterationWrapper & arg1);
	virtual SwFlyFrmFmt * InsertOLE(const SwPaM & rRg, const String & rObjName, sal_Int64 nAspect, const SfxItemSet * pFlyAttrSet, const SfxItemSet * pGrfAttrSet, SwFrmFmt * arg1);
	virtual bool SplitNode(const SwPosition & rPos, bool bChkTableStart);
	virtual bool AppendTxtNode(SwPosition & rPos);
	virtual void SetModified(SwPaM & rPaM);
	virtual bool ReplaceRange(SwPaM & rPam, const String & rNewStr, const bool bRegExReplace);
	virtual void RemoveLeadingWhiteSpace(const SwPosition & rPos);
	virtual SwTxtFmtColl * GetTxtCollFromPool(sal_uInt16 nId, bool bRegardLanguage= true);
	virtual SwFmt * GetFmtFromPool(sal_uInt16 nId);
	virtual SwFrmFmt * GetFrmFmtFromPool(sal_uInt16 nId);
	virtual SwCharFmt * GetCharFmtFromPool(sal_uInt16 nId);
	virtual SwPageDesc * GetPageDescFromPool(sal_uInt16 nId, bool bRegardLanguage= true);
	virtual SwNumRule * GetNumRuleFromPool(sal_uInt16 nId);
	virtual bool IsPoolTxtCollUsed(sal_uInt16 nId) const;
	virtual bool IsPoolFmtUsed(sal_uInt16 nId) const;
	virtual bool IsPoolPageDescUsed(sal_uInt16 nId) const;
	virtual const SwLineNumberInfo & GetLineNumberInfo() const;
	virtual void SetLineNumberInfo(const SwLineNumberInfo & rInfo);
	virtual void DocInfoChgd();
	virtual const SwDocStat & GetDocStat() const;
	virtual const SwDocStat & GetUpdatedDocStat();
	virtual void SetDocStat(const SwDocStat & rStat);
	virtual void UpdateDocStat();
	virtual void SetModified();
	virtual void ResetModified();
	virtual bool IsModified() const;
	virtual bool IsLoaded() const;
	virtual bool IsUpdateExpFld() const;
	virtual bool IsNewDoc() const;
	virtual bool IsPageNums() const;
	virtual void SetPageNums(bool b);
	virtual void SetNewDoc(bool b);
	virtual void SetUpdateExpFldStat(bool b);
	virtual void SetLoaded(bool b);
	virtual const SdrModel * GetDrawModel() const;
	virtual SdrModel * GetDrawModel();
	virtual SdrLayerID GetHeavenId() const;
	virtual SdrLayerID GetHellId() const;
	virtual SdrLayerID GetControlsId() const;
	virtual SdrLayerID GetInvisibleHeavenId() const;
	virtual SdrLayerID GetInvisibleHellId() const;
	virtual SdrLayerID GetInvisibleControlsId() const;
	virtual void NotifyInvisibleLayers(SdrPageView & _rSdrPageView);
	virtual bool IsVisibleLayerId(const SdrLayerID & _nLayerId) const;
	virtual SdrLayerID GetVisibleLayerIdByInvisibleOne(const SdrLayerID & _nInvisibleLayerId);
	virtual SdrLayerID GetInvisibleLayerIdByVisibleOne(const SdrLayerID & _nVisibleLayerId);
	virtual SdrModel * _MakeDrawModel();
	virtual SdrModel * GetOrCreateDrawModel();
	virtual void SetCurrentViewShell(ViewShell * pNew);
	virtual SwLayouter * GetLayouter();
	virtual const SwLayouter * GetLayouter() const;
	virtual void SetLayouter(SwLayouter * pNew);
	virtual SwFrmFmt * MakeLayoutFmt(RndStdIds eRequest, const SfxItemSet * pSet);
	virtual void DelLayoutFmt(SwFrmFmt * pFmt);
	virtual SwFrmFmt * CopyLayoutFmt(const SwFrmFmt & rSrc, const SwFmtAnchor & rNewAnchor, bool bSetTxtFlyAtt, bool bMakeFrms);
	virtual const ViewShell * GetCurrentViewShell() const;
	virtual ViewShell * GetCurrentViewShell();
	virtual const SwRootFrm * GetCurrentLayout() const;
	virtual SwRootFrm * GetCurrentLayout();
	virtual bool HasLayout() const;
	virtual void ClearSwLayouterEntries();
	virtual void StartIdling();
	virtual void StopIdling();
	virtual void BlockIdling();
	virtual void UnblockIdling();
	virtual SwChartDataProvider * GetChartDataProvider(bool bCreate= false) const;
	virtual void CreateChartInternalDataProviders(const SwTable * pTable);
	virtual SwChartLockController_Helper & GetChartControllerHelper();
	virtual void addListItem(const SwNodeNum & rNodeNum);
	virtual void removeListItem(const SwNodeNum & rNodeNum);
	virtual String getListItemText(const SwNodeNum & rNodeNum, const bool bWithNumber= true, const bool bWithSpacesForLevel= false) const;
	virtual void getListItems(IDocumentListItems::tSortedNodeNumList & orNodeNumList) const;
	virtual void getNumItems(IDocumentListItems::tSortedNodeNumList & orNodeNumList) const;
	virtual sal_Int32 getOutlineNodesCount() const;
	virtual int getOutlineLevel(const sal_Int32 nIdx) const;
	virtual String getOutlineText(const sal_Int32 nIdx, const bool bWithNumber, const bool bWithSpacesForLevel) const;
	virtual SwTxtNode * getOutlineNode(const sal_Int32 nIdx) const;
	virtual void getOutlineNodes(IDocumentOutlineNodes::tSortedOutlineNodeList & orOutlineNodeList) const;
	virtual SwList * createList(String sListId, const String sDefaultListStyleName);
	virtual void deleteList(const String sListId);
	virtual SwList * getListByName(const String sListId) const;
	virtual SwList * createListForListStyle(const String sListStyleName);
	virtual SwList * getListForListStyle(const String sListStyleName) const;
	virtual void deleteListForListStyle(const String sListStyleName);
	virtual void trackChangeOfListStyleName(const String sListStyleName, const String sNewListStyleName);
	virtual void setExternalData(::sw::tExternalDataType eType, ::sw::tExternalDataPointer pPayload);
	virtual ::sw::tExternalDataPointer getExternalData(::sw::tExternalDataType eType);
	virtual bool IsOLEPrtNotifyPending() const;
	virtual void SetOLEPrtNotifyPending(bool bSet= true);
	virtual void PrtOLENotify(sal_Bool bAll);
	virtual bool InXMLExport() const;
	virtual void SetXMLExport(bool bFlag);
	virtual void GetAllFlyFmts(SwPosFlyFrms & rPosFlyFmts, const SwPaM * arg1= 0, sal_Bool bDrawAlso= sal_False, sal_Bool bAsCharAlso= sal_False) const;
	virtual SwFlyFrmFmt * MakeFlyFrmFmt(const String & rFmtName, SwFrmFmt * pDerivedFrom);
	virtual SwDrawFrmFmt * MakeDrawFrmFmt(const String & rFmtName, SwFrmFmt * pDerivedFrom);
	virtual SwFlyFrmFmt * MakeFlySection(RndStdIds eAnchorType, const SwPosition * pAnchorPos, const SfxItemSet * pSet= 0, SwFrmFmt * pParent= 0, sal_Bool bCalledFromShell= sal_False);
	virtual SwFlyFrmFmt * MakeFlyAndMove(const SwPaM & rPam, const SfxItemSet & rSet, const SwSelBoxes * pSelBoxes= 0, SwFrmFmt * pParent= 0);
	virtual void CopyWithFlyInFly(const SwNodeRange & rRg, const xub_StrLen nEndContentIndex, const SwNodeIndex & rInsPos, sal_Bool bMakeNewFrms= sal_True, sal_Bool bDelRedlines= sal_True, sal_Bool bCopyFlyAtFly= sal_False) const;
	virtual sal_Bool SetFlyFrmAttr(SwFrmFmt & rFlyFmt, SfxItemSet & rSet);
	virtual sal_Bool SetFrmFmtToFly(SwFrmFmt & rFlyFmt, SwFrmFmt & rNewFmt, SfxItemSet * pSet= 0, sal_Bool bKeepOrient= sal_False);
	virtual void SetFlyFrmTitle(SwFlyFrmFmt & rFlyFrmFmt, const String & sNewTitle);
	virtual void SetFlyFrmDescription(SwFlyFrmFmt & rFlyFrmFmt, const String & sNewDescription);
	virtual const SwFtnInfo & GetFtnInfo() const;
	virtual void SetFtnInfo(const SwFtnInfo & rInfo);
	virtual const SwEndNoteInfo & GetEndNoteInfo() const;
	virtual void SetEndNoteInfo(const SwEndNoteInfo & rInfo);
	virtual SwFtnIdxs & GetFtnIdxs();
	virtual const SwFtnIdxs & GetFtnIdxs() const;
	virtual bool SetCurFtn(const SwPaM & rPam, const String & rNumStr, sal_uInt16 nNumber, bool bIsEndNote);
	virtual ::com::sun::star::uno::Any Spell(SwPaM & arg1, ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> & arg2, sal_uInt16 * pPageCnt, sal_uInt16 * pPageSt, bool bGrammarCheck, SwConversionArgs * pConvArgs= 0) const;
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord> Hyphenate(SwPaM * pPam, const Point & rCrsrPos, sal_uInt16 * pPageCnt, sal_uInt16 * pPageSt);
	virtual void CountWords(const SwPaM & rPaM, SwDocStat & rStat) const;
	virtual void SetGlossDoc(bool bGlssDc= true);
	virtual bool IsInsOnlyTextGlossary() const;
	virtual void Summary(SwDoc * pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, sal_Bool bImpress);
	virtual void ChangeAuthorityData(const SwAuthEntry * pNewData);
	virtual bool IsInCallModified() const;
	virtual sal_Bool IsInHeaderFooter(const SwNodeIndex & rIdx) const;
	virtual short GetTextDirection(const SwPosition & rPos, const Point * pPt= 0) const;
	virtual sal_Bool IsInVerticalText(const SwPosition & rPos, const Point * pPt= 0) const;
	virtual void SetNewDBMgr(SwNewDBMgr * pNewMgr);
	virtual SwNewDBMgr * GetNewDBMgr() const;
	virtual void ChangeDBFields(const std::vector<String> & rOldNames, const String & rNewName);
	virtual void SetInitDBFields(sal_Bool b);
	virtual void GetAllUsedDB(std::vector<String> & rDBNameList, const std::vector<String> * pAllDBNames= 0);
	virtual void ChgDBData(const SwDBData & rNewData);
	virtual SwDBData GetDBData();
	virtual const SwDBData & GetDBDesc();
	virtual const SwDBData & _GetDBDesc() const;
	virtual String GetUniqueGrfName() const;
	virtual String GetUniqueOLEName() const;
	virtual String GetUniqueFrameName() const;
	virtual std::set<SwRootFrm*> GetAllLayouts();
	virtual void SetFlyName(SwFlyFrmFmt & rFmt, const String & rName);
	virtual const SwFlyFrmFmt * FindFlyByName(const String & rName, sal_Int8 nNdTyp= 0) const;
	virtual void GetGrfNms(const SwFlyFrmFmt & rFmt, String * pGrfName, String * pFltName) const;
	virtual void SetAllUniqueFlyNames();
	virtual void ResetAttrs(const SwPaM & rRg, sal_Bool bTxtAttr= sal_True, const std::set<sal_uInt16> & rAttrs= std::set<sal_uInt16>(), const bool bSendDataChangedEvents= true);
	virtual void RstTxtAttrs(const SwPaM & rRg, sal_Bool bInclRefToxMark= sal_False);
	virtual void SetAttr(const SfxPoolItem & arg1, SwFmt & arg2);
	virtual void SetAttr(const SfxItemSet & arg1, SwFmt & arg2);
	virtual void ResetAttrAtFormat(const sal_uInt16 nWhichId, SwFmt & rChangedFormat);
	virtual void SetDefault(const SfxPoolItem & arg1);
	virtual void SetDefault(const SfxItemSet & arg1);
	virtual const SfxPoolItem & GetDefault(sal_uInt16 nFmtHint) const;
	virtual sal_Bool DontExpandFmt(const SwPosition & rPos, sal_Bool bFlag= sal_True);
	virtual const SwFrmFmts * GetFrmFmts() const;
	virtual SwFrmFmts * GetFrmFmts();
	virtual const SwCharFmts * GetCharFmts() const;
	virtual const SwSpzFrmFmts * GetSpzFrmFmts() const;
	virtual SwSpzFrmFmts * GetSpzFrmFmts();
	virtual const SwFrmFmt * GetDfltFrmFmt() const;
	virtual SwFrmFmt * GetDfltFrmFmt();
	virtual const SwFrmFmt * GetEmptyPageFmt() const;
	virtual SwFrmFmt * GetEmptyPageFmt();
	virtual const SwFrmFmt * GetColumnContFmt() const;
	virtual SwFrmFmt * GetColumnContFmt();
	virtual const SwCharFmt * GetDfltCharFmt() const;
	virtual SwCharFmt * GetDfltCharFmt();
	virtual IStyleAccess & GetIStyleAccess();
	virtual void RemoveAllFmtLanguageDependencies();
	virtual SwFrmFmt * MakeFrmFmt(const String & rFmtName, SwFrmFmt * pDerivedFrom, sal_Bool bBroadcast= sal_False, sal_Bool bAuto= sal_True);
	virtual void DelFrmFmt(SwFrmFmt * pFmt, sal_Bool bBroadcast= sal_False);
	virtual SwFrmFmt * FindFrmFmtByName(const String & rName) const;
	virtual SwCharFmt * MakeCharFmt(const String & rFmtName, SwCharFmt * pDerivedFrom, sal_Bool bBroadcast= sal_False, sal_Bool bAuto= sal_True);
	virtual void DelCharFmt(sal_uInt16 nFmt, sal_Bool bBroadcast= sal_False);
	virtual void DelCharFmt(SwCharFmt * pFmt, sal_Bool bBroadcast= sal_False);
	virtual SwCharFmt * FindCharFmtByName(const String & rName) const;
	virtual const SwTxtFmtColl * GetDfltTxtFmtColl() const;
	virtual const SwTxtFmtColls * GetTxtFmtColls() const;
	virtual SwTxtFmtColl * MakeTxtFmtColl(const String & rFmtName, SwTxtFmtColl * pDerivedFrom, sal_Bool bBroadcast= sal_False, sal_Bool bAuto= sal_True);
	virtual SwConditionTxtFmtColl * MakeCondTxtFmtColl(const String & rFmtName, SwTxtFmtColl * pDerivedFrom, sal_Bool bBroadcast= sal_False);
	virtual void DelTxtFmtColl(sal_uInt16 nFmt, sal_Bool bBroadcast= sal_False);
	virtual void DelTxtFmtColl(SwTxtFmtColl * pColl, sal_Bool bBroadcast= sal_False);
	virtual sal_Bool SetTxtFmtColl(const SwPaM & rRg, SwTxtFmtColl * pFmt, bool bReset= true, bool bResetListAttrs= false);
	virtual SwTxtFmtColl * FindTxtFmtCollByName(const String & rName) const;
	virtual void ChkCondColls();
	virtual const SwGrfFmtColl * GetDfltGrfFmtColl() const;
	virtual const SwGrfFmtColls * GetGrfFmtColls() const;
	virtual SwGrfFmtColl * MakeGrfFmtColl(const String & rFmtName, SwGrfFmtColl * pDerivedFrom);
	virtual SwGrfFmtColl * FindGrfFmtCollByName(const String & rName) const;
	virtual const SwFrmFmts * GetTblFrmFmts() const;
	virtual SwFrmFmts * GetTblFrmFmts();
	virtual sal_uInt16 GetTblFrmFmtCount(sal_Bool bUsed) const;
	virtual SwFrmFmt & GetTblFrmFmt(sal_uInt16 nFmt, sal_Bool bUsed) const;
	virtual SwTableFmt * MakeTblFrmFmt(const String & rFmtName, SwFrmFmt * pDerivedFrom);
	virtual void DelTblFrmFmt(SwTableFmt * pFmt);
	virtual SwTableFmt * FindTblFmtByName(const String & rName, sal_Bool bAll= sal_False) const;
	virtual sal_uInt16 GetFlyCount(FlyCntType eType= FLYCNTTYPE_ALL) const;
	virtual SwFrmFmt * GetFlyNum(sal_uInt16 nIdx, FlyCntType eType= FLYCNTTYPE_ALL);
	virtual SwFrmFmt * CopyFrmFmt(const SwFrmFmt & arg1);
	virtual SwCharFmt * CopyCharFmt(const SwCharFmt & arg1);
	virtual SwTxtFmtColl * CopyTxtColl(const SwTxtFmtColl & rColl);
	virtual SwGrfFmtColl * CopyGrfColl(const SwGrfFmtColl & rColl);
	virtual void ReplaceStyles(const SwDoc & rSource);
	virtual void ReplaceDefaults(const SwDoc & rSource);
	virtual void ReplaceCompatabilityOptions(const SwDoc & rSource);
	virtual sal_Bool IsUsed(const SwModify & arg1) const;
	virtual sal_Bool IsUsed(const SwNumRule & arg1) const;
	virtual sal_uInt16 SetDocPattern(const String & rPatternName);
	virtual const String * GetDocPattern(sal_uInt16 nPos) const;
	virtual void GCFieldTypes();
	virtual void SetGlossaryDoc(SwDoc * pDoc);
	virtual sal_Bool InsertGlossary(SwTextBlocks & rBlock, const String & rEntry, SwPaM & rPaM, SwCrsrShell * pShell= 0);
	virtual void CalculatePagesForPrinting(const SwRootFrm & rLayout, SwRenderData & rData, const SwPrintUIOptions & rOptions, bool bIsPDFExport, sal_Int32 nDocPageCount);
	virtual void UpdatePagesForPrintingWithPostItData(SwRenderData & rData, const SwPrintUIOptions & rOptions, bool bIsPDFExport, sal_Int32 nDocPageCount);
	virtual void CalculatePagePairsForProspectPrinting(const SwRootFrm & rLayout, SwRenderData & rData, const SwPrintUIOptions & rOptions, sal_Int32 nDocPageCount);
	virtual sal_uInt16 GetPageDescCnt() const;
	virtual const SwPageDesc & GetPageDesc(const sal_uInt16 i) const;
	virtual SwPageDesc * FindPageDescByName(const String & rName, sal_uInt16 * pPos= 0) const;
	virtual void CopyPageDesc(const SwPageDesc & rSrcDesc, SwPageDesc & rDstDesc, sal_Bool bCopyPoolIds= sal_True);
	virtual void CopyHeader(const SwFrmFmt & rSrcFmt, SwFrmFmt & rDestFmt);
	virtual void CopyFooter(const SwFrmFmt & rSrcFmt, SwFrmFmt & rDestFmt);
	virtual SwPageDesc * GetPageDesc(const String & rName);
	virtual SwPageDesc & _GetPageDesc(sal_uInt16 i) const;
	virtual void ChgPageDesc(const String & rName, const SwPageDesc & arg1);
	virtual void ChgPageDesc(sal_uInt16 i, const SwPageDesc & arg1);
	virtual sal_Bool FindPageDesc(const String & rName, sal_uInt16 * pFound);
	virtual void DelPageDesc(const String & rName, sal_Bool bBroadcast= sal_False);
	virtual void DelPageDesc(sal_uInt16 i, sal_Bool bBroadcast= sal_False);
	virtual void PreDelPageDesc(SwPageDesc * pDel);
	virtual sal_uInt16 MakePageDesc(const String & rName, const SwPageDesc * pCpy= 0, sal_Bool bRegardLanguage= sal_True, sal_Bool bBroadcast= sal_False);
	virtual void BroadcastStyleOperation(String rName, SfxStyleFamily eFamily, sal_uInt16 nOp);
	virtual void CheckDefaultPageFmt();
	virtual sal_uInt16 GetCurTOXMark(const SwPosition & rPos, SwTOXMarks & arg1) const;
	virtual void DeleteTOXMark(const SwTOXMark * pTOXMark);
	virtual const SwTOXMark & GotoTOXMark(const SwTOXMark & rCurTOXMark, SwTOXSearch eDir, sal_Bool bInReadOnly);
	virtual const SwTOXBaseSection * InsertTableOf(const SwPosition & rPos, const SwTOXBase & rTOX, const SfxItemSet * pSet= 0, sal_Bool bExpand= sal_False);
	virtual const SwTOXBaseSection * InsertTableOf(sal_uLong nSttNd, sal_uLong nEndNd, const SwTOXBase & rTOX, const SfxItemSet * pSet= 0);
	virtual const SwTOXBase * GetCurTOX(const SwPosition & rPos) const;
	virtual const SwAttrSet & GetTOXBaseAttrSet(const SwTOXBase & rTOX) const;
	virtual sal_Bool DeleteTOX(const SwTOXBase & rTOXBase, sal_Bool bDelNodes= sal_False);
	virtual String GetUniqueTOXBaseName(const SwTOXType & rType, const String * pChkStr= 0) const;
	virtual sal_Bool SetTOXBaseName(const SwTOXBase & rTOXBase, const String & rName);
	virtual void SetTOXBaseProtection(const SwTOXBase & rTOXBase, sal_Bool bProtect);
	virtual void SetUpdateTOX(bool bFlag= true);
	virtual bool IsUpdateTOX() const;
	virtual const String & GetTOIAutoMarkURL() const;
	virtual void SetTOIAutoMarkURL(const String & rSet);
	virtual void ApplyAutoMark();
	virtual bool IsInReading() const;
	virtual void SetInReading(bool bNew);
	virtual bool IsClipBoard() const;
	virtual void SetClipBoard(bool bNew);
	virtual bool IsColumnSelection() const;
	virtual void SetColumnSelection(bool bNew);
	virtual bool IsInXMLImport() const;
	virtual void SetInXMLImport(bool bNew);
	virtual sal_uInt16 GetTOXTypeCount(TOXTypes eTyp) const;
	virtual const SwTOXType * GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const;
	virtual sal_Bool DeleteTOXType(TOXTypes eTyp, sal_uInt16 nId);
	virtual const SwTOXType * InsertTOXType(const SwTOXType & rTyp);
	virtual const SwTOXTypes & GetTOXTypes() const;
	virtual const SwTOXBase * GetDefaultTOXBase(TOXTypes eTyp, sal_Bool bCreate= sal_False);
	virtual void SetDefaultTOXBase(const SwTOXBase & rBase);
	virtual sal_uInt16 GetTOIKeys(SwTOIKeyType eTyp, std::vector<String> & rArr) const;
	virtual sal_Bool SortTbl(const SwSelBoxes & rBoxes, const SwSortOptions & arg1);
	virtual sal_Bool SortText(const SwPaM & arg1, const SwSortOptions & arg2);
	virtual void CorrAbs(const SwNodeIndex & rOldNode, const SwPosition & rNewPos, const xub_StrLen nOffset= 0, sal_Bool bMoveCrsr= sal_False);
	virtual void CorrAbs(const SwNodeIndex & rStartNode, const SwNodeIndex & rEndNode, const SwPosition & rNewPos, sal_Bool bMoveCrsr= sal_False);
	virtual void CorrAbs(const SwPaM & rRange, const SwPosition & rNewPos, sal_Bool bMoveCrsr= sal_False);
	virtual void CorrRel(const SwNodeIndex & rOldNode, const SwPosition & rNewPos, const xub_StrLen nOffset= 0, sal_Bool bMoveCrsr= sal_False);
	virtual SwNumRule * GetOutlineNumRule() const;
	virtual void SetOutlineNumRule(const SwNumRule & rRule);
	virtual void PropagateOutlineRule();
	virtual sal_Bool OutlineUpDown(const SwPaM & rPam, short nOffset= 1);
	virtual sal_Bool MoveOutlinePara(const SwPaM & rPam, short nOffset= 1);
	virtual sal_Bool GotoOutline(SwPosition & rPos, const String & rName) const;
	virtual void SetNumRule(const SwPaM & arg1, const SwNumRule & arg2, const bool bCreateNewList, const String sContinuedListId= String(), sal_Bool bSetItem= sal_True, const bool bResetIndentAttrs= false);
	virtual void SetCounted(const SwPaM & arg1, bool bCounted);
	virtual void MakeUniqueNumRules(const SwPaM & rPaM);
	virtual void SetNumRuleStart(const SwPosition & rPos, sal_Bool bFlag= sal_True);
	virtual void SetNodeNumStart(const SwPosition & rPos, sal_uInt16 nStt);
	virtual SwNumRule * GetCurrNumRule(const SwPosition & rPos) const;
	virtual const SwNumRuleTbl & GetNumRuleTbl() const;
	virtual void AddNumRule(SwNumRule * pRule);
	virtual sal_uInt16 MakeNumRule(const String & rName, const SwNumRule * pCpy= 0, sal_Bool bBroadcast= sal_False, const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode= SvxNumberFormat::LABEL_WIDTH_AND_POSITION);
	virtual sal_uInt16 FindNumRule(const String & rName) const;
	virtual SwNumRule * FindNumRulePtr(const String & rName) const;
	virtual sal_Bool RenameNumRule(const String & aOldName, const String & aNewName, sal_Bool bBroadcast= sal_False);
	virtual sal_Bool DelNumRule(const String & rName, sal_Bool bBroadCast= sal_False);
	virtual String GetUniqueNumRuleName(const String * pChkStr= 0, sal_Bool bAutoNum= sal_True) const;
	virtual void UpdateNumRule();
	virtual void ChgNumRuleFmts(const SwNumRule & rRule, const String * pOldName= 0);
	virtual sal_Bool ReplaceNumRule(const SwPosition & rPos, const String & rOldRule, const String & rNewRule);
	virtual sal_Bool GotoNextNum(SwPosition & arg1, sal_Bool bOverUpper= sal_True, sal_uInt8 * pUpper= 0, sal_uInt8 * pLower= 0);
	virtual sal_Bool GotoPrevNum(SwPosition & arg1, sal_Bool bOverUpper= sal_True, sal_uInt8 * pUpper= 0, sal_uInt8 * pLower= 0);
	virtual const SwNumRule * SearchNumRule(const SwPosition & rPos, const bool bForward, const bool bNum, const bool bOutline, int nNonEmptyAllowed, String & sListId, const bool bInvestigateStartNode= false);
	virtual sal_Bool NoNum(const SwPaM & arg1);
	virtual void DelNumRules(const SwPaM & arg1);
	virtual void InvalidateNumRules();
	virtual sal_Bool NumUpDown(const SwPaM & arg1, sal_Bool bDown= sal_True);
	virtual sal_Bool MoveParagraph(const SwPaM & arg1, long nOffset= 1, sal_Bool bIsOutlMv= sal_False);
	virtual sal_Bool NumOrNoNum(const SwNodeIndex & rIdx, sal_Bool bDel= sal_False);
	virtual void StopNumRuleAnimations(OutputDevice * arg1);
	virtual const SwTable * InsertTable(const SwInsertTableOptions & rInsTblOpts, const SwPosition & rPos, sal_uInt16 nRows, sal_uInt16 nCols, short eAdjust, const SwTableAutoFmt * pTAFmt= 0, const std::vector<sal_uInt16> * pColArr= 0, sal_Bool bCalledFromShell= sal_False, sal_Bool bNewModel= sal_True);
	virtual SwTableNode * IsIdxInTbl(const SwNodeIndex & rIdx);
	virtual const SwTableNode * IsIdxInTbl(const SwNodeIndex & rIdx) const;
	virtual const SwTable * TextToTable(const SwInsertTableOptions & rInsTblOpts, const SwPaM & rRange, sal_Unicode cCh, short eAdjust, const SwTableAutoFmt * arg1= 0);
	virtual const SwTable * TextToTable(const std::vector<std::vector<SwNodeRange>> & rTableNodes);
	virtual sal_Bool TableToText(const SwTableNode * pTblNd, sal_Unicode cCh);
	virtual sal_Bool InsertCol(const SwCursor & rCursor, sal_uInt16 nCnt= 1, sal_Bool bBehind= sal_True);
	virtual sal_Bool InsertCol(const SwSelBoxes & rBoxes, sal_uInt16 nCnt= 1, sal_Bool bBehind= sal_True);
	virtual sal_Bool InsertRow(const SwCursor & rCursor, sal_uInt16 nCnt= 1, sal_Bool bBehind= sal_True);
	virtual sal_Bool InsertRow(const SwSelBoxes & rBoxes, sal_uInt16 nCnt= 1, sal_Bool bBehind= sal_True);
	virtual sal_Bool DeleteRowCol(const SwSelBoxes & rBoxes, bool bColumn= false);
	virtual sal_Bool DeleteRow(const SwCursor & rCursor);
	virtual sal_Bool DeleteCol(const SwCursor & rCursor);
	virtual sal_Bool SplitTbl(const SwSelBoxes & rBoxes, sal_Bool bVert= sal_True, sal_uInt16 nCnt= 1, sal_Bool bSameHeight= sal_False);
	virtual sal_uInt16 MergeTbl(SwPaM & rPam);
	virtual String GetUniqueTblName() const;
	virtual sal_Bool IsInsTblFormatNum() const;
	virtual sal_Bool IsInsTblChangeNumFormat() const;
	virtual sal_Bool IsInsTblAlignNum() const;
	virtual void GetTabCols(SwTabCols & rFill, const SwCursor * pCrsr, const SwCellFrm * pBoxFrm= 0) const;
	virtual void SetTabCols(const SwTabCols & rNew, sal_Bool bCurRowOnly, const SwCursor * pCrsr, const SwCellFrm * pBoxFrm= 0);
	virtual void GetTabRows(SwTabCols & rFill, const SwCursor * pCrsr, const SwCellFrm * pBoxFrm= 0) const;
	virtual void SetTabRows(const SwTabCols & rNew, sal_Bool bCurColOnly, const SwCursor * pCrsr, const SwCellFrm * pBoxFrm= 0);
	virtual void SetTabCols(SwTable & rTab, const SwTabCols & rNew, const SwTabCols & rOld, const SwTableBox * pStart, sal_Bool bCurRowOnly);
	virtual void SetRowsToRepeat(SwTable & rTable, sal_uInt16 nSet);
	virtual sal_Bool SetTableAutoFmt(const SwSelBoxes & rBoxes, const SwTableAutoFmt & rNew);
	virtual sal_Bool GetTableAutoFmt(const SwSelBoxes & rBoxes, SwTableAutoFmt & rGet);
	virtual void AppendUndoForInsertFromDB(const SwPaM & rPam, sal_Bool bIsTable);
	virtual sal_Bool SetColRowWidthHeight(SwTableBox & rAktBox, sal_uInt16 eType, SwTwips nAbsDiff, SwTwips nRelDiff);
	virtual SwTableBoxFmt * MakeTableBoxFmt();
	virtual SwTableLineFmt * MakeTableLineFmt();
	virtual void ChkBoxNumFmt(SwTableBox & rAktBox, sal_Bool bCallUpdate);
	virtual void SetTblBoxFormulaAttrs(SwTableBox & rBox, const SfxItemSet & rSet);
	virtual void ClearBoxNumAttrs(const SwNodeIndex & rNode);
	virtual void ClearLineNumAttrs(SwPosition & rPos);
	virtual sal_Bool InsCopyOfTbl(SwPosition & rInsPos, const SwSelBoxes & rBoxes, const SwTable * pCpyTbl= 0, sal_Bool bCpyName= sal_False, sal_Bool bCorrPos= sal_False);
	virtual sal_Bool UnProtectCells(const String & rTblName);
	virtual sal_Bool UnProtectCells(const SwSelBoxes & rBoxes);
	virtual sal_Bool UnProtectTbls(const SwPaM & rPam);
	virtual sal_Bool HasTblAnyProtection(const SwPosition * pPos, const String * pTblName= 0, sal_Bool * pFullTblProtection= 0);
	virtual sal_Bool SplitTable(const SwPosition & rPos, sal_uInt16 eMode= 0, sal_Bool bCalcNewSize= sal_False);
	virtual sal_Bool MergeTable(const SwPosition & rPos, sal_Bool bWithPrev= sal_True, sal_uInt16 nMode= 0);
	virtual void UpdateCharts(const String & rName) const;
	virtual void UpdateAllCharts();
	virtual void SetTableName(SwFrmFmt & rTblFmt, const String & rNewName);
	virtual const SwFmtRefMark * GetRefMark(const String & rName) const;
	virtual const SwFmtRefMark * GetRefMark(sal_uInt16 nIndex) const;
	virtual sal_uInt16 GetRefMarks(std::vector<String> * arg1= 0) const;
	virtual SwFlyFrmFmt * InsertLabel(const SwLabelType eType, const String & rTxt, const String & rSeparator, const String & rNumberingSeparator, const sal_Bool bBefore, const sal_uInt16 nId, const sal_uLong nIdx, const String & rCharacterStyle, const sal_Bool bCpyBrd= sal_True);
	virtual SwFlyFrmFmt * InsertDrawLabel(const String & rTxt, const String & rSeparator, const String & rNumberSeparator, const sal_uInt16 nId, const String & rCharacterStyle, SdrObject & rObj);
	virtual const SwAttrPool & GetAttrPool() const;
	virtual SwAttrPool & GetAttrPool();
	virtual SwEditShell * GetEditShell(ViewShell * * ppSh= 0) const;
	virtual ::sw::IShellCursorSupplier * GetIShellCursorSupplier();
	virtual void SetOle2Link(const Link & rLink);
	virtual const Link & GetOle2Link() const;
	virtual SwSection * InsertSwSection(SwPaM const & rRange, SwSectionData & arg1, SwTOXBase const * const pTOXBase= 0, SfxItemSet const * const pAttr= 0, bool const bUpdate= true);
	virtual sal_uInt16 IsInsRegionAvailable(const SwPaM & rRange, const SwNode * * ppSttNd= 0) const;
	virtual SwSection * GetCurrSection(const SwPosition & rPos) const;
	virtual SwSectionFmts & GetSections();
	virtual const SwSectionFmts & GetSections() const;
	virtual SwSectionFmt * MakeSectionFmt(SwSectionFmt * pDerivedFrom);
	virtual void DelSectionFmt(SwSectionFmt * pFmt, sal_Bool bDelNodes= sal_False);
	virtual void UpdateSection(sal_uInt16 const nSect, SwSectionData & arg1, SfxItemSet const * const arg2= 0, bool const bPreventLinkUpdate= false);
	virtual String GetUniqueSectionName(const String * pChkStr= 0) const;
	virtual SwDocShell * GetDocShell();
	virtual const SwDocShell * GetDocShell() const;
	virtual void SetDocShell(SwDocShell * pDSh);
	virtual void SetTmpDocShell(SfxObjectShellLock rLock);
	virtual SfxObjectShellLock GetTmpDocShell();
	virtual SfxObjectShell * GetPersist() const;
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> GetDocStorage();
	virtual bool IsInLoadAsynchron() const;
	virtual void SetInLoadAsynchron(bool bFlag);
	virtual sal_Bool SelectServerObj(const String & rStr, SwPaM * & rpPam, SwNodeRange * & rpRange) const;
	virtual bool IsCopyIsMove() const;
	virtual void SetCopyIsMove(bool bFlag);
	virtual SwDrawContact * GroupSelection(SdrView & arg1);
	virtual void UnGroupSelection(SdrView & arg1);
	virtual sal_Bool DeleteSelection(SwDrawView & arg1);
	virtual void SpellItAgainSam(sal_Bool bInvalid, sal_Bool bOnlyWrong, sal_Bool bSmartTags);
	virtual void InvalidateAutoCompleteFlag();
	virtual void SetCalcFieldValueHdl(Outliner * pOutliner);
	virtual sal_Bool IsVisitedURL(const String & rURL) const;
	virtual void SetAutoCorrExceptWord(SwAutoCorrExceptWord * pNew);
	virtual SwAutoCorrExceptWord * GetAutoCorrExceptWord();
	virtual const SwFmtINetFmt * FindINetAttr(const String & rName) const;
	virtual sal_Bool ExecMacro(const SvxMacro & rMacro, String * pRet= 0, SbxArray * pArgs= 0);
	virtual sal_uInt16 CallEvent(sal_uInt16 nEvent, const SwCallMouseEvent & rCallEvent, sal_Bool bChkPtr= sal_False, SbxArray * pArgs= 0, const Link * pCallBack= 0);
	virtual void MoveLeftMargin(const SwPaM & rPam, sal_Bool bRight= sal_True, sal_Bool bModulus= sal_True);
	virtual SvNumberFormatter * GetNumberFormatter(sal_Bool bCreate= sal_True);
	virtual const SvNumberFormatter * GetNumberFormatter(sal_Bool bCreate= sal_True) const;
	virtual bool HasInvisibleContent() const;
	virtual bool RemoveInvisibleContent();
	virtual bool RestoreInvisibleContent();
	virtual sal_Bool ConvertFieldsToText();
	virtual sal_Bool GenerateGlobalDoc(const String & rPath, const SwTxtFmtColl * pSplitColl= 0);
	virtual sal_Bool GenerateGlobalDoc(const String & rPath, int nOutlineLevel= 0);
	virtual sal_Bool GenerateHTMLDoc(const String & rPath, const SwTxtFmtColl * pSplitColl= 0);
	virtual sal_Bool GenerateHTMLDoc(const String & rPath, int nOutlineLevel= 0);
	virtual long CompareDoc(const SwDoc & rDoc);
	virtual long MergeDoc(const SwDoc & rDoc);
	virtual void SetAutoFmtRedlineComment(const String * pTxt, sal_uInt16 nSeqNo= 0);
	virtual bool IsAutoFmtRedline() const;
	virtual void SetAutoFmtRedline(bool bFlag);
	virtual void SetTxtFmtCollByAutoFmt(const SwPosition & rPos, sal_uInt16 nPoolId, const SfxItemSet * pSet= 0);
	virtual void SetFmtItemByAutoFmt(const SwPaM & rPam, const SfxItemSet & arg1);
	virtual void ClearDoc();
	virtual const SwPagePreViewPrtData * GetPreViewPrtData() const;
	virtual void SetPreViewPrtData(const SwPagePreViewPrtData * pData);
	virtual void SetOLEObjModified();
	virtual const SwUnoCrsrTbl & GetUnoCrsrTbl() const;
	virtual SwUnoCrsr * CreateUnoCrsr(const SwPosition & rPos, sal_Bool bTblCrsr= sal_False);
	virtual sal_Bool ChgAnchor(const SdrMarkList & _rMrkList, RndStdIds _eAnchorType, const sal_Bool _bSameOnly, const sal_Bool _bPosCorr);
	virtual void SetRowHeight(const SwCursor & rCursor, const SwFmtFrmSize & rNew);
	virtual void GetRowHeight(const SwCursor & rCursor, SwFmtFrmSize * & rpSz) const;
	virtual void SetRowSplit(const SwCursor & rCursor, const SwFmtRowSplit & rNew);
	virtual void GetRowSplit(const SwCursor & rCursor, SwFmtRowSplit * & rpSz) const;
	virtual sal_Bool BalanceRowHeight(const SwCursor & rCursor, sal_Bool bTstOnly= sal_True);
	virtual void SetRowBackground(const SwCursor & rCursor, const SvxBrushItem & rNew);
	virtual sal_Bool GetRowBackground(const SwCursor & rCursor, SvxBrushItem & rToFill) const;
	virtual void SetTabBorders(const SwCursor & rCursor, const SfxItemSet & rSet);
	virtual void SetTabLineStyle(const SwCursor & rCursor, const Color * pColor, sal_Bool bSetLine, const editeng::SvxBorderLine * pBorderLine);
	virtual void GetTabBorders(const SwCursor & rCursor, SfxItemSet & rSet) const;
	virtual void SetBoxAttr(const SwCursor & rCursor, const SfxPoolItem & rNew);
	virtual sal_Bool GetBoxAttr(const SwCursor & rCursor, SfxPoolItem & rToFill) const;
	virtual void SetBoxAlign(const SwCursor & rCursor, sal_uInt16 nAlign);
	virtual sal_uInt16 GetBoxAlign(const SwCursor & rCursor) const;
	virtual void AdjustCellWidth(const SwCursor & rCursor, sal_Bool bBalance= sal_False);
	virtual int Chainable(const SwFrmFmt & rSource, const SwFrmFmt & rDest);
	virtual int Chain(SwFrmFmt & rSource, const SwFrmFmt & rDest);
	virtual void Unchain(SwFrmFmt & rFmt);
	virtual SdrObject * CloneSdrObj(const SdrObject & arg1, sal_Bool bMoveWithinDoc= sal_False, sal_Bool bInsInPage= sal_True);
	virtual SwExtTextInput * CreateExtTextInput(const SwPaM & rPam);
	virtual void DeleteExtTextInput(SwExtTextInput * pDel);
	virtual SwExtTextInput * GetExtTextInput(const SwNode & rNd, xub_StrLen nCntntPos= STRING_NOTFOUND) const;
	virtual SwExtTextInput * GetExtTextInput() const;
	static SwAutoCompleteWord& GetAutoCompleteWords() { return *pACmpltWords; }
	virtual bool ContainsMSVBasic() const;
	virtual void SetContainsMSVBasic(bool bFlag);
	virtual sal_uInt16 FillRubyList(const SwPaM & rPam, SwRubyList & rList, sal_uInt16 nMode);
	virtual sal_uInt16 SetRubyList(const SwPaM & rPam, const SwRubyList & rList, sal_uInt16 nMode);
	virtual void ReadLayoutCache(SvStream & rStream);
	virtual void WriteLayoutCache(SvStream & rStream);
	virtual SwLayoutCache * GetLayoutCache() const;
	virtual bool ContainsHiddenChars() const;
	virtual SwModify * GetUnoCallBack() const;
	virtual IGrammarContact * getGrammarContact() const;
	virtual void MarkListLevel(const String & sListId, const int nListLevel, const sal_Bool bValue);
	virtual void MarkListLevel(SwList & rList, const int nListLevel, const sal_Bool bValue);
	virtual void ChgFmt(SwFmt & rFmt, const SfxItemSet & rSet);
	virtual void RenameFmt(SwFmt & rFmt, const String & sNewName, sal_Bool bBroadcast= sal_False);
	virtual void ChgTOX(SwTOXBase & rTOX, const SwTOXBase & rNew);
	virtual String GetPaMDescr(const SwPaM & rPaM) const;
	virtual sal_Bool IsFirstOfNumRule(SwPosition & rPos);
	virtual com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> getXForms() const;
	virtual com::sun::star::uno::Reference<com::sun::star::linguistic2::XProofreadingIterator> GetGCIterator() const;
	virtual bool isXForms() const;
	virtual void initXForms(bool bCreateDefaultModel);
	virtual bool ApplyWorkaroundForB6375613() const;
	virtual void SetApplyWorkaroundForB6375613(bool p_bApplyWorkaroundForB6375613);
	virtual void SetDefaultPageMode(bool bSquaredPageMode);
	virtual sal_Bool IsSquaredPageMode() const;
	virtual void Setn32DummyCompatabilityOptions1(sal_uInt32 CompatabilityOptions1);
	virtual sal_uInt32 Getn32DummyCompatabilityOptions1();
	virtual void Setn32DummyCompatabilityOptions2(sal_uInt32 CompatabilityOptions2);
	virtual sal_uInt32 Getn32DummyCompatabilityOptions2();
	virtual com::sun::star::uno::Reference<com::sun::star::script::vba::XVBAEventProcessor> GetVbaEventProcessor();
	virtual void SetVBATemplateToProjectCache(com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> & xCache);
	virtual com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> GetVBATemplateToProjectCache();
	virtual ::sfx2::IXmlIdRegistry & GetXmlIdRegistry();
	virtual ::sw::MetaFieldManager & GetMetaFieldManager();
	virtual ::sw::UndoManager & GetUndoManager();
	virtual ::sw::UndoManager const & GetUndoManager() const;
	virtual SfxObjectShell * CreateCopy(bool bCallInitNew) const;
	virtual void dumpAsXml(xmlTextWriterPtr writer= NULL);
	virtual void SetDrawDefaults();

        DECL_LINK( AddDrawUndo, SdrUndoAction * );
        DECL_STATIC_LINK( SwDoc, BackgroundDone, SvxBrushItem *);
        DECL_LINK(CalcFieldValueHdl, EditFieldInfo*);
};

// This method is called in Dtor of SwDoc and deletes cache of ContourObjects.
void ClrContourCache();

// namespace <docfunc> for functions and procedures working on a Writer document.
namespace docfunc
{
    /** method to check, if given Writer document contains at least one drawing object

        @author OD

        @param p_rDoc
        input parameter - reference to the Writer document, which is investigated.
    */
    bool ExistsDrawObjs( SwDoc& p_rDoc );

    /** method to check, if given Writer document contains only drawing objects,
        which are completely on its page.

        @author OD

        @param p_rDoc
        input parameter - reference to the Writer document, which is investigated.
    */
    bool AllDrawObjsOnPage( SwDoc& p_rDoc );

    /** method to check, if the outline style has to written as a normal list style

        #i69627#
        The outline style has to written as a normal list style, if a parent
        paragraph style of one of the paragraph styles, which are assigned to
        the list levels of the outline style, has a list style set or inherits
        a list style from its parent paragraphs style.
        This information is needed for the OpenDocument file format export.

        @author OD

        @param rDoc
        input parameter - reference to the text document, which is investigated.

        @return boolean
        indicating, if the outline style has to written as a normal list style
    */
    bool HasOutlineStyleToBeWrittenAsNormalListStyle( SwDoc& rDoc );
}

#endif
