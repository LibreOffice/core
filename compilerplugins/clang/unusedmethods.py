#!/usr/bin/python

import sys

definitionSet = set()
callSet = set()
# things we need to exclude for reasons like :
# - it's a weird template thingy that confuses the plugin
exclusionSet = set([
    "double basegfx::DoubleTraits::maxVal()",
    "double basegfx::DoubleTraits::minVal()",
    "double basegfx::DoubleTraits::neutral()",
    "int basegfx::Int32Traits::maxVal()",
    "int basegfx::Int32Traits::minVal()",
    "int basegfx::Int32Traits::neutral()",
    "unsigned long UniqueIndexImpl::Insert(void *)",

    # used from a yacc lexer
    "struct unoidl::detail::SourceProviderExpr unoidl::detail::SourceProviderExpr::Float(double)",
    "struct unoidl::detail::SourceProviderExpr unoidl::detail::SourceProviderExpr::Int(long)",
    "struct unoidl::detail::SourceProviderExpr unoidl::detail::SourceProviderExpr::Uint(unsigned long)",
    "class rtl::OUString unoidl::detail::SourceProviderType::getName() const",
    "_Bool unoidl::detail::SourceProviderType::equals(const struct unoidl::detail::SourceProviderType &) const",
    "_Bool unoidl::detail::SourceProviderEntityPad::isPublished() const",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::checkMemberClashes(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &,const class rtl::OUString &,_Bool) const",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::checkBaseClashes(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &,const class rtl::Reference<class unoidl::InterfaceTypeEntity> &,_Bool,_Bool,_Bool,class std::__debug::set<class rtl::OUString, struct std::less<class rtl::OUString>, class std::allocator<class rtl::OUString> > *) const",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::addDirectBase(int,void *,struct unoidl::detail::SourceProviderScannerData *,const struct unoidl::detail::SourceProviderInterfaceTypeEntityPad::DirectBase &,_Bool)",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::addBase(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &,const class rtl::OUString &,const class rtl::Reference<class unoidl::InterfaceTypeEntity> &,_Bool,_Bool)",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::addDirectMember(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &)",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::addOptionalBaseMembers(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &,const class rtl::Reference<class unoidl::InterfaceTypeEntity> &)",
    "void unoidl::detail::SourceProviderScannerData::setSource(const void *,unsigned long)",
    "void Idlc::setParseState(enum ParseState)",
    "_Bool AstDeclaration::isPredefined()",
    "void Idlc::setOffset(unsigned int,unsigned int)",
    "_Bool Idlc::isInMainFile()",
    "void Idlc::setPublished(_Bool)",
    "enum ParseState Idlc::getParseState()",
    "void Idlc::setLineNumber(unsigned int)",
    "void Idlc::incLineNumber()",
    "void Idlc::setDocumentation(const class rtl::OString &)",
    "void Idlc::setInMainfile(_Bool)",
    "void AstInterface::forwardDefined(const class AstInterface &)",
    "_Bool AstInterface::hasMandatoryInheritedInterfaces() const",
    "_Bool AstInterface::usesSingleInheritance() const",
    "struct AstInterface::DoubleDeclarations AstInterface::checkInheritedInterfaceClashes(const class AstInterface *,_Bool) const",
    "class std::__debug::vector<struct AstInterface::DoubleMemberDeclaration, class std::allocator<struct AstInterface::DoubleMemberDeclaration> > AstInterface::checkMemberClashes(const class AstDeclaration *) const",
    "void AstOperation::setExceptions(const class std::__debug::list<class AstDeclaration *, class std::allocator<class AstDeclaration *> > *)",
    "_Bool AstOperation::isConstructor() const",
    "_Bool AstService::isSingleInterfaceBasedService() const",
    "void AstService::setSingleInterfaceBasedService()",
    "void AstService::setDefaultConstructor(_Bool)",
    "_Bool AstService::checkLastConstructor() const",
    "class AstScope * AstStack::nextToTop()",
    "class AstScope * AstStack::bottom()",
    "const class AstDeclaration * AstStruct::findTypeParameter(const class rtl::OString &) const",
    "int AstEnum::getEnumValueCount()",
    "class AstConstant * AstEnum::checkValue(class AstExpression *)",
    "void ErrorHandler::flagError(enum ErrorCode,unsigned int)",
    "void ErrorHandler::error0(enum ErrorCode)",
    "void ErrorHandler::syntaxError(enum ParseState,int,const char *)",
    "void ErrorHandler::lookupError(enum ErrorCode,const class rtl::OString &,class AstDeclaration *)",
    "void ErrorHandler::error3(enum ErrorCode,class AstDeclaration *,class AstDeclaration *,class AstDeclaration *)",
    "void ErrorHandler::warning0(enum WarningCode,const char *)",
    "void ErrorHandler::coercionError(class AstExpression *,enum ExprType)",
    "class AstDeclaration * FeInheritanceHeader::getInherits()",
    "const class std::__debug::vector<class rtl::OString, class std::allocator<class rtl::OString> > & FeInheritanceHeader::getTypeParameters() const",
    "const class AstType * FeDeclarator::compose(const class AstDeclaration *)",
    "const class rtl::OString & FeDeclarator::getName()",
    "_Bool FeDeclarator::checkType(const class AstDeclaration *)",
    "enum connectivity::IParseContext::InternationalKeyCode connectivity::IParseContext::getIntlKeyCode(const class rtl::OString &) const",
    "void connectivity::OSQLParser::error(const char *)",
    "void connectivity::OSQLParseNode::insert(unsigned int,class connectivity::OSQLParseNode *)",

    # TODO track instantiations of template class constructors
    "void comphelper::IEventProcessor::release()",
    "void SotMutexHolder::acquire()",
    "void SotMutexHolder::release()",

    # only used by Windows build
    "_Bool basegfx::B2ITuple::equalZero() const",
    "class basegfx::B2DPolyPolygon basegfx::unotools::UnoPolyPolygon::getPolyPolygonUnsafe() const",
    "void basegfx::B2IRange::expand(const class basegfx::B2IRange &)",
    "void OpenGLContext::requestSingleBufferedRendering()",
	"_Bool TabitemValue::isBothAligned() const",
	"_Bool TabitemValue::isNotAligned() const",
	"void StyleSettings::SetSpinSize(long)",
	"void StyleSettings::SetFloatTitleHeight(long)",
    "void StyleSettings::SetTitleHeight(long)",
    "void StyleSettings::SetUseFlatBorders(_Bool)",
    "void StyleSettings::SetUseFlatMenus(_Bool)",
    "void StyleSettings::SetCursorSize(long)",
    "_Bool CommandMediaData::GetPassThroughToOS() const",
    "void Application::AppEvent(const class ApplicationEvent &)",
    "int PhysicalFontFace::GetWidth() const",
    "void PhysicalFontFace::SetBitmapSize(int,int)",
    "class boost::intrusive_ptr<class FontCharMap> FontCharMap::GetDefaultMap(_Bool)",
    "_Bool SalObject::IsEraseBackgroundEnabled()",
    "const class rtl::OUString & connectivity::OColumn::getCatalogName() const",
    "const class rtl::OUString & connectivity::OColumn::getSchemaName() const",
    "_Bool connectivity::OColumn::isDefinitelyWritable() const",
    "_Bool connectivity::OColumn::isReadOnly() const",
    "_Bool connectivity::OColumn::isWritable() const",
    "_Bool IDocumentLinksAdministration::GetData(const class rtl::OUString &,const class rtl::OUString &,class com::sun::star::uno::Any &) const",
    "_Bool IDocumentLinksAdministration::SetData(const class rtl::OUString &,const class rtl::OUString &,const class com::sun::star::uno::Any &)",
    "_Bool ScImportExport::ImportData(const class rtl::OUString &,const class com::sun::star::uno::Any &)",
	"void* ScannerManager::GetData()",
	"void ScannerManager::SetData(void *)",
    # instantiated from templates, not sure why it is not being picked up
    "class basegfx::B2DPolygon OutputDevice::PixelToLogic(const class basegfx::B2DPolygon &,const class MapMode &) const",
    "type-parameter-0-0 * detail::cloner::clone(type-parameter-0-0 *const)",
    "const class rtl::OUString writerperfect::DocumentHandlerFor::name()",
    # only used by OSX build
    "void StyleSettings::SetHideDisabledMenuItems(_Bool)",
    # debugging methods
    "void oox::drawingml::TextParagraphProperties::dump() const",
    "void oox::PropertyMap::dumpCode(class com::sun::star::uno::Reference<class com::sun::star::beans::XPropertySet>)",
    "void oox::PropertyMap::dumpData(class com::sun::star::uno::Reference<class com::sun::star::beans::XPropertySet>)",
    "class std::basic_string<char, struct std::char_traits<char>, class std::allocator<char> > writerfilter::ooxml::OOXMLPropertySet::toString()",
    # called from the writerfilter generated code in $WORKDIR, not sure why it is not being picked up
    "_Bool writerfilter::ooxml::OOXMLFactory_ns::getElementId(unsigned int,unsigned int,enum writerfilter::ooxml::ResourceType_t &,unsigned int &)",
    "void writerfilter::ooxml::OOXMLFastContextHandler::setDefaultIntegerValue()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::positivePercentage(const class rtl::OUString &)",
    "void writerfilter::ooxml::OOXMLFastContextHandler::positionOffset(const class rtl::OUString &)",
    "void writerfilter::ooxml::OOXMLFastContextHandler::alignH(const class rtl::OUString &)",
    "void writerfilter::ooxml::OOXMLFastContextHandler::alignV(const class rtl::OUString &)",
    "void writerfilter::ooxml::OOXMLFastContextHandler::setDefaultStringValue()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::clearProps()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::sendPropertiesWithId(const unsigned int &)",
    "void writerfilter::ooxml::OOXMLFastContextHandler::cr()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::softHyphen()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::noBreakHyphen()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::setDefaultBooleanValue()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::endField()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::propagateCharacterPropertiesAsSet(const unsigned int &)",
    "void writerfilter::ooxml::OOXMLFastContextHandler::tab()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::ftnednref()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::ftnednsep()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::ftnedncont()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::pgNum()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::sendRowProperties()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::setLastSectionGroup()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::endTxbxContent()",
    "void writerfilter::ooxml::OOXMLFastContextHandler::setDefaultHexValue()",
    "void writerfilter::ooxml::OOXMLFastContextHandlerProperties::handleComment()",
    "void writerfilter::ooxml::OOXMLFastContextHandlerProperties::handleOLE()",
    "void writerfilter::ooxml::OOXMLFastContextHandlerProperties::handlePicture()",
    "void writerfilter::ooxml::OOXMLFastContextHandlerProperties::handleHdrFtr()",
    "void writerfilter::ooxml::OOXMLFastContextHandlerProperties::handleBreak()",
    # deep template magic in SW
    "Ring<value_type> * sw::Ring::Ring_node_traits::get_next(const Ring<value_type> *)",
    "Ring<value_type> * sw::Ring::Ring_node_traits::get_previous(const Ring<value_type> *)",
    "void sw::Ring::Ring_node_traits::set_next(Ring<value_type> *,Ring<value_type> *)",
    "void sw::Ring::Ring_node_traits::set_previous(Ring<value_type> *,Ring<value_type> *)",
    # I need to teach the plugin that for loops with range expressions call begin() and end()
    "class __gnu_debug::_Safe_iterator<class __gnu_cxx::__normal_iterator<class SwAnchoredObject *const *, class std::__cxx1998::vector<class SwAnchoredObject *, class std::allocator<class SwAnchoredObject *> > >, class std::__debug::vector<class SwAnchoredObject *, class std::allocator<class SwAnchoredObject *> > > SwSortedObjs::begin() const",
    "class __gnu_debug::_Safe_iterator<class __gnu_cxx::__normal_iterator<class SwAnchoredObject *const *, class std::__cxx1998::vector<class SwAnchoredObject *, class std::allocator<class SwAnchoredObject *> > >, class std::__debug::vector<class SwAnchoredObject *, class std::allocator<class SwAnchoredObject *> > > SwSortedObjs::end() const",
    ])


# The parsing here is designed to also grab output which is mixed into the output from gbuild.
# I have not yet found a way of suppressing the gbuild output.
with open(sys.argv[1]) as txt:
    for line in txt:
        if line.find("definition:\t") != -1:
            idx1 = line.find("definition:\t")
            idx2 = line.find("\t", idx1+12)
            clazzName = line[idx1+12 : idx2]
            definitionSet.add(clazzName)
        elif line.find("call:\t") != -1:
            idx1 = line.find("call:\t")
            idx2 = line.find("\t", idx1+6)
            clazzName = line[idx1+6 : idx2]
            callSet.add(clazzName)

for clazz in sorted(definitionSet - callSet - exclusionSet):
    # ignore operators, they are normally called from inside STL code
    if (clazz.find("::operator") != -1):
        continue
    # ignore the custom RTTI stuff
    if (    (clazz.find("::CreateType()") != -1)
         or (clazz.find("::IsA(") != -1)
         or (clazz.find("::Type()") != -1)):
        continue
    # if this method is const, and there is a non-const variant of it, and the non-const variant is in use, then leave it alone
    if (clazz.startswith("const ") and clazz.endswith(" const")):
        clazz2 = clazz[6:len(clazz)-6]
        if (clazz2 in callSet):
           continue
    elif (clazz.endswith(" const")):
        clazz2 = clazz[:len(clazz)-6] # strip off " const"
        if (clazz2 in callSet):
           continue
    if (clazz.endswith(" const") and clazz.find("::iterator") != -1):
        clazz2 = clazz[:len(clazz)-6] # strip off " const"
        clazz2 = clazz2.replace("::const_iterator", "::iterator")
        if (clazz2 in callSet):
           continue
    # if this method is non-const, and there is a const variant of it, and the const variant is in use, then leave it alone
    if ((not clazz.endswith(" const")) and ("const " + clazz + " const") in callSet):
       continue
    if ((not clazz.endswith(" const")) and clazz.find("::iterator") != -1):
        clazz2 = clazz.replace("::iterator", "::const_iterator") + " const"
        if (clazz2 in callSet):
           continue
    # There is lots of macro magic going on in /home/noel/libo4/include/sax/fshelper.hxx that should be using C++11 varag templates
    if clazz.startswith("void sax_fastparser::FastSerializerHelper::"):
       continue
    # used by Windows build
    if (clazz.find("DdeTopic::") != -1 
        or clazz.find("DdeData::") != -1
        or clazz.find("DdeService::") != -1
        or clazz.find("DdeTransaction::") != -1
        or clazz.find("DdeConnection::") != -1
        or clazz.find("DdeLink::") != -1
        or clazz.find("DdeItem::") != -1
        or clazz.find("DdeGetPutItem::") != -1):
       continue
    # the include/tools/rtti.hxx stuff
    if (clazz.find("::StaticType()") != -1
        or clazz.find("::IsOf(void *(*)(void))") != -1):
       continue
    print clazz

# add an empty line at the end to make it easier for the unusedmethodsremove plugin to mmap() the output file 
print
        

