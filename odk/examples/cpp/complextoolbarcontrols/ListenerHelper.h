/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <vector>
#include <boost/unordered_map.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XDispatch.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>

struct hashObjectName_Impl
{
    size_t operator()(const ::rtl::OUString Str) const
    {
        return (size_t)Str.hashCode();
    }
};

struct eqObjectName_Impl
{
    sal_Bool operator()(const ::rtl::OUString Str1, const ::rtl::OUString Str2) const
    {
        return ( Str1 == Str2 );
    }
};

typedef std::vector < com::sun::star::uno::Reference < com::sun::star::frame::XStatusListener > > StatusListeners;

typedef boost::unordered_map
<
    ::rtl::OUString,
    StatusListeners,
    hashObjectName_Impl,
    eqObjectName_Impl
>
ListenerMap;

// For every frame there is *one* Dispatch object for all possible commands
// this struct contains an array of listeners for every supported command
// these arrays are accessed by a boost::unordered_map (with the command string as index)
struct ListenerItem
{
    ListenerMap aContainer;
    ::com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > xDispatch;
    ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame;
};

typedef std::vector < ListenerItem > AllListeners;

class ListenerHelper
{
public:
    void AddListener(
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const com::sun::star::uno::Reference < com::sun::star::frame::XStatusListener > xControl,
        const ::rtl::OUString& aCommand );
    void RemoveListener(
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const com::sun::star::uno::Reference < com::sun::star::frame::XStatusListener > xControl,
        const ::rtl::OUString& aCommand );
    void Notify(
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const ::rtl::OUString& aCommand,
        com::sun::star::frame::FeatureStateEvent& rEvent );
    com::sun::star::uno::Reference < com::sun::star::frame::XDispatch > GetDispatch(
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const ::rtl::OUString& aCommand );
    void AddDispatch(
        const com::sun::star::uno::Reference < com::sun::star::frame::XDispatch > xDispatch,
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const ::rtl::OUString& aCommand );
};

class ListenerItemEventListener : public cppu::WeakImplHelper1 < ::com::sun::star::lang::XEventListener >
{
    ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame > mxFrame;
public:
    ListenerItemEventListener( const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame)
        : mxFrame(xFrame)
    {}
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
