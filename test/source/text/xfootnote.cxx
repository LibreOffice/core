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
    elidestringvar  oldLabel = xFootnote->getLabel();
    CPPUNIT_ASSERT(!oldLabel.isEmpty());
}

void XFootnote::testsetLabel()
{
    uno::Reference<text::XFootnote> xFootnote(init(), UNO_QUERY_THROW);
    elidestringvar str = "New XFootnote Label";
    elidestringvar oldLabel = xFootnote->getLabel();
    xFootnote->setLabel(str);

    elidestringvar res = xFootnote->getLabel();

    CPPUNIT_ASSERT_EQUAL(oldLabel, res);
}
}
