/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits>
#include <type_traits>

#include <dconf/dconf.h>

#include <com/sun/star/uno/Sequence.hxx>

#include <data.hxx>
#include <groupnode.hxx>
#include <localizedpropertynode.hxx>
#include <localizedvaluenode.hxx>
#include <nodemap.hxx>
#include <propertynode.hxx>
#include <readdconflayer.hxx>
#include <setnode.hxx>

// component-data is encoded in dconf as follows:
//
// * The node hierarchy (starting at component nodes with names like
//   "org.openoffice.Setup") maps to dconf paths underneath
//   "/org/libreoffice/registry/".
//
// * Component, group, set, and localized property nodes map to dconf dirs
//   (except for removal of set elements, see below), while property and
//   localized value nodes map to dconf keys.
//
// * The names of nodes that are not set elements are used directly as dconf
//   path segments.  (The syntax for node names is any non-empty sequences of
//   any Unicode scalar values except U+0000--0008, U+000B--000C, U+000E--001F,
//   U+002F SOLIDUS, and U+FFFE--FFFF.  TODO: "<aruiz> sberg, in general I think
//   it'd be nice if you used path separators instead of dots though, they have
//   meaning in dconf/gvdb world :-)"?)
//
// * Set element "fuse" and "replace" operations are encoded as dconf path
//   segments as concatenations
//
//     N ; T ; O
//
//   where ";" represents U+003B SEMICOLON; N is an encoding of the node name,
//   where each occurrence of U+003B SEMICOLON is replaced by the three
//   characters "\3B" and each ocurrence of U+005C REVERSE SOLIDUS is replaced
//   by the three characters "\5C"; T is an encoding of the full template name,
//   where each occurrence of U+002F SOLIDUS is replaced by the three characters
//   "\2F", each occurrence of U+003B SEMICOLON is replaced by the three
//   characters "\3B", and each ocurrence of U+005C REVERSE SOLIDUS is replaced
//   by the three characters "\5C"; and O is "fuse" or "replace", respectively.
//
// * Set element and property "remove" operations are encoded as dconf key path
//   segments directly using the node name, and the associated value being a
//   GVariant of empty tuple type.
//
// * Property and localized property value "fuse" operations map to GVariant
//   instances as follows:
//
// ** Non-nillable boolean values map to GVariant boolean instances.
//
// ** Non-nillable short values map to GVariant int16 instances.
//
// ** Non-nillable int values map to GVariant int32 instances.
//
// ** Non-nillable long values map to GVariant int64 instances.
//
// ** Non-nillable double values map to GVariant double instances.
//
// ** Non-nillable string values map to GVariant string instances, with the
//    following encoding: each occurrence of U+0000 NULL is replace by the three
//    characters "\00" and each occurrence of U+005C REVERSE SOLIDUS is replaced
//    by the three characters "\5C".
//
// ** Non-nillable hexbinary values map to GVariant byte array instances.
//
// ** Non-nillable list values recursively map to GVariant array instances.
//
// ** Nillable values recursively map to GVariant maybe instances.
//
// TODO: support "finalized", "mandatory", and "external"?

namespace configmgr {

namespace {

template<typename T> class GObjectHolder {
public:
    explicit GObjectHolder(T * object): object_(object) {}

    ~GObjectHolder() {
        if (object_ != nullptr) {
            g_object_unref(object_);
        }
    }

    T * get() const { return object_; }

private:
    GObjectHolder(GObjectHolder &) = delete;
    void operator =(GObjectHolder) = delete;

    T * object_;
};

class GVariantHolder {
public:
    explicit GVariantHolder(GVariant * variant): variant_(variant) {}

    ~GVariantHolder() { unref(); }

    void reset(GVariant * variant) {
        unref();
        variant_ = variant;
    }

    GVariant * get() const { return variant_; }

private:
    GVariantHolder(GVariantHolder &) = delete;
    void operator =(GVariantHolder) = delete;

    void unref() {
        if (variant_ != nullptr) {
            g_variant_unref(variant_);
        }
    }

    GVariant * variant_;
};

class StringArrayHolder {
public:
    explicit StringArrayHolder(gchar ** array): array_(array) {}

    ~StringArrayHolder() { g_strfreev(array_); }

    gchar ** get() const { return array_; }

private:
    StringArrayHolder(StringArrayHolder &) = delete;
    void operator =(StringArrayHolder) = delete;

    gchar ** array_;
};

bool decode(OUString * string, bool nul, bool slash, bool semicolon) {
    for (sal_Int32 i = 0;; ++i) {
        i = string->indexOf('\\', i);
        if (i == -1) {
            return true;
        }
        if (nul && string->match("00", i + 1)) {
            *string = string->replaceAt(i, 3, OUString(sal_Unicode(0)));
        } else if (slash && string->match("2F", i + 1)) {
            *string = string->replaceAt(i, 3, "/");
        } else if (semicolon && string->match("3B", i + 1)) {
            *string = string->replaceAt(i, 3, ";");
        } else if (string->match("5C", i + 1)) {
            *string = string->replaceAt(i + 1, 2, "");
        } else {
            SAL_WARN("configmgr.dconf", "bad escape in " << *string);
            return false;
        }
    }
}

bool getBoolean(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (!g_variant_is_of_type(variant.get(), G_VARIANT_TYPE_BOOLEAN)) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match boolean property");
        return false;
    }
    *value <<= bool(g_variant_get_boolean(variant.get()));
    return true;
}

bool getShort(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (!g_variant_is_of_type(variant.get(), G_VARIANT_TYPE_INT16)) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match short property");
        return false;
    }
    *value <<= sal_Int16(g_variant_get_int16(variant.get()));
    return true;
}

bool getInt(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (!g_variant_is_of_type(variant.get(), G_VARIANT_TYPE_INT32)) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match int property");
        return false;
    }
    *value <<= sal_Int32(g_variant_get_int32(variant.get()));
    return true;
}

bool getLong(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (!g_variant_is_of_type(variant.get(), G_VARIANT_TYPE_INT64)) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match long property");
        return false;
    }
    *value <<= sal_Int64(g_variant_get_int64(variant.get()));
    return true;
}

bool getDouble(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (!g_variant_is_of_type(variant.get(), G_VARIANT_TYPE_DOUBLE)) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match double property");
        return false;
    }
    *value <<= double(g_variant_get_double(variant.get()));
    return true;
}

bool getStringValue(
    OString const & key, GVariantHolder const & variant, OUString * value)
{
    assert(value != nullptr);
    if (!g_variant_is_of_type(variant.get(), G_VARIANT_TYPE_STRING)) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match string property");
        return false;
    }
    gsize n;
    char const * p = g_variant_get_string(variant.get(), &n);
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long string value for key " << key);
        return false;
    }
    if (!rtl_convertStringToUString(
            &value->pData, p, static_cast<sal_Int32>(n), RTL_TEXTENCODING_UTF8,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        SAL_WARN("configmgr.dconf", "non--UTF-8 string value for key " << key);
        return false;
    }
    return decode(value, true, false, false);
}

bool getString(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    OUString v;
    if (!getStringValue(key, variant, &v)) {
        return false;
    }
    *value <<= v;
    return true;
}

bool getHexbinaryValue(
    OString const & key, GVariantHolder const & variant,
    css::uno::Sequence<sal_Int8> * value)
{
    assert(value != nullptr);
    if (std::strcmp(g_variant_get_type_string(variant.get()), "ay") != 0) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match hexbinary property");
        return false;
    }
    gsize n;
    gconstpointer p = g_variant_get_fixed_array(
        variant.get(), &n, sizeof (guchar));
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long hexbinary value for key " << key);
        return false;
    }
    value->realloc(static_cast<sal_Int32>(n));
    static_assert(sizeof (sal_Int8) == sizeof (guchar), "size mismatch");
    std::memcpy(value->getArray(), p, n * sizeof (guchar));
        // assuming that n * sizeof (guchar) is small enough for std::size_t
    return true;
}

bool getHexbinary(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    css::uno::Sequence<sal_Int8> v;
    if (!getHexbinaryValue(key, variant, &v)) {
        return false;
    }
    *value <<= v;
    return true;
}

bool getBooleanList(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (std::strcmp(g_variant_get_type_string(variant.get()), "ab") != 0) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match boolean list property");
        return false;
    }
    gsize n;
    gconstpointer p = g_variant_get_fixed_array(
        variant.get(), &n, sizeof (guchar));
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long boolean list for key " << key);
        return false;
    }
    css::uno::Sequence<sal_Bool> v(static_cast<sal_Int32>(n));
    static_assert(sizeof (sal_Bool) == sizeof (guchar), "size mismatch");
    std::memcpy(v.getArray(), p, n * sizeof (guchar));
        // assuming that n * sizeof (guchar) is small enough for std::size_t
    *value <<= v;
    return true;
}

bool getShortList(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (std::strcmp(g_variant_get_type_string(variant.get()), "an") != 0) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match short list property");
        return false;
    }
    gsize n;
    gconstpointer p = g_variant_get_fixed_array(
        variant.get(), &n, sizeof (gint16));
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long short list for key " << key);
        return false;
    }
    css::uno::Sequence<sal_Int16> v(static_cast<sal_Int32>(n));
    static_assert(sizeof (sal_Int16) == sizeof (gint16), "size mismatch");
    std::memcpy(v.getArray(), p, n * sizeof (gint16));
        // assuming that n * sizeof (gint16) is small enough for std::size_t
    *value <<= v;
    return true;
}

bool getIntList(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (std::strcmp(g_variant_get_type_string(variant.get()), "ai") != 0) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match int list property");
        return false;
    }
    gsize n;
    gconstpointer p = g_variant_get_fixed_array(
        variant.get(), &n, sizeof (gint32));
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long int list for key " << key);
        return false;
    }
    css::uno::Sequence<sal_Int32> v(static_cast<sal_Int32>(n));
    static_assert(sizeof (sal_Int32) == sizeof (gint32), "size mismatch");
    std::memcpy(v.getArray(), p, n * sizeof (gint32));
        // assuming that n * sizeof (gint32) is small enough for std::size_t
    *value <<= v;
    return true;
}

bool getLongList(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (std::strcmp(g_variant_get_type_string(variant.get()), "ax") != 0) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match long list property");
        return false;
    }
    gsize n;
    gconstpointer p = g_variant_get_fixed_array(
        variant.get(), &n, sizeof (gint64));
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long long list for key " << key);
        return false;
    }
    css::uno::Sequence<sal_Int64> v(static_cast<sal_Int32>(n));
    static_assert(sizeof (sal_Int64) == sizeof (gint64), "size mismatch");
    std::memcpy(v.getArray(), p, n * sizeof (gint64));
        // assuming that n * sizeof (gint64) is small enough for std::size_t
    *value <<= v;
    return true;
}

bool getDoubleList(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (std::strcmp(g_variant_get_type_string(variant.get()), "ad") != 0) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match double list property");
        return false;
    }
    gsize n;
    gconstpointer p = g_variant_get_fixed_array(
        variant.get(), &n, sizeof (gdouble));
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long double list for key " << key);
        return false;
    }
    css::uno::Sequence<double> v(static_cast<sal_Int32>(n));
    static_assert(std::is_same<double, gdouble>::value, "type mismatch");
    std::memcpy(v.getArray(), p, n * sizeof (gdouble));
        // assuming that n * sizeof (gdouble) is small enough for std::size_t
    *value <<= v;
    return true;
}

bool getStringList(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (std::strcmp(g_variant_get_type_string(variant.get()), "as") != 0) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match string list property");
        return false;
    }
    gsize n = g_variant_n_children(variant.get());
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long string list for key " << key);
        return false;
    }
    css::uno::Sequence<OUString> v(static_cast<sal_Int32>(n));
    for (gsize i = 0; i != n; ++i) {
        GVariantHolder c(g_variant_get_child_value(variant.get(), i));
        if (!getStringValue(key, c, v.getArray() + i)) {
            return false;
        }
    }
    *value <<= v;
    return true;
}

bool getHexbinaryList(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    assert(value != nullptr);
    if (std::strcmp(g_variant_get_type_string(variant.get()), "aay") != 0) {
        SAL_WARN(
            "configmgr.dconf",
            "bad key " << key << " does not match hexbinary list property");
        return false;
    }
    gsize n = g_variant_n_children(variant.get());
    if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
            std::numeric_limits<sal_Int32>::max()))
    {
        SAL_WARN("configmgr.dconf", "too long hexbinary list for key " << key);
        return false;
    }
    css::uno::Sequence<css::uno::Sequence<sal_Int8>> v(
        static_cast<sal_Int32>(n));
    for (gsize i = 0; i != n; ++i) {
        GVariantHolder c(g_variant_get_child_value(variant.get(), i));
        if (!getHexbinaryValue(key, c, v.getArray() + i)) {
            return false;
        }
    }
    *value <<= v;
    return true;
}

bool getAny(
    OString const & key, GVariantHolder const & variant, css::uno::Any * value)
{
    char const * t = g_variant_get_type_string(variant.get());
    if (std::strcmp(t, "b") == 0) {
        return getBoolean(key, variant, value);
    }
    if (std::strcmp(t, "n") == 0) {
        return getShort(key, variant, value);
    }
    if (std::strcmp(t, "i") == 0) {
        return getInt(key, variant, value);
    }
    if (std::strcmp(t, "x") == 0) {
        return getLong(key, variant, value);
    }
    if (std::strcmp(t, "d") == 0) {
        return getDouble(key, variant, value);
    }
    if (std::strcmp(t, "s") == 0) {
        return getString(key, variant, value);
    }
    if (std::strcmp(t, "ay") == 0) {
        return getHexbinary(key, variant, value);
    }
    if (std::strcmp(t, "ab") == 0) {
        return getBooleanList(key, variant, value);
    }
    if (std::strcmp(t, "an") == 0) {
        return getShortList(key, variant, value);
    }
    if (std::strcmp(t, "ai") == 0) {
        return getIntList(key, variant, value);
    }
    if (std::strcmp(t, "ax") == 0) {
        return getLongList(key, variant, value);
    }
    if (std::strcmp(t, "ad") == 0) {
        return getDoubleList(key, variant, value);
    }
    if (std::strcmp(t, "as") == 0) {
        return getStringList(key, variant, value);
    }
    if (std::strcmp(t, "aay") == 0) {
        return getHexbinaryList(key, variant, value);
    }
    SAL_WARN(
        "configmgr.dconf", "bad key " << key << " does not match any property");
    return false;
}

enum class ReadValue { Error, Value, Remove };

ReadValue readValue(
    GObjectHolder<DConfClient> const & client, OString const & path, Type type,
    bool nillable, bool removable, css::uno::Any * value)
{
    assert(value != nullptr);
    assert(!value->hasValue());
    assert(!path.endsWith("/"));
    GVariantHolder v(dconf_client_read(client.get(), path.getStr()));
    if (v.get() == nullptr) {
        SAL_WARN("configmgr.dconf", "cannot read key " << path);
        return ReadValue::Error;
    }
    if (removable && std::strcmp(g_variant_get_type_string(v.get()), "()") == 0)
    {
        return ReadValue::Remove;
    }
    bool nil;
    if (nillable) {
        if (g_variant_classify(v.get()) != G_VARIANT_CLASS_MAYBE) {
            SAL_WARN(
                "configmgr.dconf",
                "bad key " << path << " does not match nillable property");
        }
        v.reset(g_variant_get_maybe(v.get()));
        nil = v.get() == nullptr;
    } else {
        nil = false;
    }
    if (!nil) {
        switch (type) {
        case TYPE_ANY:
            if (!getAny(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_BOOLEAN:
            if (!getBoolean(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_SHORT:
            if (!getShort(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_INT:
            if (!getInt(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_LONG:
            if (!getLong(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_DOUBLE:
            if (!getDouble(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_STRING:
            if (!getString(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_HEXBINARY:
            if (!getHexbinary(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_BOOLEAN_LIST:
            if (!getBooleanList(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_SHORT_LIST:
            if (!getShortList(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_INT_LIST:
            if (!getIntList(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_LONG_LIST:
            if (!getLongList(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_DOUBLE_LIST:
            if (!getDoubleList(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_STRING_LIST:
            if (!getStringList(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        case TYPE_HEXBINARY_LIST:
            if (!getHexbinaryList(path, v, value)) {
                return ReadValue::Error;
            }
            break;
        default:
            assert(false); // cannot happen
        }
    }
    return ReadValue::Value;
}

void readDir(
    Data & data, int layer, rtl::Reference<Node> const & node,
    NodeMap & members, GObjectHolder<DConfClient> const & client,
    OString const & dir)
{
    StringArrayHolder a(dconf_client_list(client.get(), dir.getStr(), nullptr));
    for (char const * const * p = a.get(); *p != nullptr; ++p) {
        std::size_t n = std::strlen(*p);
        if (n > static_cast<typename std::make_unsigned<sal_Int32>::type>(
                std::numeric_limits<sal_Int32>::max()))
        {
            SAL_WARN("configmgr.dconf", "too long dir/key in dir " << dir);
            continue;
        }
        OString s(*p, static_cast<sal_Int32>(n));
        OString path(dir + s);
        OUString seg;
        if (!rtl_convertStringToUString(
                &seg.pData, s.getStr(), s.getLength(), RTL_TEXTENCODING_UTF8,
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            SAL_WARN("configmgr.dconf", "non--UTF-8 dir/key in dir " << dir);
            continue;
        }
        bool isDir = seg.endsWith("/", &seg);
        bool remove;
        OUString name;
        OUString templ;
        bool replace;
        if (node.is() && node->kind() == Node::KIND_SET) {
            if (isDir) {
                remove = false;
                sal_Int32 i1 = seg.indexOf(';');
                if (i1 == -1) {
                    SAL_WARN(
                        "configmgr.dconf", "bad set element syntax " << path);
                    continue;
                }
                name = seg.copy(0, i1);
                if (!decode(&name, false, false, true)) {
                    continue;
                }
                ++i1;
                sal_Int32 i2 = seg.indexOf(';', i1);
                if (i2 == -1) {
                    SAL_WARN(
                        "configmgr.dconf", "bad set element syntax " << path);
                    continue;
                }
                templ = seg.copy(i1, i2 - i1);
                if (!decode(&templ, false, true, true)) {
                    continue;
                }
                ++i2;
                if (rtl_ustr_asciil_reverseCompare_WithLength(
                        seg.getStr() + i2, seg.getLength() - i2, "fuse",
                        std::strlen("fuse"))
                    == 0)
                {
                    replace = false;
                } else if (rtl_ustr_asciil_reverseCompare_WithLength(
                               seg.getStr() + i2, seg.getLength() - i2,
                               "replace", std::strlen("replace"))
                           == 0)
                {
                    replace = true;
                } else {
                    SAL_WARN(
                        "configmgr.dconf", "bad set element syntax " << path);
                    continue;
                }
                rtl::Reference<SetNode> set(static_cast<SetNode *>(node.get()));
                if (!set->isValidTemplate(templ)) {
                    SAL_WARN(
                        "configmgr.dconf",
                        "bad " << path
                            << " denotes unsupported set element template");
                    continue;
                }
            } else {
                remove = true;
                name = seg;
                replace = false;
                assert(!path.endsWith("/"));
                GVariantHolder v(
                    dconf_client_read(client.get(), path.getStr()));
                if (v.get() == nullptr) {
                    SAL_WARN("configmgr.dconf", "cannot read key " << path);
                    continue;
                }
                if (std::strcmp(g_variant_get_type_string(v.get()), "()") != 0)
                {
                    SAL_WARN(
                        "configmgr.dconf",
                        "bad " << path
                            << " does not denote set element removal");
                    continue;
                }
            }
        } else {
            remove = false;
            name = seg;
            replace = false;
        }
        rtl::Reference<Node> member(members.findNode(layer, name));
        bool insert = !member.is();
        if (!remove && (replace || insert)) {
            if (!node.is()) {
                SAL_WARN("configmgr.dconf", "bad unmatched " << path);
                continue;
            }
            switch (node->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                member.set(new LocalizedValueNode(layer));
                break;
            case Node::KIND_GROUP:
                if (!static_cast<GroupNode *>(node.get())->isExtensible()) {
                    SAL_WARN("configmgr.dconf", "bad unmatched " << path);
                    continue;
                }
                member.set(
                    new PropertyNode(
                        layer, TYPE_ANY, true, css::uno::Any(), true));
                break;
            case Node::KIND_SET:
                assert(!templ.isEmpty());
                member = data.getTemplate(layer, templ);
                if (!member.is()) {
                    SAL_WARN(
                        "configmgr.dconf",
                        "bad " << path << " denoting undefined template "
                            << templ);
                    continue;
                }
                break;
            default:
                assert(false); // cannot happen
            }
        } else if (!(templ.isEmpty()
                     || (node.is() && templ == node->getTemplateName())))
        {
            SAL_WARN(
                "configmgr.dconf",
                "bad " << path
                    << " denoting set element of non-matching template "
                    << node->getTemplateName());
            continue;
        }
        if (member->getFinalized() < layer) {
            continue;
        }
        switch (member->kind()) {
        case Node::KIND_PROPERTY:
            {
                if (isDir) {
                    SAL_WARN(
                        "configmgr.dconf",
                        "bad dir " << path << " does not match property");
                    continue;
                }
                rtl::Reference<PropertyNode> prop(
                    static_cast<PropertyNode *>(member.get()));
                css::uno::Any value;
                switch (readValue(
                            client, path, prop->getStaticType(),
                            prop->isNillable(), prop->isExtension(), &value))
                {
                case ReadValue::Error:
                    continue;
                case ReadValue::Value:
                    prop->setValue(layer, value);
                    break;
                case ReadValue::Remove:
                    remove = true;
                    break;
                }
                break;
            }
        case Node::KIND_LOCALIZED_VALUE:
            {
                if (isDir) {
                    SAL_WARN(
                        "configmgr.dconf",
                        "bad dir " << path
                            << " does not match localized value");
                    continue;
                }
                assert(
                    node.is() && node->kind() == Node::KIND_LOCALIZED_PROPERTY);
                rtl::Reference<LocalizedPropertyNode> locProp(
                    static_cast<LocalizedPropertyNode *>(node.get()));
                css::uno::Any value;
                if (readValue(
                        client, path, locProp->getStaticType(),
                        locProp->isNillable(), false, &value)
                    == ReadValue::Error)
                {
                    continue;
                }
                static_cast<LocalizedValueNode *>(member.get())->setValue(
                    layer, value);
                break;
            }
        case Node::KIND_LOCALIZED_PROPERTY:
        case Node::KIND_GROUP:
        case Node::KIND_SET:
            if (!isDir) {
                SAL_WARN(
                    "configmgr.dconf",
                    "bad key " << path
                        << " does not match localized property, group, or set,"
                        " respectively");
                continue;
            }
            assert(path.endsWith("/"));
            readDir(data, layer, member, member->getMembers(), client, path);
            break;
        default:
            assert(false); // cannot happen
        }
        if (remove) {
            if (!member->getMandatory()) {
                members.erase(name);
            }
        } else if (replace) {
            members.erase(name);
            members.insert(NodeMap::value_type(name, member));
        } else if (insert) {
            members.insert(NodeMap::value_type(name, member));
        }
    }
}

}

void readDconfLayer(Data & data, int layer) {
    GObjectHolder<DConfClient> client(dconf_client_new());
    readDir(
        data, layer, rtl::Reference<Node>(), data.getComponents(), client,
        "/org/libreoffice/registry/");
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
