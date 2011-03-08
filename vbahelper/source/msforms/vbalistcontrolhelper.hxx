/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef SC_VBA_LISTCONTROLHELPER
#define SC_VBA_LISTCONTROLHELPER

#include <vbahelper/vbahelper.hxx>

class ListControlHelper
{
    css::uno::Reference< css::beans::XPropertySet > m_xProps;

public:
    ListControlHelper( const css::uno::Reference< css::beans::XPropertySet >& rxControl ) : m_xProps( rxControl ){}
    virtual ~ListControlHelper() {}
    virtual void SAL_CALL AddItem( const css::uno::Any& pvargItem, const css::uno::Any& pvargIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeItem( const css::uno::Any& index ) throw (css::uno::RuntimeException);
        virtual void SAL_CALL setRowSource( const rtl::OUString& _rowsource ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getListCount() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL List( const css::uno::Any& pvargIndex, const css::uno::Any& pvarColumn ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Clear(  ) throw (css::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
