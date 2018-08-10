#include <test/text/xfootnote.hxx>

#include <com/sun/star/text/XFootnote.hpp>
#include <cppunit/extensions/HelperMacros.h>



using namespace css;
using namespace css::uno;

namespace apitest {

  void XFootnote::testgetLabel()
  {
    uno::Reference< text::XFootnote > xFootnote(init(), UNO_QUERY_THROW);
    OUString oldLabel = xFootnote->getLabel();
    CPPUNIT_ASSERT(!oldLabel.isEmpty());
  }


  void XFootnote::testsetLabel()
  {
    uno::Reference< text::XFootnote > xFootnote(init(), UNO_QUERY_THROW);
    OUString str = "New XFootnote Label";
    OUString oldLabel = xFootnote->getLabel();
    xFootnote->setLabel(str);

    OUString res = xFootnote->getLabel();

    CPPUNIT_ASSERT_EQUAL(oldLabel,res);
  }
}