#include <test/text/xfootnote.hxx>

#include <com/sun/star/text/XFootnote.hpp>
#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XFootnote::testGetLabel()
{
    uno::Reference<text::XFootnote> xFootnote(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xFootnote->getLabel().isEmpty());
}

void XFootnote::testSetLabel()
{
    uno::Reference<text::XFootnote> xFootnote(init(), UNO_QUERY_THROW);
    OUString oldLabel = xFootnote->getLabel();
    OUString newLabel = "New XFootnote Label";
    xFootnote->setLabel(newLabel);
    CPPUNIT_ASSERT_EQUAL(xFootnote->getLabel(), newLabel);
    //restoring old Label value
    xFootnote->setLabel(oldLabel);
}
}
