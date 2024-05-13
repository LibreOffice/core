/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <bitset>

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/reflection/XConstantTypeDescription.hpp>
#include <com/sun/star/reflection/XConstantsTypeDescription.hpp>
#include <com/sun/star/reflection/XDump.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <config_typesizes.h>
#include <cppu/unotype.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <o3tl/unreachable.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <typelib/typedescription.h>
#include <typelib/typedescription.hxx>
#include <uno/sequence2.h>

namespace com::sun::star::uno
{
class XInterface;
}

namespace
{
template <typename T> OUString hex(T value, sal_Int32 width)
{
    OUStringBuffer buf(OUString::number(value, 16));
    while (buf.getLength() < width)
    {
        buf.insert(0, '0');
    }
    return buf.makeStringAndClear();
}

css::uno::TypeDescription getTypeDescription(css::uno::Type const& type)
{
    typelib_TypeDescription* d = nullptr;
    type.getDescription(&d);
    return css::uno::TypeDescription(d);
}

OUString
getIdentifier(css::uno::Reference<css::reflection::XConstantTypeDescription> const& constant)
{
    auto const n = constant->getName();
    auto const i = n.lastIndexOf('.');
    if (i == -1 || i == n.getLength() - 1)
    {
        throw css::uno::DeploymentException("bad constant name " + n);
    }
    return n.copy(i + 1);
}

OUString
dumpBitset(css::uno::Sequence<css::uno::Reference<css::reflection::XConstantTypeDescription>> const&
               constants,
           sal_uInt64 value)
{
    OUStringBuffer buf;
    auto a = value;
    for (auto const& i : constants)
    {
        sal_uInt64 c;
        if ((i->getConstantValue() >>= c) && std::bitset<64>{ c }.count() == 1 && (a & c) != 0)
        {
            if (!buf.isEmpty())
            {
                buf.append('+');
            }
            buf.append(getIdentifier(i));
            a &= ~c;
        }
    }
    return a == 0 && !buf.isEmpty() ? buf.makeStringAndClear() : OUString::number(value);
}

class Dump : public cppu::BaseMutex, public cppu::WeakComponentImplHelper<css::reflection::XDump>
{
public:
    explicit Dump(css::uno::Reference<css::uno::XComponentContext> const& context)
        : WeakComponentImplHelper(m_aMutex)
        , manager_(context->getValueByName(
                       u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr),
                   css::uno::UNO_QUERY_THROW)
    {
    }

    void SAL_CALL disposing() override
    {
        osl::MutexGuard g(m_aMutex);
        manager_.clear();
    }

    OUString SAL_CALL dumpValue(css::uno::Any const& value) override
    {
        switch (value.getValueTypeClass())
        {
            case css::uno::TypeClass_VOID:
                return u"void"_ustr;
            case css::uno::TypeClass_BOOLEAN:
                return OUString::boolean(value.get<bool>());
            case css::uno::TypeClass_BYTE:
                return OUString::number(value.get<sal_Int8>());
            case css::uno::TypeClass_SHORT:
                return OUString::number(value.get<sal_Int16>());
            case css::uno::TypeClass_UNSIGNED_SHORT:
                return OUString::number(value.get<sal_uInt16>());
            case css::uno::TypeClass_LONG:
                return OUString::number(value.get<sal_Int32>());
            case css::uno::TypeClass_UNSIGNED_LONG:
                return OUString::number(value.get<sal_uInt32>());
            case css::uno::TypeClass_HYPER:
                return OUString::number(value.get<sal_Int64>());
            case css::uno::TypeClass_UNSIGNED_HYPER:
                return OUString::number(value.get<sal_uInt64>());
            case css::uno::TypeClass_FLOAT:
                return OUString::number(value.get<float>());
            case css::uno::TypeClass_DOUBLE:
                return OUString::number(value.get<double>());
            case css::uno::TypeClass_CHAR:
                return "U+" + hex(value.get<sal_Unicode>(), 16);
            case css::uno::TypeClass_STRING:
            {
                auto const s = value.get<OUString>();
                OUStringBuffer buf;
                for (sal_Int32 i = 0; i != s.getLength();)
                {
                    auto const c = s.iterateCodePoints(&i);
                    if (c >= u8' ' && c <= u8'~')
                    {
                        if (c == u8'\"' || c == u8'\\')
                        {
                            buf.append('\\');
                        }
                        buf.append(char(c));
                    }
                    else if (c <= 0xFFFF)
                    {
                        buf.append("\\u" + hex(c, 4));
                    }
                    else
                    {
                        buf.append("\\U" + hex(c, 8));
                    }
                }
                return "\"" + buf + "\"";
            }
            case css::uno::TypeClass_TYPE:
                return value.get<css::uno::Type>().getTypeName();
            case css::uno::TypeClass_SEQUENCE:
            {
                css::uno::Type const t(reinterpret_cast<typelib_IndirectTypeDescription const*>(
                                           getTypeDescription(value.getValueType()).get())
                                           ->pType);
                auto const n = getTypeDescription(t).get()->nSize;
                auto const s = *static_cast<uno_Sequence* const*>(value.getValue());
                OUStringBuffer buf;
                for (sal_Int32 i = 0; i != s->nElements; ++i)
                {
                    if (i != 0)
                    {
                        buf.append(", ");
                    }
                    css::uno::Any const e(s->elements + i * n, t);
                    buf.append(t == cppu::UnoType<css::uno::Any>::get() ? dumpAny(e)
                                                                        : dumpValue(e));
                }
                return "[" + buf + "]";
            }
            case css::uno::TypeClass_ENUM:
            {
                auto const d = getTypeDescription(value.getValueType());
                auto const ed = reinterpret_cast<typelib_EnumTypeDescription const*>(d.get());
                auto const e = *static_cast<sal_Int32 const*>(value.getValue());
                for (sal_Int32 i = 0; i != ed->nEnumValues; ++i)
                {
                    if (ed->pEnumValues[i] == e)
                    {
                        return OUString(ed->ppEnumNames[i]);
                    }
                }
                return OUString::number(e);
            }
            case css::uno::TypeClass_STRUCT:
            case css::uno::TypeClass_EXCEPTION:
            {
                auto const d = getTypeDescription(value.getValueType());
                OUStringBuffer buf;
                dumpCompoundType(reinterpret_cast<typelib_CompoundTypeDescription const*>(d.get()),
                                 value.getValue(), &buf);
                return "[" + buf + "]";
            }
            case css::uno::TypeClass_INTERFACE:
            {
                auto const p = *static_cast<void* const*>(value.getValue());
                return p == nullptr ? u"null"_ustr
                                    : OUString("0x"
                                               + hex(reinterpret_cast<sal_uIntPtr>(p),
                                                     SAL_TYPES_SIZEOFPOINTER * 2));
            }
            default:
                O3TL_UNREACHABLE;
        }
    }

    OUString SAL_CALL dumpAny(css::uno::Any const& value) override
    {
        return "[" + value.getValueType().getTypeName() + ": " + dumpValue(value) + "]";
    }

    OUString SAL_CALL dumpConstant(OUString const& constantsGroup,
                                   css::uno::Any const& value) override
    {
        css::uno::Reference<css::container::XHierarchicalNameAccess> manager;
        {
            osl::MutexGuard g(m_aMutex);
            if (rBHelper.bDisposed)
            {
                throw css::lang::DisposedException(u"css.reflection.Dumper"_ustr);
            }
            manager = manager_;
        }
        css::uno::Reference<css::reflection::XConstantsTypeDescription> g;
        try
        {
            manager_->getByHierarchicalName(constantsGroup) >>= g;
        }
        catch (css::container::NoSuchElementException)
        {
        }
        if (!g.is())
        {
            throw css::lang::IllegalArgumentException("not a constants group: " + constantsGroup,
                                                      {}, 0);
        }
        auto const s = g->getConstants();
        switch (value.getValueTypeClass())
        {
            case css::uno::TypeClass_BOOLEAN:
                for (auto const& i : s)
                {
                    if (i->getConstantValue() == value)
                    {
                        return getIdentifier(i);
                    }
                }
                return OUString::boolean(value.get<bool>());
            case css::uno::TypeClass_BYTE:
            case css::uno::TypeClass_SHORT:
            case css::uno::TypeClass_LONG:
            case css::uno::TypeClass_HYPER:
            {
                auto const v = value.get<sal_Int64>();
                for (auto const& i : s)
                {
                    sal_Int64 c;
                    if ((i->getConstantValue() >>= c) && c == v)
                    {
                        return getIdentifier(i);
                    }
                }
                return v >= 0 ? dumpBitset(s, v) : OUString::number(v);
            }
            case css::uno::TypeClass_UNSIGNED_SHORT:
            case css::uno::TypeClass_UNSIGNED_LONG:
            case css::uno::TypeClass_UNSIGNED_HYPER:
            {
                auto const v = value.get<sal_uInt64>();
                for (auto const& i : s)
                {
                    sal_uInt64 c;
                    if ((i->getConstantValue() >>= c) && c == v)
                    {
                        return getIdentifier(i);
                    }
                }
                return dumpBitset(s, v);
            }
            case css::uno::TypeClass_FLOAT:
            case css::uno::TypeClass_DOUBLE:
            {
                auto const v = value.get<double>();
                for (auto const& i : s)
                {
                    double c;
                    if ((i->getConstantValue() >>= c) && c == v)
                    {
                        return getIdentifier(i);
                    }
                }
                return OUString::number(v);
            }
            default:
                throw css::lang::IllegalArgumentException(
                    "not a numeric type: " + value.getValueTypeName(), {}, 1);
        }
    }

private:
    css::uno::Reference<css::container::XHierarchicalNameAccess> manager_;

    void dumpCompoundType(typelib_CompoundTypeDescription const* description, void const* data,
                          OUStringBuffer* buffer)
    {
        if (auto base = description->pBaseTypeDescription)
        {
            dumpCompoundType(base, data, buffer);
        }
        for (sal_Int32 i = 0; i != description->nMembers; ++i)
        {
            if (!buffer->isEmpty())
            {
                buffer->append(", ");
            }
            buffer->append(OUString::unacquired(description->ppMemberNames + i) + ": ");
            css::uno::Type t(description->ppTypeRefs[i]);
            css::uno::Any const m(static_cast<char const*>(data) + description->pMemberOffsets[i],
                                  t);
            buffer->append(t == cppu::UnoType<css::uno::Any>::get() ? dumpAny(m) : dumpValue(m));
        }
    }
};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_stoc_Dump_get_implementation(css::uno::XComponentContext* context,
                                               css::uno::Sequence<css::uno::Any> const& arguments)
{
    SAL_WARN_IF(arguments.hasElements(), "stoc", "unexpected singleton arguments");
    return cppu::acquire(new Dump(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
