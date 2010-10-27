/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _DESKTOP_CFGFILTER_HXX_
#define _DESKTOP_CFGFILTER_HXX_

#include <stack>

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <com/sun/star/configuration/backend/TemplateIdentifier.hpp>


#include "migration_impl.hxx"

#define NS_CSS com::sun::star
#define NS_UNO com::sun::star::uno


namespace desktop {

struct element
{
    rtl::OUString name;
    rtl::OUString path;
    sal_Bool use;

};

typedef std::stack< element > element_stack;

// XInitialization:
//  -> Source : XLayer
// XLayer
// XLayerHandler
class CConfigFilter : public cppu::WeakImplHelper3<
    NS_CSS::configuration::backend::XLayer,
    NS_CSS::configuration::backend::XLayerHandler,
    NS_CSS::lang::XInitialization>
{

private:
    NS_UNO::Reference< NS_CSS::configuration::backend::XLayerHandler > m_xLayerHandler;
    NS_UNO::Reference< NS_CSS::configuration::backend::XLayer > m_xSourceLayer;

    rtl::OUString m_aCurrentComponent;

    const strings_v *m_pvInclude;
    const strings_v *m_pvExclude;

    element_stack m_elementStack;

    void pushElement(rtl::OUString aName, sal_Bool bUse = sal_True);
    void popElement();
    sal_Bool checkElement(rtl::OUString aName);
    sal_Bool checkCurrentElement();

public:
    CConfigFilter(const strings_v* include,  const strings_v* exclude);

    // XInitialization
    virtual void SAL_CALL initialize(const NS_UNO::Sequence< NS_UNO::Any >& seqArgs)
        throw (NS_UNO::Exception);

    // XLayer
    virtual void SAL_CALL readData(
        const NS_UNO::Reference< NS_CSS::configuration::backend::XLayerHandler >& layerHandler)
        throw (NS_CSS::lang::NullPointerException, NS_CSS::lang::WrappedTargetException,
               NS_CSS::configuration::backend::MalformedDataException);

    // XLayerHandler
    virtual void SAL_CALL startLayer()
        throw(::com::sun::star::lang::WrappedTargetException);

    virtual void SAL_CALL endLayer()
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL overrideNode(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            sal_Bool bClear)
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL addOrReplaceNode(
            const rtl::OUString& aName,
            sal_Int16 aAttributes)
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  addOrReplaceNodeFromTemplate(
            const rtl::OUString& aName,
            const NS_CSS::configuration::backend::TemplateIdentifier& aTemplate,
            sal_Int16 aAttributes )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  endNode()
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  dropNode(
            const rtl::OUString& aName )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  overrideProperty(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            const NS_UNO::Type& aType,
            sal_Bool bClear )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  setPropertyValue(
            const NS_UNO::Any& aValue )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL setPropertyValueForLocale(
            const NS_UNO::Any& aValue,
            const rtl::OUString& aLocale )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  endProperty()
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  addProperty(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            const NS_UNO::Type& aType )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

    virtual void SAL_CALL  addPropertyWithValue(
            const rtl::OUString& aName,
            sal_Int16 aAttributes,
            const NS_UNO::Any& aValue )
        throw(
            ::com::sun::star::configuration::backend::MalformedDataException,
            ::com::sun::star::lang::WrappedTargetException );

};

} // namespace desktop
#undef NS_CSS
#undef NS_UNO

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
