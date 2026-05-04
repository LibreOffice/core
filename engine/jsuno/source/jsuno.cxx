/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <quickjs.h>

#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#include <com/sun/star/reflection/XCompoundTypeDescription.hpp>
#include <com/sun/star/reflection/XConstantTypeDescription.hpp>
#include <com/sun/star/reflection/XConstantsTypeDescription.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/reflection/XStructTypeDescription.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/script/Invocation.hpp>
#include <com/sun/star/script/InvocationInfo.hpp>
#include <com/sun/star/script/XInvocation2.hpp>
#include <com/sun/star/script/provider/ScriptExceptionRaisedException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <jsuno/detail/dllapi.hxx>
#include <jsuno/jsuno.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/unreachable.hxx>
#include <rtl/string.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <typelib/typedescription.hxx>

namespace
{
struct JsException
{
};

class AtomRef
{
public:
    AtomRef(JSRuntime* rt)
        : rt_(rt)
        , atom_(JS_ATOM_NULL)
    {
    }

    AtomRef(JSContext* ctx, JSAtom atom)
        : rt_(JS_GetRuntime(ctx))
        , atom_(atom)
    {
    }

    ~AtomRef() { JS_FreeAtomRT(rt_, atom_); }

    AtomRef& operator=(JSAtom atom)
    {
        std::swap(atom_, atom);
        JS_FreeAtomRT(rt_, atom);
        return *this;
    }

    operator JSAtom() const { return atom_; }

private:
    AtomRef(AtomRef const&) = delete;
    AtomRef(AtomRef&&) = delete;
    void operator=(AtomRef const&) = delete;
    void operator=(AtomRef&&) = delete;

    JSRuntime* rt_;
    JSAtom atom_;
};

class ValueRef
{
public:
    ValueRef(JSContext* ctx, JSValue val = JS_UNINITIALIZED)
        : ctx_(ctx)
        , val_(val)
    {
    }

    ValueRef(ValueRef&& ref)
        : ctx_(ref.ctx_)
        , val_(ref.val_)
    {
        ref.val_ = JS_UNINITIALIZED;
    }

    ~ValueRef() { JS_FreeValue(ctx_, val_); }

    ValueRef& operator=(ValueRef&& ref)
    {
        assert(ctx_ == ref.ctx_);
        JS_FreeValue(ctx_, val_);
        val_ = ref.val_;
        ref.val_ = JS_UNINITIALIZED;
        return *this;
    }

    ValueRef& operator=(JSValue val)
    {
        JS_FreeValue(ctx_, val_);
        val_ = val;
        return *this;
    }

    operator JSValueConst() const { return val_; }

    JSValue dup() const { return JS_DupValue(ctx_, val_); }

    JSValue release()
    {
        auto const val = val_;
        val_ = JS_UNINITIALIZED;
        return val;
    }

    JSValue* ptr() { return &val_; }

private:
    ValueRef(ValueRef const&) = delete;
    void operator=(ValueRef const&) = delete;

    JSContext* ctx_;
    JSValue val_;
};

class UniqueCString8
{
public:
    UniqueCString8(JSContext* ctx, char const* ptr)
        : ctx_(ctx)
        , ptr_(ptr)
    {
    }

    ~UniqueCString8() { JS_FreeCString(ctx_, ptr_); }

    char const* get() const { return ptr_; }

private:
    UniqueCString8(UniqueCString8 const&) = delete;
    UniqueCString8(UniqueCString8&&) = delete;
    void operator=(UniqueCString8 const&) = delete;
    void operator=(UniqueCString8&&) = delete;

    JSContext* ctx_;
    char const* ptr_;
};

class UniqueCString16
{
public:
    UniqueCString16(JSContext* ctx, std::uint16_t const* ptr)
        : ctx_(ctx)
        , ptr_(ptr)
    {
    }

    ~UniqueCString16() { JS_FreeCStringUTF16(ctx_, ptr_); }

    char16_t const* get() const { return reinterpret_cast<char16_t const*>(ptr_); }

private:
    UniqueCString16(UniqueCString16 const&) = delete;
    UniqueCString16(UniqueCString16&&) = delete;
    void operator=(UniqueCString16 const&) = delete;
    void operator=(UniqueCString16&&) = delete;

    JSContext* ctx_;
    std::uint16_t const* ptr_;
};

ValueRef createDefaultValue(JSContext* ctx, css::uno::Type const& type);
css::uno::Any fromJs(JSContext* ctx, css::uno::Type const& type, JSValueConst val);
JSValue invokeUno(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic,
                  JSValueConst* func_data);
ValueRef mapTypeToJs(JSContext* ctx, css::uno::Type const& type);
ValueRef toJs(JSContext* ctx, css::uno::Any const& value);

#if defined DBG_UTIL
class Counter
{
public:
    ~Counter() { assert(count_ == 0); }

    void inc()
    {
        assert(count_ != std::numeric_limits<std::uintptr_t>::max());
        ++count_;
    }

    void dec()
    {
        assert(count_ != 0);
        --count_;
    }

private:
    std::uintptr_t count_ = 0;
};
#endif

struct RuntimeData
{
    RuntimeData(JSRuntime* rt)
        : symbolIteratorAtom(rt)
    {
    }

    void clear() { symbolIteratorAtom = JS_ATOM_NULL; }

    JSClassID pointerClassId = 0;
    JSClassID wrapperClassId = 0;
    JSClassID enumeratorClassId = 0;
    JSClassID compoundClassId = 0;
    JSClassID typeClassId = 0;
    JSClassID enumClassId = 0;
    JSClassID structClassId = 0;
    JSClassID exceptionClassId = 0;
    JSClassID interfaceClassId = 0;
    JSClassID ctorClassId = 0;
    JSClassID singletonClassId = 0;
    JSClassID moduleClassId = 0;

    AtomRef symbolIteratorAtom;

#if defined DBG_UTIL
    Counter toFinalize;
#endif
};

RuntimeData* getRuntimeData(JSRuntime* rt)
{
    return static_cast<RuntimeData*>(JS_GetRuntimeOpaque(rt));
}

RuntimeData* getRuntimeData(JSContext* ctx) { return getRuntimeData(JS_GetRuntime(ctx)); }

template <typename F> JSValue callFromJs(JSContext* ctx, F&& f)
{
    try
    {
        return f();
    }
    catch (JsException)
    {
        return JS_EXCEPTION;
    }
    catch (css::uno::Exception)
    {
        auto const e = cppu::getCaughtException();
        ValueRef val = toJs(ctx, e);
        return JS_Throw(ctx, val.release());
    }
    catch (std::exception& e)
    {
        return JS_ThrowPlainError(ctx, "std exception: %s", e.what());
    }
}

// A stripped-down and modified version of <https://console.spec.whatwg.org/#assert> (which only
// takes a single argument and aborts when the assertion is not met), just enough for using it in
// jsunit/qa/unit/testuno.cxx:
JSValue consoleAssert(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, argc, argv] {
        assert(argc >= 1);
        auto const ok = JS_ToBool(ctx, argv[0]);
        if (ok == -1)
        {
            throw JsException();
        }
        if (ok == 0)
        {
            ValueRef const errorCtor(ctx, JS_GetPropertyStr(ctx, JS_GetGlobalObject(ctx), "Error"));
            assert(!JS_IsException(errorCtor));
            ValueRef const err(ctx, JS_CallConstructor(ctx, errorCtor, 0, nullptr));
            assert(!JS_IsException(err));
            ValueRef const stack(ctx, JS_GetPropertyStr(ctx, err, "stack"));
            assert(!JS_IsException(stack));
            UniqueCString8 const s(ctx, JS_ToCString(ctx, stack));
            assert(s.get() != nullptr);
            std::cerr << "console.assert at: " << s.get() << "\n";
            std::abort();
        }
        return JS_UNDEFINED;
    });
}

// A stripped-down version of <https://console.spec.whatwg.org/#log> (which simply prints all its
// arguments, not using the <https://console.spec.whatwg.org/#formatter> logic):
JSValue consoleLog(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, argc, argv] {
        OUStringBuffer buf("console.log:");
        for (int i = 0; i != argc; ++i)
        {
            std::size_t n;
            UniqueCString16 const s(ctx, JS_ToCStringLenUTF16(ctx, &n, argv[i]));
            if (s.get() == nullptr)
            {
                throw JsException();
            }
            buf.append(OUString::Concat(" ") + std::u16string_view(s.get(), n));
        }
        buf.append('\n');
        std::cout << buf.makeStringAndClear() << std::flush;
        return JS_UNDEFINED;
    });
}

rtl_uString* stringAcquire(OUString const& id)
{
    rtl_uString_acquire(id.pData);
    return id.pData;
}

void pointerFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    delete static_cast<css::script::InvocationInfo const*>(
        JS_GetOpaque(val, getRuntimeData(rt)->pointerClassId));
}

JSValue enumerationIteratorNext(JSContext* ctx, JSValueConst, int, JSValueConst*, int,
                                JSValueConst* func_data)
{
    return callFromJs(ctx, [ctx, func_data] {
        css::uno::Reference<css::container::XEnumeration> en(
            static_cast<css::uno::XInterface*>(
                JS_GetOpaque(func_data[0], getRuntimeData(ctx)->wrapperClassId)),
            css::uno::UNO_QUERY_THROW);
        ValueRef val(ctx, JS_NewObject(ctx));
        if (en->hasMoreElements())
        {
            JS_SetPropertyStr(ctx, val, "value", toJs(ctx, en->nextElement()).release());
            JS_SetPropertyStr(ctx, val, "done", JS_FALSE);
        }
        else
        {
            JS_SetPropertyStr(ctx, val, "value", JS_UNDEFINED);
            JS_SetPropertyStr(ctx, val, "done", JS_TRUE);
        }
        return val.release();
    });
}

JSValue enumerationIterator(JSContext* ctx, JSValueConst this_val, int, JSValueConst*)
{
    return callFromJs(ctx, [ctx, this_val] {
        ValueRef iter(ctx, JS_NewObject(ctx));
        JS_SetPropertyStr(ctx, iter, "next",
                          JS_NewCFunctionData(ctx, enumerationIteratorNext, 0, 0, 1,
                                              const_cast<JSValueConst*>(&this_val)));
        return iter.release();
    });
}

JSValue wrapperGetProperty(JSContext* ctx, JSValueConst obj, JSAtom atom, JSValueConst receiver)
{
    return callFromJs(ctx, [ctx, obj, atom, receiver] {
        if (atom == getRuntimeData(ctx)->symbolIteratorAtom)
        {
            css::uno::Reference<css::container::XEnumeration> en(
                static_cast<css::uno::XInterface*>(
                    JS_GetOpaque(obj, getRuntimeData(ctx)->wrapperClassId)),
                css::uno::UNO_QUERY);
            if (!en.is())
            {
                return JS_UNDEFINED;
            }
            ValueRef val(ctx, JS_NewCFunction(ctx, enumerationIterator, "[Symbol.iterator]", 0));
            JS_SetProperty(ctx, receiver, atom, val.dup());
            return val.release();
        }
        ValueRef const v(ctx, JS_AtomToString(ctx, atom));
        if (!JS_IsString(v))
        {
            return JS_UNDEFINED;
        }
        css::uno::Reference<css::script::XInvocation2> invoke(
            css::script::Invocation::create(comphelper::getProcessComponentContext())
                ->createInstanceWithArguments(
                    { css::uno::Any(css::uno::Reference(static_cast<css::uno::XInterface*>(
                        JS_GetOpaque(obj, getRuntimeData(ctx)->wrapperClassId)))) }),
            css::uno::UNO_QUERY_THROW);
        css::script::InvocationInfo info;
        try
        {
            info = invoke->getInfoForName(OUString::fromUtf8(JS_AtomToCString(ctx, atom)), false);
        }
        catch (css::lang::IllegalArgumentException)
        {
            return JS_UNDEFINED;
        }
        switch (info.eMemberType)
        {
            case css::script::MemberType_METHOD:
            {
                ValueRef data(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->pointerClassId));
                [[maybe_unused]] auto const e
                    = JS_SetOpaque(data, new css::script::InvocationInfo(info));
                assert(e == 0); //TODO
#if defined DBG_UTIL
                getRuntimeData(ctx)->toFinalize.inc();
#endif
                ValueRef val(ctx, JS_NewCFunctionData(ctx, invokeUno, info.aParamTypes.getLength(),
                                                      0, 1, data.ptr()));
                JS_SetProperty(ctx, receiver, atom, val.dup());
                return val.release();
            }
            case css::script::MemberType_PROPERTY:
                return toJs(ctx, invoke->getValue(info.aName)).release();
            default:
                O3TL_UNREACHABLE;
        }
    });
}

int wrapperSetProperty(JSContext* ctx, JSValueConst obj, JSAtom atom, JSValueConst value,
                       JSValueConst, int flags)
{
    try
    {
        ValueRef const v(ctx, JS_AtomToString(ctx, atom));
        if (JS_IsString(v))
        {
            css::uno::Reference<css::script::XInvocation2> invoke(
                css::script::Invocation::create(comphelper::getProcessComponentContext())
                    ->createInstanceWithArguments(
                        { css::uno::Any(css::uno::Reference(static_cast<css::uno::XInterface*>(
                            JS_GetOpaque(obj, getRuntimeData(ctx)->wrapperClassId)))) }),
                css::uno::UNO_QUERY_THROW);
            css::script::InvocationInfo info;
            auto prop = false;
            try
            {
                info = invoke->getInfoForName(OUString::fromUtf8(JS_AtomToCString(ctx, atom)),
                                              false);
                prop = info.eMemberType == css::script::MemberType_PROPERTY;
            }
            catch (css::lang::IllegalArgumentException)
            {
            }
            if (prop)
            {
                invoke->setValue(info.aName, fromJs(ctx, info.aType, value));
                return 1;
            }
        }
        return JS_DefinePropertyValue(ctx, obj, atom, JS_DupValue(ctx, value),
                                      JS_PROP_C_W_E | (flags & JS_PROP_THROW));
    }
    catch (JsException)
    {
        return -1;
    }
    catch (css::uno::Exception)
    {
        auto const e = cppu::getCaughtException();
        ValueRef val(ctx, toJs(ctx, e).release());
        JS_Throw(ctx, val.release());
        return -1;
    }
    catch (std::exception& e)
    {
        JS_ThrowPlainError(ctx, "std exception: %s", e.what());
        return -1;
    }
}

void wrapperFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    static_cast<css::uno::XInterface*>(JS_GetOpaque(val, getRuntimeData(rt)->wrapperClassId))
        ->release();
}

JSValue wrapperToString(JSContext* ctx, JSValueConst this_val, int, JSValueConst*)
{
    return callFromJs(ctx, [ctx, this_val] {
        std::ostringstream s;
        s << css::uno::Reference(static_cast<css::uno::XInterface*>(
            JS_GetOpaque(this_val, getRuntimeData(ctx)->wrapperClassId)));
        return JS_NewString(ctx, s.str().c_str());
    });
}

JSValue wrapUnoObject(JSContext* ctx, css::uno::Reference<css::uno::XInterface> const& obj)
{
    if (!obj.is())
    {
        return JS_NULL;
    }
    auto const val = JS_NewObjectClass(ctx, getRuntimeData(ctx)->wrapperClassId);
    [[maybe_unused]] auto const e = JS_SetOpaque(val, obj.get());
    assert(e == 0); //TODO
    obj->acquire();
#if defined DBG_UTIL
    getRuntimeData(ctx)->toFinalize.inc();
#endif
    JS_SetPropertyStr(
        ctx, val, "toString",
        JS_NewCFunction(ctx, wrapperToString, "toString", 0)); //TODO: add to prototype
    return val;
}

struct EnumeratorData
{
    css::uno::Type type;
    sal_Int32 value;
    OUString name;
};

void enumeratorFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    delete static_cast<EnumeratorData const*>(
        JS_GetOpaque(val, getRuntimeData(rt)->enumeratorClassId));
}

JSValue enumeratorToString(JSContext* ctx, JSValueConst this_val, int, JSValueConst*)
{
    return callFromJs(ctx, [ctx, this_val] {
        auto const data = static_cast<EnumeratorData const*>(
            JS_GetOpaque(this_val, getRuntimeData(ctx)->enumeratorClassId));
        OUString const s(data->type.getTypeName() + "." + data->name);
        return JS_NewStringUTF16(ctx, reinterpret_cast<std::uint16_t const*>(s.getStr()),
                                 s.getLength());
    });
}

void compoundFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    typelib_typedescriptionreference_release(static_cast<typelib_TypeDescriptionReference*>(
        JS_GetOpaque(val, getRuntimeData(rt)->compoundClassId)));
}

typelib_TypeDescriptionReference* typeAcquire(css::uno::Type const& type)
{
    auto const ref = type.getTypeLibType();
    typelib_typedescriptionreference_acquire(ref);
    return ref;
}

void typeFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    typelib_typedescriptionreference_release(static_cast<typelib_TypeDescriptionReference*>(
        JS_GetOpaque(val, getRuntimeData(rt)->typeClassId)));
}

JSValue typeToString(JSContext* ctx, JSValueConst this_val, int, JSValueConst*)
{
    return callFromJs(ctx, [ctx, this_val] {
        auto const name
            = css::uno::Type(static_cast<typelib_TypeDescriptionReference*>(
                                 JS_GetOpaque(this_val, getRuntimeData(ctx)->typeClassId)))
                  .getTypeName();
        return JS_NewStringUTF16(ctx, reinterpret_cast<std::uint16_t const*>(name.getStr()),
                                 name.getLength());
    });
}

void setTypeProperty(JSContext* ctx, JSValueConst obj, char const* prop, css::uno::Type const& type)
{
    ValueRef val(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->typeClassId));
    [[maybe_unused]] auto const e = JS_SetOpaque(val, typeAcquire(type));
    assert(e == 0); //TODO
#if defined DBG_UTIL
    getRuntimeData(ctx)->toFinalize.inc();
#endif
    JS_SetPropertyStr(ctx, val, "toString",
                      JS_NewCFunction(ctx, typeToString, "toString", 0)); //TODO: add to prototype
    JS_SetPropertyStr(ctx, obj, prop, val.release());
}

JSValue unoTypeSequence(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, argc, argv] {
        assert(argc >= 1);
        if (JS_GetClassID(argv[0]) != getRuntimeData(ctx)->typeClassId)
        {
            JS_ThrowTypeError(ctx, "TODO: BAD UNO TYPE VALUE");
            throw JsException();
        }
        css::uno::Type const type(
            css::uno::TypeClass_SEQUENCE,
            "[]"
                + css::uno::Type(static_cast<typelib_TypeDescriptionReference*>(
                                     JS_GetOpaque(argv[0], getRuntimeData(ctx)->typeClassId)))
                      .getTypeName());
        ValueRef val(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->typeClassId));
        [[maybe_unused]] auto const e = JS_SetOpaque(val, typeAcquire(type));
        assert(e == 0); //TODO
#if defined DBG_UTIL
        getRuntimeData(ctx)->toFinalize.inc();
#endif
        JS_SetPropertyStr(
            ctx, val, "toString",
            JS_NewCFunction(ctx, typeToString, "toString", 0)); //TODO: add to prototype
        return val.release();
    });
}

JSValue unoTypeEnum(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, argc, argv] {
        assert(argc >= 1);
        if (JS_GetClassID(argv[0]) != getRuntimeData(ctx)->enumClassId)
        {
            JS_ThrowTypeError(ctx, "TODO: BAD UNO TYPE VALUE");
            throw JsException();
        }
        css::uno::Type const type(static_cast<typelib_TypeDescriptionReference*>(
            JS_GetOpaque(argv[0], getRuntimeData(ctx)->enumClassId)));
        ValueRef val(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->typeClassId));
        [[maybe_unused]] auto const e = JS_SetOpaque(val, typeAcquire(type));
        assert(e == 0); //TODO
#if defined DBG_UTIL
        getRuntimeData(ctx)->toFinalize.inc();
#endif
        JS_SetPropertyStr(
            ctx, val, "toString",
            JS_NewCFunction(ctx, typeToString, "toString", 0)); //TODO: add to prototype
        return val.release();
    });
}

JSValue unoTypeStruct(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, argc, argv] {
        assert(argc >= 1);
        ValueRef data(ctx, JS_GetPropertyStr(ctx, argv[0], "'data"));
        if (JS_IsException(data))
        {
            throw JsException();
        }
        if (JS_GetClassID(data) != getRuntimeData(ctx)->structClassId)
        {
            JS_ThrowTypeError(ctx, "TODO: BAD UNO TYPE VALUE");
            throw JsException();
        }
        css::uno::Reference td(static_cast<css::reflection::XStructTypeDescription*>(
            JS_GetOpaque(data, getRuntimeData(ctx)->structClassId)));
        OUStringBuffer buf(td->getName());
        auto const params = td->getTypeParameters();
        if (params.hasElements())
        {
            if (argc < 2)
            {
                JS_ThrowSyntaxError(ctx, "TODO: MISSING TYPE ARGUMENTS");
                throw JsException();
            }
            if (!JS_IsArray(argv[1]))
            {
                JS_ThrowTypeError(ctx, "TODO: BAD TYPE ARGUMENTS");
                throw JsException();
            }
            ValueRef len1(ctx, JS_GetPropertyStr(ctx, argv[1], "length"));
            if (JS_IsException(len1))
            {
                throw JsException();
            }
            std::int64_t len2;
            auto const e = JS_ToInt64(ctx, &len2, len1);
            if (e != 0)
            {
                throw JsException();
            }
            if (len2 != params.getLength())
            {
                JS_ThrowTypeError(ctx, "TODO: BAD TYPE ARGUMENTS");
                throw JsException();
            }
            buf.append('<');
            for (sal_Int32 i = 0; i != params.getLength(); ++i)
            {
                if (i != 0)
                {
                    buf.append(',');
                }
                ValueRef arg(ctx, JS_GetPropertyInt64(ctx, argv[1], i));
                if (JS_IsException(arg))
                {
                    throw JsException();
                }
                if (JS_GetClassID(arg) != getRuntimeData(ctx)->typeClassId)
                {
                    JS_ThrowTypeError(ctx, "TODO: BAD TYPE ARGUMENT");
                    throw JsException();
                }
                css::uno::Type argType(static_cast<typelib_TypeDescriptionReference*>(
                    JS_GetOpaque(arg, getRuntimeData(ctx)->typeClassId)));
                switch (argType.getTypeClass())
                {
                    case css::uno::TypeClass_VOID:
                    case css::uno::TypeClass_UNSIGNED_SHORT:
                    case css::uno::TypeClass_UNSIGNED_LONG:
                    case css::uno::TypeClass_UNSIGNED_HYPER:
                    case css::uno::TypeClass_EXCEPTION:
                        JS_ThrowTypeError(ctx, "TODO: BAD TYPE ARGUMENT");
                        throw JsException();
                    default:
                        break;
                }
                buf.append(argType.getTypeName());
            }
            buf.append('>');
        }
        css::uno::Type const type(td->getTypeClass(), buf.makeStringAndClear());
        ValueRef val(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->typeClassId));
        [[maybe_unused]] auto const e = JS_SetOpaque(val, typeAcquire(type));
        assert(e == 0); //TODO
#if defined DBG_UTIL
        getRuntimeData(ctx)->toFinalize.inc();
#endif
        JS_SetPropertyStr(
            ctx, val, "toString",
            JS_NewCFunction(ctx, typeToString, "toString", 0)); //TODO: add to prototype
        return val.release();
    });
}

JSValue unoTypeException(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, argc, argv] {
        assert(argc >= 1);
        assert(argc >= 1);
        ValueRef data(ctx, JS_GetPropertyStr(ctx, argv[0], "'data"));
        if (JS_IsException(data))
        {
            throw JsException();
        }
        if (JS_GetClassID(data) != getRuntimeData(ctx)->exceptionClassId)
        {
            JS_ThrowTypeError(ctx, "TODO: BAD UNO TYPE VALUE");
            throw JsException();
        }
        css::uno::Type const type(static_cast<typelib_TypeDescriptionReference*>(
            JS_GetOpaque(data, getRuntimeData(ctx)->exceptionClassId)));
        ValueRef val(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->typeClassId));
        [[maybe_unused]] auto const e = JS_SetOpaque(val, typeAcquire(type));
        assert(e == 0); //TODO
#if defined DBG_UTIL
        getRuntimeData(ctx)->toFinalize.inc();
#endif
        JS_SetPropertyStr(
            ctx, val, "toString",
            JS_NewCFunction(ctx, typeToString, "toString", 0)); //TODO: add to prototype
        return val.release();
    });
}

JSValue unoTypeInterface(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, argc, argv] {
        assert(argc >= 1);
        if (JS_GetClassID(argv[0]) != getRuntimeData(ctx)->interfaceClassId)
        {
            JS_ThrowTypeError(ctx, "TODO: BAD UNO TYPE VALUE");
            throw JsException();
        }
        css::uno::Type const type(static_cast<typelib_TypeDescriptionReference*>(
            JS_GetOpaque(argv[0], getRuntimeData(ctx)->interfaceClassId)));
        ValueRef val(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->typeClassId));
        [[maybe_unused]] auto const e = JS_SetOpaque(val, typeAcquire(type));
        assert(e == 0); //TODO
#if defined DBG_UTIL
        getRuntimeData(ctx)->toFinalize.inc();
#endif
        JS_SetPropertyStr(
            ctx, val, "toString",
            JS_NewCFunction(ctx, typeToString, "toString", 0)); //TODO: add to prototype
        return val.release();
    });
}

void enumFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    typelib_typedescriptionreference_release(static_cast<typelib_TypeDescriptionReference*>(
        JS_GetOpaque(val, getRuntimeData(rt)->enumClassId)));
}

void structFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    static_cast<css::reflection::XStructTypeDescription*>(
        JS_GetOpaque(val, getRuntimeData(rt)->structClassId))
        ->release();
}

JSValue structCtor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, new_target, argc, argv] {
        ValueRef data(ctx, JS_GetPropertyStr(ctx, new_target, "'data"));
        if (JS_IsException(data))
        {
            throw JsException();
        }
        css::uno::Reference td(static_cast<css::reflection::XStructTypeDescription*>(
            JS_GetOpaque(data, getRuntimeData(ctx)->structClassId)));
        int argIdx = 0;
        auto const params = td->getTypeParameters();
        std::vector<css::uno::Type> typeArgs;
        OUStringBuffer buf(td->getName());
        if (params.hasElements())
        {
            assert(!td->getBaseType());
            if (argc < 1)
            {
                JS_ThrowSyntaxError(ctx, "TODO: MISSING TYPE ARGUMENTS");
                throw JsException();
            }
            if (!JS_IsArray(argv[0]))
            {
                JS_ThrowTypeError(ctx, "TODO: BAD TYPE ARGUMENTS");
                throw JsException();
            }
            ValueRef len1(ctx, JS_GetPropertyStr(ctx, argv[0], "length"));
            if (JS_IsException(len1))
            {
                throw JsException();
            }
            std::int64_t len2;
            auto const e = JS_ToInt64(ctx, &len2, len1);
            if (e != 0)
            {
                throw JsException();
            }
            if (len2 != params.getLength())
            {
                JS_ThrowTypeError(ctx, "TODO: BAD TYPE ARGUMENTS");
                throw JsException();
            }
            buf.append('<');
            for (std::int64_t i = 0; i != len2; ++i)
            {
                if (i != 0)
                {
                    buf.append(',');
                }
                ValueRef arg(ctx, JS_GetPropertyInt64(ctx, argv[0], i));
                if (JS_IsException(arg))
                {
                    throw JsException();
                }
                if (JS_GetClassID(arg) != getRuntimeData(ctx)->typeClassId)
                {
                    JS_ThrowTypeError(ctx, "TODO: BAD TYPE ARGUMENT");
                    throw JsException();
                }
                css::uno::Type argType(static_cast<typelib_TypeDescriptionReference*>(
                    JS_GetOpaque(arg, getRuntimeData(ctx)->typeClassId)));
                switch (argType.getTypeClass())
                {
                    case css::uno::TypeClass_VOID:
                    case css::uno::TypeClass_UNSIGNED_SHORT:
                    case css::uno::TypeClass_UNSIGNED_LONG:
                    case css::uno::TypeClass_UNSIGNED_HYPER:
                    case css::uno::TypeClass_EXCEPTION:
                        JS_ThrowTypeError(ctx, "TODO: BAD TYPE ARGUMENT");
                        throw JsException();
                    default:
                        break;
                }
                typeArgs.push_back(argType);
                buf.append(argType.getTypeName());
            }
            buf.append('>');
            ++argIdx;
        }
        css::uno::Type type(css::uno::TypeClass_STRUCT, buf.makeStringAndClear());
        ValueRef proto(ctx, JS_GetPropertyStr(ctx, new_target, "prototype"));
        if (JS_IsException(proto))
        {
            throw JsException();
        }
        ValueRef obj(ctx, JS_NewObjectProtoClass(ctx, proto, getRuntimeData(ctx)->compoundClassId));
        if (JS_IsException(obj))
        {
            throw JsException();
        }
        JS_SetOpaque(obj, typeAcquire(type));
#if defined DBG_UTIL
        getRuntimeData(ctx)->toFinalize.inc();
#endif
        for (css::uno::Reference<css::reflection::XCompoundTypeDescription> ctd(
                 td, css::uno::UNO_QUERY_THROW);
             ;)
        {
            auto const memberNames = ctd->getMemberNames();
            auto const memberTypes = ctd->getMemberTypes();
            assert(memberNames.getLength() == memberTypes.getLength());
            for (sal_Int32 i = 0; i != memberNames.getLength(); ++i)
            {
                auto const name = memberNames[i].toUtf8();
                AtomRef const a(ctx, JS_NewAtomLen(ctx, name.getStr(), name.getLength()));
                assert(a != JS_ATOM_NULL); //TODO
                ValueRef val(ctx);
                int has = 0;
                if (argc > argIdx)
                {
                    has = JS_HasProperty(ctx, argv[argIdx], a);
                    if (has == -1)
                    {
                        throw JsException();
                    }
                    if (has == 1)
                    {
                        val = JS_GetProperty(ctx, argv[argIdx], a);
                        if (JS_IsException(val))
                        {
                            throw JsException();
                        }
                    }
                }
                if (has == 0)
                {
                    css::uno::Type memType;
                    if (memberTypes[i]->getTypeClass() == css::uno::TypeClass_UNKNOWN)
                    {
                        sal_Int32 j = 0;
                        for (auto const& param : params)
                        {
                            if (memberTypes[i]->getName() == param)
                            {
                                break;
                            }
                            ++j;
                        }
                        assert(o3tl::make_unsigned(j) < typeArgs.size());
                        memType = typeArgs[j];
                    }
                    else
                    {
                        memType = css::uno::Type(memberTypes[i]->getTypeClass(),
                                                 memberTypes[i]->getName());
                    }
                    val = createDefaultValue(ctx, memType);
                }
                JS_SetProperty(ctx, obj, a, val.release());
            }
            auto const base = ctd->getBaseType();
            if (!base.is())
            {
                break;
            }
            ctd.set(base, css::uno::UNO_QUERY_THROW);
        }
        return obj.release();
    });
}

void exceptionFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    typelib_typedescriptionreference_release(static_cast<typelib_TypeDescriptionReference*>(
        JS_GetOpaque(val, getRuntimeData(rt)->exceptionClassId)));
}

JSValue exceptionCtor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, new_target, argc, argv] {
        ValueRef data(ctx, JS_GetPropertyStr(ctx, new_target, "'data"));
        if (JS_IsException(data))
        {
            throw JsException();
        }
        css::uno::Type const type(static_cast<typelib_TypeDescriptionReference*>(
            JS_GetOpaque(data, getRuntimeData(ctx)->exceptionClassId)));
        css::uno::TypeDescription desc(type);
        auto compDesc = reinterpret_cast<typelib_CompoundTypeDescription const*>(desc.get());
        ValueRef proto(ctx, JS_GetPropertyStr(ctx, new_target, "prototype"));
        if (JS_IsException(proto))
        {
            throw JsException();
        }
        ValueRef obj(ctx, JS_NewObjectProtoClass(ctx, proto, getRuntimeData(ctx)->compoundClassId));
        if (JS_IsException(obj))
        {
            throw JsException();
        }
        JS_SetOpaque(obj, typeAcquire(type));
#if defined DBG_UTIL
        getRuntimeData(ctx)->toFinalize.inc();
#endif
        for (;;)
        {
            for (sal_Int32 i = 0; i != compDesc->nMembers; ++i)
            {
                auto const name = OUString::unacquired(&compDesc->ppMemberNames[i]).toUtf8();
                AtomRef const a(ctx, JS_NewAtomLen(ctx, name.getStr(), name.getLength()));
                assert(a != JS_ATOM_NULL); //TODO
                ValueRef val(ctx);
                int has = 0;
                if (argc >= 1)
                {
                    has = JS_HasProperty(ctx, argv[0], a);
                    if (has == -1)
                    {
                        throw JsException();
                    }
                    if (has == 1)
                    {
                        val = JS_GetProperty(ctx, argv[0], a);
                        if (JS_IsException(val))
                        {
                            throw JsException();
                        }
                    }
                }
                if (has == 0)
                {
                    val = createDefaultValue(ctx, compDesc->ppTypeRefs[i]);
                }
                JS_SetProperty(ctx, obj, a, val.release());
            }
            compDesc = compDesc->pBaseTypeDescription;
            if (compDesc == nullptr)
            {
                break;
            }
        }
        return obj.release();
    });
}

void interfaceFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    typelib_typedescriptionreference_release(static_cast<typelib_TypeDescriptionReference*>(
        JS_GetOpaque(val, getRuntimeData(rt)->interfaceClassId)));
}

struct CtorData
{
    OUString service;
    css::uno::Reference<css::reflection::XServiceConstructorDescription> ctor;
};

void ctorFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    delete static_cast<CtorData*>(JS_GetOpaque(val, getRuntimeData(rt)->ctorClassId));
}

JSValue createService(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv, int,
                      JSValueConst* func_data)
{
    return callFromJs(ctx, [ctx, argc, argv, func_data] {
        auto const data
            = static_cast<CtorData*>(JS_GetOpaque(func_data[0], getRuntimeData(ctx)->ctorClassId));
        auto const params = data->ctor->getParameters();
        auto const variadic
            = params.hasElements() && params[params.getLength() - 1]->isRestParameter();
        if (variadic ? (argc < params.getLength()) : (argc != params.getLength() + 1))
        {
            JS_ThrowSyntaxError(ctx, "TODO: BAD NUMBER OF ARGUMENTS");
            throw JsException();
        }
        css::uno::Reference<css::uno::XComponentContext> context(
            fromJs(ctx, cppu::UnoType<css::uno::XComponentContext>::get(), argv[0]),
            css::uno::UNO_QUERY_THROW);
        css::uno::Sequence<css::uno::Any> args(argc - 1);
        for (sal_Int32 i = 0; i != params.getLength(); ++i)
        {
            assert(params[i]->isIn());
            assert(!params[i]->isOut());
            if (params[i]->isRestParameter())
            {
                assert(i == params.getLength() - 1);
                for (sal_Int32 j = i; j != argc - 1; ++j)
                {
                    args.getArray()[j] = fromJs(ctx,
                                                css::uno::Type(params[i]->getType()->getTypeClass(),
                                                               params[i]->getType()->getName()),
                                                argv[j + 1]);
                }
            }
            else
            {
                args.getArray()[i] = fromJs(ctx,
                                            css::uno::Type(params[i]->getType()->getTypeClass(),
                                                           params[i]->getType()->getName()),
                                            argv[i + 1]);
            }
        }
        css::uno::Reference<css::uno::XInterface> ifc(
            data->ctor->isDefaultConstructor()
                ? context->getServiceManager()->createInstanceWithContext(data->service, context)
                : context->getServiceManager()->createInstanceWithArgumentsAndContext(
                      data->service, args, context),
            css::uno::UNO_SET_THROW);
        return toJs(ctx, css::uno::Any(ifc)).release();
    });
}

void singletonFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    rtl_uString_release(
        static_cast<rtl_uString*>(JS_GetOpaque(val, getRuntimeData(rt)->singletonClassId)));
}

JSValue getSingleton(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv, int,
                     JSValueConst* func_data)
{
    return callFromJs(ctx, [ctx, argc, argv, func_data] {
        assert(argc >= 1);
        auto const s = static_cast<rtl_uString*>(
            JS_GetOpaque(func_data[0], getRuntimeData(ctx)->singletonClassId));
        css::uno::Reference<css::uno::XComponentContext> context(
            fromJs(ctx, cppu::UnoType<css::uno::XComponentContext>::get(), argv[0]),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::uno::XInterface> ifc(
            context->getValueByName("/singletons/" + OUString::unacquired(&s)),
            css::uno::UNO_QUERY_THROW);
        return toJs(ctx, css::uno::Any(ifc)).release();
    });
}

void moduleFinalizer(JSRuntime* rt, JSValueConst val)
{
#if defined DBG_UTIL
    getRuntimeData(rt)->toFinalize.dec();
#endif
    rtl_uString_release(
        static_cast<rtl_uString*>(JS_GetOpaque(val, getRuntimeData(rt)->moduleClassId)));
}

JSValue moduleGetProperty(JSContext* ctx, JSValueConst obj, JSAtom atom, JSValueConst receiver)
{
    ValueRef const v(ctx, JS_AtomToString(ctx, atom));
    if (!JS_IsString(v))
    {
        return JS_UNDEFINED;
    }
    auto const s = static_cast<rtl_uString*>(JS_GetOpaque(obj, getRuntimeData(ctx)->moduleClassId));
    OUStringBuffer buf(OUString::unacquired(&s));
    if (!buf.isEmpty())
    {
        buf.append('.');
    }
    buf.append(OUString::fromUtf8(JS_AtomToCString(ctx, atom)));
    auto const id = buf.makeStringAndClear();
    css::uno::Reference<css::container::XHierarchicalNameAccess> mgr(
        comphelper::getProcessComponentContext()->getValueByName(
            u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr),
        css::uno::UNO_QUERY_THROW);
    if (!mgr->hasByHierarchicalName(id))
    {
        return JS_UNDEFINED;
    }
    css::uno::Reference<css::reflection::XTypeDescription> td(mgr->getByHierarchicalName(id),
                                                              css::uno::UNO_QUERY_THROW);
    auto const tc = td->getTypeClass();
    ValueRef val(ctx);
    switch (tc)
    {
        case css::uno::TypeClass_ENUM:
        {
            css::uno::Type type(tc, id);
            val = JS_NewObjectClass(ctx, getRuntimeData(ctx)->enumClassId);
            [[maybe_unused]] auto e = JS_SetOpaque(val, typeAcquire(type));
            assert(e == 0); //TODO
#if defined DBG_UTIL
            getRuntimeData(ctx)->toFinalize.inc();
#endif
            css::uno::TypeDescription desc(type);
            auto const enumDesc = reinterpret_cast<typelib_EnumTypeDescription const*>(desc.get());
            for (sal_Int32 i = 0; i != enumDesc->nEnumValues; ++i)
            {
                ValueRef val2(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->enumeratorClassId));
                e = JS_SetOpaque(
                    val2, new EnumeratorData{ type, enumDesc->pEnumValues[i],
                                              OUString::unacquired(&enumDesc->ppEnumNames[i]) });
                assert(e == 0); //TODO
#if defined DBG_UTIL
                getRuntimeData(ctx)->toFinalize.inc();
#endif
                JS_SetPropertyStr(ctx, val2, "toString",
                                  JS_NewCFunction(ctx, enumeratorToString, "toString",
                                                  0)); //TODO: add to prototype
                JS_SetPropertyStr(ctx, val,
                                  OUString::unacquired(&enumDesc->ppEnumNames[i]).toUtf8().getStr(),
                                  val2.release());
            }
            break;
        }
        case css::uno::TypeClass_STRUCT:
        {
            css::uno::Reference<css::reflection::XStructTypeDescription> std(
                td, css::uno::UNO_QUERY_THROW);
            assert(!std->getTypeArguments().hasElements());
            ValueRef proto(ctx, JS_NewObject(ctx));
            assert(!JS_IsException(proto)); //TODO
            val = JS_NewCFunction2(ctx, structCtor, "TODO", 0, JS_CFUNC_constructor, 0);
            assert(!JS_IsException(val)); //TODO
            JS_SetConstructor(ctx, val, proto);
            ValueRef data(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->structClassId));
            [[maybe_unused]] auto const e = JS_SetOpaque(data, std.get());
            assert(e == 0); //TODO
            std->acquire();
#if defined DBG_UTIL
            getRuntimeData(ctx)->toFinalize.inc();
#endif
            JS_DefinePropertyValueStr(ctx, val, "'data", data.release(), 0);
            break;
        }
        case css::uno::TypeClass_EXCEPTION:
        {
            ValueRef proto(ctx, JS_NewObject(ctx));
            assert(!JS_IsException(proto)); //TODO
            val = JS_NewCFunction2(ctx, exceptionCtor, "TODO", 0, JS_CFUNC_constructor, 0);
            assert(!JS_IsException(val)); //TODO
            JS_SetConstructor(ctx, val, proto);
            ValueRef data(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->exceptionClassId));
            [[maybe_unused]] auto const e = JS_SetOpaque(data, typeAcquire(css::uno::Type(tc, id)));
            assert(e == 0); //TODO
#if defined DBG_UTIL
            getRuntimeData(ctx)->toFinalize.inc();
#endif
            JS_DefinePropertyValueStr(ctx, val, "'data", data.release(), 0);
            break;
        }
        case css::uno::TypeClass_INTERFACE:
        {
            val = JS_NewObjectClass(ctx, getRuntimeData(ctx)->interfaceClassId);
            [[maybe_unused]] auto const e = JS_SetOpaque(val, typeAcquire(css::uno::Type(tc, id)));
            assert(e == 0); //TODO
#if defined DBG_UTIL
            getRuntimeData(ctx)->toFinalize.inc();
#endif
            break;
        }
        case css::uno::TypeClass_MODULE:
        {
            val = JS_NewObjectClass(ctx, getRuntimeData(ctx)->moduleClassId);
            [[maybe_unused]] auto const e = JS_SetOpaque(val, stringAcquire(id));
            assert(e == 0); //TODO
#if defined DBG_UTIL
            getRuntimeData(ctx)->toFinalize.inc();
#endif
            break;
        }
        case css::uno::TypeClass_CONSTANTS:
        {
            val = JS_NewObject(ctx);
            if (JS_IsException(val))
            {
                throw JsException();
            }
            css::uno::Reference<css::reflection::XConstantsTypeDescription> cstd(
                td, css::uno::UNO_QUERY_THROW);
            for (auto const& ctd : cstd->getConstants())
            {
                ValueRef con = toJs(ctx, ctd->getConstantValue());
                auto const name = ctd->getName();
                auto const n = name.lastIndexOf('.');
                if (JS_SetPropertyStr(ctx, val, name.copy(n + 1).toUtf8().getStr(), con.release())
                    == -1)
                {
                    throw JsException();
                }
            }
            break;
        }
        case css::uno::TypeClass_SERVICE:
        {
            css::uno::Reference<css::reflection::XServiceTypeDescription2> std(
                td, css::uno::UNO_QUERY_THROW);
            if (!std->isSingleInterfaceBased())
            {
                return JS_UNDEFINED;
            }
            val = JS_NewObject(ctx);
            if (JS_IsException(val))
            {
                throw JsException();
            }
            for (auto const& ctor : std->getConstructors())
            {
                ValueRef data(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->ctorClassId));
                [[maybe_unused]] auto const e = JS_SetOpaque(data, new CtorData{ id, ctor });
                assert(e == 0); //TODO
#if defined DBG_UTIL
                getRuntimeData(ctx)->toFinalize.inc();
#endif
                ValueRef fun(ctx, JS_NewCFunctionData(ctx, createService,
                                                      ctor->getParameters().getLength() + 1, 0, 1,
                                                      data.ptr()));
                if (JS_IsException(fun))
                {
                    throw JsException();
                }
                if (JS_SetPropertyStr(
                        ctx, val,
                        ctor->isDefaultConstructor() ? "create" : ctor->getName().toUtf8().getStr(),
                        fun.release())
                    == -1)
                {
                    throw JsException();
                }
            }
            break;
        }
        case css::uno::TypeClass_SINGLETON:
        {
            ValueRef data(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->singletonClassId));
            [[maybe_unused]] auto const e = JS_SetOpaque(data, stringAcquire(id));
            assert(e == 0); //TODO
#if defined DBG_UTIL
            getRuntimeData(ctx)->toFinalize.inc();
#endif
            val = JS_NewCFunctionData(ctx, getSingleton, 1, 0, 1, data.ptr());
            if (JS_IsException(val))
            {
                throw JsException();
            }
            break;
        }
        case css::uno::TypeClass_TYPEDEF:
            return JS_UNDEFINED;
        default:
            O3TL_UNREACHABLE;
    }
    JS_SetProperty(ctx, receiver, atom, val.dup());
    return val.release();
}

ValueRef getUnoidlRepresentation(JSContext* ctx, std::u16string_view id)
{
    ValueRef const global(ctx, JS_GetGlobalObject(ctx));
    ValueRef const uno(ctx, JS_GetPropertyStr(ctx, global, "uno"));
    assert(JS_IsObject(uno));
    ValueRef obj(ctx, JS_GetPropertyStr(ctx, uno, "idl"));
    assert(JS_IsObject(obj));
    std::size_t idx = 0;
    do
    {
        ValueRef next(
            ctx,
            JS_GetPropertyStr(ctx, obj, OUString(o3tl::getToken(id, u'.', idx)).toUtf8().getStr()));
        assert(JS_IsObject(next)); //TODO
        obj = std::move(next);
    } while (idx != std::u16string_view::npos);
    return obj;
}

ValueRef createDefaultValue(JSContext* ctx, css::uno::Type const& type)
{
    switch (type.getTypeClass())
    {
        case css::uno::TypeClass_BOOLEAN:
            return ValueRef(ctx, JS_NewBool(ctx, false));
        case css::uno::TypeClass_BYTE:
        case css::uno::TypeClass_SHORT:
        case css::uno::TypeClass_UNSIGNED_SHORT:
        case css::uno::TypeClass_LONG:
        case css::uno::TypeClass_UNSIGNED_LONG:
        case css::uno::TypeClass_FLOAT:
        case css::uno::TypeClass_DOUBLE:
            return ValueRef(ctx, JS_NewInt32(ctx, 0));
        case css::uno::TypeClass_HYPER:
        case css::uno::TypeClass_UNSIGNED_HYPER:
            return ValueRef(ctx, JS_NewBigInt64(ctx, 0));
        case css::uno::TypeClass_CHAR:
            return ValueRef(ctx, JS_NewStringLen(ctx, "\0", 1));
        case css::uno::TypeClass_STRING:
            return ValueRef(ctx, JS_NewString(ctx, ""));
        case css::uno::TypeClass_TYPE:
            return mapTypeToJs(ctx, cppu::UnoType<void>::get());
        case css::uno::TypeClass_ANY:
            return ValueRef(ctx, JS_UNDEFINED);
        case css::uno::TypeClass_SEQUENCE:
            return ValueRef(ctx, JS_NewArray(ctx));
        case css::uno::TypeClass_ENUM:
        {
            css::uno::TypeDescription desc(type);
            auto const enumDesc = reinterpret_cast<typelib_EnumTypeDescription const*>(desc.get());
            auto val = getUnoidlRepresentation(
                ctx, Concat2View(type.getTypeName() + "."
                                 + OUString::unacquired(&enumDesc->ppEnumNames[0])));
            assert(JS_GetClassID(val) == getRuntimeData(ctx)->enumeratorClassId); //TODO
            return val;
        }
        case css::uno::TypeClass_STRUCT:
        {
            auto const id = type.getTypeName();
            auto n = id.indexOf('<');
            auto const rep = getUnoidlRepresentation(ctx, n == -1 ? id : id.subView(0, n));
            ValueRef val(ctx);
            if (n == -1)
            {
                val = JS_CallConstructor(ctx, rep, 0, nullptr);
                if (JS_IsException(val))
                {
                    throw JsException();
                }
            }
            else
            {
                ValueRef args(ctx, JS_NewArray(ctx));
                if (JS_IsException(args))
                {
                    throw JsException();
                }
                std::uint32_t arg = 0;
                assert(id.endsWith(">"));
                sal_Int32 nest = 0;
                ++n;
                css::uno::Reference<css::container::XHierarchicalNameAccess> mgr(
                    comphelper::getProcessComponentContext()->getValueByName(
                        u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr),
                    css::uno::UNO_QUERY_THROW);
                for (sal_Int32 i = n;; ++i)
                {
                    if (i == id.getLength() - 1 || (nest == 0 && id[i] == ','))
                    {
                        assert(nest == 0);
                        css::uno::Reference<css::reflection::XTypeDescription> td(
                            mgr->getByHierarchicalName(id.copy(n, i - n)),
                            css::uno::UNO_QUERY_THROW);
                        if (JS_SetPropertyUint32(
                                ctx, args, arg++,
                                mapTypeToJs(ctx, css::uno::Type(td->getTypeClass(), td->getName()))
                                    .release())
                            == -1)
                        {
                            throw JsException();
                        }
                        if (i == id.getLength() - 1)
                        {
                            break;
                        }
                        n = i + 1;
                    }
                    else if (id[i] == '<')
                    {
                        ++nest;
                    }
                    else if (id[i] == '>')
                    {
                        assert(nest != 0);
                        --nest;
                    }
                }
                val = JS_CallConstructor(ctx, rep, 1, args.ptr());
                if (JS_IsException(val))
                {
                    throw JsException();
                }
            }
            return val;
        }
        case css::uno::TypeClass_INTERFACE:
            return ValueRef(ctx, JS_NULL);
        default:
            O3TL_UNREACHABLE;
    }
}

JSValue unoAny(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, new_target, argc, argv] {
        assert(argc >= 1);
        ValueRef proto(ctx, JS_GetPropertyStr(ctx, new_target, "prototype"));
        if (JS_IsException(proto))
        {
            throw JsException();
        }
        ValueRef obj(ctx, JS_NewObjectProto(ctx, proto));
        if (JS_IsException(obj))
        {
            throw JsException();
        }
        JS_SetPropertyStr(ctx, obj, "type", JS_DupValue(ctx, argv[0]));
        JS_SetPropertyStr(ctx, obj, "val", argc == 1 ? JS_UNDEFINED : JS_DupValue(ctx, argv[1]));
        return obj.release();
    });
}

css::uno::Any fromJsString(JSContext* ctx, JSValueConst val)
{
    std::size_t n;
    UniqueCString16 const p(ctx, JS_ToCStringLenUTF16(ctx, &n, val));
    if (p.get() == nullptr)
    {
        throw JsException();
    }
    if (n > std::numeric_limits<sal_Int32>::max())
    {
        JS_ThrowRangeError(ctx, "Bad UNO string value");
        throw JsException();
    }
    OUString s(p.get(), n);
    //TODO: check for valid UTF-16
    return css::uno::Any(s);
}

css::uno::Any fromJsSequence(JSContext* ctx, css::uno::Type const& type, JSValueConst val)
{
    if (!JS_IsArray(val))
    {
        JS_ThrowTypeError(ctx, "TODO: BAD UNO SEQUENCE VALUE");
        throw JsException();
    }
    ValueRef len1(ctx, JS_GetPropertyStr(ctx, val, "length"));
    if (JS_IsException(len1))
    {
        throw JsException();
    }
    std::int64_t len2;
    auto const e = JS_ToInt64(ctx, &len2, len1);
    if (e != 0)
    {
        throw JsException();
    }
    if (len2 < 0 || len2 > std::numeric_limits<sal_Int32>::max())
    {
        JS_ThrowTypeError(ctx, "TODO: BAD UNO SEQUENCE VALUE");
        throw JsException();
    }
    css::uno::TypeDescription desc(type);
    css::uno::TypeDescription elemDesc(
        reinterpret_cast<typelib_IndirectTypeDescription const*>(desc.get())->pType);
    uno_Sequence* seq;
    uno_sequence_construct(&seq, desc.get(), nullptr, len2, css::uno::cpp_acquire);
    for (std::int64_t i = 0; i != len2; ++i)
    {
        ValueRef elem(ctx, JS_GetPropertyInt64(ctx, val, i));
        if (JS_IsException(elem))
        {
            uno_type_sequence_destroy(seq, desc.get()->pWeakRef, css::uno::cpp_release);
            throw JsException();
        }
        auto const any = fromJs(ctx, elemDesc.get()->pWeakRef, elem);
        uno_copyData(
            seq->elements + (i * elemDesc.get()->nSize),
            const_cast<void*>(type == cppu::UnoType<css::uno::Sequence<css::uno::Any>>::get()
                                  ? &any
                                  : any.getValue()),
            elemDesc.get(), css::uno::cpp_acquire);
    }
    return { &seq, type };
}

css::uno::Any fromJsEnum(JSContext* ctx, JSValueConst val)
{
    if (JS_GetClassID(val) != getRuntimeData(ctx)->enumeratorClassId)
    {
        JS_ThrowTypeError(ctx, "TODO: BAD UNO ENUM VALUE");
        throw JsException();
    }
    auto const data = static_cast<EnumeratorData const*>(
        JS_GetOpaque(val, getRuntimeData(ctx)->enumeratorClassId));
    return { &data->value, data->type };
}

css::uno::Any fromJsInterface(JSContext* ctx, css::uno::Type const& type, JSValueConst val)
{
    if (JS_IsNull(val))
    {
        return { nullptr, type };
    }
    if (JS_GetClassID(val) != getRuntimeData(ctx)->wrapperClassId)
    {
        JS_ThrowTypeError(ctx, "TODO: BAD UNO ENUM VALUE");
        throw JsException();
    }
    auto const a = css::uno::Reference(static_cast<css::uno::XInterface*>(
                                           JS_GetOpaque(val, getRuntimeData(ctx)->wrapperClassId)))
                       ->queryInterface(type);
    if (!a.hasValue())
    {
        JS_ThrowTypeError(ctx, "TODO: BAD UNO INTERFACE VALUE");
        throw JsException();
    }
    return a;
}

css::uno::Any fromJs(JSContext* ctx, css::uno::Type const& type, JSValueConst val)
{
    switch (type.getTypeClass())
    {
        case css::uno::TypeClass_VOID:
            return {};
        case css::uno::TypeClass_BOOLEAN:
        {
            auto const n = JS_ToBool(ctx, val);
            assert(n != -1);
            return css::uno::Any(n == 1);
        }
        case css::uno::TypeClass_BYTE:
        {
            double n;
            auto const e = JS_ToFloat64(ctx, &n, val);
            if (e != 0)
            {
                throw JsException();
            }
            n = std::trunc(n);
            if (n < SAL_MIN_INT8 || n > SAL_MAX_INT8)
            {
                JS_ThrowRangeError(ctx, "Bad UNO byte val %f", n);
                throw JsException();
            }
            return css::uno::Any(static_cast<sal_Int8>(n));
        }
        case css::uno::TypeClass_SHORT:
        {
            double n;
            auto const e = JS_ToFloat64(ctx, &n, val);
            if (e != 0)
            {
                throw JsException();
            }
            n = std::trunc(n);
            if (n < SAL_MIN_INT16 || n > SAL_MAX_INT16)
            {
                JS_ThrowRangeError(ctx, "Bad UNO short value %f", n);
                throw JsException();
            }
            return css::uno::Any(static_cast<sal_Int16>(n));
        }
        case css::uno::TypeClass_UNSIGNED_SHORT:
        {
            double n;
            auto const e = JS_ToFloat64(ctx, &n, val);
            if (e != 0)
            {
                throw JsException();
            }
            n = std::trunc(n);
            if (n < 0 || n > SAL_MAX_UINT16)
            {
                JS_ThrowRangeError(ctx, "Bad UNO unsigned short value %f", n);
                throw JsException();
            }
            return css::uno::Any(static_cast<sal_uInt16>(n));
        }
        case css::uno::TypeClass_LONG:
        {
            double n;
            auto const e = JS_ToFloat64(ctx, &n, val);
            if (e != 0)
            {
                throw JsException();
            }
            n = std::trunc(n);
            if (n < SAL_MIN_INT32 || n > SAL_MAX_INT32)
            {
                JS_ThrowRangeError(ctx, "Bad UNO short value %f", n);
                throw JsException();
            }
            return css::uno::Any(static_cast<sal_Int32>(n));
        }
        case css::uno::TypeClass_UNSIGNED_LONG:
        {
            double n;
            auto const e = JS_ToFloat64(ctx, &n, val);
            if (e != 0)
            {
                throw JsException();
            }
            n = std::trunc(n);
            if (n < 0 || n > SAL_MAX_UINT32)
            {
                JS_ThrowRangeError(ctx, "Bad UNO unsigned long value %f", n);
                throw JsException();
            }
            return css::uno::Any(static_cast<sal_uInt32>(n));
        }
        case css::uno::TypeClass_HYPER:
        {
            ValueRef const check(ctx, JS_Eval(ctx,
                                              RTL_CONSTASCII_STRINGPARAM(
                                                  "(v) => v >= -(2n ** 63n) && v < 2n ** 63n"),
                                              "<input>", JS_EVAL_TYPE_GLOBAL));
            ValueRef const res(ctx, JS_Call(ctx, check, JS_NULL, 1, &val));
            if (JS_IsException(res))
            {
                throw JsException();
            }
            auto const ok = JS_ToBool(ctx, res);
            assert(ok != -1);
            if (ok == 0)
            {
                JS_ThrowRangeError(ctx, "Bad UNO hyper value");
                throw JsException();
            }
            std::int64_t n;
            [[maybe_unused]] auto const e = JS_ToBigInt64(ctx, &n, val);
            assert(e == 0);
            return css::uno::Any(static_cast<sal_Int64>(n));
        }
        case css::uno::TypeClass_UNSIGNED_HYPER:
        {
            ValueRef const check(
                ctx, JS_Eval(ctx, RTL_CONSTASCII_STRINGPARAM("(v) => v >= 0n && v < 2n ** 64n"),
                             "<input>", JS_EVAL_TYPE_GLOBAL));
            ValueRef const res(ctx, JS_Call(ctx, check, JS_NULL, 1, &val));
            if (JS_IsException(res))
            {
                throw JsException();
            }
            auto const ok = JS_ToBool(ctx, res);
            assert(ok != -1);
            if (ok == 0)
            {
                JS_ThrowRangeError(ctx, "Bad UNO unsigned hyper value");
                throw JsException();
            }
            std::uint64_t n;
            [[maybe_unused]] auto const e = JS_ToBigUint64(ctx, &n, val);
            assert(e == 0);
            return css::uno::Any(static_cast<sal_uInt64>(n));
        }
        case css::uno::TypeClass_FLOAT:
        {
            double n;
            auto const e = JS_ToFloat64(ctx, &n, val);
            if (e != 0)
            {
                throw JsException();
            }
            return css::uno::Any(static_cast<float>(n));
        }
        case css::uno::TypeClass_DOUBLE:
        {
            double n;
            auto const e = JS_ToFloat64(ctx, &n, val);
            if (e != 0)
            {
                throw JsException();
            }
            return css::uno::Any(n);
        }
        case css::uno::TypeClass_CHAR:
        {
            std::size_t n;
            UniqueCString16 const p(ctx, JS_ToCStringLenUTF16(ctx, &n, val));
            if (p.get() == nullptr)
            {
                throw JsException();
            }
            if (n != 1)
            {
                JS_ThrowRangeError(ctx, "Bad UNO char value");
                throw JsException();
            }
            auto const c = p.get()[0];
            return css::uno::Any(char16_t(c));
        }
        case css::uno::TypeClass_STRING:
            return fromJsString(ctx, val);
        case css::uno::TypeClass_TYPE:
            if (JS_GetClassID(val) != getRuntimeData(ctx)->typeClassId)
            {
                JS_ThrowTypeError(ctx, "TODO: BAD UNO TYPE VALUE");
                throw JsException();
            }
            return css::uno::Any(css::uno::Type(static_cast<typelib_TypeDescriptionReference*>(
                JS_GetOpaque(val, getRuntimeData(ctx)->typeClassId))));
        case css::uno::TypeClass_ANY:
            if (JS_IsUndefined(val))
            {
                return {};
            }
            if (JS_IsBool(val))
            {
                auto const b = JS_ToBool(ctx, val);
                if (b == -1)
                {
                    throw JsException();
                }
                return css::uno::Any(b == 1);
            }
            if (JS_IsNumber(val))
            {
                double n;
                [[maybe_unused]] auto const e = JS_ToFloat64(ctx, &n, val); //TODO
                assert(e == 0); //TODO
                if (n == std::trunc(n))
                {
                    if (n >= SAL_MIN_INT32 && n <= SAL_MAX_INT32)
                    {
                        return css::uno::Any(sal_Int32(n));
                    }
                    if (n >= 0 && n <= SAL_MAX_UINT32)
                    {
                        return css::uno::Any(sal_uInt32(n));
                    }
                }
                return css::uno::Any(n);
            }
            if (JS_IsBigInt(val))
            {
                ValueRef const check1(ctx, JS_Eval(ctx,
                                                   RTL_CONSTASCII_STRINGPARAM(
                                                       "(v) => v >= -(2n ** 63n) && v < 2n ** 63n"),
                                                   "<input>", JS_EVAL_TYPE_GLOBAL));
                ValueRef const res1(ctx, JS_Call(ctx, check1, JS_NULL, 1, &val));
                if (JS_IsException(res1))
                {
                    throw JsException();
                }
                auto const ok1 = JS_ToBool(ctx, res1);
                assert(ok1 != -1);
                if (ok1 == 1)
                {
                    std::int64_t n;
                    [[maybe_unused]] auto const e = JS_ToBigInt64(ctx, &n, val);
                    assert(e == 0);
                    return css::uno::Any(static_cast<sal_Int64>(n));
                }
                ValueRef const check2(
                    ctx, JS_Eval(ctx, RTL_CONSTASCII_STRINGPARAM("(v) => v >= 0n && v < 2n ** 64n"),
                                 "<input>", JS_EVAL_TYPE_GLOBAL));
                ValueRef const res2(ctx, JS_Call(ctx, check2, JS_NULL, 1, &val));
                if (JS_IsException(res2))
                {
                    throw JsException();
                }
                auto const ok2 = JS_ToBool(ctx, res2);
                assert(ok2 != -1);
                if (ok2 == 1)
                {
                    std::uint64_t n;
                    [[maybe_unused]] auto const e = JS_ToBigUint64(ctx, &n, val);
                    assert(e == 0);
                    return css::uno::Any(static_cast<sal_uInt64>(n));
                }
                JS_ThrowRangeError(ctx, "Bad BigInt UNO any value");
                throw JsException();
            }
            if (JS_IsString(val))
            {
                return fromJsString(ctx, val);
            }
            if (JS_IsArray(val))
            {
                return fromJsSequence(ctx, cppu::UnoType<css::uno::Sequence<css::uno::Any>>::get(),
                                      val);
            }
            if (JS_IsNull(val))
            {
                return css::uno::Any(css::uno::Reference<css::uno::XInterface>());
            }
            if (JS_IsObject(val))
            {
                if (JS_GetClassID(val) == getRuntimeData(ctx)->wrapperClassId)
                {
                    return fromJsInterface(ctx, cppu::UnoType<css::uno::XInterface>::get(), val);
                }
                if (JS_GetClassID(val) == getRuntimeData(ctx)->enumeratorClassId)
                {
                    return fromJsEnum(ctx, val);
                }
                if (JS_GetClassID(val) == getRuntimeData(ctx)->compoundClassId)
                {
                    return fromJs(ctx,
                                  css::uno::Type(static_cast<typelib_TypeDescriptionReference*>(
                                      JS_GetOpaque(val, getRuntimeData(ctx)->compoundClassId))),
                                  val);
                }
                if (JS_GetClassID(val) == getRuntimeData(ctx)->typeClassId)
                {
                    return css::uno::Any(
                        css::uno::Type(static_cast<typelib_TypeDescriptionReference*>(
                            JS_GetOpaque(val, getRuntimeData(ctx)->typeClassId))));
                }
                ValueRef const global(ctx, JS_GetGlobalObject(ctx));
                if (JS_IsException(global))
                {
                    throw JsException();
                }
                ValueRef const uno(ctx, JS_GetPropertyStr(ctx, global, "uno"));
                if (JS_IsException(uno))
                {
                    throw JsException();
                }
                ValueRef const anyCtor(ctx, JS_GetPropertyStr(ctx, uno, "Any"));
                if (JS_IsException(anyCtor))
                {
                    throw JsException();
                }
                auto const isAny = JS_IsInstanceOf(ctx, val, anyCtor);
                if (isAny == -1)
                {
                    throw JsException();
                }
                if (isAny == 1)
                {
                    ValueRef const anyType(ctx, JS_GetPropertyStr(ctx, val, "type"));
                    if (JS_IsException(anyType))
                    {
                        throw JsException();
                    }
                    ValueRef const anyVal(ctx, JS_GetPropertyStr(ctx, val, "val"));
                    if (JS_IsException(anyVal))
                    {
                        throw JsException();
                    }
                    if (JS_GetClassID(anyType) != getRuntimeData(ctx)->typeClassId)
                    {
                        JS_ThrowTypeError(ctx, "TODO: BAD ANY TYPE VALUE");
                        throw JsException();
                    }
                    return fromJs(ctx,
                                  css::uno::Type(static_cast<typelib_TypeDescriptionReference*>(
                                      JS_GetOpaque(anyType, getRuntimeData(ctx)->typeClassId))),
                                  anyVal);
                }
            }
            JS_ThrowRangeError(ctx, "TODO: BAD UNO ANY VALUE");
            throw JsException();
        case css::uno::TypeClass_SEQUENCE:
            return fromJsSequence(ctx, type, val);
        case css::uno::TypeClass_ENUM:
            return fromJsEnum(ctx, val);
        case css::uno::TypeClass_STRUCT:
        case css::uno::TypeClass_EXCEPTION:
        {
            css::uno::TypeDescription desc(type);
            auto compDesc = reinterpret_cast<typelib_CompoundTypeDescription const*>(desc.get());
            std::vector<css::uno::Any> mems;
            for (;;)
            {
                for (sal_Int32 i = 0; i != compDesc->nMembers; ++i)
                {
                    auto const name = OUString::unacquired(&compDesc->ppMemberNames[i]).toUtf8();
                    AtomRef const a(ctx, JS_NewAtomLen(ctx, name.getStr(), name.getLength()));
                    assert(a != JS_ATOM_NULL); //TODO
                    auto const has = JS_HasProperty(ctx, val, a);
                    if (has == -1)
                    {
                        throw JsException();
                    }
                    if (has == 0)
                    {
                        JS_ThrowTypeError(ctx, "TODO: BAD UNO STRUCT VALUE");
                        throw JsException();
                    }
                    ValueRef mem(ctx, JS_GetProperty(ctx, val, a));
                    if (JS_IsException(mem))
                    {
                        throw JsException();
                    }
                    mems.push_back(fromJs(ctx, compDesc->ppTypeRefs[i], mem));
                }
                compDesc = compDesc->pBaseTypeDescription;
                if (compDesc == nullptr)
                {
                    break;
                }
            }
            auto const buf = rtl_allocateMemory(desc.get()->nSize);
            std::size_t mem = 0;
            for (compDesc = reinterpret_cast<typelib_CompoundTypeDescription const*>(desc.get());;)
            {
                for (sal_Int32 i = 0; i != compDesc->nMembers; ++i)
                {
                    css::uno::TypeDescription memDesc(compDesc->ppTypeRefs[i]);
                    uno_copyData(static_cast<std::byte*>(buf) + compDesc->pMemberOffsets[i],
                                 const_cast<void*>(compDesc->ppTypeRefs[i]->eTypeClass
                                                           == typelib_TypeClass_ANY
                                                       ? &mems[mem]
                                                       : mems[mem].getValue()),
                                 memDesc.get(), css::uno::cpp_acquire);
                    ++mem;
                }
                compDesc = compDesc->pBaseTypeDescription;
                if (compDesc == nullptr)
                {
                    break;
                }
            }
            return { buf, type };
        }
        case css::uno::TypeClass_INTERFACE:
            return fromJsInterface(ctx, type, val);
        default:
            O3TL_UNREACHABLE;
    }
}

ValueRef mapTypeToJs(JSContext* ctx, css::uno::Type const& type)
{
    switch (type.getTypeClass())
    {
        case css::uno::TypeClass_VOID:
        case css::uno::TypeClass_BOOLEAN:
        case css::uno::TypeClass_BYTE:
        case css::uno::TypeClass_SHORT:
        case css::uno::TypeClass_UNSIGNED_SHORT:
        case css::uno::TypeClass_LONG:
        case css::uno::TypeClass_UNSIGNED_LONG:
        case css::uno::TypeClass_HYPER:
        case css::uno::TypeClass_UNSIGNED_HYPER:
        case css::uno::TypeClass_FLOAT:
        case css::uno::TypeClass_DOUBLE:
        case css::uno::TypeClass_CHAR:
        case css::uno::TypeClass_STRING:
        case css::uno::TypeClass_TYPE:
        case css::uno::TypeClass_ANY:
        {
            ValueRef const global(ctx, JS_GetGlobalObject(ctx));
            ValueRef const uno(ctx, JS_GetPropertyStr(ctx, global, "uno"));
            assert(JS_IsObject(uno));
            ValueRef typeObj(ctx, JS_GetPropertyStr(ctx, uno, "type"));
            assert(JS_IsObject(typeObj));
            ValueRef val(ctx,
                         JS_GetPropertyStr(ctx, typeObj, type.getTypeName().toUtf8().getStr()));
            assert(JS_GetClassID(val) == getRuntimeData(ctx)->typeClassId); //TODO
            return val;
        }
        case css::uno::TypeClass_SEQUENCE:
        {
            css::uno::TypeDescription desc(type);
            css::uno::TypeDescription elemDesc(
                reinterpret_cast<typelib_IndirectTypeDescription const*>(desc.get())->pType);
            auto elem = mapTypeToJs(ctx, elemDesc.get()->pWeakRef);
            ValueRef val(ctx, unoTypeSequence(ctx, JS_NULL, 1, elem.ptr()));
            if (JS_IsException(val))
            {
                throw JsException();
            }
            return val;
        }
        case css::uno::TypeClass_ENUM:
        {
            auto rep = getUnoidlRepresentation(ctx, type.getTypeName());
            ValueRef val(ctx, unoTypeEnum(ctx, JS_NULL, 1, rep.ptr()));
            if (JS_IsException(val))
            {
                throw JsException();
            }
            return val;
        }
        case css::uno::TypeClass_STRUCT:
        {
            auto rep = getUnoidlRepresentation(ctx, type.getTypeName());
            ValueRef val(ctx, unoTypeStruct(ctx, JS_NULL, 1, rep.ptr()));
            if (JS_IsException(val))
            {
                throw JsException();
            }
            return val;
        }
        case css::uno::TypeClass_EXCEPTION:
        {
            auto rep = getUnoidlRepresentation(ctx, type.getTypeName());
            ValueRef val(ctx, unoTypeException(ctx, JS_NULL, 1, rep.ptr()));
            if (JS_IsException(val))
            {
                throw JsException();
            }
            return val;
        }
        case css::uno::TypeClass_INTERFACE:
        {
            auto rep = getUnoidlRepresentation(ctx, type.getTypeName());
            ValueRef val(ctx, unoTypeInterface(ctx, JS_NULL, 1, rep.ptr()));
            if (JS_IsException(val))
            {
                throw JsException();
            }
            return val;
        }
        default:
            O3TL_UNREACHABLE;
    }
}

ValueRef toJs(JSContext* ctx, css::uno::Type const& type, void const* value)
{
    switch (type.getTypeClass())
    {
        case css::uno::TypeClass_VOID:
            return ValueRef(ctx, JS_UNDEFINED);
        case css::uno::TypeClass_BOOLEAN:
            return ValueRef(ctx, JS_NewBool(ctx, *static_cast<sal_Bool const*>(value)));
        case css::uno::TypeClass_BYTE:
            return ValueRef(ctx, JS_NewInt32(ctx, *static_cast<sal_Int8 const*>(value)));
        case css::uno::TypeClass_SHORT:
            return ValueRef(ctx, JS_NewInt32(ctx, *static_cast<sal_Int16 const*>(value)));
        case css::uno::TypeClass_UNSIGNED_SHORT:
            return ValueRef(ctx, JS_NewInt32(ctx, *static_cast<sal_uInt16 const*>(value)));
        case css::uno::TypeClass_LONG:
            return ValueRef(ctx, JS_NewInt32(ctx, *static_cast<sal_Int32 const*>(value)));
        case css::uno::TypeClass_UNSIGNED_LONG:
            return ValueRef(ctx, JS_NewUint32(ctx, *static_cast<sal_uInt32 const*>(value)));
        case css::uno::TypeClass_HYPER:
            return ValueRef(ctx, JS_NewBigInt64(ctx, *static_cast<sal_Int64 const*>(value)));
        case css::uno::TypeClass_UNSIGNED_HYPER:
            return ValueRef(ctx, JS_NewBigUint64(ctx, *static_cast<sal_uInt64 const*>(value)));
        case css::uno::TypeClass_FLOAT:
            return ValueRef(ctx, JS_NewFloat64(ctx, *static_cast<float const*>(value)));
        case css::uno::TypeClass_DOUBLE:
            return ValueRef(ctx, JS_NewFloat64(ctx, *static_cast<double const*>(value)));
        case css::uno::TypeClass_CHAR:
        {
            auto const s = *static_cast<char16_t const*>(value);
            return ValueRef(ctx,
                            JS_NewStringUTF16(ctx, reinterpret_cast<std::uint16_t const*>(&s), 1));
        }
        case css::uno::TypeClass_STRING:
        {
            auto const& s = OUString::unacquired(static_cast<rtl_uString* const*>(value));
            return ValueRef(
                ctx, JS_NewStringUTF16(ctx, reinterpret_cast<std::uint16_t const*>(s.getStr()),
                                       s.getLength()));
        }
        case css::uno::TypeClass_TYPE:
            return mapTypeToJs(ctx, *static_cast<typelib_TypeDescriptionReference* const*>(value));
        case css::uno::TypeClass_ANY:
        {
            auto const any = static_cast<uno_Any const*>(value);
            return toJs(ctx, *reinterpret_cast<css::uno::Type const*>(&any->pType), any->pData);
        }
        case css::uno::TypeClass_SEQUENCE:
        {
            auto const seq = *static_cast<uno_Sequence* const*>(value);
            css::uno::TypeDescription desc(type);
            css::uno::TypeDescription elemDesc(
                reinterpret_cast<typelib_IndirectTypeDescription const*>(desc.get())->pType);
            std::vector<ValueRef> refs;
            for (sal_Int32 i = 0; i != seq->nElements; ++i)
            {
                refs.push_back(toJs(ctx, elemDesc.get()->pWeakRef,
                                    seq->elements + (i * elemDesc.get()->nSize)));
            }
            std::vector<JSValue> vals;
            for (auto& ref : refs)
            {
                vals.push_back(ref);
            }
            ValueRef arr(ctx, JS_NewArrayFrom(ctx, vals.size(), vals.data()));
            for (auto& ref : refs)
            {
                ref.release();
            }
            if (JS_IsException(arr))
            {
                throw JsException();
            }
            return arr;
        }
        case css::uno::TypeClass_ENUM:
        {
            auto const val = *static_cast<sal_Int32 const*>(value);
            css::uno::TypeDescription desc(type);
            auto const enumDesc = reinterpret_cast<typelib_EnumTypeDescription const*>(desc.get());
            sal_Int32 i = 0;
            for (; i != enumDesc->nEnumValues; ++i)
            {
                if (enumDesc->pEnumValues[i] == val)
                {
                    break;
                }
            }
            assert(i != enumDesc->nEnumValues); //TODO
            auto obj = getUnoidlRepresentation(
                ctx, Concat2View(type.getTypeName() + "."
                                 + OUString::unacquired(&enumDesc->ppEnumNames[i])));
            assert(JS_GetClassID(obj) == getRuntimeData(ctx)->enumeratorClassId); //TODO
            return obj;
        }
        case css::uno::TypeClass_STRUCT:
        case css::uno::TypeClass_EXCEPTION:
        {
            ValueRef mems(ctx, JS_NewObject(ctx));
            if (JS_IsException(mems))
            {
                throw JsException();
            }
            css::uno::TypeDescription desc(type);
            auto compDesc = reinterpret_cast<typelib_CompoundTypeDescription const*>(desc.get());
            for (;;)
            {
                for (sal_Int32 i = 0; i != compDesc->nMembers; ++i)
                {
                    auto mem
                        = toJs(ctx, compDesc->ppTypeRefs[i],
                               static_cast<std::byte const*>(value) + compDesc->pMemberOffsets[i]);
                    if (JS_SetPropertyStr(
                            ctx, mems,
                            OUString::unacquired(&compDesc->ppMemberNames[i]).toUtf8().getStr(),
                            mem.release())
                        == -1)
                    {
                        throw JsException();
                    }
                }
                compDesc = compDesc->pBaseTypeDescription;
                if (compDesc == nullptr)
                {
                    break;
                }
            }
            auto const id = type.getTypeName();
            auto n = id.indexOf('<');
            auto const rep = getUnoidlRepresentation(ctx, n == -1 ? id : id.subView(0, n));
            ValueRef val(ctx);
            if (n == -1)
            {
                val = JS_CallConstructor(ctx, rep, 1, mems.ptr());
                if (JS_IsException(val))
                {
                    throw JsException();
                }
            }
            else
            {
                ValueRef args(ctx, JS_NewArray(ctx));
                if (JS_IsException(args))
                {
                    throw JsException();
                }
                std::uint32_t arg = 0;
                assert(id.endsWith(">"));
                sal_Int32 nest = 0;
                ++n;
                css::uno::Reference<css::container::XHierarchicalNameAccess> mgr(
                    comphelper::getProcessComponentContext()->getValueByName(
                        u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr),
                    css::uno::UNO_QUERY_THROW);
                for (sal_Int32 i = n;; ++i)
                {
                    if (i == id.getLength() - 1 || (nest == 0 && id[i] == ','))
                    {
                        assert(nest == 0);
                        css::uno::Reference<css::reflection::XTypeDescription> td(
                            mgr->getByHierarchicalName(id.copy(n, i - n)),
                            css::uno::UNO_QUERY_THROW);
                        if (JS_SetPropertyUint32(
                                ctx, args, arg++,
                                mapTypeToJs(ctx, css::uno::Type(td->getTypeClass(), td->getName()))
                                    .release())
                            == -1)
                        {
                            throw JsException();
                        }
                        if (i == id.getLength() - 1)
                        {
                            break;
                        }
                        n = i + 1;
                    }
                    else if (id[i] == '<')
                    {
                        ++nest;
                    }
                    else if (id[i] == '>')
                    {
                        assert(nest != 0);
                        --nest;
                    }
                }
                JSValueConst argv[] = { args, mems };
                val = JS_CallConstructor(ctx, rep, 2, argv);
                if (JS_IsException(val))
                {
                    throw JsException();
                }
            }
            return val;
        }
        case css::uno::TypeClass_INTERFACE:
            return ValueRef(ctx,
                            wrapUnoObject(ctx, *static_cast<css::uno::XInterface* const*>(value)));
        default:
            O3TL_UNREACHABLE;
    }
}

ValueRef toJs(JSContext* ctx, css::uno::Any const& value)
{
    return toJs(ctx, value.getValueType(), value.getValue());
}

JSValue sameUnoObject(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv)
{
    return callFromJs(ctx, [ctx, argc, argv] {
        assert(argc >= 2);
        return JS_NewBool(
            ctx,
            *o3tl::forceAccess<css::uno::Reference<css::uno::XInterface>>(
                fromJsInterface(ctx, cppu::UnoType<css::uno::XInterface>::get(), argv[0]))
                == *o3tl::forceAccess<css::uno::Reference<css::uno::XInterface>>(
                       fromJsInterface(ctx, cppu::UnoType<css::uno::XInterface>::get(), argv[1])));
    });
}

JSValue invokeUno(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int,
                  JSValueConst* func_data)
{
    return callFromJs(ctx, [ctx, this_val, argc, argv, func_data] {
        auto const info = static_cast<css::script::InvocationInfo const*>(
            JS_GetOpaque(func_data[0], getRuntimeData(ctx)->pointerClassId));
        if (argc != info->aParamTypes.getLength())
        {
            JS_ThrowSyntaxError(ctx, "TODO: BAD NUMBER OF ARGUMENTS");
            throw JsException();
        }
        css::uno::Sequence<css::uno::Any> args(info->aParamTypes.getLength());
        for (int i = 0; i != argc; ++i)
        {
            if (info->aParamModes[i] == css::reflection::ParamMode_OUT)
            {
                continue;
            }
            ValueRef arg(ctx);
            if (info->aParamModes[i] == css::reflection::ParamMode_IN)
            {
                arg = JS_DupValue(ctx, argv[i]);
            }
            else
            {
                AtomRef const a(ctx, JS_NewAtom(ctx, "val"));
                assert(a != JS_ATOM_NULL); //TODO
                auto const has = JS_HasProperty(ctx, argv[i], a);
                if (has == -1)
                {
                    throw JsException();
                }
                if (has == 0)
                {
                    JS_ThrowTypeError(ctx, "TODO: BAD INOUT ARG");
                    throw JsException();
                }
                arg = JS_GetProperty(ctx, argv[i], a);
                if (JS_IsException(arg))
                {
                    throw JsException();
                }
            }
            args.getArray()[i] = fromJs(ctx, info->aParamTypes[i], arg);
        }
        css::uno::Sequence<sal_Int16> outParamIndex; //TODO
        css::uno::Sequence<css::uno::Any> outParam;
        ValueRef ret(ctx);
        try
        {
            ret = toJs(
                ctx,
                css::uno::Reference<css::script::XInvocation>(
                    css::script::Invocation::create(comphelper::getProcessComponentContext())
                        ->createInstanceWithArguments(
                            { css::uno::Any(css::uno::Reference(static_cast<css::uno::XInterface*>(
                                JS_GetOpaque(this_val, getRuntimeData(ctx)->wrapperClassId)))) }),
                    css::uno::UNO_QUERY_THROW)
                    ->invoke(info->aName, args, outParamIndex, outParam));
        }
        catch (css::reflection::InvocationTargetException e)
        {
            cppu::throwException(e.TargetException);
        }
        assert(outParamIndex.getLength() == outParam.getLength());
        for (sal_Int32 i = 0; i != outParamIndex.getLength(); ++i)
        {
            auto out = toJs(ctx, outParam[i]);
            if (JS_SetPropertyStr(ctx, argv[outParamIndex[i]], "val", out.release()) == -1)
            {
                throw JsException();
            }
        }
        return ret.release();
    });
}

struct ExceptionData
{
    OUString type;
    OUString message;
};

ExceptionData extractExceptionData(JSContext* ctx, ValueRef const& err)
{
    ExceptionData exc;
    bool haveMessage = false;
    if (JS_IsObject(err))
    {
        ValueRef const nameVal(ctx, JS_GetPropertyStr(ctx, err, "name"));
        if (JS_IsString(nameVal))
        {
            std::size_t n;
            UniqueCString16 const p(ctx, JS_ToCStringLenUTF16(ctx, &n, nameVal));
            if (p.get() != nullptr)
            {
                exc.type = OUString(p.get(), n);
            }
        }
        ValueRef const msgVal(ctx, JS_GetPropertyStr(ctx, err, "message"));
        if (JS_IsString(msgVal))
        {
            std::size_t n;
            UniqueCString16 const p(ctx, JS_ToCStringLenUTF16(ctx, &n, msgVal));
            if (p.get() != nullptr)
            {
                exc.message = OUString(p.get(), n);
                haveMessage = true;
            }
        }
    }
    if (!haveMessage)
    {
        ValueRef const str(ctx, JS_ToString(ctx, err));
        if (!JS_IsException(str))
        {
            std::size_t n;
            UniqueCString16 const p(ctx, JS_ToCStringLenUTF16(ctx, &n, str));
            if (p.get() != nullptr)
            {
                exc.message = OUString(p.get(), n);
            }
        }
    }
    return exc;
}
}

void jsuno::execute(OUString const& script)
{
    auto const rt = JS_NewRuntime();
    JS_SetRuntimeOpaque(rt, new RuntimeData(rt));
    JS_NewClassID(rt, &getRuntimeData(rt)->pointerClassId);
    JSClassDef pointerClass{ "InternalPointer", pointerFinalizer, nullptr, nullptr, nullptr };
    [[maybe_unused]] auto e = JS_NewClass(rt, getRuntimeData(rt)->pointerClassId, &pointerClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->wrapperClassId);
    JSClassExoticMethods wrapperMethods{ nullptr, nullptr /*TODO*/,   nullptr,           nullptr,
                                         nullptr, wrapperGetProperty, wrapperSetProperty };
    JSClassDef wrapperClass{ "UnoWrapper", wrapperFinalizer, nullptr, nullptr, &wrapperMethods };
    e = JS_NewClass(rt, getRuntimeData(rt)->wrapperClassId, &wrapperClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->enumeratorClassId);
    JSClassDef enumeratorClass("UnoidlEnumerator", enumeratorFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->enumeratorClassId, &enumeratorClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->compoundClassId);
    JSClassDef compoundClass("UnoStruct", compoundFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->compoundClassId, &compoundClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->typeClassId);
    JSClassDef typeClass("UnoType", typeFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->typeClassId, &typeClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->enumClassId);
    JSClassDef enumClass("UnoidlEnumType", enumFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->enumClassId, &enumClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->structClassId);
    JSClassDef structClass("UnoidlStructType", structFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->structClassId, &structClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->exceptionClassId);
    JSClassDef exceptionClass("UnoidlExceptionType", exceptionFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->exceptionClassId, &exceptionClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->interfaceClassId);
    JSClassDef interfaceClass("UnoidlInterfaceType", interfaceFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->interfaceClassId, &interfaceClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->ctorClassId);
    JSClassDef ctorClass("UnoidlSingleton", ctorFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->ctorClassId, &ctorClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->singletonClassId);
    JSClassDef singletonClass("UnoidlSingleton", singletonFinalizer, nullptr, nullptr, nullptr);
    e = JS_NewClass(rt, getRuntimeData(rt)->singletonClassId, &singletonClass);
    assert(e == 0); //TODO
    JS_NewClassID(rt, &getRuntimeData(rt)->moduleClassId);
    JSClassExoticMethods moduleMethods{ nullptr, nullptr /*TODO*/,  nullptr, nullptr,
                                        nullptr, moduleGetProperty, nullptr };
    JSClassDef moduleClass("UnoidlModule", moduleFinalizer, nullptr, nullptr, &moduleMethods);
    e = JS_NewClass(rt, getRuntimeData(rt)->moduleClassId, &moduleClass);
    assert(e == 0); //TODO
    auto const ctx = JS_NewContext(rt);
    std::optional<ExceptionData> exc;
    {
        ValueRef const global(ctx, JS_GetGlobalObject(ctx));
        getRuntimeData(ctx)->symbolIteratorAtom = JS_ValueToAtom(
            ctx, ValueRef(ctx, JS_GetPropertyStr(
                                   ctx, ValueRef(ctx, JS_GetPropertyStr(ctx, global, "Symbol")),
                                   "iterator")));
        ValueRef console(ctx, JS_NewObject(ctx));
        JS_SetPropertyStr(ctx, console, "assert", JS_NewCFunction(ctx, consoleAssert, "assert", 1));
        JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, consoleLog, "log", 0));
        JS_SetPropertyStr(ctx, global, "console", console.release());
        ValueRef uno(ctx, JS_NewObject(ctx));
        ValueRef type(ctx, JS_NewObject(ctx));
        setTypeProperty(ctx, type, "void", cppu::UnoType<void>::get());
        setTypeProperty(ctx, type, "boolean", cppu::UnoType<bool>::get());
        setTypeProperty(ctx, type, "byte", cppu::UnoType<sal_Int8>::get());
        setTypeProperty(ctx, type, "short", cppu::UnoType<sal_Int16>::get());
        setTypeProperty(ctx, type, "unsigned_short", cppu::UnoType<sal_uInt16>::get());
        setTypeProperty(ctx, type, "long", cppu::UnoType<sal_Int32>::get());
        setTypeProperty(ctx, type, "unsigned_long", cppu::UnoType<sal_uInt32>::get());
        setTypeProperty(ctx, type, "hyper", cppu::UnoType<sal_Int64>::get());
        setTypeProperty(ctx, type, "unsigned_hyper", cppu::UnoType<sal_uInt64>::get());
        setTypeProperty(ctx, type, "float", cppu::UnoType<float>::get());
        setTypeProperty(ctx, type, "double", cppu::UnoType<double>::get());
        setTypeProperty(ctx, type, "char", cppu::UnoType<char16_t>::get());
        setTypeProperty(ctx, type, "string", cppu::UnoType<OUString>::get());
        setTypeProperty(ctx, type, "type", cppu::UnoType<css::uno::Type>::get());
        setTypeProperty(ctx, type, "any", cppu::UnoType<css::uno::Any>::get());
        JS_SetPropertyStr(ctx, type, "sequence",
                          JS_NewCFunction(ctx, unoTypeSequence, "sequence", 1));
        JS_SetPropertyStr(ctx, type, "enum", JS_NewCFunction(ctx, unoTypeEnum, "enum", 1));
        JS_SetPropertyStr(ctx, type, "struct", JS_NewCFunction(ctx, unoTypeStruct, "struct", 1));
        JS_SetPropertyStr(ctx, type, "exception",
                          JS_NewCFunction(ctx, unoTypeException, "exception", 1));
        JS_SetPropertyStr(ctx, type, "interface",
                          JS_NewCFunction(ctx, unoTypeInterface, "interface", 1));
        JS_SetPropertyStr(ctx, uno, "type", type.release());
        ValueRef idl(ctx, JS_NewObjectClass(ctx, getRuntimeData(ctx)->moduleClassId));
        e = JS_SetOpaque(idl, stringAcquire(u""_ustr));
#if defined DBG_UTIL
        getRuntimeData(ctx)->toFinalize.inc();
#endif
        assert(e == 0); //TODO
        JS_SetPropertyStr(ctx, uno, "idl", idl.release());
        ValueRef anyProto(ctx, JS_NewObject(ctx));
        assert(!JS_IsException(anyProto)); //TODO
        ValueRef anyCtor(ctx, JS_NewCFunction2(ctx, unoAny, "Any", 1, JS_CFUNC_constructor, 0));
        assert(!JS_IsException(anyCtor)); //TODO
        JS_SetConstructor(ctx, anyCtor, anyProto);
        JS_SetPropertyStr(ctx, uno, "Any", anyCtor.release());
        JS_SetPropertyStr(ctx, uno, "sameUnoObject",
                          JS_NewCFunction(ctx, sameUnoObject, "sameUnoObject", 2));
        JS_SetPropertyStr(ctx, uno, "componentContext",
                          wrapUnoObject(ctx, comphelper::getProcessComponentContext()));
        JS_SetPropertyStr(ctx, global, "uno", uno.release());
        auto const input = script.toUtf8();
        ValueRef const evalRes(
            ctx, JS_Eval(ctx, input.getStr(), input.getLength(), "<input>", JS_EVAL_TYPE_GLOBAL));
        if (JS_IsException(evalRes))
        {
            //TODO: reconstruct UNO exceptions
            ValueRef const err(ctx, JS_GetException(ctx));
            assert(!JS_IsException(err)); //TODO?
            exc = extractExceptionData(ctx, err);
        }
    }
    JS_FreeContext(ctx);
    std::unique_ptr<RuntimeData> data(getRuntimeData(rt));
    data->clear();
    JS_FreeRuntime(rt);
    if (exc)
    {
        throw css::script::provider::ScriptExceptionRaisedException(
            exc->message, {}, u"<input>"_ustr, u"JavaScript"_ustr, -1, exc->type);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
