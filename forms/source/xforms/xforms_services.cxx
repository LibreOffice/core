

#include "services.hxx"

#include "binding.hxx"
#include "model.hxx"
#include "NameContainer.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::uno::RuntimeException;
using com::sun::star::form::binding::XValueBinding;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameContainer;
using rtl::OUString;


namespace frm
{

Reference<XInterface> Model_CreateInstance(
    const Reference<XMultiServiceFactory>& )
    throw( RuntimeException )
{
    return static_cast<XPropertySet*>( new xforms::Model );
}

Reference<XInterface> XForms_CreateInstance(
    const Reference<XMultiServiceFactory>& )
    throw( RuntimeException )
{
    return static_cast<XNameContainer*>( new NameContainer<Reference<XPropertySet> >() );
}

}
