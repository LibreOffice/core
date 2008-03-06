#ifndef HELPER_HXX
#define HELPER_HXX

#include <toolkit/dllapi.h>
#include <vector>

#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XLayoutContainer.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/xml/input/XRoot.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/propshlp.hxx>
#include <rtl/ustring.hxx>

namespace layoutimpl
{

namespace css = ::com::sun::star;

/* ChildProps -- a helper to set child properties for the XLayoutContainer interface. */

class LockHelper
{
public:
    osl::Mutex               maGuard;
    cppu::OBroadcastHelper maBrdcstHelper;
    LockHelper() : maBrdcstHelper( maGuard )
    {
    }
};

class PropHelper : public LockHelper
                 , public cppu::OPropertySetHelper
                 , public cppu::OWeakObject
{
    cppu::OPropertyArrayHelper *pHelper;

    struct PropDetails
    {
        rtl::OUString  aName;
        css::uno::Type aType;
        void          *pValue;
    };
    std::vector< PropDetails > maDetails;

protected:
    void addProp( char const *pName, sal_Int32 nNameLen, rtl_TextEncoding e,
                  css::uno::Type aType, void *pPtr );

public:
    PropHelper();

    // com::sun::star::uno::XInterface
    void SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    void SAL_CALL release() throw() { OWeakObject::release(); }
    ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // cppu::OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
    virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any &,
                                                        css::uno::Any &, sal_Int32 nHandle, const css::uno::Any & )
        throw(css::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,
                                                            const css::uno::Any& rValue ) throw (css::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue,
                                                sal_Int32 nHandle ) const;

    // you -must- use this macro in sub-classes that define new properties.
    // NB. 'static' ...
    // com::sun::star::beans::XMultiPropertySet
#define PROPHELPER_SET_INFO \
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL \
    getPropertySetInfo() throw(css::uno::RuntimeException) \
    { \
        static css::uno::Reference< css::beans::XPropertySetInfo > xInfo( \
            createPropertySetInfo( getInfoHelper() ) ); \
        return xInfo; \
    }
    PROPHELPER_SET_INFO

    struct Listener
    {
        virtual void propertiesChanged() = 0;
    };
    void setChangeListener( Listener *pListener )
    {
        mpListener = pListener;
    }

protected:
    Listener *mpListener;
};

css::uno::Any anyFromString (const rtl::OUString &value, const css::uno::Type &type);

// The native widgets wrapper hierarchy may not reflect that of the layout
// hierarchy as some containers don't have an associated native widget.
// Use this function to get the native parent of the given peer.
css::uno::Reference< css::awt::XWindowPeer >
getParent( css::uno::Reference< css::uno::XInterface > xPeer );


struct WidgetFactory
{
    virtual css::uno::Reference< css::awt::XLayoutConstrains >
    createWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
                  css::uno::Reference< css::uno::XInterface > xParent,
                  const rtl::OUString &rName, long nProps );
};

// A local factory method - should use UNO services in due course
css::uno::Reference< css::awt::XLayoutConstrains > TOOLKIT_DLLPUBLIC
createWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
              css::uno::Reference< css::uno::XInterface > xParent,
              const rtl::OUString &rName, long nProps );

// Factory for containers (not visible ones)
css::uno::Reference< css::awt::XLayoutContainer >
createContainer( const rtl::OUString &rName );

} // namespace layoutimpl

#endif /* HELPER_HXX */
