#include <test/text/xfootnote.hxx>

#include <com/sun/star/text/XFootnote.hpp>
#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XFootnote::testgetLabel()
{
    uno::Reference<text::XFootnote> xFootnote(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xFootnote->getLabel().isEmpty());
}

void XFootnote::testsetLabel()
{
    uno::Reference<text::XFootnote> xFootnote(init(), UNO_QUERY_THROW);
    xFootnote->setLabel("New XFootnote Label");
    CPPUNIT_ASSERT_EQUAL(xFootnote->getLabel(), xFootnote->getLabel());
}
}
