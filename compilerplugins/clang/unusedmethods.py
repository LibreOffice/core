#!/usr/bin/python

import sys
import re
import io

# --------------------------------------------------------------------------------------------
# globals
# --------------------------------------------------------------------------------------------

definitionSet = set() # set of tuple(return_type, name_and_params)
definitionToSourceLocationMap = dict()

# for the "unused methods" analysis
callSet = set() # set of tuple(return_type, name_and_params)

# for the "unnecessary public" analysis
publicDefinitionSet = set() # set of tuple(return_type, name_and_params)
calledFromOutsideSet = set() # set of tuple(return_type, name_and_params)

# for the "unused return types" analysis
usedReturnSet = set() # set of tuple(return_type, name_and_params)


# things we need to exclude for reasons like :
# - it's a weird template thingy that confuses the plugin
unusedMethodsExclusionSet = set([
    "double basegfx::DoubleTraits::maxVal()",
    "double basegfx::DoubleTraits::minVal()",
    "double basegfx::DoubleTraits::neutral()",
    "int basegfx::Int32Traits::maxVal()",
    "int basegfx::Int32Traits::minVal()",
    "int basegfx::Int32Traits::neutral()",
    "unsigned long UniqueIndexImpl::Insert(void *)",
    "class XMLPropertyBackpatcher<short> & XMLTextImportHelper::GetFootnoteBP()",
    "class XMLPropertyBackpatcher<short> & XMLTextImportHelper::GetSequenceIdBP()",
    "void XclExpPivotCache::SaveXml(class XclExpXmlStream &)",


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
	"void TabitemValue::isLast() const",
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
    # deep template magic in SW
    "Ring<value_type> * sw::Ring::Ring_node_traits::get_next(const Ring<value_type> *)",
    "Ring<value_type> * sw::Ring::Ring_node_traits::get_previous(const Ring<value_type> *)",
    "void sw::Ring::Ring_node_traits::set_next(Ring<value_type> *,Ring<value_type> *)",
    "void sw::Ring::Ring_node_traits::set_previous(Ring<value_type> *,Ring<value_type> *)",
    "type-parameter-0-0 checking_cast(type-parameter-0-0,type-parameter-0-0)",
    # I need to teach the plugin that for loops with range expressions call begin() and end()
    "class __gnu_debug::_Safe_iterator<class __gnu_cxx::__normal_iterator<class SwAnchoredObject *const *, class std::__cxx1998::vector<class SwAnchoredObject *, class std::allocator<class SwAnchoredObject *> > >, class std::__debug::vector<class SwAnchoredObject *, class std::allocator<class SwAnchoredObject *> > > SwSortedObjs::begin() const",
    "class __gnu_debug::_Safe_iterator<class __gnu_cxx::__normal_iterator<class SwAnchoredObject *const *, class std::__cxx1998::vector<class SwAnchoredObject *, class std::allocator<class SwAnchoredObject *> > >, class std::__debug::vector<class SwAnchoredObject *, class std::allocator<class SwAnchoredObject *> > > SwSortedObjs::end() const",
    # loaded by dlopen()
    "void * getStandardAccessibleFactory()",
    "void * getSvtAccessibilityComponentFactory()",
    "struct _rtl_uString * basicide_choose_macro(void *,unsigned char,struct _rtl_uString *)",
    "void basicide_macro_organizer(short)",
    "long basicide_handle_basic_error(void *)",
    "class com::sun::star::uno::XInterface * org_libreoffice_chart2_Chart2ToolboxController(class com::sun::star::uno::XComponentContext *,const class com::sun::star::uno::Sequence<class com::sun::star::uno::Any> &)",
    "class com::sun::star::uno::XInterface * org_libreoffice_comp_chart2_sidebar_ChartPanelFactory(class com::sun::star::uno::XComponentContext *,const class com::sun::star::uno::Sequence<class com::sun::star::uno::Any> &)",
    "class chart::opengl::OpenglShapeFactory * getOpenglShapeFactory()",
    "class VclAbstractDialogFactory * CreateDialogFactory()",
    "_Bool GetSpecialCharsForEdit(class vcl::Window *,const class vcl::Font &,class rtl::OUString &)",
    "const struct ImplTextEncodingData * sal_getFullTextEncodingData(unsigned short)",
    "class SalInstance * create_SalInstance()",
    "class SwAbstractDialogFactory * SwCreateDialogFactory()",
    "class com::sun::star::uno::Reference<class com::sun::star::uno::XInterface> WordPerfectImportFilterDialog_createInstance(const class com::sun::star::uno::Reference<class com::sun::star::uno::XComponentContext> &)",
    "class UnoWrapperBase * CreateUnoWrapper()",
    "class SwAbstractDialogFactory * SwCreateDialogFactory()",
    "unsigned long GetSaveWarningOfMSVBAStorage_ww8(class SfxObjectShell &)",
    "unsigned long SaveOrDelMSVBAStorage_ww8(class SfxObjectShell &,class SotStorage &,unsigned char,const class rtl::OUString &)",
    "void ExportRTF(const class rtl::OUString &,const class rtl::OUString &,class tools::SvRef<class Writer> &)",
    "void ExportDOC(const class rtl::OUString &,const class rtl::OUString &,class tools::SvRef<class Writer> &)",
    "class Reader * ImportRTF()",
    "void ImportXE(class SwDoc &,class SwPaM &,const class rtl::OUString &)",
    "_Bool TestImportDOC(const class rtl::OUString &,const class rtl::OUString &)",
    "class vcl::Window * CreateWindow(class VCLXWindow **,const struct com::sun::star::awt::WindowDescriptor *,class vcl::Window *,long)",
    # only used when the ODBC driver is enabled
    "_Bool getImplementation(type-parameter-?-? *&,const class com::sun::star::uno::Reference<class com::sun::star::uno::XInterface> &)",
    ])

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# --------------------------------------------------------------------------------------------
# primary input loop
# --------------------------------------------------------------------------------------------

# The parsing here is designed to avoid grabbing stuff which is mixed in from gbuild.
# I have not yet found a way of suppressing the gbuild output.
with io.open("loplugin.unusedmethods.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        if line.startswith("definition:\t"):
            idx1 = line.find("\t",12)
            idx2 = line.find("\t",idx1+1)
            idx3 = line.find("\t",idx2+1)
            access = line[12:idx1]
            returnType = line[idx1+1:idx2]
            nameAndParams = line[idx2+1:idx3]
            sourceLocation = line[idx3+1:].strip()
            funcInfo = (normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams))
            definitionSet.add(funcInfo)
            if access == "public":
                publicDefinitionSet.add(funcInfo)
            definitionToSourceLocationMap[funcInfo] = sourceLocation
        elif line.startswith("call:\t"):
            idx1 = line.find("\t",6)
            returnType = line[6:idx1]
            nameAndParams = line[idx1+1:].strip()
            callSet.add((normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams)))
        elif line.startswith("usedReturn:\t"):
            idx1 = line.find("\t",12)
            returnType = line[12:idx1]
            nameAndParams = line[idx1+1:].strip()
            usedReturnSet.add((normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams)))
        elif line.startswith("calledFromOutsideSet:\t"):
            idx1 = line.find("\t",22)
            returnType = line[22:idx1]
            nameAndParams = line[idx1+1:].strip()
            calledFromOutsideSet.add((normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams)))

# Invert the definitionToSourceLocationMap.
# If we see more than one method at the same sourceLocation, it's being autogenerated as part of a template
# and we should just ignore it.
sourceLocationToDefinitionMap = {}
for k, v in definitionToSourceLocationMap.iteritems():
    sourceLocationToDefinitionMap[v] = sourceLocationToDefinitionMap.get(v, [])
    sourceLocationToDefinitionMap[v].append(k)
for k, definitions in sourceLocationToDefinitionMap.iteritems():
    if len(definitions) > 1:
        for d in definitions:
            definitionSet.remove(d)

def isOtherConstness( d, callSet ):
    method = d[0] + " " + d[1]
    # if this method is const, and there is a non-const variant of it, and the non-const variant is in use, then leave it alone
    if d[0].startswith("const ") and d[1].endswith(" const"):
        if ((d[0][6:],d[1][:-6]) in callSet):
           return True
    elif method.endswith(" const"):
        method2 = method[:len(method)-6] # strip off " const"
        if ((d[0],method2) in callSet):
           return True
    if method.endswith(" const") and ("::iterator" in method):
        method2 = method[:len(method)-6] # strip off " const"
        method2 = method2.replace("::const_iterator", "::iterator")
        if ((d[0],method2) in callSet):
           return True
    # if this method is non-const, and there is a const variant of it, and the const variant is in use, then leave it alone
    if (not method.endswith(" const")) and ((d[0],"const " + method + " const") in callSet):
           return True
    if (not method.endswith(" const")) and ("::iterator" in method):
        method2 = method.replace("::iterator", "::const_iterator") + " const"
        if ((d[0],method2) in callSet):
           return True
    return False

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
def sort_set_by_natural_key(s):
    return sorted(s, key=lambda v: natural_sort_key(v[1]))

    
# --------------------------------------------------------------------------------------------
#  "unused methods" analysis
# --------------------------------------------------------------------------------------------

tmp1set = set() # set of tuple(method, source_location)
unusedSet = set() # set of tuple(return_type, name_and_params)
for d in definitionSet:
    method = d[0] + " " + d[1]
    if method in unusedMethodsExclusionSet:
        continue
    if d in callSet:
        continue
    if isOtherConstness(d, callSet):
        continue
    # include assigment operators, if we remove them, the compiler creates a default one, which can have odd consequences
    if "::operator=(" in d[1]:
        continue
    # these are only invoked implicitly, so the plugin does not see the calls
    if "::operator new(" in d[1] or "::operator delete(" in d[1]:
        continue
    # just ignore iterators, they normally occur in pairs, and we typically want to leave one constness version alone
    # alone if the other one is in use.
    if d[1] == "begin() const" or d[1] == "begin()" or d[1] == "end()" or d[1] == "end() const":
        continue
    # There is lots of macro magic going on in SRCDIR/include/sax/fshelper.hxx that should be using C++11 varag templates
    if d[1].startswith("sax_fastparser::FastSerializerHelper::"):
       continue
    # used by Windows build
    if any(x in d[1] for x in ["DdeTopic::", "DdeData::", "DdeService::", "DdeTransaction::", "DdeConnection::", "DdeLink::", "DdeItem::", "DdeGetPutItem::"]):
       continue
    # too much template magic here for my plugin
    if (   ("cairocanvas::" in d[1])
        or ("canvas::" in d[1])
        or ("oglcanvas::" in d[1])
        or ("vclcanvas::" in d[1])):
       continue
    # these are loaded by dlopen() from somewhere
    if "get_implementation" in d[1]:
       continue
    if "component_getFactory" in d[1]:
       continue
    if d[0]=="_Bool" and "_supportsService(const class rtl::OUString &)" in d[1]:
       continue
    if (d[0]=="class com::sun::star::uno::Reference<class com::sun::star::uno::XInterface>"
        and "Instance(const class com::sun::star::uno::Reference<class com::sun::star::lang::XMultiServiceFactory> &)" in d[1]):
       continue
    # ignore the Java symbols, loaded from the JavaVM
    if d[1].startswith("Java_"):
       continue
    # ignore external code
    if definitionToSourceLocationMap[d].startswith("external/"):
       continue
    # ignore the VCL_BUILDER_DECL_FACTORY stuff
    if d[0]=="void" and d[1].startswith("make") and ("(class VclPtr<class vcl::Window> &" in d[1]):
       continue
    # ignore methods used to dump objects to stream - normally used for debugging
    if d[0] == "class std::basic_ostream<char> &" and d[1].startswith("operator<<(class std::basic_ostream<char> &"):
       continue
    if d[0] == "basic_ostream<type-parameter-?-?, type-parameter-?-?> &" and d[1].startswith("operator<<(basic_ostream<type-parameter-?-?"):
       continue
    # ignore the SfxPoolItem CreateDefault methods for now
    if d[1].endswith("::CreateDefault()"):
        continue
    if "::operator" in d[1]:
        continue

    unusedSet.add(d) # used by the "unused return types" analysis
    tmp1set.add((method, definitionToSourceLocationMap[d]))

# print out the results, sorted by name and line number
with open("loplugin.unusedmethods.report-methods", "wt") as f:
    for t in sort_set_by_natural_key(tmp1set):
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")

# --------------------------------------------------------------------------------------------
# "unused return types" analysis
# --------------------------------------------------------------------------------------------

tmp2set = set()
for d in definitionSet:
    method = d[0] + " " + d[1]
    if d in usedReturnSet:
        continue
    if d in unusedSet:
        continue
    if isOtherConstness(d, usedReturnSet):
        continue
    # ignore methods with no return type, and constructors
    if d[0] == "void" or d[0] == "":
        continue
    # ignore bool returns, provides important documentation in the code
    if d[0] == "_Bool":
        continue
    # ignore UNO constructor method entrypoints
    if "_get_implementation" in d[1] or "_getFactory" in d[1]:
        continue
    # the plugin can't see calls to these
    if "operator new" in d[1]:
        continue
    # unused return type is not a problem here
    if ("operator=(" in d[1] or "operator&=" in d[1] or "operator|=" in d[1] or "operator^=" in d[1]
        or "operator+=" in d[1] or "operator-=" in d[1]
        or "operator<<" in d[1] or "operator>>" in d[1]
        or "operator++" in d[1] or "operator--" in d[1]):
        continue
    # ignore external code
    if definitionToSourceLocationMap[d].startswith("external/"):
       continue
    # ignore the SfxPoolItem CreateDefault methods for now
    if d[1].endswith("::CreateDefault()"):
        continue
    tmp2set.add((method, definitionToSourceLocationMap[d]))

# print output, sorted by name and line number
with open("loplugin.unusedmethods.report-returns", "wt") as f:
    for t in sort_set_by_natural_key(tmp2set):
        f.write(t[1] + "\n")
        f.write("    " +  t[0] + "\n")


# --------------------------------------------------------------------------------------------
# "unnecessary public" analysis
# --------------------------------------------------------------------------------------------

tmp3set = set()
for d in publicDefinitionSet:
    method = d[0] + " " + d[1]
    if d in calledFromOutsideSet:
        continue
    if isOtherConstness(d, calledFromOutsideSet):
        continue
    # ignore external code
    if definitionToSourceLocationMap[d].startswith("external/"):
       continue
    tmp3set.add((method, definitionToSourceLocationMap[d]))

# print output, sorted by name and line number
with open("loplugin.unusedmethods.report-public", "wt") as f:
    for t in sort_set_by_natural_key(tmp3set):
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")

        

