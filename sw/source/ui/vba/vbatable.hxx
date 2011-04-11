/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef SW_VBA_TABLE_HXX
#define SW_VBA_TABLE_HXX
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <ooo/vba/word/XRange.hpp>
#include <ooo/vba/word/XTable.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XTable > SwVbaTable_BASE;

class SwVbaTable : public SwVbaTable_BASE
{
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XTextTable > mxTextTable;
public:
    SwVbaTable( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rDocument, const css::uno::Reference< css::text::XTextTable >& xTextTable) throw ( css::uno::RuntimeException);
    css::uno::Reference< css::text::XTextDocument > getDocument() const { return mxTextDocument; };
    virtual css::uno::Reference< ::ooo::vba::word::XRange > SAL_CALL Range(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Select(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::word::XRange > SAL_CALL ConvertToText( const css::uno::Any& Separator, const css::uno::Any& NestedTables ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getName( ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Rows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Columns( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
