/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */


namespace com { namespace sun { namespace star {
    namespace uno { class XInterface; }
    namespace uno { template<class T> class Reference<T>; }
    namespace uno { class RuntimeException; }
    namespace lang { class XMultiServiceFactory; }
} } }


namespace frm
{
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
    SAL_CALL Binding_CreateInstance(
        const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
        throw( com::sun::star::uno::RuntimeException );

    com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
    SAL_CALL Model_CreateInstance(
        const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
        throw( com::sun::star::uno::RuntimeException );

    com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
    SAL_CALL XForms_CreateInstance(
        const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
        throw( com::sun::star::uno::RuntimeException );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
