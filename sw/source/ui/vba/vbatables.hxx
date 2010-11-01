/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef SW_VBA_TABLES_HXX
#define SW_VBA_TABLES_HXX

#include <ooo/vba/word/XTables.hpp>
#include <vbahelper/vbacollectionimpl.hxx>


typedef CollTestImplHelper< ov::word::XTables > SwVbaTables_BASE;

class SwVbaTables : public SwVbaTables_BASE
{
    css::uno::Reference< css::frame::XModel > mxDocument;
public:
    SwVbaTables( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xDocument );
    // XTables
    virtual css::uno::Reference< ov::word::XTable > SAL_CALL Add( const css::uno::Reference< ::ooo::vba::word::XRange >& Range, const css::uno::Any& NumRows, const css::uno::Any& NumColumns, const css::uno::Any& DefaultTableBehavior, const css::uno::Any& AutoFitBehavior ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
