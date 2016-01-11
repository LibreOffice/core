#!/usr/bin/python

import sys
import re
import io

definitionSet = set()
definitionToSourceLocationMap = dict()
callSet = set()
sourceLocationSet = set()
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

# The parsing here is designed to avoid grabbing stuff which is mixed in from gbuild.
# I have not yet found a way of suppressing the gbuild output.
with io.open(sys.argv[1], "rb", buffering=1024*1024) as txt:
    for line in txt:
        if line.startswith("definition:\t"):
            idx1 = line.find("\t",12)
            idx2 = line.find("\t",idx1+1)
            funcInfo = (normalizeTypeParams(line[12:idx1]), normalizeTypeParams(line[idx1+1:idx2]))
            definitionSet.add(funcInfo)
            definitionToSourceLocationMap[funcInfo] = line[idx2+1:].strip()
        elif line.startswith("call:\t"):
            idx1 = line.find("\t",6)
            callSet.add((normalizeTypeParams(line[6:idx1]), normalizeTypeParams(line[idx1+1:].strip())))

# Invert the definitionToSourceLocationMap
# If we see more than one method at the same sourceLocation, it's being autogenerated as part of a template
# and we should just ignore
sourceLocationToDefinitionMap = {}
for k, v in definitionToSourceLocationMap.iteritems():
    sourceLocationToDefinitionMap[v] = sourceLocationToDefinitionMap.get(v, [])
    sourceLocationToDefinitionMap[v].append(k)
for k, definitions in sourceLocationToDefinitionMap.iteritems():
    if len(definitions) > 1:
        for d in definitions:
            definitionSet.remove(d)

def isOtherConstness( d, callSet ):
    clazz = d[0] + " " + d[1]
    # if this method is const, and there is a non-const variant of it, and the non-const variant is in use, then leave it alone
    if d[0].startswith("const ") and d[1].endswith(" const"):
        if ((d[0][6:],d[1][:-6]) in callSet):
           return True
    elif clazz.endswith(" const"):
        clazz2 = clazz[:len(clazz)-6] # strip off " const"
        if ((d[0],clazz2) in callSet):
           return True
    if clazz.endswith(" const") and ("::iterator" in clazz):
        clazz2 = clazz[:len(clazz)-6] # strip off " const"
        clazz2 = clazz2.replace("::const_iterator", "::iterator")
        if ((d[0],clazz2) in callSet):
           return True
    # if this method is non-const, and there is a const variant of it, and the const variant is in use, then leave it alone
    if (not clazz.endswith(" const")) and ((d[0],"const " + clazz + " const") in callSet):
           return True
    if (not clazz.endswith(" const")) and ("::iterator" in clazz):
        clazz2 = clazz.replace("::iterator", "::const_iterator") + " const"
        if ((d[0],clazz2) in callSet):
           return True
    return False

    
tmp1set = set()
for d in definitionSet:
    clazz = d[0] + " " + d[1]
    if clazz in exclusionSet:
        continue
    if d in callSet:
        continue
    # ignore operators, they are normally called from inside STL code
    if "::operator" in d[1]:
        continue
    # ignore the custom RTTI stuff
    if (    ("::CreateType()" in d[1])
        or ("::IsA(" in d[1])
        or ("::Type()" in d[1])):
        continue
    if isOtherConstness(d, callSet):
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
    # the include/tools/rtti.hxx stuff
    if ("::StaticType()" in d[1]) or ("::IsOf(void *(*)(void))" in d[1]):
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

    tmp1set.add((clazz, definitionToSourceLocationMap[d]))

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]

# sort results by name and line number
tmp1list = sorted(tmp1set, key=lambda v: natural_sort_key(v[1]))

# print out the results
for t in tmp1list:
    print t[1]
    print "    ", t[0]

    

# add an empty line at the end to make it easier for the unusedmethodsremove plugin to mmap() the output file 
print
        

