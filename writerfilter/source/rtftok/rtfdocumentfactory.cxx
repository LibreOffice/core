#include <rtfdocumentimpl.hxx>

namespace writerfilter {
namespace rtftok {

RTFDocument::Pointer_t RTFDocumentFactory::createDocument(uno::Reference< io::XInputStream > const & xInputStream)
{
    return RTFDocument::Pointer_t(new RTFDocumentImpl(xInputStream));
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
