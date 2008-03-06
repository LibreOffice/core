#ifndef CORE_DIALOGBUTTONHBOX_HXX
#define CORE_DIALOGBUTTONHBOX_HXX

#include "box.hxx"
#include "flow.hxx"

namespace layoutimpl
{

class DialogButtonHBox : public HBox
{
public:
    DialogButtonHBox();

    void setOrdering( rtl::OUString const& ordering );
    void SAL_CALL addChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild ) throw ( css::uno::RuntimeException, css::awt::MaxChildrenException );
    void SAL_CALL removeChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild ) throw ( css::uno::RuntimeException );

private:
    enum Ordering { PLATFORM, GNOME, KDE, MACOS, WINDOWS };

    void orderChildren();
    void gnomeOrdering();
    void kdeOrdering();
    void macosOrdering();
    void windowsOrdering();

    static Ordering const DEFAULT_ORDERING;
    Ordering mnOrdering;
    Flow mFlow;

    ChildData *mpAction; /* [..]?, [Retry?] */
    ChildData *mpAffirmative; /* OK, Yes, Save */
    ChildData *mpAlternate; /* NO, [Ignore?], Don't save, Quit without saving */
    ChildData *mpApply; /* Deprecated? */
    ChildData *mpCancel; /* Cancel, Close */
    ChildData *mpFlow;
    ChildData *mpHelp;
    ChildData *mpReset;

    std::list< ChildData *> maOther;
};

} // namespace layoutimpl

#endif /* CORE_DIALOGBUTTONHBOX_HXX */
