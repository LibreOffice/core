#!/usr/bin/python

import sys

definitionSet = set()
overridingSet = set()
# things we need to exclude for reasons like :
# - we can't see the override because it's a MS-Windows-only thing.
# - they involve function template instantiations, which I can't handle
exclusionSet = set(
    "basegfx::unotools::UnoPolyPolygon::void-modifying()const",
    "SalLayout::_Bool-DrawTextSpecial(class SalGraphics &,sal_uInt32,)const",
    "SalLayout::_Bool-IsKashidaPosValid(int,)const",
    "SalLayout::void-DisableGlyphInjection(_Bool,)",
    "SalObject::void-Enable(_Bool,)",
    "PropertyWrapperBase::void-SetValue(const ::com::sun::star::uno::Any &,)"
    "canvas::IColorBuffer::enum canvas::IColorBuffer::Format-getFormat()const",
    "canvas::IColorBuffer::sal_uInt32-getHeight()const",
    "canvas::IColorBuffer::sal_uInt32-getStride()const",
    "canvas::IColorBuffer::sal_uInt32-getWidth()const",
    "canvas::IColorBuffer::sal_uInt8 *-lock()const",
    "canvas::IColorBuffer::void-unlock()const",
    "canvas::IRenderModule::::basegfx::B2IVector-getPageSize()",
    "canvas::IRenderModule::::boost::shared_ptr<ISurface>-createSurface(const ::basegfx::B2IVector &,)",
    "canvas::IRenderModule::_Bool-isError()",
    "canvas::IRenderModule::void-beginPrimitive(enum canvas::IRenderModule::PrimitiveType,)",
    "canvas::IRenderModule::void-endPrimitive()",
    "canvas::IRenderModule::void-lock()const",
    "canvas::IRenderModule::void-pushVertex(const struct canvas::Vertex &,)",
    "canvas::IRenderModule::void-unlock()const",
    "canvas::ISurface::_Bool-isValid()",
    "canvas::ISurface::_Bool-selectTexture()",
    "canvas::ISurface::_Bool-update(const ::basegfx::B2IPoint &,const ::basegfx::B2IRange &,struct canvas::IColorBuffer &,)",
    "SalFrame::void-Flush(const class Rectangle &,)",
    "SalFrame::void-SetRepresentedURL(const class rtl::OUString &,)",
    "SalLayout::_Bool-DrawTextSpecial(class SalGraphics &,sal_uInt32,)const",
    "SalLayout::_Bool-GetBoundRect(class SalGraphics &,class Rectangle &,)const",
    "SalLayout::_Bool-IsKashidaPosValid(int,)const",
    "SalLayout::void-DisableGlyphInjection(_Bool,)",
    "writerfilter::ooxml::OOXMLFactory_ns::Id-getResourceId(Id,sal_Int32,)",
    "writerfilter::ooxml::OOXMLFactory_ns::_Bool-getElementId(Id,Id,enum writerfilter::ooxml::ResourceType_t &,Id &,)",
    "writerfilter::ooxml::OOXMLFactory_ns::_Bool-getListValue(Id,const class rtl::OUString &,sal_uInt32 &,)",
    "writerfilter::ooxml::OOXMLFactory_ns::const struct writerfilter::ooxml::AttributeInfo *-getAttributeInfoArray(Id,)",
    "sd::ZeroconfService::void-clear()",
    "sd::ZeroconfService::void-setup()",
    "slideshow::internal::EnumAnimation::ValueType-getUnderlyingValue()const",
    "slideshow::internal::EnumAnimation::_Bool-operator()(ValueType,)",
    "basegfx::unotools::UnoPolyPolygon::void-modifying()const",
    "DdeTopic::_Bool-Execute(const class rtl::OUString *,)",
    "DdeTopic::class DdeData *-Get(enum SotClipboardFormatId,)",
    "DdeTopic::_Bool-Put(const class DdeData *,)",
    "DdeTopic::_Bool-MakeItem(const class rtl::OUString &,)",
    "DdeTopic::_Bool-StartAdviseLoop()",
    )


with open(sys.argv[1]) as txt:
    for line in txt:
    
        if line.startswith("definition:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            definitionSet.add(clazzName)
            
        elif line.startswith("overriding:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            overridingSet.add(clazzName)
            
for clazz in sorted(definitionSet - overridingSet - exclusionSet):
    # these involve function template instantiations, which I can't handle
    if not clazz.startswith("basebmp::BitmapDevice::"):
        print clazz

# add an empty line at the end to make it easier for the removevirtuals plugin to mmap() the output file 
print

