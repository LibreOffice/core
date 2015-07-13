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
    # TODO track instantiations of template class constructors
    "void comphelper::IEventProcessor::release()",
    "void SotMutexHolder::acquire()",
    "void SotMutexHolder::release()",
    "class Rectangle ComboBox::GetDropDownPosSizePixel() const"
    # only used by Windows build
    "_Bool basegfx::B2ITuple::equalZero() const",
    "class basegfx::B2DPolyPolygon basegfx::unotools::UnoPolyPolygon::getPolyPolygonUnsafe() const",
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
    "_Bool SalObject::IsEraseBackgroundEnabled()",
    # instantiated from a template in VCL, not sure why it is not being picked up
    "class basegfx::B2DPolygon OutputDevice::PixelToLogic(const class basegfx::B2DPolygon &,const class MapMode &) const",
    # only used by OSX build
    "void StyleSettings::SetHideDisabledMenuItems(_Bool)",
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
    if (clazz.endswith(" const")
        and clazz[6:len(clazz)-6] in definitionSet
        and clazz[6:len(clazz)-6] in callSet):
       continue
    # if this method is non-const, and there is a const variant of it, and the const variant is in use, then leave it alone
    if ((not clazz.endswith(" const"))
        and ("const " + clazz + " const") in definitionSet
        and ("const " + clazz + " const") in callSet):
       continue
    # There is lots of macro magic going on in /home/noel/libo4/include/sax/fshelper.hxx that should be using C++11 varag templates
    if clazz.startswith("void sax_fastparser::FastSerializerHelper::"):
       continue
    # used by Windows build
    if clazz.find("DdeTopic::") != -1 or clazz.find("DdeData::") != -1 or clazz.find("DdeService::") != -1:
       continue
    print clazz

# add an empty line at the end to make it easier for the unusedmethodsremove plugin to mmap() the output file 
print
        

