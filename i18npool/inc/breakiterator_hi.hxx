/*************************************************************************
#*
#*  Copyright (c) 2002 Sun Microsystems Inc.
#*
#* Bugs! - Contact Prabhat.Hegde@sun.com
#*************************************************************************/

#ifndef _I18N_BREAKITERATOR_HI_HXX_
#define _I18N_BREAKITERATOR_HI_HXX_

#include <breakiterator_ctl.hxx>

namespace com {
namespace sun {
namespace star {
namespace i18n {

//  ----------------------------------------------------
//  class BreakIterator_hi
//  ----------------------------------------------------
class BreakIterator_hi : public BreakIterator_CTL
{
public:
    BreakIterator_hi();
    ~BreakIterator_hi();

protected:
    void SAL_CALL makeIndex(const rtl::OUString& text, sal_Int32 pos)
    throw(com::sun::star::uno::RuntimeException);
};

}
}
}
}

#endif
