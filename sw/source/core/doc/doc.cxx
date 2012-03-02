#include <doc.hxx>
#include <iostream>
#include "UndoManager.hxx"
#include "fmtmeta.hxx"
#include "printdata.hxx"
#include "docary.hxx"
#include "docfld.hxx"
#include "lineinfo.hxx"
#include "docstat.hxx"
#include "ftninfo.hxx"
#include "tox.hxx"
#include "pagedesc.hxx"
#include "frmfmt.hxx"
#include "ftnidx.hxx"
#include "unochart.hxx"
#include "istyleaccess.hxx"
#include "fmtcntnt.hxx"
#include "swstylemanager.hxx"
#include "fmtfld.hxx"
#include "switerator.hxx"
#include "fldbas.hxx"
#include "txtfld.hxx"
#include "ndtxt.hxx"
#include "docufld.hxx"
#include <editeng/forbiddencharacterstable.hxx>
#include <sfx2/Metadatable.hxx>
#include <sfx2/linkmgr.hxx>



// Page descriptors
SV_IMPL_PTRARR(SwPageDescs,SwPageDescPtr);
// Table Of ...
SV_IMPL_PTRARR( SwTOXTypes, SwTOXTypePtr )
// Field types
SV_IMPL_PTRARR( SwFldTypes, SwFldTypePtr)



using namespace std;

SwDoc::SwDoc()
{
    cout << "Constructor" << std::endl;
}

SwDoc::~SwDoc()
{

}

bool SwDoc::IsInDtor() const{
	bool out = false;
	cout << "IsInDtor(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwNodes & SwDoc::GetNodes(){
	SwNodes out(this);
	cout << "GetNodes(";
	cout << ") -> ";
	cout << "type: \"SwNodes\"";
	cout << "\n";
	return out;
}

SwNodes const & SwDoc::GetNodes() const{
	SwNodes out(const_cast<SwDoc*>(this));
	cout << "GetNodes(";
	cout << ") -> ";
	cout << "type: \"SwNodes\"";
	cout << "\n";
	return out;
}

sal_Int32 SwDoc::acquire(){
    return osl_incrementInterlockedCount(&mReferenceCount);
}

sal_Int32 SwDoc::release(){
    return osl_decrementInterlockedCount(&mReferenceCount);
}

sal_Int32 SwDoc::getReferenceCount() const{
	sal_Int32 out = 0;
	cout << "getReferenceCount(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::get(DocumentSettingId id) const{
	bool out = false;
	cout << "get(";
	cout << "id";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::set(DocumentSettingId id, bool value){
	cout << "set(";
	cout << "id";
	cout << ",";
	cout << value;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const com::sun::star::i18n::ForbiddenCharacters * SwDoc::getForbiddenCharacters(sal_uInt16 nLang, bool bLocaleData) const{
	const com::sun::star::i18n::ForbiddenCharacters * out = 0;
	cout << "getForbiddenCharacters(";
	cout << nLang;
	cout << ",";
	cout << bLocaleData;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::setForbiddenCharacters(sal_uInt16 nLang, const com::sun::star::i18n::ForbiddenCharacters & rForbiddenCharacters){
	cout << "setForbiddenCharacters(";
	cout << nLang;
	cout << ",";
	cout << "rForbiddenCharacters";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

rtl::Reference<SvxForbiddenCharactersTable> & SwDoc::getForbiddenCharacterTable(){
	rtl::Reference<SvxForbiddenCharactersTable> out;
	cout << "getForbiddenCharacterTable(";
	cout << ") -> ";
	cout << "type: \"rtl::Reference<SvxForbiddenCharactersTable>\"";
	cout << "\n";
	return out;
}

const rtl::Reference<SvxForbiddenCharactersTable> & SwDoc::getForbiddenCharacterTable() const{
	rtl::Reference<SvxForbiddenCharactersTable> out;
	cout << "getForbiddenCharacterTable(";
	cout << ") -> ";
	cout << "type: \"rtl::Reference<SvxForbiddenCharactersTable>\"";
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::getLinkUpdateMode(bool bGlobalSettings) const{
	sal_uInt16 out = 0;
	cout << "getLinkUpdateMode(";
	cout << bGlobalSettings;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::setLinkUpdateMode(sal_uInt16 nMode){
	cout << "setLinkUpdateMode(";
	cout << nMode;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwFldUpdateFlags SwDoc::getFieldUpdateFlags(bool bGlobalSettings) const{
	SwFldUpdateFlags out = AUTOUPD_OFF;
	cout << "getFieldUpdateFlags(";
	cout << bGlobalSettings;
	cout << ") -> ";
	cout << "type: \"SwFldUpdateFlags\"";
	cout << "\n";
	return out;
}

void SwDoc::setFieldUpdateFlags(SwFldUpdateFlags eMode){
	cout << "setFieldUpdateFlags(";
	cout << "eMode";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwCharCompressType SwDoc::getCharacterCompressionType() const{
	SwCharCompressType out = CHARCOMPRESS_NONE;
	cout << "getCharacterCompressionType(";
	cout << ") -> ";
	cout << "type: \"SwCharCompressType\"";
	cout << "\n";
	return out;
}

void SwDoc::setCharacterCompressionType(SwCharCompressType nType){
	cout << "setCharacterCompressionType(";
	cout << "nType";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt32 SwDoc::getRsid() const{
	sal_uInt32 out = 0;
	cout << "getRsid(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::setRsid(sal_uInt32 nVal){
	cout << "setRsid(";
	cout << nVal;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt32 SwDoc::getRsidRoot() const{
	sal_uInt32 out = 0;
	cout << "getRsidRoot(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::setRsidRoot(sal_uInt32 nVal){
	cout << "setRsidRoot(";
	cout << nVal;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SfxPrinter * SwDoc::getPrinter(bool bCreate) const{
	SfxPrinter * out = 0;
	cout << "getPrinter(";
	cout << bCreate;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::setPrinter(SfxPrinter * pP, bool bDeleteOld, bool bCallPrtDataChanged){
	cout << "setPrinter(";
	cout << pP;
	cout << ",";
	cout << bDeleteOld;
	cout << ",";
	cout << bCallPrtDataChanged;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

VirtualDevice * SwDoc::getVirtualDevice(bool bCreate) const{
	VirtualDevice * out = 0;
	cout << "getVirtualDevice(";
	cout << bCreate;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::setVirtualDevice(VirtualDevice * pVd, bool bDeleteOld, bool bCallVirDevDataChanged){
	cout << "setVirtualDevice(";
	cout << pVd;
	cout << ",";
	cout << bDeleteOld;
	cout << ",";
	cout << bCallVirDevDataChanged;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

OutputDevice * SwDoc::getReferenceDevice(bool bCreate) const{
    VirtualDevice* pNewVir = new VirtualDevice( 1 );

    pNewVir->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );

    MapMode aMapMode( pNewVir->GetMapMode() );
    aMapMode.SetMapUnit( MAP_TWIP );
    pNewVir->SetMapMode( aMapMode );

    return pNewVir;
}

void SwDoc::setReferenceDeviceType(bool bNewVirtual, bool bNewHiRes){
	cout << "setReferenceDeviceType(";
	cout << bNewVirtual;
	cout << ",";
	cout << bNewHiRes;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const JobSetup * SwDoc::getJobsetup() const{
	const JobSetup * out = 0;
	cout << "getJobsetup(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::setJobsetup(const JobSetup & rJobSetup){
	cout << "setJobsetup(";
	cout << "rJobSetup";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwPrintData & SwDoc::getPrintData() const{
	const SwPrintData out;
	cout << "getPrintData(";
	cout << ") -> ";
	cout << "type: \"SwPrintData\"";
	cout << "\n";
	return out;
}

void SwDoc::setPrintData(const SwPrintData & rPrtData){
	cout << "setPrintData(";
	cout << "rPrtData";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

IDocumentMarkAccess * SwDoc::getIDocumentMarkAccess(){
	IDocumentMarkAccess * out = 0;
	cout << "getIDocumentMarkAccess(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const IDocumentMarkAccess * SwDoc::getIDocumentMarkAccess() const{
	const IDocumentMarkAccess * out = 0;
	cout << "getIDocumentMarkAccess(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

RedlineMode_t SwDoc::GetRedlineMode() const{
	RedlineMode_t out = 0;
	cout << "GetRedlineMode(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetRedlineMode_intern(RedlineMode_t eMode){
	cout << "SetRedlineMode_intern(";
	cout << eMode;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetRedlineMode(RedlineMode_t eMode){
	cout << "SetRedlineMode(";
	cout << eMode;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsRedlineOn() const{
	bool out = false;
	cout << "IsRedlineOn(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsIgnoreRedline() const{
	bool out = false;
	cout << "IsIgnoreRedline(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsInRedlines(const SwNode & rNode) const{
	bool out = false;
	cout << "IsInRedlines(";
	cout << "rNode";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwRedlineTbl & SwDoc::GetRedlineTbl() const{
	const SwRedlineTbl out(0);
	cout << "GetRedlineTbl(";
	cout << ") -> ";
	cout << "type: \"SwRedlineTbl\"";
	cout << "\n";
	return out;
}

bool SwDoc::AppendRedline(SwRedline * pPtr, bool bCallDelete){
	bool out = false;
	cout << "AppendRedline(";
	cout << pPtr;
	cout << ",";
	cout << bCallDelete;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::SplitRedline(const SwPaM & rPam){
	bool out = false;
	cout << "SplitRedline(";
	cout << "rPam";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::DeleteRedline(const SwPaM & rPam, bool bSaveInUndo, sal_uInt16 nDelType){
	bool out = false;
	cout << "DeleteRedline(";
	cout << "rPam";
	cout << ",";
	cout << bSaveInUndo;
	cout << ",";
	cout << nDelType;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::DeleteRedline(const SwStartNode & rSection, bool bSaveInUndo, sal_uInt16 nDelType){
	bool out = false;
	cout << "DeleteRedline(";
	cout << "rSection";
	cout << ",";
	cout << bSaveInUndo;
	cout << ",";
	cout << nDelType;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::GetRedlinePos(const SwNode & rNode, sal_uInt16 nType) const{
	sal_uInt16 out = 0;
	cout << "GetRedlinePos(";
	cout << "rNode";
	cout << ",";
	cout << nType;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::CompressRedlines(){
	cout << "CompressRedlines(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwRedline * SwDoc::GetRedline(const SwPosition & rPos, sal_uInt16 * pFndPos) const{
	const SwRedline * out = 0;
	cout << "GetRedline(";
	cout << "rPos";
	cout << ",";
	cout << pFndPos;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsRedlineMove() const{
	bool out = false;
	cout << "IsRedlineMove(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetRedlineMove(bool bFlag){
	cout << "SetRedlineMove(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::AcceptRedline(sal_uInt16 nPos, bool bCallDelete){
	bool out = false;
	cout << "AcceptRedline(";
	cout << nPos;
	cout << ",";
	cout << bCallDelete;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::AcceptRedline(const SwPaM & rPam, bool bCallDelete){
	bool out = false;
	cout << "AcceptRedline(";
	cout << "rPam";
	cout << ",";
	cout << bCallDelete;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::RejectRedline(sal_uInt16 nPos, bool bCallDelete){
	bool out = false;
	cout << "RejectRedline(";
	cout << nPos;
	cout << ",";
	cout << bCallDelete;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::RejectRedline(const SwPaM & rPam, bool bCallDelete){
	bool out = false;
	cout << "RejectRedline(";
	cout << "rPam";
	cout << ",";
	cout << bCallDelete;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwRedline * SwDoc::SelNextRedline(SwPaM & rPam) const{
	const SwRedline * out = 0;
	cout << "SelNextRedline(";
	cout << "rPam";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwRedline * SwDoc::SelPrevRedline(SwPaM & rPam) const{
	const SwRedline * out = 0;
	cout << "SelPrevRedline(";
	cout << "rPam";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::UpdateRedlineAttr(){
	cout << "UpdateRedlineAttr(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::GetRedlineAuthor(){
	sal_uInt16 out = 0;
	cout << "GetRedlineAuthor(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::InsertRedlineAuthor(const String & rAuthor){
	sal_uInt16 out = 0;
	cout << "InsertRedlineAuthor(";
	cout << rAuthor.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::SetRedlineComment(const SwPaM & rPam, const String & rComment){
	bool out = false;
	cout << "SetRedlineComment(";
	cout << "rPam";
	cout << ",";
	cout << rComment.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const ::com::sun::star::uno::Sequence<sal_Int8> & SwDoc::GetRedlinePassword() const{
	::com::sun::star::uno::Sequence<sal_Int8> out;
	cout << "GetRedlinePassword(";
	cout << ") -> ";
	cout << "type: \"::com::sun::star::uno::Sequence<sal_Int8>\"";
	cout << "\n";
	return out;
}

void SwDoc::SetRedlinePassword(const ::com::sun::star::uno::Sequence<sal_Int8> & rNewPassword){
	cout << "SetRedlinePassword(";
	cout << "rNewPassword";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

IDocumentUndoRedo & SwDoc::GetIDocumentUndoRedo(){
    sw::UndoManager out(::std::auto_ptr<SwNodes>(new SwNodes(this)),static_cast<IDocumentDrawModelAccess&>(*this),static_cast<IDocumentRedlineAccess&>(*this),static_cast<IDocumentState&>(*this));
	cout << "GetIDocumentUndoRedo(";
	cout << ") -> ";
	cout << "type: \"IDocumentUndoRedo\"";
	cout << "\n";
	return out;
}

IDocumentUndoRedo const & SwDoc::GetIDocumentUndoRedo() const{
    sw::UndoManager out(::std::auto_ptr<SwNodes>(new SwNodes(const_cast<SwDoc*>(this))),static_cast<IDocumentDrawModelAccess&>(*const_cast<SwDoc*>(this)),static_cast<IDocumentRedlineAccess&>(*const_cast<SwDoc*>(this)),static_cast<IDocumentState&>(*const_cast<SwDoc*>(this)));
	cout << "GetIDocumentUndoRedo(";
	cout << ") -> ";
	cout << "type: \"IDocumentUndoRedo\"";
	cout << "\n";
	return out;
}

bool SwDoc::IsVisibleLinks() const{
	bool out = false;
	cout << "IsVisibleLinks(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetVisibleLinks(bool bFlag){
	cout << "SetVisibleLinks(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sfx2::LinkManager & SwDoc::GetLinkManager(){
	sfx2::LinkManager out(NULL);
	cout << "GetLinkManager(";
	cout << ") -> ";
	cout << "type: \"sfx2::LinkManager\"";
	cout << "\n";
	return out;
}

const sfx2::LinkManager & SwDoc::GetLinkManager() const{
	sfx2::LinkManager out(NULL);
	cout << "GetLinkManager(";
	cout << ") -> ";
	cout << "type: \"sfx2::LinkManager\"";
	cout << "\n";
	return out;
}

void SwDoc::UpdateLinks(sal_Bool bUI){
	cout << "UpdateLinks(";
	cout << bUI;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::GetData(const rtl::OUString & rItem, const String & rMimeType, ::com::sun::star::uno::Any & rValue) const{
	bool out = false;
	cout << "GetData(";
	cout << "rItem";
	cout << ",";
	cout << rMimeType.GetBuffer();
	cout << ",";
	cout << "rValue";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::SetData(const rtl::OUString & rItem, const String & rMimeType, const ::com::sun::star::uno::Any & rValue){
	bool out = false;
	cout << "SetData(";
	cout << "rItem";
	cout << ",";
	cout << rMimeType.GetBuffer();
	cout << ",";
	cout << "rValue";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

::sfx2::SvLinkSource * SwDoc::CreateLinkSource(const rtl::OUString & rItem){
	::sfx2::SvLinkSource * out = 0;
	cout << "CreateLinkSource(";
	cout << "rItem";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::EmbedAllLinks(){
	bool out = false;
	cout << "EmbedAllLinks(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetLinksUpdated(const bool bNewLinksUpdated){
	cout << "SetLinksUpdated(";
	cout << bNewLinksUpdated;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::LinksUpdated() const{
	bool out = false;
	cout << "LinksUpdated(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwFldTypes * SwDoc::GetFldTypes() const{
	const SwFldTypes * out = 0;
	cout << "GetFldTypes(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFieldType * SwDoc::InsertFldType(const SwFieldType & arg1){
	SwFieldType * out = 0;
	cout << "InsertFldType(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFieldType * SwDoc::GetSysFldType(const sal_uInt16 eWhich) const{
	SwFieldType * out = 0;
	cout << "GetSysFldType(";
	cout << eWhich;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFieldType * SwDoc::GetFldType(sal_uInt16 nResId, const String & rName, bool bDbFieldMatching) const{
	SwFieldType * out = 0;
	cout << "GetFldType(";
	cout << nResId;
	cout << ",";
	cout << rName.GetBuffer();
	cout << ",";
	cout << bDbFieldMatching;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::RemoveFldType(sal_uInt16 nFld){
	cout << "RemoveFldType(";
	cout << nFld;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdateFlds(SfxPoolItem * pNewHt, bool bCloseDB){
	cout << "UpdateFlds(";
	cout << pNewHt;
	cout << ",";
	cout << bCloseDB;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::InsDeletedFldType(SwFieldType & arg1){
	cout << "InsDeletedFldType(";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::PutValueToField(const SwPosition & rPos, const com::sun::star::uno::Any & rVal, sal_uInt16 nWhich){
	bool out = false;
	cout << "PutValueToField(";
	cout << "rPos";
	cout << ",";
	cout << "rVal";
	cout << ",";
	cout << nWhich;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::UpdateFld(SwTxtFld * rDstFmtFld, SwField & rSrcFld, SwMsgPoolItem * pMsgHnt, bool bUpdateTblFlds){
	bool out = false;
	cout << "UpdateFld(";
	cout << rDstFmtFld;
	cout << ",";
	cout << "rSrcFld";
	cout << ",";
	cout << pMsgHnt;
	cout << ",";
	cout << bUpdateTblFlds;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::UpdateRefFlds(SfxPoolItem * pHt){
	cout << "UpdateRefFlds(";
	cout << pHt;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdateTblFlds(SfxPoolItem * pHt){
	cout << "UpdateTblFlds(";
	cout << pHt;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdateExpFlds(SwTxtFld * pFld, bool bUpdateRefFlds){
	cout << "UpdateExpFlds(";
	cout << pFld;
	cout << ",";
	cout << bUpdateRefFlds;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdateUsrFlds(){
	cout << "UpdateUsrFlds(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdatePageFlds(SfxPoolItem * arg1){
	cout << "UpdatePageFlds(";
	cout << arg1;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::LockExpFlds(){
	cout << "LockExpFlds(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UnlockExpFlds(){
	cout << "UnlockExpFlds(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsExpFldsLocked() const{
	bool out = false;
	cout << "IsExpFldsLocked(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwDocUpdtFld & SwDoc::GetUpdtFlds() const{
	SwDocUpdtFld out;
	cout << "GetUpdtFlds(";
	cout << ") -> ";
	cout << "type: \"SwDocUpdtFld\"";
	cout << "\n";
	return out;
}

bool SwDoc::SetFieldsDirty(bool b, const SwNode * pChk, sal_uLong nLen){
	bool out = false;
	cout << "SetFieldsDirty(";
	cout << b;
	cout << ",";
	cout << pChk;
	cout << ",";
	cout << nLen;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetFixFields(bool bOnlyTimeDate, const DateTime * pNewDateTime){
	cout << "SetFixFields(";
	cout << bOnlyTimeDate;
	cout << ",";
	cout << pNewDateTime;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::FldsToCalc(SwCalc & rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt){
	cout << "FldsToCalc(";
	cout << "rCalc";
	cout << ",";
	cout << nLastNd;
	cout << ",";
	cout << nLastCnt;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::FldsToCalc(SwCalc & rCalc, const _SetGetExpFld & rToThisFld){
	cout << "FldsToCalc(";
	cout << "rCalc";
	cout << ",";
	cout << "rToThisFld";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::FldsToExpand(SwHash * * & ppTbl, sal_uInt16 & rTblSize, const _SetGetExpFld & rToThisFld){
	cout << "FldsToExpand(";
	cout << ppTbl;
	cout << ",";
	cout << rTblSize;
	cout << ",";
	cout << "rToThisFld";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsNewFldLst() const{
	bool out = false;
	cout << "IsNewFldLst(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetNewFldLst(bool bFlag){
	cout << "SetNewFldLst(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::InsDelFldInFldLst(bool bIns, const SwTxtFld & rFld){
	cout << "InsDelFldInFldLst(";
	cout << bIns;
	cout << ",";
	cout << "rFld";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwField * SwDoc::GetField(const SwPosition & rPos){
	SwField * out = 0;
	cout << "GetField(";
	cout << "rPos";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwTxtFld * SwDoc::GetTxtFld(const SwPosition & rPos){
	SwTxtFld * out = 0;
	cout << "GetTxtFld(";
	cout << "rPos";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::CopyRange(SwPaM & arg1, SwPosition & arg2, const bool bCopyAll) const{
	bool out = false;
	cout << "CopyRange(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ",";
	cout << bCopyAll;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DeleteSection(SwNode * pNode){
	cout << "DeleteSection(";
	cout << pNode;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::DeleteRange(SwPaM & arg1){
	bool out = false;
	cout << "DeleteRange(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::DelFullPara(SwPaM & arg1){
	bool out = false;
	cout << "DelFullPara(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::DeleteAndJoin(SwPaM & arg1, const bool bForceJoinNext){
	bool out = false;
	cout << "DeleteAndJoin(";
	cout << "arg1";
	cout << ",";
	cout << bForceJoinNext;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::MoveRange(SwPaM & arg1, SwPosition & arg2, SwMoveFlags arg3){
	bool out = false;
	cout << "MoveRange(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ",";
	cout << "arg3";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::MoveNodeRange(SwNodeRange & arg1, SwNodeIndex & arg2, SwMoveFlags arg3){
	bool out = false;
	cout << "MoveNodeRange(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ",";
	cout << "arg3";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::MoveAndJoin(SwPaM & arg1, SwPosition & arg2, SwMoveFlags arg3){
	bool out = false;
	cout << "MoveAndJoin(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ",";
	cout << "arg3";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::Overwrite(const SwPaM & rRg, const String & rStr){
	bool out = false;
	cout << "Overwrite(";
	cout << "rRg";
	cout << ",";
	cout << rStr.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::UpdateRsid(SwTxtNode * pTxtNode, xub_StrLen nStt, xub_StrLen nEnd){
	bool out = false;
	cout << "UpdateRsid(";
	cout << pTxtNode;
	cout << ",";
	cout << "nStt";
	cout << ",";
	cout << "nEnd";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::UpdateParRsid(SwTxtNode * pTxtNode, sal_uInt32 nVal){
	bool out = false;
	cout << "UpdateParRsid(";
	cout << pTxtNode;
	cout << ",";
	cout << nVal;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::UpdateRsid(const SwPaM & rRg, xub_StrLen nLen){
	bool out = false;
	cout << "UpdateRsid(";
	cout << "rRg";
	cout << ",";
	cout << "nLen";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFlyFrmFmt * SwDoc::Insert(const SwPaM & rRg, const String & rGrfName, const String & rFltName, const Graphic * pGraphic, const SfxItemSet * pFlyAttrSet, const SfxItemSet * pGrfAttrSet, SwFrmFmt * arg1){
	SwFlyFrmFmt * out = 0;
	cout << "Insert(";
	cout << "rRg";
	cout << ",";
	cout << rGrfName.GetBuffer();
	cout << ",";
	cout << rFltName.GetBuffer();
	cout << ",";
	cout << pGraphic;
	cout << ",";
	cout << pFlyAttrSet;
	cout << ",";
	cout << pGrfAttrSet;
	cout << ",";
	cout << arg1;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFlyFrmFmt * SwDoc::Insert(const SwPaM & rRg, const GraphicObject & rGrfObj, const SfxItemSet * pFlyAttrSet, const SfxItemSet * pGrfAttrSet, SwFrmFmt * arg1){
	SwFlyFrmFmt * out = 0;
	cout << "Insert(";
	cout << "rRg";
	cout << ",";
	cout << "rGrfObj";
	cout << ",";
	cout << pFlyAttrSet;
	cout << ",";
	cout << pGrfAttrSet;
	cout << ",";
	cout << arg1;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwDrawFrmFmt * SwDoc::Insert(const SwPaM & rRg, SdrObject & rDrawObj, const SfxItemSet * pFlyAttrSet, SwFrmFmt * arg1){
	SwDrawFrmFmt * out = 0;
	cout << "Insert(";
	cout << "rRg";
	cout << ",";
	cout << "rDrawObj";
	cout << ",";
	cout << pFlyAttrSet;
	cout << ",";
	cout << arg1;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFlyFrmFmt * SwDoc::Insert(const SwPaM & rRg, const svt::EmbeddedObjectRef & xObj, const SfxItemSet * pFlyAttrSet, const SfxItemSet * pGrfAttrSet, SwFrmFmt * arg1){
	SwFlyFrmFmt * out = 0;
	cout << "Insert(";
	cout << "rRg";
	cout << ",";
	cout << "xObj";
	cout << ",";
	cout << pFlyAttrSet;
	cout << ",";
	cout << pGrfAttrSet;
	cout << ",";
	cout << arg1;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::InsertPoolItem(const SwPaM & rRg, const SfxPoolItem & arg1, const SetAttrMode nFlags){
	bool out = false;
	cout << "InsertPoolItem(";
	cout << "rRg";
	cout << ",";
	cout << "arg1";
	cout << ",";
	cout << "nFlags";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::InsertItemSet(const SwPaM & rRg, const SfxItemSet & arg1, const SetAttrMode nFlags){
	bool out = false;
	cout << "InsertItemSet(";
	cout << "rRg";
	cout << ",";
	cout << "arg1";
	cout << ",";
	cout << "nFlags";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::ReRead(SwPaM & arg1, const String & rGrfName, const String & rFltName, const Graphic * pGraphic, const GraphicObject * pGrfObj){
	cout << "ReRead(";
	cout << "arg1";
	cout << ",";
	cout << rGrfName.GetBuffer();
	cout << ",";
	cout << rFltName.GetBuffer();
	cout << ",";
	cout << pGraphic;
	cout << ",";
	cout << pGrfObj;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::TransliterateText(const SwPaM & rPaM, utl::TransliterationWrapper & arg1){
	cout << "TransliterateText(";
	cout << "rPaM";
	cout << ",";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwFlyFrmFmt * SwDoc::InsertOLE(const SwPaM & rRg, const String & rObjName, sal_Int64 nAspect, const SfxItemSet * pFlyAttrSet, const SfxItemSet * pGrfAttrSet, SwFrmFmt * arg1){
	SwFlyFrmFmt * out = 0;
	cout << "InsertOLE(";
	cout << "rRg";
	cout << ",";
	cout << rObjName.GetBuffer();
	cout << ",";
	cout << nAspect;
	cout << ",";
	cout << pFlyAttrSet;
	cout << ",";
	cout << pGrfAttrSet;
	cout << ",";
	cout << arg1;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::SplitNode(const SwPosition & rPos, bool bChkTableStart){
	bool out = false;
	cout << "SplitNode(";
	cout << "rPos";
	cout << ",";
	cout << bChkTableStart;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::AppendTxtNode(SwPosition & rPos){
	bool out = false;
	cout << "AppendTxtNode(";
	cout << "rPos";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetModified(SwPaM & rPaM){
	cout << "SetModified(";
	cout << "rPaM";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::ReplaceRange(SwPaM & rPam, const String & rNewStr, const bool bRegExReplace){
	bool out = false;
	cout << "ReplaceRange(";
	cout << "rPam";
	cout << ",";
	cout << rNewStr.GetBuffer();
	cout << ",";
	cout << bRegExReplace;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::RemoveLeadingWhiteSpace(const SwPosition & rPos){
	cout << "RemoveLeadingWhiteSpace(";
	cout << "rPos";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwTxtFmtColl * SwDoc::GetTxtCollFromPool(sal_uInt16 nId, bool bRegardLanguage){
	SwTxtFmtColl * out = 0;
	cout << "GetTxtCollFromPool(";
	cout << nId;
	cout << ",";
	cout << bRegardLanguage;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFmt * SwDoc::GetFmtFromPool(sal_uInt16 nId){
	SwFmt * out = 0;
	cout << "GetFmtFromPool(";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmt * SwDoc::GetFrmFmtFromPool(sal_uInt16 nId){
	SwFrmFmt * out = 0;
	cout << "GetFrmFmtFromPool(";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwCharFmt * SwDoc::GetCharFmtFromPool(sal_uInt16 nId){
	SwCharFmt * out = 0;
	cout << "GetCharFmtFromPool(";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwPageDesc * SwDoc::GetPageDescFromPool(sal_uInt16 nId, bool bRegardLanguage){
	SwPageDesc * out = 0;
	cout << "GetPageDescFromPool(";
	cout << nId;
	cout << ",";
	cout << bRegardLanguage;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwNumRule * SwDoc::GetNumRuleFromPool(sal_uInt16 nId){
	SwNumRule * out = 0;
	cout << "GetNumRuleFromPool(";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsPoolTxtCollUsed(sal_uInt16 nId) const{
	bool out = false;
	cout << "IsPoolTxtCollUsed(";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsPoolFmtUsed(sal_uInt16 nId) const{
	bool out = false;
	cout << "IsPoolFmtUsed(";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsPoolPageDescUsed(sal_uInt16 nId) const{
	bool out = false;
	cout << "IsPoolPageDescUsed(";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwLineNumberInfo & SwDoc::GetLineNumberInfo() const{
	const SwLineNumberInfo out;
	cout << "GetLineNumberInfo(";
	cout << ") -> ";
	cout << "type: \"SwLineNumberInfo\"";
	cout << "\n";
	return out;
}

void SwDoc::SetLineNumberInfo(const SwLineNumberInfo & rInfo){
	cout << "SetLineNumberInfo(";
	cout << "rInfo";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::DocInfoChgd(){
	cout << "DocInfoChgd(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwDocStat & SwDoc::GetDocStat() const{
	const SwDocStat out;
	cout << "GetDocStat(";
	cout << ") -> ";
	cout << "type: \"SwDocStat\"";
	cout << "\n";
	return out;
}

const SwDocStat & SwDoc::GetUpdatedDocStat(){
	const SwDocStat out;
	cout << "GetUpdatedDocStat(";
	cout << ") -> ";
	cout << "type: \"SwDocStat\"";
	cout << "\n";
	return out;
}

void SwDoc::SetDocStat(const SwDocStat & rStat){
	cout << "SetDocStat(";
	cout << "rStat";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdateDocStat(){
	cout << "UpdateDocStat(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetModified(){
	cout << "SetModified(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ResetModified(){
	cout << "ResetModified(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsModified() const{
	bool out = false;
	cout << "IsModified(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsLoaded() const{
	bool out = false;
	cout << "IsLoaded(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsUpdateExpFld() const{
	bool out = false;
	cout << "IsUpdateExpFld(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsNewDoc() const{
	bool out = false;
	cout << "IsNewDoc(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsPageNums() const{
	bool out = false;
	cout << "IsPageNums(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetPageNums(bool b){
	cout << "SetPageNums(";
	cout << b;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetNewDoc(bool b){
	cout << "SetNewDoc(";
	cout << b;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetUpdateExpFldStat(bool b){
	cout << "SetUpdateExpFldStat(";
	cout << b;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetLoaded(bool b){
	cout << "SetLoaded(";
	cout << b;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SdrModel * SwDoc::GetDrawModel() const{
	const SdrModel * out = 0;
	cout << "GetDrawModel(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrModel * SwDoc::GetDrawModel(){
	SdrModel * out = 0;
	cout << "GetDrawModel(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrLayerID SwDoc::GetHeavenId() const{
	SdrLayerID out = 0;
	cout << "GetHeavenId(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrLayerID SwDoc::GetHellId() const{
	SdrLayerID out = 0;
	cout << "GetHellId(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrLayerID SwDoc::GetControlsId() const{
	SdrLayerID out = 0;
	cout << "GetControlsId(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrLayerID SwDoc::GetInvisibleHeavenId() const{
	SdrLayerID out = 0;
	cout << "GetInvisibleHeavenId(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrLayerID SwDoc::GetInvisibleHellId() const{
	SdrLayerID out = 0;
	cout << "GetInvisibleHellId(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrLayerID SwDoc::GetInvisibleControlsId() const{
	SdrLayerID out = 0;
	cout << "GetInvisibleControlsId(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::NotifyInvisibleLayers(SdrPageView & _rSdrPageView){
	cout << "NotifyInvisibleLayers(";
	cout << "_rSdrPageView";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsVisibleLayerId(const SdrLayerID & _nLayerId) const{
	bool out = false;
	cout << "IsVisibleLayerId(";
	cout << _nLayerId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrLayerID SwDoc::GetVisibleLayerIdByInvisibleOne(const SdrLayerID & _nInvisibleLayerId){
	SdrLayerID out = 0;
	cout << "GetVisibleLayerIdByInvisibleOne(";
	cout << _nInvisibleLayerId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrLayerID SwDoc::GetInvisibleLayerIdByVisibleOne(const SdrLayerID & _nVisibleLayerId){
	SdrLayerID out = 0;
	cout << "GetInvisibleLayerIdByVisibleOne(";
	cout << _nVisibleLayerId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrModel * SwDoc::_MakeDrawModel(){
	SdrModel * out = 0;
	cout << "_MakeDrawModel(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SdrModel * SwDoc::GetOrCreateDrawModel(){
	SdrModel * out = 0;
	cout << "GetOrCreateDrawModel(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetCurrentViewShell(ViewShell * pNew){
	cout << "SetCurrentViewShell(";
	cout << pNew;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwLayouter * SwDoc::GetLayouter(){
	SwLayouter * out = 0;
	cout << "GetLayouter(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwLayouter * SwDoc::GetLayouter() const{
	const SwLayouter * out = 0;
	cout << "GetLayouter(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetLayouter(SwLayouter * pNew){
	cout << "SetLayouter(";
	cout << pNew;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwFrmFmt * SwDoc::MakeLayoutFmt(RndStdIds eRequest, const SfxItemSet * pSet){
	SwFrmFmt * out = 0;
	cout << "MakeLayoutFmt(";
	cout << "eRequest";
	cout << ",";
	cout << pSet;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DelLayoutFmt(SwFrmFmt * pFmt){
	cout << "DelLayoutFmt(";
	cout << pFmt;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwFrmFmt * SwDoc::CopyLayoutFmt(const SwFrmFmt & rSrc, const SwFmtAnchor & rNewAnchor, bool bSetTxtFlyAtt, bool bMakeFrms){
	SwFrmFmt * out = 0;
	cout << "CopyLayoutFmt(";
	cout << "rSrc";
	cout << ",";
	cout << "rNewAnchor";
	cout << ",";
	cout << bSetTxtFlyAtt;
	cout << ",";
	cout << bMakeFrms;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const ViewShell * SwDoc::GetCurrentViewShell() const{
	const ViewShell * out = 0;
	cout << "GetCurrentViewShell(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

ViewShell * SwDoc::GetCurrentViewShell(){
	ViewShell * out = 0;
	cout << "GetCurrentViewShell(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwRootFrm * SwDoc::GetCurrentLayout() const{
	const SwRootFrm * out = 0;
	cout << "GetCurrentLayout(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwRootFrm * SwDoc::GetCurrentLayout(){
	SwRootFrm * out = 0;
	cout << "GetCurrentLayout(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::HasLayout() const{
	bool out = false;
	cout << "HasLayout(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::ClearSwLayouterEntries(){
	cout << "ClearSwLayouterEntries(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::StartIdling(){
	cout << "StartIdling(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::StopIdling(){
	cout << "StopIdling(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::BlockIdling(){
	cout << "BlockIdling(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UnblockIdling(){
	cout << "UnblockIdling(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwChartDataProvider * SwDoc::GetChartDataProvider(bool bCreate) const{
	SwChartDataProvider * out = 0;
	cout << "GetChartDataProvider(";
	cout << bCreate;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::CreateChartInternalDataProviders(const SwTable * pTable){
	cout << "CreateChartInternalDataProviders(";
	cout << pTable;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwChartLockController_Helper & SwDoc::GetChartControllerHelper(){
	SwChartLockController_Helper out(this);
	cout << "GetChartControllerHelper(";
	cout << ") -> ";
	cout << "type: \"SwChartLockController_Helper\"";
	cout << "\n";
	return out;
}

void SwDoc::addListItem(const SwNodeNum & rNodeNum){
	cout << "addListItem(";
	cout << "rNodeNum";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::removeListItem(const SwNodeNum & rNodeNum){
	cout << "removeListItem(";
	cout << "rNodeNum";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

String SwDoc::getListItemText(const SwNodeNum & rNodeNum, const bool bWithNumber, const bool bWithSpacesForLevel) const{
	String out;
	cout << "getListItemText(";
	cout << "rNodeNum";
	cout << ",";
	cout << bWithNumber;
	cout << ",";
	cout << bWithSpacesForLevel;
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

void SwDoc::getListItems(IDocumentListItems::tSortedNodeNumList & orNodeNumList) const{
	cout << "getListItems(";
	cout << "orNodeNumList";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::getNumItems(IDocumentListItems::tSortedNodeNumList & orNodeNumList) const{
	cout << "getNumItems(";
	cout << "orNodeNumList";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Int32 SwDoc::getOutlineNodesCount() const{
	sal_Int32 out = 0;
	cout << "getOutlineNodesCount(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

int SwDoc::getOutlineLevel(const sal_Int32 nIdx) const{
	int out = 0;
	cout << "getOutlineLevel(";
	cout << nIdx;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

String SwDoc::getOutlineText(const sal_Int32 nIdx, const bool bWithNumber, const bool bWithSpacesForLevel) const{
	String out;
	cout << "getOutlineText(";
	cout << nIdx;
	cout << ",";
	cout << bWithNumber;
	cout << ",";
	cout << bWithSpacesForLevel;
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

SwTxtNode * SwDoc::getOutlineNode(const sal_Int32 nIdx) const{
	SwTxtNode * out = 0;
	cout << "getOutlineNode(";
	cout << nIdx;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::getOutlineNodes(IDocumentOutlineNodes::tSortedOutlineNodeList & orOutlineNodeList) const{
	cout << "getOutlineNodes(";
	cout << "orOutlineNodeList";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwList * SwDoc::createList(String sListId, const String sDefaultListStyleName){
	SwList * out = 0;
	cout << "createList(";
	cout << sListId.GetBuffer();
	cout << ",";
	cout << sDefaultListStyleName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::deleteList(const String sListId){
	cout << "deleteList(";
	cout << sListId.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwList * SwDoc::getListByName(const String sListId) const{
	SwList * out = 0;
	cout << "getListByName(";
	cout << sListId.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwList * SwDoc::createListForListStyle(const String sListStyleName){
	SwList * out = 0;
	cout << "createListForListStyle(";
	cout << sListStyleName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwList * SwDoc::getListForListStyle(const String sListStyleName) const{
	SwList * out = 0;
	cout << "getListForListStyle(";
	cout << sListStyleName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::deleteListForListStyle(const String sListStyleName){
	cout << "deleteListForListStyle(";
	cout << sListStyleName.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::trackChangeOfListStyleName(const String sListStyleName, const String sNewListStyleName){
	cout << "trackChangeOfListStyleName(";
	cout << sListStyleName.GetBuffer();
	cout << ",";
	cout << sNewListStyleName.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::setExternalData(::sw::tExternalDataType eType, ::sw::tExternalDataPointer pPayload){
	cout << "setExternalData(";
	cout << "eType";
	cout << ",";
	cout << "pPayload";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

::sw::tExternalDataPointer SwDoc::getExternalData(::sw::tExternalDataType eType){
	::sw::tExternalDataPointer out;
	cout << "getExternalData(";
	cout << "eType";
	cout << ") -> ";
	cout << "type: \"::sw::tExternalDataPointer\"";
	cout << "\n";
	return out;
}

bool SwDoc::IsOLEPrtNotifyPending() const{
	bool out = false;
	cout << "IsOLEPrtNotifyPending(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetOLEPrtNotifyPending(bool bSet){
	cout << "SetOLEPrtNotifyPending(";
	cout << bSet;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::PrtOLENotify(sal_Bool bAll){
	cout << "PrtOLENotify(";
	cout << bAll;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::InXMLExport() const{
	bool out = false;
	cout << "InXMLExport(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetXMLExport(bool bFlag){
	cout << "SetXMLExport(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::GetAllFlyFmts(SwPosFlyFrms & rPosFlyFmts, const SwPaM * arg1, sal_Bool bDrawAlso, sal_Bool bAsCharAlso) const{
	cout << "GetAllFlyFmts(";
	cout << "rPosFlyFmts";
	cout << ",";
	cout << arg1;
	cout << ",";
	cout << bDrawAlso;
	cout << ",";
	cout << bAsCharAlso;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwFlyFrmFmt * SwDoc::MakeFlyFrmFmt(const String & rFmtName, SwFrmFmt * pDerivedFrom){
	SwFlyFrmFmt * out = 0;
	cout << "MakeFlyFrmFmt(";
	cout << rFmtName.GetBuffer();
	cout << ",";
	cout << pDerivedFrom;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwDrawFrmFmt * SwDoc::MakeDrawFrmFmt(const String & rFmtName, SwFrmFmt * pDerivedFrom){
	SwDrawFrmFmt * out = 0;
	cout << "MakeDrawFrmFmt(";
	cout << rFmtName.GetBuffer();
	cout << ",";
	cout << pDerivedFrom;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFlyFrmFmt * SwDoc::MakeFlySection(RndStdIds eAnchorType, const SwPosition * pAnchorPos, const SfxItemSet * pSet, SwFrmFmt * pParent, sal_Bool bCalledFromShell){
	SwFlyFrmFmt * out = 0;
	cout << "MakeFlySection(";
	cout << "eAnchorType";
	cout << ",";
	cout << pAnchorPos;
	cout << ",";
	cout << pSet;
	cout << ",";
	cout << pParent;
	cout << ",";
	cout << bCalledFromShell;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFlyFrmFmt * SwDoc::MakeFlyAndMove(const SwPaM & rPam, const SfxItemSet & rSet, const SwSelBoxes * pSelBoxes, SwFrmFmt * pParent){
	SwFlyFrmFmt * out = 0;
	cout << "MakeFlyAndMove(";
	cout << "rPam";
	cout << ",";
	cout << "rSet";
	cout << ",";
	cout << pSelBoxes;
	cout << ",";
	cout << pParent;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::CopyWithFlyInFly(const SwNodeRange & rRg, const xub_StrLen nEndContentIndex, const SwNodeIndex & rInsPos, sal_Bool bMakeNewFrms, sal_Bool bDelRedlines, sal_Bool bCopyFlyAtFly) const{
	cout << "CopyWithFlyInFly(";
	cout << "rRg";
	cout << ",";
	cout << "nEndContentIndex";
	cout << ",";
	cout << "rInsPos";
	cout << ",";
	cout << bMakeNewFrms;
	cout << ",";
	cout << bDelRedlines;
	cout << ",";
	cout << bCopyFlyAtFly;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::SetFlyFrmAttr(SwFrmFmt & rFlyFmt, SfxItemSet & rSet){
	sal_Bool out = false;
	cout << "SetFlyFrmAttr(";
	cout << "rFlyFmt";
	cout << ",";
	cout << "rSet";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::SetFrmFmtToFly(SwFrmFmt & rFlyFmt, SwFrmFmt & rNewFmt, SfxItemSet * pSet, sal_Bool bKeepOrient){
	sal_Bool out = false;
	cout << "SetFrmFmtToFly(";
	cout << "rFlyFmt";
	cout << ",";
	cout << "rNewFmt";
	cout << ",";
	cout << pSet;
	cout << ",";
	cout << bKeepOrient;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetFlyFrmTitle(SwFlyFrmFmt & rFlyFrmFmt, const String & sNewTitle){
	cout << "SetFlyFrmTitle(";
	cout << "rFlyFrmFmt";
	cout << ",";
	cout << sNewTitle.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetFlyFrmDescription(SwFlyFrmFmt & rFlyFrmFmt, const String & sNewDescription){
	cout << "SetFlyFrmDescription(";
	cout << "rFlyFrmFmt";
	cout << ",";
	cout << sNewDescription.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwFtnInfo & SwDoc::GetFtnInfo() const{
	const SwFtnInfo out(0);
	cout << "GetFtnInfo(";
	cout << ") -> ";
	cout << "type: \"SwFtnInfo\"";
	cout << "\n";
	return out;
}

void SwDoc::SetFtnInfo(const SwFtnInfo & rInfo){
	cout << "SetFtnInfo(";
	cout << "rInfo";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwEndNoteInfo & SwDoc::GetEndNoteInfo() const{
	const SwEndNoteInfo out;
	cout << "GetEndNoteInfo(";
	cout << ") -> ";
	cout << "type: \"SwEndNoteInfo\"";
	cout << "\n";
	return out;
}

void SwDoc::SetEndNoteInfo(const SwEndNoteInfo & rInfo){
	cout << "SetEndNoteInfo(";
	cout << "rInfo";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwFtnIdxs & SwDoc::GetFtnIdxs(){
	SwFtnIdxs out;
	cout << "GetFtnIdxs(";
	cout << ") -> ";
	cout << "type: \"SwFtnIdxs\"";
	cout << "\n";
	return out;
}

const SwFtnIdxs & SwDoc::GetFtnIdxs() const{
	const SwFtnIdxs out;
	cout << "GetFtnIdxs(";
	cout << ") -> ";
	cout << "type: \"SwFtnIdxs\"";
	cout << "\n";
	return out;
}

bool SwDoc::SetCurFtn(const SwPaM & rPam, const String & rNumStr, sal_uInt16 nNumber, bool bIsEndNote){
	bool out = false;
	cout << "SetCurFtn(";
	cout << "rPam";
	cout << ",";
	cout << rNumStr.GetBuffer();
	cout << ",";
	cout << nNumber;
	cout << ",";
	cout << bIsEndNote;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

::com::sun::star::uno::Any SwDoc::Spell(SwPaM & arg1, ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1> & arg2, sal_uInt16 * pPageCnt, sal_uInt16 * pPageSt, bool bGrammarCheck, SwConversionArgs * pConvArgs) const{
	::com::sun::star::uno::Any out;
	cout << "Spell(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ",";
	cout << pPageCnt;
	cout << ",";
	cout << pPageSt;
	cout << ",";
	cout << bGrammarCheck;
	cout << ",";
	cout << pConvArgs;
	cout << ") -> ";
	cout << "type: \"::com::sun::star::uno::Any\"";
	cout << "\n";
	return out;
}

::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord> SwDoc::Hyphenate(SwPaM * pPam, const Point & rCrsrPos, sal_uInt16 * pPageCnt, sal_uInt16 * pPageSt){
	::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord> out;
	cout << "Hyphenate(";
	cout << pPam;
	cout << ",";
	cout << "rCrsrPos";
	cout << ",";
	cout << pPageCnt;
	cout << ",";
	cout << pPageSt;
	cout << ") -> ";
	cout << "type: \"::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord>\"";
	cout << "\n";
	return out;
}

void SwDoc::CountWords(const SwPaM & rPaM, SwDocStat & rStat) const{
	cout << "CountWords(";
	cout << "rPaM";
	cout << ",";
	cout << "rStat";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetGlossDoc(bool bGlssDc){
	cout << "SetGlossDoc(";
	cout << bGlssDc;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsInsOnlyTextGlossary() const{
	bool out = false;
	cout << "IsInsOnlyTextGlossary(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::Summary(SwDoc * pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, sal_Bool bImpress){
	cout << "Summary(";
	cout << pExtDoc;
	cout << ",";
	cout << nLevel;
	cout << ",";
	cout << nPara;
	cout << ",";
	cout << bImpress;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ChangeAuthorityData(const SwAuthEntry * pNewData){
	cout << "ChangeAuthorityData(";
	cout << pNewData;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsInCallModified() const{
	bool out = false;
	cout << "IsInCallModified(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::IsInHeaderFooter(const SwNodeIndex & rIdx) const{
	sal_Bool out = false;
	cout << "IsInHeaderFooter(";
	cout << "rIdx";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

short SwDoc::GetTextDirection(const SwPosition & rPos, const Point * pPt) const{
	short out = 0;
	cout << "GetTextDirection(";
	cout << "rPos";
	cout << ",";
	cout << pPt;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::IsInVerticalText(const SwPosition & rPos, const Point * pPt) const{
	sal_Bool out = false;
	cout << "IsInVerticalText(";
	cout << "rPos";
	cout << ",";
	cout << pPt;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetNewDBMgr(SwNewDBMgr * pNewMgr){
	cout << "SetNewDBMgr(";
	cout << pNewMgr;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwNewDBMgr * SwDoc::GetNewDBMgr() const{
	SwNewDBMgr * out = 0;
	cout << "GetNewDBMgr(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::ChangeDBFields(const std::vector<String> & rOldNames, const String & rNewName){
	cout << "ChangeDBFields(";
	cout << "rOldNames";
	cout << ",";
	cout << rNewName.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetInitDBFields(sal_Bool b){
	cout << "SetInitDBFields(";
	cout << b;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::GetAllUsedDB(std::vector<String> & rDBNameList, const std::vector<String> * pAllDBNames){
	cout << "GetAllUsedDB(";
	cout << "rDBNameList";
	cout << ",";
	cout << pAllDBNames;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ChgDBData(const SwDBData & rNewData){
	cout << "ChgDBData(";
	cout << "rNewData";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwDBData SwDoc::GetDBData(){
	SwDBData out;
	cout << "GetDBData(";
	cout << ") -> ";
	cout << "type: \"SwDBData\"";
	cout << "\n";
	return out;
}

const SwDBData & SwDoc::GetDBDesc(){
	const SwDBData out;
	cout << "GetDBDesc(";
	cout << ") -> ";
	cout << "type: \"SwDBData\"";
	cout << "\n";
	return out;
}

const SwDBData & SwDoc::_GetDBDesc() const{
	const SwDBData out;
	cout << "_GetDBDesc(";
	cout << ") -> ";
	cout << "type: \"SwDBData\"";
	cout << "\n";
	return out;
}

String SwDoc::GetUniqueGrfName() const{
	String out;
	cout << "GetUniqueGrfName(";
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

String SwDoc::GetUniqueOLEName() const{
	String out;
	cout << "GetUniqueOLEName(";
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

String SwDoc::GetUniqueFrameName() const{
	String out;
	cout << "GetUniqueFrameName(";
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

std::set<SwRootFrm*> SwDoc::GetAllLayouts(){
	std::set<SwRootFrm*> out;
	cout << "GetAllLayouts(";
	cout << ") -> ";
	cout << "type: \"std::set<SwRootFrm*>\"";
	cout << "\n";
	return out;
}

void SwDoc::SetFlyName(SwFlyFrmFmt & rFmt, const String & rName){
	cout << "SetFlyName(";
	cout << "rFmt";
	cout << ",";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwFlyFrmFmt * SwDoc::FindFlyByName(const String & rName, sal_Int8 nNdTyp) const{
	const SwFlyFrmFmt * out = 0;
	cout << "FindFlyByName(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << nNdTyp;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::GetGrfNms(const SwFlyFrmFmt & rFmt, String * pGrfName, String * pFltName) const{
	cout << "GetGrfNms(";
	cout << "rFmt";
	cout << ",";
	cout << pGrfName;
	cout << ",";
	cout << pFltName;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetAllUniqueFlyNames(){
	cout << "SetAllUniqueFlyNames(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ResetAttrs(const SwPaM & rRg, sal_Bool bTxtAttr, const std::set<sal_uInt16> & rAttrs, const bool bSendDataChangedEvents){
	cout << "ResetAttrs(";
	cout << "rRg";
	cout << ",";
	cout << bTxtAttr;
	cout << ",";
	cout << "rAttrs";
	cout << ",";
	cout << bSendDataChangedEvents;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::RstTxtAttrs(const SwPaM & rRg, sal_Bool bInclRefToxMark){
	cout << "RstTxtAttrs(";
	cout << "rRg";
	cout << ",";
	cout << bInclRefToxMark;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetAttr(const SfxPoolItem & arg1, SwFmt & arg2){
	cout << "SetAttr(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetAttr(const SfxItemSet & arg1, SwFmt & arg2){
	cout << "SetAttr(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ResetAttrAtFormat(const sal_uInt16 nWhichId, SwFmt & rChangedFormat){
	cout << "ResetAttrAtFormat(";
	cout << nWhichId;
	cout << ",";
	cout << "rChangedFormat";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetDefault(const SfxPoolItem & arg1){
	cout << "SetDefault(";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetDefault(const SfxItemSet & arg1){
	cout << "SetDefault(";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SfxPoolItem & SwDoc::GetDefault(sal_uInt16 nFmtHint) const{
	const SfxPoolItem* pItem = new SwFmtCntnt();
	cout << "GetDefault(";
	cout << nFmtHint;
	cout << ") -> ";
	cout << "type: \"SfxPoolItem\"";
	cout << "\n";
	return *pItem;
}

sal_Bool SwDoc::DontExpandFmt(const SwPosition & rPos, sal_Bool bFlag){
	sal_Bool out = false;
	cout << "DontExpandFmt(";
	cout << "rPos";
	cout << ",";
	cout << bFlag;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwFrmFmts * SwDoc::GetFrmFmts() const{
	const SwFrmFmts * out = 0;
	cout << "GetFrmFmts(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmts * SwDoc::GetFrmFmts(){
	SwFrmFmts * out = 0;
	cout << "GetFrmFmts(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwCharFmts * SwDoc::GetCharFmts() const{
	const SwCharFmts * out = 0;
	cout << "GetCharFmts(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwSpzFrmFmts * SwDoc::GetSpzFrmFmts() const{
	const SwSpzFrmFmts * out = 0;
	cout << "GetSpzFrmFmts(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwSpzFrmFmts * SwDoc::GetSpzFrmFmts(){
	SwSpzFrmFmts * out = 0;
	cout << "GetSpzFrmFmts(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwFrmFmt * SwDoc::GetDfltFrmFmt() const{
	const SwFrmFmt * out = 0;
	cout << "GetDfltFrmFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmt * SwDoc::GetDfltFrmFmt(){
	SwFrmFmt * out = 0;
	cout << "GetDfltFrmFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwFrmFmt * SwDoc::GetEmptyPageFmt() const{
	const SwFrmFmt * out = 0;
	cout << "GetEmptyPageFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmt * SwDoc::GetEmptyPageFmt(){
	SwFrmFmt * out = 0;
	cout << "GetEmptyPageFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwFrmFmt * SwDoc::GetColumnContFmt() const{
	const SwFrmFmt * out = 0;
	cout << "GetColumnContFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmt * SwDoc::GetColumnContFmt(){
	SwFrmFmt * out = 0;
	cout << "GetColumnContFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwCharFmt * SwDoc::GetDfltCharFmt() const{
	const SwCharFmt * out = 0;
	cout << "GetDfltCharFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwCharFmt * SwDoc::GetDfltCharFmt(){
	SwCharFmt * out = 0;
	cout << "GetDfltCharFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

IStyleAccess & SwDoc::GetIStyleAccess(){
	IStyleAccess* out = createStyleManager(NULL);
	cout << "GetIStyleAccess(";
	cout << ") -> ";
	cout << "type: \"IStyleAccess\"";
	cout << "\n";
	return *out;
}

void SwDoc::RemoveAllFmtLanguageDependencies(){
	cout << "RemoveAllFmtLanguageDependencies(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwFrmFmt * SwDoc::MakeFrmFmt(const String & rFmtName, SwFrmFmt * pDerivedFrom, sal_Bool bBroadcast, sal_Bool bAuto){
	SwFrmFmt * out = 0;
	cout << "MakeFrmFmt(";
	cout << rFmtName.GetBuffer();
	cout << ",";
	cout << pDerivedFrom;
	cout << ",";
	cout << bBroadcast;
	cout << ",";
	cout << bAuto;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DelFrmFmt(SwFrmFmt * pFmt, sal_Bool bBroadcast){
	cout << "DelFrmFmt(";
	cout << pFmt;
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwFrmFmt * SwDoc::FindFrmFmtByName(const String & rName) const{
	SwFrmFmt * out = 0;
	cout << "FindFrmFmtByName(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwCharFmt * SwDoc::MakeCharFmt(const String & rFmtName, SwCharFmt * pDerivedFrom, sal_Bool bBroadcast, sal_Bool bAuto){
	SwCharFmt * out = 0;
	cout << "MakeCharFmt(";
	cout << rFmtName.GetBuffer();
	cout << ",";
	cout << pDerivedFrom;
	cout << ",";
	cout << bBroadcast;
	cout << ",";
	cout << bAuto;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DelCharFmt(sal_uInt16 nFmt, sal_Bool bBroadcast){
	cout << "DelCharFmt(";
	cout << nFmt;
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::DelCharFmt(SwCharFmt * pFmt, sal_Bool bBroadcast){
	cout << "DelCharFmt(";
	cout << pFmt;
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwCharFmt * SwDoc::FindCharFmtByName(const String & rName) const{
	SwCharFmt * out = 0;
	cout << "FindCharFmtByName(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTxtFmtColl * SwDoc::GetDfltTxtFmtColl() const{
	const SwTxtFmtColl * out = 0;
	cout << "GetDfltTxtFmtColl(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTxtFmtColls * SwDoc::GetTxtFmtColls() const{
	const SwTxtFmtColls * out = 0;
	cout << "GetTxtFmtColls(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwTxtFmtColl * SwDoc::MakeTxtFmtColl(const String & rFmtName, SwTxtFmtColl * pDerivedFrom, sal_Bool bBroadcast, sal_Bool bAuto){
	SwTxtFmtColl * out = 0;
	cout << "MakeTxtFmtColl(";
	cout << rFmtName.GetBuffer();
	cout << ",";
	cout << pDerivedFrom;
	cout << ",";
	cout << bBroadcast;
	cout << ",";
	cout << bAuto;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwConditionTxtFmtColl * SwDoc::MakeCondTxtFmtColl(const String & rFmtName, SwTxtFmtColl * pDerivedFrom, sal_Bool bBroadcast){
	SwConditionTxtFmtColl * out = 0;
	cout << "MakeCondTxtFmtColl(";
	cout << rFmtName.GetBuffer();
	cout << ",";
	cout << pDerivedFrom;
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DelTxtFmtColl(sal_uInt16 nFmt, sal_Bool bBroadcast){
	cout << "DelTxtFmtColl(";
	cout << nFmt;
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::DelTxtFmtColl(SwTxtFmtColl * pColl, sal_Bool bBroadcast){
	cout << "DelTxtFmtColl(";
	cout << pColl;
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::SetTxtFmtColl(const SwPaM & rRg, SwTxtFmtColl * pFmt, bool bReset, bool bResetListAttrs){
	sal_Bool out = false;
	cout << "SetTxtFmtColl(";
	cout << "rRg";
	cout << ",";
	cout << pFmt;
	cout << ",";
	cout << bReset;
	cout << ",";
	cout << bResetListAttrs;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwTxtFmtColl * SwDoc::FindTxtFmtCollByName(const String & rName) const{
	SwTxtFmtColl * out = 0;
	cout << "FindTxtFmtCollByName(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::ChkCondColls(){
	cout << "ChkCondColls(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwGrfFmtColl * SwDoc::GetDfltGrfFmtColl() const{
	const SwGrfFmtColl * out = 0;
	cout << "GetDfltGrfFmtColl(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwGrfFmtColls * SwDoc::GetGrfFmtColls() const{
	const SwGrfFmtColls * out = 0;
	cout << "GetGrfFmtColls(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwGrfFmtColl * SwDoc::MakeGrfFmtColl(const String & rFmtName, SwGrfFmtColl * pDerivedFrom){
	SwGrfFmtColl * out = 0;
	cout << "MakeGrfFmtColl(";
	cout << rFmtName.GetBuffer();
	cout << ",";
	cout << pDerivedFrom;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwGrfFmtColl * SwDoc::FindGrfFmtCollByName(const String & rName) const{
	SwGrfFmtColl * out = 0;
	cout << "FindGrfFmtCollByName(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwFrmFmts * SwDoc::GetTblFrmFmts() const{
	const SwFrmFmts * out = 0;
	cout << "GetTblFrmFmts(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmts * SwDoc::GetTblFrmFmts(){
	SwFrmFmts * out = 0;
	cout << "GetTblFrmFmts(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::GetTblFrmFmtCount(sal_Bool bUsed) const{
	sal_uInt16 out = 0;
	cout << "GetTblFrmFmtCount(";
	cout << bUsed;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmt & SwDoc::GetTblFrmFmt(sal_uInt16 nFmt, sal_Bool bUsed) const{
    SwAttrPool* pPool = new SwAttrPool(const_cast<SwDoc*>(this));
	SwFrmFmt out(*pPool, "test", NULL);
	cout << "GetTblFrmFmt(";
	cout << nFmt;
	cout << ",";
	cout << bUsed;
	cout << ") -> ";
	cout << "type: \"SwFrmFmt\"";
	cout << "\n";
	return out;
}

SwTableFmt * SwDoc::MakeTblFrmFmt(const String & rFmtName, SwFrmFmt * pDerivedFrom){
	SwTableFmt * out = 0;
	cout << "MakeTblFrmFmt(";
	cout << rFmtName.GetBuffer();
	cout << ",";
	cout << pDerivedFrom;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DelTblFrmFmt(SwTableFmt * pFmt){
	cout << "DelTblFrmFmt(";
	cout << pFmt;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwTableFmt * SwDoc::FindTblFmtByName(const String & rName, sal_Bool bAll) const{
	SwTableFmt * out = 0;
	cout << "FindTblFmtByName(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << bAll;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::GetFlyCount(FlyCntType eType) const{
	sal_uInt16 out = 0;
	cout << "GetFlyCount(";
	cout << "eType";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmt * SwDoc::GetFlyNum(sal_uInt16 nIdx, FlyCntType eType){
	SwFrmFmt * out = 0;
	cout << "GetFlyNum(";
	cout << nIdx;
	cout << ",";
	cout << "eType";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFrmFmt * SwDoc::CopyFrmFmt(const SwFrmFmt & arg1){
	SwFrmFmt * out = 0;
	cout << "CopyFrmFmt(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwCharFmt * SwDoc::CopyCharFmt(const SwCharFmt & arg1){
	SwCharFmt * out = 0;
	cout << "CopyCharFmt(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwTxtFmtColl * SwDoc::CopyTxtColl(const SwTxtFmtColl & rColl){
	SwTxtFmtColl * out = 0;
	cout << "CopyTxtColl(";
	cout << "rColl";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwGrfFmtColl * SwDoc::CopyGrfColl(const SwGrfFmtColl & rColl){
	SwGrfFmtColl * out = 0;
	cout << "CopyGrfColl(";
	cout << "rColl";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::ReplaceStyles(const SwDoc & rSource){
	cout << "ReplaceStyles(";
	cout << "rSource";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ReplaceDefaults(const SwDoc & rSource){
	cout << "ReplaceDefaults(";
	cout << "rSource";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ReplaceCompatabilityOptions(const SwDoc & rSource){
	cout << "ReplaceCompatabilityOptions(";
	cout << "rSource";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::IsUsed(const SwModify & arg1) const{
	sal_Bool out = false;
	cout << "IsUsed(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::IsUsed(const SwNumRule & arg1) const{
	sal_Bool out = false;
	cout << "IsUsed(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::SetDocPattern(const String & rPatternName){
	sal_uInt16 out = 0;
	cout << "SetDocPattern(";
	cout << rPatternName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const String * SwDoc::GetDocPattern(sal_uInt16 nPos) const{
	const String * out = 0;
	cout << "GetDocPattern(";
	cout << nPos;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::GCFieldTypes(){
	cout << "GCFieldTypes(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetGlossaryDoc(SwDoc * pDoc){
	cout << "SetGlossaryDoc(";
	cout << pDoc;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::InsertGlossary(SwTextBlocks & rBlock, const String & rEntry, SwPaM & rPaM, SwCrsrShell * pShell){
	sal_Bool out = false;
	cout << "InsertGlossary(";
	cout << "rBlock";
	cout << ",";
	cout << rEntry.GetBuffer();
	cout << ",";
	cout << "rPaM";
	cout << ",";
	cout << pShell;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::CalculatePagesForPrinting(const SwRootFrm & rLayout, SwRenderData & rData, const SwPrintUIOptions & rOptions, bool bIsPDFExport, sal_Int32 nDocPageCount){
	cout << "CalculatePagesForPrinting(";
	cout << "rLayout";
	cout << ",";
	cout << "rData";
	cout << ",";
	cout << "rOptions";
	cout << ",";
	cout << bIsPDFExport;
	cout << ",";
	cout << nDocPageCount;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdatePagesForPrintingWithPostItData(SwRenderData & rData, const SwPrintUIOptions & rOptions, bool bIsPDFExport, sal_Int32 nDocPageCount){
	cout << "UpdatePagesForPrintingWithPostItData(";
	cout << "rData";
	cout << ",";
	cout << "rOptions";
	cout << ",";
	cout << bIsPDFExport;
	cout << ",";
	cout << nDocPageCount;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::CalculatePagePairsForProspectPrinting(const SwRootFrm & rLayout, SwRenderData & rData, const SwPrintUIOptions & rOptions, sal_Int32 nDocPageCount){
	cout << "CalculatePagePairsForProspectPrinting(";
	cout << "rLayout";
	cout << ",";
	cout << "rData";
	cout << ",";
	cout << "rOptions";
	cout << ",";
	cout << nDocPageCount;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::GetPageDescCnt() const{
	sal_uInt16 out = 0;
	cout << "GetPageDescCnt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwPageDesc & SwDoc::GetPageDesc(const sal_uInt16 i) const{
	SwPageDesc out(String(), NULL, const_cast<SwDoc*>(this));
	cout << "GetPageDesc(";
	cout << i;
	cout << ") -> ";
	cout << "type: \"SwPageDesc\"";
	cout << "\n";
	return out;
}

SwPageDesc * SwDoc::FindPageDescByName(const String & rName, sal_uInt16 * pPos) const{
	SwPageDesc * out = 0;
	cout << "FindPageDescByName(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << pPos;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::CopyPageDesc(const SwPageDesc & rSrcDesc, SwPageDesc & rDstDesc, sal_Bool bCopyPoolIds){
	cout << "CopyPageDesc(";
	cout << "rSrcDesc";
	cout << ",";
	cout << "rDstDesc";
	cout << ",";
	cout << bCopyPoolIds;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::CopyHeader(const SwFrmFmt & rSrcFmt, SwFrmFmt & rDestFmt){
	cout << "CopyHeader(";
	cout << "rSrcFmt";
	cout << ",";
	cout << "rDestFmt";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::CopyFooter(const SwFrmFmt & rSrcFmt, SwFrmFmt & rDestFmt){
	cout << "CopyFooter(";
	cout << "rSrcFmt";
	cout << ",";
	cout << "rDestFmt";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwPageDesc * SwDoc::GetPageDesc(const String & rName){
	SwPageDesc * out = 0;
	cout << "GetPageDesc(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwPageDesc & SwDoc::_GetPageDesc(sal_uInt16 i) const{
	SwPageDesc out(String(), NULL, const_cast<SwDoc*>(this));
	cout << "_GetPageDesc(";
	cout << i;
	cout << ") -> ";
	cout << "type: \"SwPageDesc\"";
	cout << "\n";
	return out;
}

void SwDoc::ChgPageDesc(const String & rName, const SwPageDesc & arg1){
	cout << "ChgPageDesc(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ChgPageDesc(sal_uInt16 i, const SwPageDesc & arg1){
	cout << "ChgPageDesc(";
	cout << i;
	cout << ",";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::FindPageDesc(const String & rName, sal_uInt16 * pFound){
	sal_Bool out = false;
	cout << "FindPageDesc(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << pFound;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DelPageDesc(const String & rName, sal_Bool bBroadcast){
	cout << "DelPageDesc(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::DelPageDesc(sal_uInt16 i, sal_Bool bBroadcast){
	cout << "DelPageDesc(";
	cout << i;
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::PreDelPageDesc(SwPageDesc * pDel){
	cout << "PreDelPageDesc(";
	cout << pDel;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::MakePageDesc(const String & rName, const SwPageDesc * pCpy, sal_Bool bRegardLanguage, sal_Bool bBroadcast){
	sal_uInt16 out = 0;
	cout << "MakePageDesc(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << pCpy;
	cout << ",";
	cout << bRegardLanguage;
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::BroadcastStyleOperation(String rName, SfxStyleFamily eFamily, sal_uInt16 nOp){
	cout << "BroadcastStyleOperation(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << "eFamily";
	cout << ",";
	cout << nOp;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::CheckDefaultPageFmt(){
	cout << "CheckDefaultPageFmt(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::GetCurTOXMark(const SwPosition & rPos, SwTOXMarks & arg1) const{
	sal_uInt16 out = 0;
	cout << "GetCurTOXMark(";
	cout << "rPos";
	cout << ",";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DeleteTOXMark(const SwTOXMark * pTOXMark){
	cout << "DeleteTOXMark(";
	cout << pTOXMark;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwTOXMark & SwDoc::GotoTOXMark(const SwTOXMark & rCurTOXMark, SwTOXSearch eDir, sal_Bool bInReadOnly){
	const SwTOXMark out(NULL);
	cout << "GotoTOXMark(";
	cout << "rCurTOXMark";
	cout << ",";
	cout << "eDir";
	cout << ",";
	cout << bInReadOnly;
	cout << ") -> ";
	cout << "type: \"SwTOXMark\"";
	cout << "\n";
	return out;
}

const SwTOXBaseSection * SwDoc::InsertTableOf(const SwPosition & rPos, const SwTOXBase & rTOX, const SfxItemSet * pSet, sal_Bool bExpand){
	const SwTOXBaseSection * out = 0;
	cout << "InsertTableOf(";
	cout << "rPos";
	cout << ",";
	cout << "rTOX";
	cout << ",";
	cout << pSet;
	cout << ",";
	cout << bExpand;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTOXBaseSection * SwDoc::InsertTableOf(sal_uLong nSttNd, sal_uLong nEndNd, const SwTOXBase & rTOX, const SfxItemSet * pSet){
	const SwTOXBaseSection * out = 0;
	cout << "InsertTableOf(";
	cout << nSttNd;
	cout << ",";
	cout << nEndNd;
	cout << ",";
	cout << "rTOX";
	cout << ",";
	cout << pSet;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTOXBase * SwDoc::GetCurTOX(const SwPosition & rPos) const{
	const SwTOXBase * out = 0;
	cout << "GetCurTOX(";
	cout << "rPos";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwAttrSet & SwDoc::GetTOXBaseAttrSet(const SwTOXBase & rTOX) const{
    SwAttrPool* pPool = new SwAttrPool(const_cast<SwDoc*>(this));
	SwAttrSet* pSet = new SwAttrSet(*pPool, 0, 0);
	cout << "GetTOXBaseAttrSet(";
	cout << "rTOX";
	cout << ") -> ";
	cout << "type: \"SwAttrSet\"";
	cout << "\n";
	return *pSet;
}

sal_Bool SwDoc::DeleteTOX(const SwTOXBase & rTOXBase, sal_Bool bDelNodes){
	sal_Bool out = false;
	cout << "DeleteTOX(";
	cout << "rTOXBase";
	cout << ",";
	cout << bDelNodes;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

String SwDoc::GetUniqueTOXBaseName(const SwTOXType & rType, const String * pChkStr) const{
	String out;
	cout << "GetUniqueTOXBaseName(";
	cout << "rType";
	cout << ",";
	cout << pChkStr;
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

sal_Bool SwDoc::SetTOXBaseName(const SwTOXBase & rTOXBase, const String & rName){
	sal_Bool out = false;
	cout << "SetTOXBaseName(";
	cout << "rTOXBase";
	cout << ",";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetTOXBaseProtection(const SwTOXBase & rTOXBase, sal_Bool bProtect){
	cout << "SetTOXBaseProtection(";
	cout << "rTOXBase";
	cout << ",";
	cout << bProtect;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetUpdateTOX(bool bFlag){
	cout << "SetUpdateTOX(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsUpdateTOX() const{
	bool out = false;
	cout << "IsUpdateTOX(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const String & SwDoc::GetTOIAutoMarkURL() const{
	const String out;
	cout << "GetTOIAutoMarkURL(";
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

void SwDoc::SetTOIAutoMarkURL(const String & rSet){
	cout << "SetTOIAutoMarkURL(";
	cout << rSet.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ApplyAutoMark(){
	cout << "ApplyAutoMark(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsInReading() const{
	bool out = false;
	cout << "IsInReading(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetInReading(bool bNew){
	cout << "SetInReading(";
	cout << bNew;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsClipBoard() const{
	bool out = false;
	cout << "IsClipBoard(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetClipBoard(bool bNew){
	cout << "SetClipBoard(";
	cout << bNew;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsColumnSelection() const{
	bool out = false;
	cout << "IsColumnSelection(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetColumnSelection(bool bNew){
	cout << "SetColumnSelection(";
	cout << bNew;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsInXMLImport() const{
	bool out = false;
	cout << "IsInXMLImport(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetInXMLImport(bool bNew){
	cout << "SetInXMLImport(";
	cout << bNew;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::GetTOXTypeCount(TOXTypes eTyp) const{
	sal_uInt16 out = 0;
	cout << "GetTOXTypeCount(";
	cout << "eTyp";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTOXType * SwDoc::GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const{
	const SwTOXType * out = 0;
	cout << "GetTOXType(";
	cout << "eTyp";
	cout << ",";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::DeleteTOXType(TOXTypes eTyp, sal_uInt16 nId){
	sal_Bool out = false;
	cout << "DeleteTOXType(";
	cout << "eTyp";
	cout << ",";
	cout << nId;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTOXType * SwDoc::InsertTOXType(const SwTOXType & rTyp){
	const SwTOXType * out = 0;
	cout << "InsertTOXType(";
	cout << "rTyp";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTOXTypes & SwDoc::GetTOXTypes() const{
	const SwTOXTypes out;
	cout << "GetTOXTypes(";
	cout << ") -> ";
	cout << "type: \"SwTOXTypes\"";
	cout << "\n";
	return out;
}

const SwTOXBase * SwDoc::GetDefaultTOXBase(TOXTypes eTyp, sal_Bool bCreate){
	const SwTOXBase * out = 0;
	cout << "GetDefaultTOXBase(";
	cout << "eTyp";
	cout << ",";
	cout << bCreate;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetDefaultTOXBase(const SwTOXBase & rBase){
	cout << "SetDefaultTOXBase(";
	cout << "rBase";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::GetTOIKeys(SwTOIKeyType eTyp, std::vector<String> & rArr) const{
	sal_uInt16 out = 0;
	cout << "GetTOIKeys(";
	cout << "eTyp";
	cout << ",";
	cout << "rArr";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::SortTbl(const SwSelBoxes & rBoxes, const SwSortOptions & arg1){
	sal_Bool out = false;
	cout << "SortTbl(";
	cout << "rBoxes";
	cout << ",";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::SortText(const SwPaM & arg1, const SwSortOptions & arg2){
	sal_Bool out = false;
	cout << "SortText(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::CorrAbs(const SwNodeIndex & rOldNode, const SwPosition & rNewPos, const xub_StrLen nOffset, sal_Bool bMoveCrsr){
	cout << "CorrAbs(";
	cout << "rOldNode";
	cout << ",";
	cout << "rNewPos";
	cout << ",";
	cout << "nOffset";
	cout << ",";
	cout << bMoveCrsr;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::CorrAbs(const SwNodeIndex & rStartNode, const SwNodeIndex & rEndNode, const SwPosition & rNewPos, sal_Bool bMoveCrsr){
	cout << "CorrAbs(";
	cout << "rStartNode";
	cout << ",";
	cout << "rEndNode";
	cout << ",";
	cout << "rNewPos";
	cout << ",";
	cout << bMoveCrsr;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::CorrAbs(const SwPaM & rRange, const SwPosition & rNewPos, sal_Bool bMoveCrsr){
	cout << "CorrAbs(";
	cout << "rRange";
	cout << ",";
	cout << "rNewPos";
	cout << ",";
	cout << bMoveCrsr;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::CorrRel(const SwNodeIndex & rOldNode, const SwPosition & rNewPos, const xub_StrLen nOffset, sal_Bool bMoveCrsr){
	cout << "CorrRel(";
	cout << "rOldNode";
	cout << ",";
	cout << "rNewPos";
	cout << ",";
	cout << "nOffset";
	cout << ",";
	cout << bMoveCrsr;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwNumRule * SwDoc::GetOutlineNumRule() const{
	SwNumRule * out = 0;
	cout << "GetOutlineNumRule(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetOutlineNumRule(const SwNumRule & rRule){
	cout << "SetOutlineNumRule(";
	cout << "rRule";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::PropagateOutlineRule(){
	cout << "PropagateOutlineRule(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::OutlineUpDown(const SwPaM & rPam, short nOffset){
	sal_Bool out = false;
	cout << "OutlineUpDown(";
	cout << "rPam";
	cout << ",";
	cout << nOffset;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::MoveOutlinePara(const SwPaM & rPam, short nOffset){
	sal_Bool out = false;
	cout << "MoveOutlinePara(";
	cout << "rPam";
	cout << ",";
	cout << nOffset;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::GotoOutline(SwPosition & rPos, const String & rName) const{
	sal_Bool out = false;
	cout << "GotoOutline(";
	cout << "rPos";
	cout << ",";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetNumRule(const SwPaM & arg1, const SwNumRule & arg2, const bool bCreateNewList, const String sContinuedListId, sal_Bool bSetItem, const bool bResetIndentAttrs){
	cout << "SetNumRule(";
	cout << "arg1";
	cout << ",";
	cout << "arg2";
	cout << ",";
	cout << bCreateNewList;
	cout << ",";
	cout << sContinuedListId.GetBuffer();
	cout << ",";
	cout << bSetItem;
	cout << ",";
	cout << bResetIndentAttrs;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetCounted(const SwPaM & arg1, bool bCounted){
	cout << "SetCounted(";
	cout << "arg1";
	cout << ",";
	cout << bCounted;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::MakeUniqueNumRules(const SwPaM & rPaM){
	cout << "MakeUniqueNumRules(";
	cout << "rPaM";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetNumRuleStart(const SwPosition & rPos, sal_Bool bFlag){
	cout << "SetNumRuleStart(";
	cout << "rPos";
	cout << ",";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetNodeNumStart(const SwPosition & rPos, sal_uInt16 nStt){
	cout << "SetNodeNumStart(";
	cout << "rPos";
	cout << ",";
	cout << nStt;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwNumRule * SwDoc::GetCurrNumRule(const SwPosition & rPos) const{
	SwNumRule * out = 0;
	cout << "GetCurrNumRule(";
	cout << "rPos";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwNumRuleTbl & SwDoc::GetNumRuleTbl() const{
	const SwNumRuleTbl out;
	cout << "GetNumRuleTbl(";
	cout << ") -> ";
	cout << "type: \"SwNumRuleTbl\"";
	cout << "\n";
	return out;
}

void SwDoc::AddNumRule(SwNumRule * pRule){
	cout << "AddNumRule(";
	cout << pRule;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::MakeNumRule(const String & rName, const SwNumRule * pCpy, sal_Bool bBroadcast, const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode){
	sal_uInt16 out = 0;
	cout << "MakeNumRule(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << pCpy;
	cout << ",";
	cout << bBroadcast;
	cout << ",";
	cout << "eDefaultNumberFormatPositionAndSpaceMode";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::FindNumRule(const String & rName) const{
	sal_uInt16 out = 0;
	cout << "FindNumRule(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwNumRule * SwDoc::FindNumRulePtr(const String & rName) const{
	SwNumRule * out = 0;
	cout << "FindNumRulePtr(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::RenameNumRule(const String & aOldName, const String & aNewName, sal_Bool bBroadcast){
	sal_Bool out = false;
	cout << "RenameNumRule(";
	cout << aOldName.GetBuffer();
	cout << ",";
	cout << aNewName.GetBuffer();
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::DelNumRule(const String & rName, sal_Bool bBroadCast){
	sal_Bool out = false;
	cout << "DelNumRule(";
	cout << rName.GetBuffer();
	cout << ",";
	cout << bBroadCast;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

String SwDoc::GetUniqueNumRuleName(const String * pChkStr, sal_Bool bAutoNum) const{
	String out;
	cout << "GetUniqueNumRuleName(";
	cout << pChkStr;
	cout << ",";
	cout << bAutoNum;
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

void SwDoc::UpdateNumRule(){
	cout << "UpdateNumRule(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ChgNumRuleFmts(const SwNumRule & rRule, const String * pOldName){
	cout << "ChgNumRuleFmts(";
	cout << "rRule";
	cout << ",";
	cout << pOldName;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::ReplaceNumRule(const SwPosition & rPos, const String & rOldRule, const String & rNewRule){
	sal_Bool out = false;
	cout << "ReplaceNumRule(";
	cout << "rPos";
	cout << ",";
	cout << rOldRule.GetBuffer();
	cout << ",";
	cout << rNewRule.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::GotoNextNum(SwPosition & arg1, sal_Bool bOverUpper, sal_uInt8 * pUpper, sal_uInt8 * pLower){
	sal_Bool out = false;
	cout << "GotoNextNum(";
	cout << "arg1";
	cout << ",";
	cout << bOverUpper;
	cout << ",";
	cout << pUpper;
	cout << ",";
	cout << pLower;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::GotoPrevNum(SwPosition & arg1, sal_Bool bOverUpper, sal_uInt8 * pUpper, sal_uInt8 * pLower){
	sal_Bool out = false;
	cout << "GotoPrevNum(";
	cout << "arg1";
	cout << ",";
	cout << bOverUpper;
	cout << ",";
	cout << pUpper;
	cout << ",";
	cout << pLower;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwNumRule * SwDoc::SearchNumRule(const SwPosition & rPos, const bool bForward, const bool bNum, const bool bOutline, int nNonEmptyAllowed, String & sListId, const bool bInvestigateStartNode){
	const SwNumRule * out = 0;
	cout << "SearchNumRule(";
	cout << "rPos";
	cout << ",";
	cout << bForward;
	cout << ",";
	cout << bNum;
	cout << ",";
	cout << bOutline;
	cout << ",";
	cout << nNonEmptyAllowed;
	cout << ",";
	cout << sListId.GetBuffer();
	cout << ",";
	cout << bInvestigateStartNode;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::NoNum(const SwPaM & arg1){
	sal_Bool out = false;
	cout << "NoNum(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DelNumRules(const SwPaM & arg1){
	cout << "DelNumRules(";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::InvalidateNumRules(){
	cout << "InvalidateNumRules(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::NumUpDown(const SwPaM & arg1, sal_Bool bDown){
	sal_Bool out = false;
	cout << "NumUpDown(";
	cout << "arg1";
	cout << ",";
	cout << bDown;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::MoveParagraph(const SwPaM & arg1, long nOffset, sal_Bool bIsOutlMv){
	sal_Bool out = false;
	cout << "MoveParagraph(";
	cout << "arg1";
	cout << ",";
	cout << nOffset;
	cout << ",";
	cout << bIsOutlMv;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::NumOrNoNum(const SwNodeIndex & rIdx, sal_Bool bDel){
	sal_Bool out = false;
	cout << "NumOrNoNum(";
	cout << "rIdx";
	cout << ",";
	cout << bDel;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::StopNumRuleAnimations(OutputDevice * arg1){
	cout << "StopNumRuleAnimations(";
	cout << arg1;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwTable * SwDoc::InsertTable(const SwInsertTableOptions & rInsTblOpts, const SwPosition & rPos, sal_uInt16 nRows, sal_uInt16 nCols, short eAdjust, const SwTableAutoFmt * pTAFmt, const std::vector<sal_uInt16> * pColArr, sal_Bool bCalledFromShell, sal_Bool bNewModel){
	const SwTable * out = 0;
	cout << "InsertTable(";
	cout << "rInsTblOpts";
	cout << ",";
	cout << "rPos";
	cout << ",";
	cout << nRows;
	cout << ",";
	cout << nCols;
	cout << ",";
	cout << eAdjust;
	cout << ",";
	cout << pTAFmt;
	cout << ",";
	cout << pColArr;
	cout << ",";
	cout << bCalledFromShell;
	cout << ",";
	cout << bNewModel;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwTableNode * SwDoc::IsIdxInTbl(const SwNodeIndex & rIdx){
	SwTableNode * out = 0;
	cout << "IsIdxInTbl(";
	cout << "rIdx";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTableNode * SwDoc::IsIdxInTbl(const SwNodeIndex & rIdx) const{
	const SwTableNode * out = 0;
	cout << "IsIdxInTbl(";
	cout << "rIdx";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTable * SwDoc::TextToTable(const SwInsertTableOptions & rInsTblOpts, const SwPaM & rRange, sal_Unicode cCh, short eAdjust, const SwTableAutoFmt * arg1){
	const SwTable * out = 0;
	cout << "TextToTable(";
	cout << "rInsTblOpts";
	cout << ",";
	cout << "rRange";
	cout << ",";
	cout << cCh;
	cout << ",";
	cout << eAdjust;
	cout << ",";
	cout << arg1;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwTable * SwDoc::TextToTable(const std::vector<std::vector<SwNodeRange>> & rTableNodes){
	const SwTable * out = 0;
	cout << "TextToTable(";
	cout << "rTableNodes";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::TableToText(const SwTableNode * pTblNd, sal_Unicode cCh){
	sal_Bool out = false;
	cout << "TableToText(";
	cout << pTblNd;
	cout << ",";
	cout << cCh;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::InsertCol(const SwCursor & rCursor, sal_uInt16 nCnt, sal_Bool bBehind){
	sal_Bool out = false;
	cout << "InsertCol(";
	cout << "rCursor";
	cout << ",";
	cout << nCnt;
	cout << ",";
	cout << bBehind;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::InsertCol(const SwSelBoxes & rBoxes, sal_uInt16 nCnt, sal_Bool bBehind){
	sal_Bool out = false;
	cout << "InsertCol(";
	cout << "rBoxes";
	cout << ",";
	cout << nCnt;
	cout << ",";
	cout << bBehind;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::InsertRow(const SwCursor & rCursor, sal_uInt16 nCnt, sal_Bool bBehind){
	sal_Bool out = false;
	cout << "InsertRow(";
	cout << "rCursor";
	cout << ",";
	cout << nCnt;
	cout << ",";
	cout << bBehind;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::InsertRow(const SwSelBoxes & rBoxes, sal_uInt16 nCnt, sal_Bool bBehind){
	sal_Bool out = false;
	cout << "InsertRow(";
	cout << "rBoxes";
	cout << ",";
	cout << nCnt;
	cout << ",";
	cout << bBehind;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::DeleteRowCol(const SwSelBoxes & rBoxes, bool bColumn){
	sal_Bool out = false;
	cout << "DeleteRowCol(";
	cout << "rBoxes";
	cout << ",";
	cout << bColumn;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::DeleteRow(const SwCursor & rCursor){
	sal_Bool out = false;
	cout << "DeleteRow(";
	cout << "rCursor";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::DeleteCol(const SwCursor & rCursor){
	sal_Bool out = false;
	cout << "DeleteCol(";
	cout << "rCursor";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::SplitTbl(const SwSelBoxes & rBoxes, sal_Bool bVert, sal_uInt16 nCnt, sal_Bool bSameHeight){
	sal_Bool out = false;
	cout << "SplitTbl(";
	cout << "rBoxes";
	cout << ",";
	cout << bVert;
	cout << ",";
	cout << nCnt;
	cout << ",";
	cout << bSameHeight;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::MergeTbl(SwPaM & rPam){
	sal_uInt16 out = 0;
	cout << "MergeTbl(";
	cout << "rPam";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

String SwDoc::GetUniqueTblName() const{
	String out;
	cout << "GetUniqueTblName(";
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

sal_Bool SwDoc::IsInsTblFormatNum() const{
	sal_Bool out = false;
	cout << "IsInsTblFormatNum(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::IsInsTblChangeNumFormat() const{
	sal_Bool out = false;
	cout << "IsInsTblChangeNumFormat(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::IsInsTblAlignNum() const{
	sal_Bool out = false;
	cout << "IsInsTblAlignNum(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::GetTabCols(SwTabCols & rFill, const SwCursor * pCrsr, const SwCellFrm * pBoxFrm) const{
	cout << "GetTabCols(";
	cout << "rFill";
	cout << ",";
	cout << pCrsr;
	cout << ",";
	cout << pBoxFrm;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetTabCols(const SwTabCols & rNew, sal_Bool bCurRowOnly, const SwCursor * pCrsr, const SwCellFrm * pBoxFrm){
	cout << "SetTabCols(";
	cout << "rNew";
	cout << ",";
	cout << bCurRowOnly;
	cout << ",";
	cout << pCrsr;
	cout << ",";
	cout << pBoxFrm;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::GetTabRows(SwTabCols & rFill, const SwCursor * pCrsr, const SwCellFrm * pBoxFrm) const{
	cout << "GetTabRows(";
	cout << "rFill";
	cout << ",";
	cout << pCrsr;
	cout << ",";
	cout << pBoxFrm;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetTabRows(const SwTabCols & rNew, sal_Bool bCurColOnly, const SwCursor * pCrsr, const SwCellFrm * pBoxFrm){
	cout << "SetTabRows(";
	cout << "rNew";
	cout << ",";
	cout << bCurColOnly;
	cout << ",";
	cout << pCrsr;
	cout << ",";
	cout << pBoxFrm;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetTabCols(SwTable & rTab, const SwTabCols & rNew, const SwTabCols & rOld, const SwTableBox * pStart, sal_Bool bCurRowOnly){
	cout << "SetTabCols(";
	cout << "rTab";
	cout << ",";
	cout << "rNew";
	cout << ",";
	cout << "rOld";
	cout << ",";
	cout << pStart;
	cout << ",";
	cout << bCurRowOnly;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetRowsToRepeat(SwTable & rTable, sal_uInt16 nSet){
	cout << "SetRowsToRepeat(";
	cout << "rTable";
	cout << ",";
	cout << nSet;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::SetTableAutoFmt(const SwSelBoxes & rBoxes, const SwTableAutoFmt & rNew){
	sal_Bool out = false;
	cout << "SetTableAutoFmt(";
	cout << "rBoxes";
	cout << ",";
	cout << "rNew";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::GetTableAutoFmt(const SwSelBoxes & rBoxes, SwTableAutoFmt & rGet){
	sal_Bool out = false;
	cout << "GetTableAutoFmt(";
	cout << "rBoxes";
	cout << ",";
	cout << "rGet";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::AppendUndoForInsertFromDB(const SwPaM & rPam, sal_Bool bIsTable){
	cout << "AppendUndoForInsertFromDB(";
	cout << "rPam";
	cout << ",";
	cout << bIsTable;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::SetColRowWidthHeight(SwTableBox & rAktBox, sal_uInt16 eType, SwTwips nAbsDiff, SwTwips nRelDiff){
	sal_Bool out = false;
	cout << "SetColRowWidthHeight(";
	cout << "rAktBox";
	cout << ",";
	cout << eType;
	cout << ",";
	cout << "nAbsDiff";
	cout << ",";
	cout << "nRelDiff";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwTableBoxFmt * SwDoc::MakeTableBoxFmt(){
	SwTableBoxFmt * out = 0;
	cout << "MakeTableBoxFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwTableLineFmt * SwDoc::MakeTableLineFmt(){
	SwTableLineFmt * out = 0;
	cout << "MakeTableLineFmt(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::ChkBoxNumFmt(SwTableBox & rAktBox, sal_Bool bCallUpdate){
	cout << "ChkBoxNumFmt(";
	cout << "rAktBox";
	cout << ",";
	cout << bCallUpdate;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetTblBoxFormulaAttrs(SwTableBox & rBox, const SfxItemSet & rSet){
	cout << "SetTblBoxFormulaAttrs(";
	cout << "rBox";
	cout << ",";
	cout << "rSet";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ClearBoxNumAttrs(const SwNodeIndex & rNode){
	cout << "ClearBoxNumAttrs(";
	cout << "rNode";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ClearLineNumAttrs(SwPosition & rPos){
	cout << "ClearLineNumAttrs(";
	cout << "rPos";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::InsCopyOfTbl(SwPosition & rInsPos, const SwSelBoxes & rBoxes, const SwTable * pCpyTbl, sal_Bool bCpyName, sal_Bool bCorrPos){
	sal_Bool out = false;
	cout << "InsCopyOfTbl(";
	cout << "rInsPos";
	cout << ",";
	cout << "rBoxes";
	cout << ",";
	cout << pCpyTbl;
	cout << ",";
	cout << bCpyName;
	cout << ",";
	cout << bCorrPos;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::UnProtectCells(const String & rTblName){
	sal_Bool out = false;
	cout << "UnProtectCells(";
	cout << rTblName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::UnProtectCells(const SwSelBoxes & rBoxes){
	sal_Bool out = false;
	cout << "UnProtectCells(";
	cout << "rBoxes";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::UnProtectTbls(const SwPaM & rPam){
	sal_Bool out = false;
	cout << "UnProtectTbls(";
	cout << "rPam";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::HasTblAnyProtection(const SwPosition * pPos, const String * pTblName, sal_Bool * pFullTblProtection){
	sal_Bool out = false;
	cout << "HasTblAnyProtection(";
	cout << pPos;
	cout << ",";
	cout << pTblName;
	cout << ",";
	cout << pFullTblProtection;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::SplitTable(const SwPosition & rPos, sal_uInt16 eMode, sal_Bool bCalcNewSize){
	sal_Bool out = false;
	cout << "SplitTable(";
	cout << "rPos";
	cout << ",";
	cout << eMode;
	cout << ",";
	cout << bCalcNewSize;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::MergeTable(const SwPosition & rPos, sal_Bool bWithPrev, sal_uInt16 nMode){
	sal_Bool out = false;
	cout << "MergeTable(";
	cout << "rPos";
	cout << ",";
	cout << bWithPrev;
	cout << ",";
	cout << nMode;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::UpdateCharts(const String & rName) const{
	cout << "UpdateCharts(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdateAllCharts(){
	cout << "UpdateAllCharts(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetTableName(SwFrmFmt & rTblFmt, const String & rNewName){
	cout << "SetTableName(";
	cout << "rTblFmt";
	cout << ",";
	cout << rNewName.GetBuffer();
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwFmtRefMark * SwDoc::GetRefMark(const String & rName) const{
	const SwFmtRefMark * out = 0;
	cout << "GetRefMark(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwFmtRefMark * SwDoc::GetRefMark(sal_uInt16 nIndex) const{
	const SwFmtRefMark * out = 0;
	cout << "GetRefMark(";
	cout << nIndex;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::GetRefMarks(std::vector<String> * arg1) const{
	sal_uInt16 out = 0;
	cout << "GetRefMarks(";
	cout << arg1;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFlyFrmFmt * SwDoc::InsertLabel(const SwLabelType eType, const String & rTxt, const String & rSeparator, const String & rNumberingSeparator, const sal_Bool bBefore, const sal_uInt16 nId, const sal_uLong nIdx, const String & rCharacterStyle, const sal_Bool bCpyBrd){
	SwFlyFrmFmt * out = 0;
	cout << "InsertLabel(";
	cout << "eType";
	cout << ",";
	cout << rTxt.GetBuffer();
	cout << ",";
	cout << rSeparator.GetBuffer();
	cout << ",";
	cout << rNumberingSeparator.GetBuffer();
	cout << ",";
	cout << bBefore;
	cout << ",";
	cout << nId;
	cout << ",";
	cout << nIdx;
	cout << ",";
	cout << rCharacterStyle.GetBuffer();
	cout << ",";
	cout << bCpyBrd;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwFlyFrmFmt * SwDoc::InsertDrawLabel(const String & rTxt, const String & rSeparator, const String & rNumberSeparator, const sal_uInt16 nId, const String & rCharacterStyle, SdrObject & rObj){
	SwFlyFrmFmt * out = 0;
	cout << "InsertDrawLabel(";
	cout << rTxt.GetBuffer();
	cout << ",";
	cout << rSeparator.GetBuffer();
	cout << ",";
	cout << rNumberSeparator.GetBuffer();
	cout << ",";
	cout << nId;
	cout << ",";
	cout << rCharacterStyle.GetBuffer();
	cout << ",";
	cout << "rObj";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwAttrPool & SwDoc::GetAttrPool() const{
	SwAttrPool* out = new SwAttrPool(const_cast<SwDoc*>(this));
	cout << "GetAttrPool(";
	cout << ") -> ";
	cout << "type: \"SwAttrPool\"";
	cout << "\n";
	return *out;
}

SwAttrPool & SwDoc::GetAttrPool(){
	SwAttrPool* out = new SwAttrPool(this);
	cout << "GetAttrPool(";
	cout << ") -> ";
	cout << "type: \"SwAttrPool\"";
	cout << "\n";
	return *out;
}

SwEditShell * SwDoc::GetEditShell(ViewShell * * ppSh) const{
	SwEditShell * out = 0;
	cout << "GetEditShell(";
	cout << ppSh;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

::sw::IShellCursorSupplier * SwDoc::GetIShellCursorSupplier(){
	::sw::IShellCursorSupplier * out = 0;
	cout << "GetIShellCursorSupplier(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetOle2Link(const Link & rLink){
	cout << "SetOle2Link(";
	cout << "rLink";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const Link & SwDoc::GetOle2Link() const{
	const Link out;
	cout << "GetOle2Link(";
	cout << ") -> ";
	cout << "type: \"Link\"";
	cout << "\n";
	return out;
}

SwSection * SwDoc::InsertSwSection(SwPaM const & rRange, SwSectionData & arg1, SwTOXBase const * const pTOXBase, SfxItemSet const * const pAttr, bool const bUpdate){
	SwSection * out = 0;
	cout << "InsertSwSection(";
	cout << "rRange";
	cout << ",";
	cout << "arg1";
	cout << ",";
	cout << pTOXBase;
	cout << ",";
	cout << pAttr;
	cout << ",";
	cout << bUpdate;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::IsInsRegionAvailable(const SwPaM & rRange, const SwNode * * ppSttNd) const{
	sal_uInt16 out = 0;
	cout << "IsInsRegionAvailable(";
	cout << "rRange";
	cout << ",";
	cout << ppSttNd;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwSection * SwDoc::GetCurrSection(const SwPosition & rPos) const{
	SwSection * out = 0;
	cout << "GetCurrSection(";
	cout << "rPos";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwSectionFmts & SwDoc::GetSections(){
	SwSectionFmts out;
	cout << "GetSections(";
	cout << ") -> ";
	cout << "type: \"SwSectionFmts\"";
	cout << "\n";
	return out;
}

const SwSectionFmts & SwDoc::GetSections() const{
	const SwSectionFmts out;
	cout << "GetSections(";
	cout << ") -> ";
	cout << "type: \"SwSectionFmts\"";
	cout << "\n";
	return out;
}

SwSectionFmt * SwDoc::MakeSectionFmt(SwSectionFmt * pDerivedFrom){
	SwSectionFmt * out = 0;
	cout << "MakeSectionFmt(";
	cout << pDerivedFrom;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DelSectionFmt(SwSectionFmt * pFmt, sal_Bool bDelNodes){
	cout << "DelSectionFmt(";
	cout << pFmt;
	cout << ",";
	cout << bDelNodes;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::UpdateSection(sal_uInt16 const nSect, SwSectionData & arg1, SfxItemSet const * const arg2, bool const bPreventLinkUpdate){
	cout << "UpdateSection(";
	cout << nSect;
	cout << ",";
	cout << "arg1";
	cout << ",";
	cout << arg2;
	cout << ",";
	cout << bPreventLinkUpdate;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

String SwDoc::GetUniqueSectionName(const String * pChkStr) const{
	String out;
	cout << "GetUniqueSectionName(";
	cout << pChkStr;
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

SwDocShell * SwDoc::GetDocShell(){
	SwDocShell * out = 0;
	cout << "GetDocShell(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwDocShell * SwDoc::GetDocShell() const{
	const SwDocShell * out = 0;
	cout << "GetDocShell(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetDocShell(SwDocShell * pDSh){
	cout << "SetDocShell(";
	cout << pDSh;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetTmpDocShell(SfxObjectShellLock rLock){
	cout << "SetTmpDocShell(";
	cout << "rLock";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SfxObjectShellLock SwDoc::GetTmpDocShell(){
	SfxObjectShellLock out;
	cout << "GetTmpDocShell(";
	cout << ") -> ";
	cout << "type: \"SfxObjectShellLock\"";
	cout << "\n";
	return out;
}

SfxObjectShell * SwDoc::GetPersist() const{
	SfxObjectShell * out = 0;
	cout << "GetPersist(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> SwDoc::GetDocStorage(){
	::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> out;
	cout << "GetDocStorage(";
	cout << ") -> ";
	cout << "type: \"::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>\"";
	cout << "\n";
	return out;
}

bool SwDoc::IsInLoadAsynchron() const{
	bool out = false;
	cout << "IsInLoadAsynchron(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetInLoadAsynchron(bool bFlag){
	cout << "SetInLoadAsynchron(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::SelectServerObj(const String & rStr, SwPaM * & rpPam, SwNodeRange * & rpRange) const{
	sal_Bool out = false;
	cout << "SelectServerObj(";
	cout << rStr.GetBuffer();
	cout << ",";
	cout << rpPam;
	cout << ",";
	cout << rpRange;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::IsCopyIsMove() const{
	bool out = false;
	cout << "IsCopyIsMove(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetCopyIsMove(bool bFlag){
	cout << "SetCopyIsMove(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwDrawContact * SwDoc::GroupSelection(SdrView & arg1){
	SwDrawContact * out = 0;
	cout << "GroupSelection(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::UnGroupSelection(SdrView & arg1){
	cout << "UnGroupSelection(";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::DeleteSelection(SwDrawView & arg1){
	sal_Bool out = false;
	cout << "DeleteSelection(";
	cout << "arg1";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SpellItAgainSam(sal_Bool bInvalid, sal_Bool bOnlyWrong, sal_Bool bSmartTags){
	cout << "SpellItAgainSam(";
	cout << bInvalid;
	cout << ",";
	cout << bOnlyWrong;
	cout << ",";
	cout << bSmartTags;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::InvalidateAutoCompleteFlag(){
	cout << "InvalidateAutoCompleteFlag(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetCalcFieldValueHdl(Outliner * pOutliner){
	cout << "SetCalcFieldValueHdl(";
	cout << pOutliner;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::IsVisitedURL(const String & rURL) const{
	sal_Bool out = false;
	cout << "IsVisitedURL(";
	cout << rURL.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetAutoCorrExceptWord(SwAutoCorrExceptWord * pNew){
	cout << "SetAutoCorrExceptWord(";
	cout << pNew;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwAutoCorrExceptWord * SwDoc::GetAutoCorrExceptWord(){
	SwAutoCorrExceptWord * out = 0;
	cout << "GetAutoCorrExceptWord(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SwFmtINetFmt * SwDoc::FindINetAttr(const String & rName) const{
	const SwFmtINetFmt * out = 0;
	cout << "FindINetAttr(";
	cout << rName.GetBuffer();
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::ExecMacro(const SvxMacro & rMacro, String * pRet, SbxArray * pArgs){
	sal_Bool out = false;
	cout << "ExecMacro(";
	cout << "rMacro";
	cout << ",";
	cout << pRet;
	cout << ",";
	cout << pArgs;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::CallEvent(sal_uInt16 nEvent, const SwCallMouseEvent & rCallEvent, sal_Bool bChkPtr, SbxArray * pArgs, const Link * pCallBack){
	sal_uInt16 out = 0;
	cout << "CallEvent(";
	cout << nEvent;
	cout << ",";
	cout << "rCallEvent";
	cout << ",";
	cout << bChkPtr;
	cout << ",";
	cout << pArgs;
	cout << ",";
	cout << pCallBack;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::MoveLeftMargin(const SwPaM & rPam, sal_Bool bRight, sal_Bool bModulus){
	cout << "MoveLeftMargin(";
	cout << "rPam";
	cout << ",";
	cout << bRight;
	cout << ",";
	cout << bModulus;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SvNumberFormatter * SwDoc::GetNumberFormatter(sal_Bool bCreate){
	SvNumberFormatter * out = 0;
	cout << "GetNumberFormatter(";
	cout << bCreate;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

const SvNumberFormatter * SwDoc::GetNumberFormatter(sal_Bool bCreate) const{
	const SvNumberFormatter * out = 0;
	cout << "GetNumberFormatter(";
	cout << bCreate;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::HasInvisibleContent() const{
	bool out = false;
	cout << "HasInvisibleContent(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::RemoveInvisibleContent(){
	bool out = false;
	cout << "RemoveInvisibleContent(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::RestoreInvisibleContent(){
	bool out = false;
	cout << "RestoreInvisibleContent(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::ConvertFieldsToText(){
	sal_Bool out = false;
	cout << "ConvertFieldsToText(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::GenerateGlobalDoc(const String & rPath, const SwTxtFmtColl * pSplitColl){
	sal_Bool out = false;
	cout << "GenerateGlobalDoc(";
	cout << rPath.GetBuffer();
	cout << ",";
	cout << pSplitColl;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::GenerateGlobalDoc(const String & rPath, int nOutlineLevel){
	sal_Bool out = false;
	cout << "GenerateGlobalDoc(";
	cout << rPath.GetBuffer();
	cout << ",";
	cout << nOutlineLevel;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::GenerateHTMLDoc(const String & rPath, const SwTxtFmtColl * pSplitColl){
	sal_Bool out = false;
	cout << "GenerateHTMLDoc(";
	cout << rPath.GetBuffer();
	cout << ",";
	cout << pSplitColl;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::GenerateHTMLDoc(const String & rPath, int nOutlineLevel){
	sal_Bool out = false;
	cout << "GenerateHTMLDoc(";
	cout << rPath.GetBuffer();
	cout << ",";
	cout << nOutlineLevel;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

long SwDoc::CompareDoc(const SwDoc & rDoc){
	long out = 0;
	cout << "CompareDoc(";
	cout << "rDoc";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

long SwDoc::MergeDoc(const SwDoc & rDoc){
	long out = 0;
	cout << "MergeDoc(";
	cout << "rDoc";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetAutoFmtRedlineComment(const String * pTxt, sal_uInt16 nSeqNo){
	cout << "SetAutoFmtRedlineComment(";
	cout << pTxt;
	cout << ",";
	cout << nSeqNo;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::IsAutoFmtRedline() const{
	bool out = false;
	cout << "IsAutoFmtRedline(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetAutoFmtRedline(bool bFlag){
	cout << "SetAutoFmtRedline(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetTxtFmtCollByAutoFmt(const SwPosition & rPos, sal_uInt16 nPoolId, const SfxItemSet * pSet){
	cout << "SetTxtFmtCollByAutoFmt(";
	cout << "rPos";
	cout << ",";
	cout << nPoolId;
	cout << ",";
	cout << pSet;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetFmtItemByAutoFmt(const SwPaM & rPam, const SfxItemSet & arg1){
	cout << "SetFmtItemByAutoFmt(";
	cout << "rPam";
	cout << ",";
	cout << "arg1";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ClearDoc(){
	cout << "ClearDoc(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwPagePreViewPrtData * SwDoc::GetPreViewPrtData() const{
	const SwPagePreViewPrtData * out = 0;
	cout << "GetPreViewPrtData(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetPreViewPrtData(const SwPagePreViewPrtData * pData){
	cout << "SetPreViewPrtData(";
	cout << pData;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetOLEObjModified(){
	cout << "SetOLEObjModified(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

const SwUnoCrsrTbl & SwDoc::GetUnoCrsrTbl() const{
	const SwUnoCrsrTbl out(0);
	cout << "GetUnoCrsrTbl(";
	cout << ") -> ";
	cout << "type: \"SwUnoCrsrTbl\"";
	cout << "\n";
	return out;
}

SwUnoCrsr * SwDoc::CreateUnoCrsr(const SwPosition & rPos, sal_Bool bTblCrsr){
	SwUnoCrsr * out = 0;
	cout << "CreateUnoCrsr(";
	cout << "rPos";
	cout << ",";
	cout << bTblCrsr;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_Bool SwDoc::ChgAnchor(const SdrMarkList & _rMrkList, RndStdIds _eAnchorType, const sal_Bool _bSameOnly, const sal_Bool _bPosCorr){
	sal_Bool out = false;
	cout << "ChgAnchor(";
	cout << "_rMrkList";
	cout << ",";
	cout << "_eAnchorType";
	cout << ",";
	cout << _bSameOnly;
	cout << ",";
	cout << _bPosCorr;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetRowHeight(const SwCursor & rCursor, const SwFmtFrmSize & rNew){
	cout << "SetRowHeight(";
	cout << "rCursor";
	cout << ",";
	cout << "rNew";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::GetRowHeight(const SwCursor & rCursor, SwFmtFrmSize * & rpSz) const{
	cout << "GetRowHeight(";
	cout << "rCursor";
	cout << ",";
	cout << rpSz;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetRowSplit(const SwCursor & rCursor, const SwFmtRowSplit & rNew){
	cout << "SetRowSplit(";
	cout << "rCursor";
	cout << ",";
	cout << "rNew";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::GetRowSplit(const SwCursor & rCursor, SwFmtRowSplit * & rpSz) const{
	cout << "GetRowSplit(";
	cout << "rCursor";
	cout << ",";
	cout << rpSz;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::BalanceRowHeight(const SwCursor & rCursor, sal_Bool bTstOnly){
	sal_Bool out = false;
	cout << "BalanceRowHeight(";
	cout << "rCursor";
	cout << ",";
	cout << bTstOnly;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetRowBackground(const SwCursor & rCursor, const SvxBrushItem & rNew){
	cout << "SetRowBackground(";
	cout << "rCursor";
	cout << ",";
	cout << "rNew";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::GetRowBackground(const SwCursor & rCursor, SvxBrushItem & rToFill) const{
	sal_Bool out = false;
	cout << "GetRowBackground(";
	cout << "rCursor";
	cout << ",";
	cout << "rToFill";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetTabBorders(const SwCursor & rCursor, const SfxItemSet & rSet){
	cout << "SetTabBorders(";
	cout << "rCursor";
	cout << ",";
	cout << "rSet";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetTabLineStyle(const SwCursor & rCursor, const Color * pColor, sal_Bool bSetLine, const editeng::SvxBorderLine * pBorderLine){
	cout << "SetTabLineStyle(";
	cout << "rCursor";
	cout << ",";
	cout << pColor;
	cout << ",";
	cout << bSetLine;
	cout << ",";
	cout << pBorderLine;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::GetTabBorders(const SwCursor & rCursor, SfxItemSet & rSet) const{
	cout << "GetTabBorders(";
	cout << "rCursor";
	cout << ",";
	cout << "rSet";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetBoxAttr(const SwCursor & rCursor, const SfxPoolItem & rNew){
	cout << "SetBoxAttr(";
	cout << "rCursor";
	cout << ",";
	cout << "rNew";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::GetBoxAttr(const SwCursor & rCursor, SfxPoolItem & rToFill) const{
	sal_Bool out = false;
	cout << "GetBoxAttr(";
	cout << "rCursor";
	cout << ",";
	cout << "rToFill";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetBoxAlign(const SwCursor & rCursor, sal_uInt16 nAlign){
	cout << "SetBoxAlign(";
	cout << "rCursor";
	cout << ",";
	cout << nAlign;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::GetBoxAlign(const SwCursor & rCursor) const{
	sal_uInt16 out = 0;
	cout << "GetBoxAlign(";
	cout << "rCursor";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::AdjustCellWidth(const SwCursor & rCursor, sal_Bool bBalance){
	cout << "AdjustCellWidth(";
	cout << "rCursor";
	cout << ",";
	cout << bBalance;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

int SwDoc::Chainable(const SwFrmFmt & rSource, const SwFrmFmt & rDest){
	int out = 0;
	cout << "Chainable(";
	cout << "rSource";
	cout << ",";
	cout << "rDest";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

int SwDoc::Chain(SwFrmFmt & rSource, const SwFrmFmt & rDest){
	int out = 0;
	cout << "Chain(";
	cout << "rSource";
	cout << ",";
	cout << "rDest";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::Unchain(SwFrmFmt & rFmt){
	cout << "Unchain(";
	cout << "rFmt";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SdrObject * SwDoc::CloneSdrObj(const SdrObject & arg1, sal_Bool bMoveWithinDoc, sal_Bool bInsInPage){
	SdrObject * out = 0;
	cout << "CloneSdrObj(";
	cout << "arg1";
	cout << ",";
	cout << bMoveWithinDoc;
	cout << ",";
	cout << bInsInPage;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwExtTextInput * SwDoc::CreateExtTextInput(const SwPaM & rPam){
	SwExtTextInput * out = 0;
	cout << "CreateExtTextInput(";
	cout << "rPam";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::DeleteExtTextInput(SwExtTextInput * pDel){
	cout << "DeleteExtTextInput(";
	cout << pDel;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwExtTextInput * SwDoc::GetExtTextInput(const SwNode & rNd, xub_StrLen nCntntPos) const{
	SwExtTextInput * out = 0;
	cout << "GetExtTextInput(";
	cout << "rNd";
	cout << ",";
	cout << "nCntntPos";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwExtTextInput * SwDoc::GetExtTextInput() const{
	SwExtTextInput * out = 0;
	cout << "GetExtTextInput(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::ContainsMSVBasic() const{
	bool out = false;
	cout << "ContainsMSVBasic(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetContainsMSVBasic(bool bFlag){
	cout << "SetContainsMSVBasic(";
	cout << bFlag;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt16 SwDoc::FillRubyList(const SwPaM & rPam, SwRubyList & rList, sal_uInt16 nMode){
	sal_uInt16 out = 0;
	cout << "FillRubyList(";
	cout << "rPam";
	cout << ",";
	cout << "rList";
	cout << ",";
	cout << nMode;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

sal_uInt16 SwDoc::SetRubyList(const SwPaM & rPam, const SwRubyList & rList, sal_uInt16 nMode){
	sal_uInt16 out = 0;
	cout << "SetRubyList(";
	cout << "rPam";
	cout << ",";
	cout << "rList";
	cout << ",";
	cout << nMode;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::ReadLayoutCache(SvStream & rStream){
	cout << "ReadLayoutCache(";
	cout << "rStream";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::WriteLayoutCache(SvStream & rStream){
	cout << "WriteLayoutCache(";
	cout << "rStream";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

SwLayoutCache * SwDoc::GetLayoutCache() const{
	SwLayoutCache * out = 0;
	cout << "GetLayoutCache(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

bool SwDoc::ContainsHiddenChars() const{
	bool out = false;
	cout << "ContainsHiddenChars(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

SwModify * SwDoc::GetUnoCallBack() const{
	SwModify * out = 0;
	cout << "GetUnoCallBack(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

IGrammarContact * SwDoc::getGrammarContact() const{
	IGrammarContact * out = 0;
	cout << "getGrammarContact(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::MarkListLevel(const String & sListId, const int nListLevel, const sal_Bool bValue){
	cout << "MarkListLevel(";
	cout << sListId.GetBuffer();
	cout << ",";
	cout << nListLevel;
	cout << ",";
	cout << bValue;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::MarkListLevel(SwList & rList, const int nListLevel, const sal_Bool bValue){
	cout << "MarkListLevel(";
	cout << "rList";
	cout << ",";
	cout << nListLevel;
	cout << ",";
	cout << bValue;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ChgFmt(SwFmt & rFmt, const SfxItemSet & rSet){
	cout << "ChgFmt(";
	cout << "rFmt";
	cout << ",";
	cout << "rSet";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::RenameFmt(SwFmt & rFmt, const String & sNewName, sal_Bool bBroadcast){
	cout << "RenameFmt(";
	cout << "rFmt";
	cout << ",";
	cout << sNewName.GetBuffer();
	cout << ",";
	cout << bBroadcast;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::ChgTOX(SwTOXBase & rTOX, const SwTOXBase & rNew){
	cout << "ChgTOX(";
	cout << "rTOX";
	cout << ",";
	cout << "rNew";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

String SwDoc::GetPaMDescr(const SwPaM & rPaM) const{
	String out;
	cout << "GetPaMDescr(";
	cout << "rPaM";
	cout << ") -> ";
	cout << out.GetBuffer();
	cout << "\n";
	return out;
}

sal_Bool SwDoc::IsFirstOfNumRule(SwPosition & rPos){
	sal_Bool out = false;
	cout << "IsFirstOfNumRule(";
	cout << "rPos";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> SwDoc::getXForms() const{
	com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> out;
	cout << "getXForms(";
	cout << ") -> ";
	cout << "type: \"com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>\"";
	cout << "\n";
	return out;
}

com::sun::star::uno::Reference<com::sun::star::linguistic2::XProofreadingIterator> SwDoc::GetGCIterator() const{
	com::sun::star::uno::Reference<com::sun::star::linguistic2::XProofreadingIterator> out;
	cout << "GetGCIterator(";
	cout << ") -> ";
	cout << "type: \"com::sun::star::uno::Reference<com::sun::star::linguistic2::XProofreadingIterator>\"";
	cout << "\n";
	return out;
}

bool SwDoc::isXForms() const{
	bool out = false;
	cout << "isXForms(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::initXForms(bool bCreateDefaultModel){
	cout << "initXForms(";
	cout << bCreateDefaultModel;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::ApplyWorkaroundForB6375613() const{
	bool out = false;
	cout << "ApplyWorkaroundForB6375613(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::SetApplyWorkaroundForB6375613(bool p_bApplyWorkaroundForB6375613){
	cout << "SetApplyWorkaroundForB6375613(";
	cout << p_bApplyWorkaroundForB6375613;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetDefaultPageMode(bool bSquaredPageMode){
	cout << "SetDefaultPageMode(";
	cout << bSquaredPageMode;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_Bool SwDoc::IsSquaredPageMode() const{
	sal_Bool out = false;
	cout << "IsSquaredPageMode(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::Setn32DummyCompatabilityOptions1(sal_uInt32 CompatabilityOptions1){
	cout << "Setn32DummyCompatabilityOptions1(";
	cout << CompatabilityOptions1;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt32 SwDoc::Getn32DummyCompatabilityOptions1(){
	sal_uInt32 out = 0;
	cout << "Getn32DummyCompatabilityOptions1(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::Setn32DummyCompatabilityOptions2(sal_uInt32 CompatabilityOptions2){
	cout << "Setn32DummyCompatabilityOptions2(";
	cout << CompatabilityOptions2;
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

sal_uInt32 SwDoc::Getn32DummyCompatabilityOptions2(){
	sal_uInt32 out = 0;
	cout << "Getn32DummyCompatabilityOptions2(";
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

com::sun::star::uno::Reference<com::sun::star::script::vba::XVBAEventProcessor> SwDoc::GetVbaEventProcessor(){
	com::sun::star::uno::Reference<com::sun::star::script::vba::XVBAEventProcessor> out;
	cout << "GetVbaEventProcessor(";
	cout << ") -> ";
	cout << "type: \"com::sun::star::uno::Reference<com::sun::star::script::vba::XVBAEventProcessor>\"";
	cout << "\n";
	return out;
}

void SwDoc::SetVBATemplateToProjectCache(com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> & xCache){
	cout << "SetVBATemplateToProjectCache(";
	cout << "xCache";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> SwDoc::GetVBATemplateToProjectCache(){
	com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> out;
	cout << "GetVBATemplateToProjectCache(";
	cout << ") -> ";
	cout << "type: \"com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>\"";
	cout << "\n";
	return out;
}

::sfx2::IXmlIdRegistry & SwDoc::GetXmlIdRegistry(){
    sfx2::IXmlIdRegistry* out = sfx2::createXmlIdRegistry(false);
    cout << "GetXmlIdRegistry(";
	cout << ") -> ";
	cout << "type: \"::sfx2::IXmlIdRegistry\"";
	cout << "\n";
	return *out;
}

::sw::MetaFieldManager & SwDoc::GetMetaFieldManager(){
	::sw::MetaFieldManager out;
	cout << "GetMetaFieldManager(";
	cout << ") -> ";
	cout << "type: \"::sw::MetaFieldManager\"";
	cout << "\n";
	return out;
}

::sw::UndoManager & SwDoc::GetUndoManager(){
    sw::UndoManager out(::std::auto_ptr<SwNodes>(new SwNodes(this)),static_cast<IDocumentDrawModelAccess&>(*this),static_cast<IDocumentRedlineAccess&>(*this),static_cast<IDocumentState&>(*this));
	cout << "GetUndoManager(";
	cout << ") -> ";
	cout << "type: \"::sw::UndoManager\"";
	cout << "\n";
	return out;
}

::sw::UndoManager const & SwDoc::GetUndoManager() const{
    sw::UndoManager out(::std::auto_ptr<SwNodes>(new SwNodes(const_cast<SwDoc*>(this))),static_cast<IDocumentDrawModelAccess&>(*const_cast<SwDoc*>(this)),static_cast<IDocumentRedlineAccess&>(*const_cast<SwDoc*>(this)),static_cast<IDocumentState&>(*const_cast<SwDoc*>(this)));
	cout << "GetUndoManager(";
	cout << ") -> ";
	cout << "type: \"::sw::UndoManager\"";
	cout << "\n";
	return out;
}

SfxObjectShell * SwDoc::CreateCopy(bool bCallInitNew) const{
	SfxObjectShell * out = 0;
	cout << "CreateCopy(";
	cout << bCallInitNew;
	cout << ") -> ";
	cout << out;
	cout << "\n";
	return out;
}

void SwDoc::dumpAsXml(xmlTextWriterPtr writer){
	cout << "dumpAsXml(";
	cout << "writer";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

void SwDoc::SetDrawDefaults(){
	cout << "SetDrawDefaults(";
	cout << ") -> ";
	cout << "type: \"void\"";
	cout << "\n";
	return;
}

bool SwDoc::InsertString(const SwPaM&, const String&, IDocumentContentOperations::InsertFlags)
{
    return false;
}

IMPL_LINK(SwDoc, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    return 0;
}

IMPL_STATIC_LINK( SwDoc, BackgroundDone, SvxBrushItem*, EMPTYARG )
{
    return 0;
}

IMPL_LINK( SwDoc, AddDrawUndo, SdrUndoAction *, pUndo )
{
    return 0;
}

struct _PostItFld : public _SetGetExpFld
{
    _PostItFld( const SwNodeIndex& rNdIdx, const SwTxtFld* pFld,  const SwIndex* pIdx = 0 )
        : _SetGetExpFld( rNdIdx, pFld, pIdx ) {}

    sal_uInt16 GetPageNo( const StringRangeEnumerator &rRangeEnum,
            const std::set< sal_Int32 > &rPossiblePages,
            sal_uInt16& rVirtPgNo, sal_uInt16& rLineNo );

    SwPostItField* GetPostIt() const
    {
        return (SwPostItField*) GetFld()->GetFld().GetFld();
    }
};

bool lcl_GetPostIts(
    IDocumentFieldsAccess* pIDFA,
    _SetGetExpFlds * pSrtLst )
{
    bool bHasPostIts = false;

    SwFieldType* pFldType = pIDFA->GetSysFldType( RES_POSTITFLD );
    OSL_ENSURE( pFldType, "kein PostItType ? ");

    if( pFldType->GetDepends() )
    {
        // Found modify object; insert all fields into the array
        SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
        const SwTxtFld* pTxtFld;
        for( SwFmtFld* pFld = aIter.First(); pFld;  pFld = aIter.Next() )
        {
            if( 0 != ( pTxtFld = pFld->GetTxtFld() ) &&
                pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                bHasPostIts = true;
                if (pSrtLst)
                {
                    SwNodeIndex aIdx( pTxtFld->GetTxtNode() );
                    _PostItFld* pNew = new _PostItFld( aIdx, pTxtFld );
                    pSrtLst->Insert( pNew );
                }
                else
                    break;  // we just wanted to check for the existence of postits ...
            }
        }
    }

    return bHasPostIts;
}
