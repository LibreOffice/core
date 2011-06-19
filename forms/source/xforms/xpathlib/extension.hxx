/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <rtl/ustring.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/xpath/XXPathExtension.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>




class CLibxml2XFormsExtension : public cppu::WeakImplHelper2<
    com::sun::star::xml::xpath::XXPathExtension, com::sun::star::lang::XInitialization>
{
private:
    com::sun::star::uno::Reference <com::sun::star::xforms::XModel>  m_aModel;
    com::sun::star::uno::Reference <com::sun::star::xml::dom::XNode> m_aContextNode;

public:
    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL Create(
        const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& aFactory);
    static rtl::OUString SAL_CALL getImplementationName_Static();
    static com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames_Static();

    com::sun::star::uno::Reference< com::sun::star::xforms::XModel > getModel();
    com::sun::star::uno::Reference< com::sun::star::xml::dom::XNode > getContextNode();

    virtual com::sun::star::xml::xpath::Libxml2ExtensionHandle SAL_CALL getLibxml2ExtensionHandle() throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL initialize(const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aSequence) throw (com::sun::star::uno::RuntimeException);

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
