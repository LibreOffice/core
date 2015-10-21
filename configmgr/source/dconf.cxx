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
#include <forward_list>
#include <limits>
#include <type_traits>
#include <vector>

extern "C" {
    // <https://bugzilla.gnome.org/show_bug.cgi?id=754245>
    // "common/dconf-changeset.h etc. lack extern "C" wrapper for C++"
#include <dconf/dconf.h>
}

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>

#include <data.hxx>
#include <dconf.hxx>
#include <groupnode.hxx>
#include <localizedpropertynode.hxx>
#include <localizedvaluenode.hxx>
#include <nodemap.hxx>
#include <propertynode.hxx>
#include <setnode.hxx>

// component-data is encoded in dconf as follows:
//
// * The node hierarchy (starting at component nodes with names like
//   "org.openoffice.Setup") maps to dconf paths underneath
//   "/org/libreoffice/registry/".
//
// * Component, group, set, and localized property nodes map to dconf dirs,
//   while property and localized value nodes map to dconf keys.
//
// * The names of nodes that are not set elements are used directly as dconf
//   path segments.  (The syntax for node names is any non-empty sequences of
//   any Unicode scalar values except U+0000--0008, U+000B--000C, U+000E--001F,
//   U+002F SOLIDUS, and U+FFFE--FFFF.  TODO: "<aruiz> sberg, in general I think
//   it'd be nice if you used path separators instead of dots though, they have
//   meaning in dconf/gvdb world :-)"?)
//
// * The names of set element nodes are encoded as dconf path segments as
//   follows: each occurrence of U+0000 NULL is replace by the three characters
//   "\00", each occurrence of U+002F SOLIDUS is replaced by the three
//   characters "\2F", and each ocurrence of U+005C REVERSE SOLIDUS is replaced
//   by the three characters "\5C".
//
// * Set elements (which must themselves be either sets or groups) map to
//   "indirection" dconf dirs as follows:
//
// ** The dir must contain a key named "op" of string type, with a value of
//    "fuse", "replace", or "remove".
//
// ** If "op" is "fuse" or "replace", the dir must contain exactly the following
//    further keys and dirs:
//
// *** The dir must contain a key named "template" of string type, containing
//     the full template name, encoded as follows: each occurrence of U+0000
//     NULL is replace by the three characters "\00" and each occurrence of
//     U+005C REVERSE SOLIDUS is replaced by the three characters "\5C".
//
// *** The dir must contain a dir named "content" that contains the set
//     element's (i.e., set or group node's) real content.
//
// ** If "op" is "remove", the dir must contain no further keys or dirs.
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
// * Property "remove" operations map to GVariant instances of empty tuple type.
//
// Finalization:  The component-update.dtd allows for finalization of
// oor:component-data, node, and prop elements, while dconf allows for locking
// of individual keys.  That does not match, but just mark the individual Node
// instances that correspond to individual dconf keys as finalized for
// non-writable dconf keys.
//
// TODO: support "mandatory" and "external"?

namespace configmgr { namespace dconf {

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
    explicit GVariantHolder(GVariant * variant = nullptr): variant_(variant) {}

    ~GVariantHolder() { unref(); }

    void reset(GVariant * variant) {
        unref();
        variant_ = variant;
    }

    void release() { variant_ = nullptr; }

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

class GVariantTypeHolder {
public:
    explicit GVariantTypeHolder(GVariantType * type): type_(type) {}

    ~GVariantTypeHolder() {
        if (type_ != nullptr) {
            g_variant_type_free(type_);
        }
    }

    GVariantType * get() const { return type_; }

private:
    GVariantTypeHolder(GVariantTypeHolder &) = delete;
    void operator =(GVariantTypeHolder) = delete;

    GVariantType * type_;
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

class ChangesetHolder {
public:
    explicit ChangesetHolder(DConfChangeset * changeset):
        changeset_(changeset)
    {}

    ~ChangesetHolder() {
        if (changeset_ != nullptr) {
            dconf_changeset_unref(changeset_);
        }
    }

    DConfChangeset * get() const { return changeset_; }

private:
    ChangesetHolder(ChangesetHolder &) = delete;
    void operator =(ChangesetHolder) = delete;

    DConfChangeset * changeset_;
};

OString getRoot() {
    return "/org/libreoffice/registry";
}

bool decode(OUString * string, bool slash) {
    for (sal_Int32 i = 0;; ++i) {
        i = string->indexOf('\\', i);
        if (i == -1) {
            return true;
        }
        if (string->match("00", i + 1)) {
            *string = string->replaceAt(i, 3, OUString(sal_Unicode(0)));
        } else if (slash && string->match("2F", i + 1)) {
            *string = string->replaceAt(i, 3, "/");
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
    return decode(value, false);
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

void finalize(
    GObjectHolder<DConfClient> const & client, OString const & path,
    rtl::Reference<Node> & node, int layer)
{
    if (!dconf_client_is_writable(client.get(), path.getStr())) {
        node->setFinalized(layer);
    }
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
        OUString name;
        if (!rtl_convertStringToUString(
                &name.pData, s.getStr(), s.getLength(), RTL_TEXTENCODING_UTF8,
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            SAL_WARN("configmgr.dconf", "non--UTF-8 dir/key in dir " << dir);
            continue;
        }
        bool isDir = name.endsWith("/", &name);
        OUString templ;
        bool remove;
        bool replace;
        if (node.is() && node->kind() == Node::KIND_SET) {
            if (!isDir) {
                SAL_WARN(
                    "configmgr.dconf",
                    "bad key " << path << " does not match set element");
                continue;
            }
            if (!decode(&name, true)) {
                continue;
            }
            enum class Op { None, Fuse, Replace, Remove };
            Op op = Op::None;
            bool content = false;
            bool bad = false;
            StringArrayHolder a2(
                dconf_client_list(client.get(), path.getStr(), nullptr));
            for (char const * const * p2 = a2.get(); *p2 != nullptr; ++p2) {
                if (std::strcmp(*p2, "op") == 0) {
                    OString path2(path + "op");
                    GVariantHolder v(
                        dconf_client_read(client.get(), path2.getStr()));
                    if (v.get() == nullptr) {
                        SAL_WARN(
                            "configmgr.dconf", "cannot read key " << path2);
                        bad = true;
                        break;
                    }
                    OUString ops;
                    if (!getStringValue(path2, v, &ops)) {
                        bad = true;
                        break;
                    }
                    if (ops == "fuse") {
                        op = Op::Fuse;
                    } else if (ops == "replace") {
                        op = Op::Replace;
                    } else if (ops == "remove") {
                        op = Op::Remove;
                    } else {
                        SAL_WARN(
                            "configmgr.dconf",
                            "bad key " << path2 << " value " << ops);
                        bad = true;
                        break;
                    }
                } else if (std::strcmp(*p2, "template") == 0) {
                    OString path2(path + "template");
                    GVariantHolder v(
                        dconf_client_read(client.get(), path2.getStr()));
                    if (v.get() == nullptr) {
                        SAL_WARN(
                            "configmgr.dconf", "cannot read key " << path2);
                        bad = true;
                        break;
                    }
                    if (!getStringValue(path2, v, &templ)) {
                        bad = true;
                        break;
                    }
                    if (!static_cast<SetNode *>(node.get())->
                        isValidTemplate(templ))
                    {
                        SAL_WARN(
                            "configmgr.dconf",
                            "bad key " << path2 << " value " << templ
                                << " denotes unsupported set element template");
                        bad = true;
                        break;
                    }
                } else if (std::strcmp(*p2, "content/") == 0) {
                    content = true;
                } else {
                    SAL_WARN(
                        "configmgr.dconf",
                        "bad dir/key " << p2
                            << " in set element indirection dir " << path);
                    bad = true;
                    break;
                }
            }
            if (bad) {
                continue;
            }
            switch (op) {
            default: // case Op::None:
                SAL_WARN(
                    "configmgr.dconf",
                    "bad set element indirection dir " << path
                        << " missing \"op\" key");
                continue;
            case Op::Fuse:
            case Op::Replace:
                if (templ.isEmpty() || !content) {
                    SAL_WARN(
                        "configmgr.dconf",
                        "missing \"content\" and/or \"template\" dir/key in "
                            "\"op\" = \"fuse\"/\"remove\" set element"
                            " indirection dir " << path);
                    continue;
                }
                path += "content/";
                remove = false;
                replace = op == Op::Replace;
                break;
            case Op::Remove:
                if (!templ.isEmpty() || content) {
                    SAL_WARN(
                        "configmgr.dconf",
                        "bad \"content\" and/or \"template\" dir/key in \"op\" "
                            "= \"remove\" set element indirection dir "
                            << path);
                    continue;
                }
                remove = true;
                replace = false;
                break;
            }
        } else {
            remove = false;
            replace = false;
        }
        rtl::Reference<Node> member(members.findNode(layer, name));
        bool insert = !member.is();
        if (!remove) {
            if (replace || insert) {
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
            } else if (!templ.isEmpty() && templ != member->getTemplateName()) {
                SAL_WARN(
                    "configmgr.dconf",
                    "bad " << path
                        << " denoting set element of non-matching template "
                        << member->getTemplateName());
                continue;
            }
        }
        if (member.is()) {
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
                                prop->isNillable(), prop->isExtension(),
                                &value))
                    {
                    case ReadValue::Error:
                        continue;
                    case ReadValue::Value:
                        prop->setValue(layer, value);
                        finalize(client, path, member, layer);
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
                        node.is()
                        && node->kind() == Node::KIND_LOCALIZED_PROPERTY);
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
                    finalize(client, path, member, layer);
                    break;
                }
            case Node::KIND_LOCALIZED_PROPERTY:
            case Node::KIND_GROUP:
            case Node::KIND_SET:
                if (!isDir) {
                    SAL_WARN(
                        "configmgr.dconf",
                        "bad key " << path
                            << " does not match localized property, group, or"
                            " set, respectively");
                    continue;
                }
                assert(path.endsWith("/"));
                readDir(
                    data, layer, member, member->getMembers(), client, path);
                break;
            default:
                assert(false); // cannot happen
            }
        }
        if (remove) {
            if (!(member.is() && member->getMandatory())) {
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

OString encodeSegment(OUString const & name, bool setElement) {
    if (!setElement) {
        return name.toUtf8();
    }
    OUStringBuffer buf;
    for (sal_Int32 i = 0; i != name.getLength(); ++i) {
        sal_Unicode c = name[i];
        switch (c) {
        case '\0':
            buf.append("\\00");
            break;
        case '/':
            buf.append("\\2F");
            break;
        case '\\':
            buf.append("\\5C");
            break;
        default:
            buf.append(c);
        }
    }
    return buf.makeStringAndClear().toUtf8();
}

OString encodeString(OUString const & value) {
    OUStringBuffer buf;
    for (sal_Int32 i = 0; i != value.getLength(); ++i) {
        sal_Unicode c = value[i];
        switch (c) {
        case '\0':
            buf.append("\\00");
            break;
        case '\\':
            buf.append("\\5C");
            break;
        default:
            buf.append(c);
        }
    }
    return buf.makeStringAndClear().toUtf8();
}

bool addProperty(
    ChangesetHolder const & changeset, OString const & pathRepresentation,
    Type type, bool nillable, css::uno::Any const & value)
{
    Type dynType = getDynamicType(value);
    assert(dynType != TYPE_ERROR);
    if (type == TYPE_ANY) {
        type = dynType;
    }
    GVariantHolder v;
    std::forward_list<GVariantHolder> children;
    if (dynType == TYPE_NIL) {
        switch (type) {
        case TYPE_BOOLEAN:
            v.reset(g_variant_new_maybe(G_VARIANT_TYPE_BOOLEAN, nullptr));
            break;
        case TYPE_SHORT:
            v.reset(g_variant_new_maybe(G_VARIANT_TYPE_INT16, nullptr));
            break;
        case TYPE_INT:
            v.reset(g_variant_new_maybe(G_VARIANT_TYPE_INT32, nullptr));
            break;
        case TYPE_LONG:
            v.reset(g_variant_new_maybe(G_VARIANT_TYPE_INT64, nullptr));
            break;
        case TYPE_DOUBLE:
            v.reset(g_variant_new_maybe(G_VARIANT_TYPE_DOUBLE, nullptr));
            break;
        case TYPE_STRING:
            v.reset(g_variant_new_maybe(G_VARIANT_TYPE_STRING, nullptr));
            break;
        case TYPE_HEXBINARY:
        case TYPE_BOOLEAN_LIST:
        case TYPE_SHORT_LIST:
        case TYPE_INT_LIST:
        case TYPE_LONG_LIST:
        case TYPE_DOUBLE_LIST:
        case TYPE_STRING_LIST:
        case TYPE_HEXBINARY_LIST:
            {
                static char const * const typeString[
                    TYPE_HEXBINARY_LIST - TYPE_HEXBINARY + 1]
                    = { "ay", "ab", "an", "ai", "ax", "ad", "as", "aay" };
                GVariantTypeHolder ty(
                    g_variant_type_new(typeString[type - TYPE_HEXBINARY]));
                if (ty.get() == nullptr) {
                    SAL_WARN("configmgr.dconf", "g_variant_type_new failed");
                    return false;
                }
                v.reset(g_variant_new_maybe(ty.get(), nullptr));
                break;
            }
        default:
            assert(false); // this cannot happen
            break;
        }
        if (v.get() == nullptr) {
            SAL_WARN("configmgr.dconf", "g_variant_new_maybe failed");
            return false;
        }
    } else {
        switch (type) {
        case TYPE_BOOLEAN:
            v.reset(g_variant_new_boolean(value.get<bool>()));
            break;
        case TYPE_SHORT:
            v.reset(g_variant_new_int16(value.get<sal_Int16>()));
            break;
        case TYPE_INT:
            v.reset(g_variant_new_int32(value.get<sal_Int32>()));
            break;
        case TYPE_LONG:
            v.reset(g_variant_new_int64(value.get<sal_Int64>()));
            break;
        case TYPE_DOUBLE:
            v.reset(g_variant_new_double(value.get<double>()));
            break;
        case TYPE_STRING:
            v.reset(
                g_variant_new_string(
                    encodeString(value.get<OUString>()).getStr()));
            break;
        case TYPE_HEXBINARY:
            {
                css::uno::Sequence<sal_Int8> seq(
                    value.get<css::uno::Sequence<sal_Int8>>());
                static_assert(
                    std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                    "G_MAXSIZE too small");
                static_assert(
                    sizeof (sal_Int8) == sizeof (guchar), "size mismatch");
                v.reset(
                    g_variant_new_fixed_array(
                        G_VARIANT_TYPE_BYTE, seq.getConstArray(),
                        seq.getLength(), sizeof (sal_Int8)));
                break;
            }
        case TYPE_BOOLEAN_LIST:
            {
                css::uno::Sequence<sal_Bool> seq(
                    value.get<css::uno::Sequence<sal_Bool>>());
                static_assert(
                    std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                    "G_MAXSIZE too small");
                static_assert(sizeof (sal_Bool) == 1, "size mismatch");
                v.reset(
                    g_variant_new_fixed_array(
                        G_VARIANT_TYPE_BOOLEAN, seq.getConstArray(),
                        seq.getLength(), sizeof (sal_Bool)));
                break;
            }
        case TYPE_SHORT_LIST:
            {
                css::uno::Sequence<sal_Int16> seq(
                    value.get<css::uno::Sequence<sal_Int16>>());
                static_assert(
                    std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                    "G_MAXSIZE too small");
                static_assert(
                    sizeof (sal_Int16) == sizeof (gint16), "size mismatch");
                v.reset(
                    g_variant_new_fixed_array(
                        G_VARIANT_TYPE_INT16, seq.getConstArray(),
                        seq.getLength(), sizeof (sal_Int16)));
                    //TODO: endian-ness?
                break;
            }
        case TYPE_INT_LIST:
            {
                css::uno::Sequence<sal_Int32> seq(
                    value.get<css::uno::Sequence<sal_Int32>>());
                static_assert(
                    std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                    "G_MAXSIZE too small");
                static_assert(
                    sizeof (sal_Int32) == sizeof (gint32), "size mismatch");
                v.reset(
                    g_variant_new_fixed_array(
                        G_VARIANT_TYPE_INT32, seq.getConstArray(),
                        seq.getLength(), sizeof (sal_Int32)));
                    //TODO: endian-ness?
                break;
            }
        case TYPE_LONG_LIST:
            {
                css::uno::Sequence<sal_Int64> seq(
                    value.get<css::uno::Sequence<sal_Int64>>());
                static_assert(
                    std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                    "G_MAXSIZE too small");
                static_assert(
                    sizeof (sal_Int64) == sizeof (gint64), "size mismatch");
                v.reset(
                    g_variant_new_fixed_array(
                        G_VARIANT_TYPE_INT64, seq.getConstArray(),
                        seq.getLength(), sizeof (sal_Int64)));
                    //TODO: endian-ness?
                break;
            }
        case TYPE_DOUBLE_LIST:
            {
                css::uno::Sequence<double> seq(
                    value.get<css::uno::Sequence<double>>());
                static_assert(
                    std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                    "G_MAXSIZE too small");
                static_assert(
                    sizeof (double) == sizeof (gdouble), "size mismatch");
                v.reset(
                    g_variant_new_fixed_array(
                        G_VARIANT_TYPE_DOUBLE, seq.getConstArray(),
                        seq.getLength(), sizeof (double)));
                    //TODO: endian-ness?
                break;
            }
        case TYPE_STRING_LIST:
            {
                css::uno::Sequence<OUString> seq(
                    value.get<css::uno::Sequence<OUString>>());
                std::vector<GVariant *> vs;
                for (sal_Int32 i = 0; i != seq.getLength(); ++i) {
                    children.emplace_front(
                        g_variant_new_string(encodeString(seq[i]).getStr()));
                    if (children.front().get() == nullptr) {
                        SAL_WARN(
                            "configmgr.dconf", "g_variant_new_string failed");
                        return false;
                    }
                    vs.push_back(children.front().get());
                }
                static_assert(
                    std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                    "G_MAXSIZE too small");
                v.reset(
                    g_variant_new_array(
                        G_VARIANT_TYPE_STRING, vs.data(), seq.getLength()));
                break;
            }
        case TYPE_HEXBINARY_LIST:
            {
                css::uno::Sequence<css::uno::Sequence<sal_Int8>> seq(
                    value.get<
                        css::uno::Sequence<css::uno::Sequence<sal_Int8>>>());
                std::vector<GVariant *> vs;
                for (sal_Int32 i = 0; i != seq.getLength(); ++i) {
                    static_assert(
                        std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                        "G_MAXSIZE too small");
                    static_assert(
                        sizeof (sal_Int8) == sizeof (guchar), "size mismatch");
                    children.emplace_front(
                        g_variant_new_fixed_array(
                            G_VARIANT_TYPE_BYTE, seq[i].getConstArray(),
                            seq[i].getLength(), sizeof (sal_Int8)));
                    if (children.front().get() == nullptr) {
                        SAL_WARN(
                            "configmgr.dconf",
                            "g_variant_new_fixed_array failed");
                        return false;
                    }
                    vs.push_back(children.front().get());
                }
                GVariantTypeHolder ty(g_variant_type_new("aay"));
                if (ty.get() == nullptr) {
                    SAL_WARN("configmgr.dconf", "g_variant_type_new failed");
                    return false;
                }
                static_assert(
                    std::numeric_limits<sal_Int32>::max() <= G_MAXSIZE,
                    "G_MAXSIZE too small");
                v.reset(
                    g_variant_new_array(ty.get(), vs.data(), seq.getLength()));
                break;
            }
        default:
            assert(false); // this cannot happen
            break;
        }
        if (v.get() == nullptr) {
            SAL_WARN("configmgr.dconf", "GVariant creation failed");
            return false;
        }
        if (nillable) {
            GVariantHolder v1(g_variant_new_maybe(nullptr, v.get()));
            if (v1.get() == nullptr) {
                SAL_WARN("configmgr.dconf", "g_variant_new_maybe failed");
                return false;
            }
            v.release();
            v.reset(v1.get());
            v1.release();
        }
    }
    dconf_changeset_set(
        changeset.get(), pathRepresentation.getStr(), v.get());
    for (auto & i: children) {
        i.release();
    }
    v.release();
    return true;
}

bool addNode(
    Components & components, ChangesetHolder const & changeset,
    rtl::Reference<Node> const & parent, OString const & pathRepresentation,
    rtl::Reference<Node> const & node)
{
    switch (node->kind()) {
    case Node::KIND_PROPERTY:
        {
            PropertyNode * prop = static_cast<PropertyNode *>(node.get());
            if (!addProperty(
                    changeset, pathRepresentation, prop->getStaticType(),
                    prop->isNillable(), prop->getValue(components)))
            {
                return false;
            }
            break;
        }
    case Node::KIND_LOCALIZED_VALUE:
        {
            //TODO: name.isEmpty()?
            LocalizedPropertyNode * locprop
                = static_cast<LocalizedPropertyNode *>(parent.get());
            if (!addProperty(
                    changeset, pathRepresentation,
                    locprop->getStaticType(), locprop->isNillable(),
                    static_cast<LocalizedValueNode *>(node.get())->getValue()))
            {
                return false;
            }
            break;
        }
    case Node::KIND_LOCALIZED_PROPERTY:
    case Node::KIND_GROUP:
    case Node::KIND_SET:
        for (auto const & i: node->getMembers()) {
            OUString templ(i.second->getTemplateName());
            OString path(
                pathRepresentation + "/"
                + encodeSegment(i.first, !templ.isEmpty()));
            if (!templ.isEmpty()) {
                path += "/";
                GVariantHolder v(g_variant_new_string("replace"));
                if (v.get() == nullptr) {
                    SAL_WARN("configmgr.dconf", "g_variant_new_string failed");
                    return false;
                }
                dconf_changeset_set(
                    changeset.get(), OString(path + "op").getStr(), v.get());
                v.release();
                v.reset(g_variant_new_string(encodeString(templ).getStr()));
                if (v.get() == nullptr) {
                    SAL_WARN("configmgr.dconf", "g_variant_new_string failed");
                    return false;
                }
                dconf_changeset_set(
                    changeset.get(), OString(path + "template").getStr(),
                    v.get());
                v.release();
                path += "content";
            }
            if (!addNode(components, changeset, parent, path, i.second)) {
                return false;
            }
        }
        break;
    case Node::KIND_ROOT:
        assert(false); // this cannot happen
        break;
    }
    return true;
}

bool addModifications(
    Components & components, ChangesetHolder const & changeset,
    OString const & parentPathRepresentation,
    rtl::Reference<Node> const & parent, OUString const & nodeName,
    rtl::Reference<Node> const & node,
    Modifications::Node const & modifications)
{
    // It is never necessary to write oor:finalized or oor:mandatory attributes,
    // as they cannot be set via the UNO API.
    if (modifications.children.empty()) {
        assert(parent.is());
            // components themselves have no parent but must have children
        if (node.is()) {
            OUString templ(node->getTemplateName());
            OString path(
                parentPathRepresentation + "/"
                + encodeSegment(nodeName, !templ.isEmpty()));
            if (!templ.isEmpty()) {
                path += "/";
                GVariantHolder v(g_variant_new_string("replace"));
                if (v.get() == nullptr) {
                    SAL_WARN("configmgr.dconf", "g_variant_new_string failed");
                    return false;
                }
                dconf_changeset_set(
                    changeset.get(), OString(path + "op").getStr(), v.get());
                v.release();
                v.reset(g_variant_new_string(encodeString(templ).getStr()));
                if (v.get() == nullptr) {
                    SAL_WARN("configmgr.dconf", "g_variant_new_string failed");
                    return false;
                }
                dconf_changeset_set(
                    changeset.get(), OString(path + "template").getStr(),
                    v.get());
                v.release();
                path += "content";
            }
            if (!addNode(components, changeset, parent, path, node)) {
                return false;
            }
        } else {
            switch (parent->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
            case Node::KIND_GROUP:
                {
                    GVariantHolder v(g_variant_new_tuple(nullptr, 0));
                    if (v.get() == nullptr) {
                        SAL_WARN(
                            "configmgr.dconf", "g_variant_new_tuple failed");
                        return false;
                    }
                    OString path(parentPathRepresentation);
                    if (!nodeName.isEmpty()) { // KIND_LOCALIZED_PROPERTY
                        path += "/" + encodeSegment(nodeName, false);
                    }
                    dconf_changeset_set(
                        changeset.get(), path.getStr(), v.get());
                    v.release();
                    break;
                }
            case Node::KIND_SET:
                {
                    OString path(
                        parentPathRepresentation + "/"
                        + encodeSegment(nodeName, true) + "/");
                    GVariantHolder v(g_variant_new_string("remove"));
                    if (v.get() == nullptr) {
                        SAL_WARN(
                            "configmgr.dconf", "g_variant_new_string failed");
                        return false;
                    }
                    dconf_changeset_set(
                        changeset.get(), OString(path + "op").getStr(),
                        v.get());
                    v.release();
                    dconf_changeset_set(
                        changeset.get(), OString(path + "template").getStr(),
                        nullptr);
                    dconf_changeset_set(
                        changeset.get(), OString(path + "content/").getStr(),
                        nullptr);
                    break;
                }
            default:
                assert(false); // this cannot happen
                break;
            }
        }
    } else {
        assert(node.is());
        OUString templ(node->getTemplateName());
        OString path(
            parentPathRepresentation + "/"
            + encodeSegment(nodeName, !templ.isEmpty()));
        if (!templ.isEmpty()) {
            path += "/";
            GVariantHolder v(g_variant_new_string("fuse"));
            if (v.get() == nullptr) {
                SAL_WARN("configmgr.dconf", "g_variant_new_string failed");
                return false;
            }
            dconf_changeset_set(
                changeset.get(), OString(path + "op").getStr(), v.get());
            v.release();
            v.reset(g_variant_new_string(encodeString(templ).getStr()));
            if (v.get() == nullptr) {
                SAL_WARN("configmgr.dconf", "g_variant_new_string failed");
                return false;
            }
            dconf_changeset_set(
                changeset.get(), OString(path + "template").getStr(), v.get());
            v.release();
            path += "content";
        }
        for (auto const & i: modifications.children) {
            if (!addModifications(
                    components, changeset, path, node, i.first,
                    node->getMember(i.first), i.second))
            {
                return false;
            }
        }
    }
    return true;
}

}

void readLayer(Data & data, int layer) {
    GObjectHolder<DConfClient> client(dconf_client_new());
    if (client.get() == nullptr) {
        SAL_WARN("configmgr.dconf", "dconf_client_new failed");
        return;
    }
    readDir(
        data, layer, rtl::Reference<Node>(), data.getComponents(), client,
        getRoot() + "/");
}

void writeModifications(Components & components, Data & data) {
    GObjectHolder<DConfClient> client(dconf_client_new());
    if (client.get() == nullptr) {
        SAL_WARN("configmgr.dconf", "dconf_client_new failed");
    }
    ChangesetHolder cs(dconf_changeset_new());
    if (cs.get() == nullptr) {
        SAL_WARN("configmgr.dconf", "dconf_changeset_new failed");
        return;
    }
    for (auto const & i: data.modifications.getRoot().children) {
        if (!addModifications(
                components, cs, getRoot(), rtl::Reference<Node>(), i.first,
                data.getComponents().findNode(Data::NO_LAYER, i.first),
                i.second))
        {
            return;
        }
    }
    if (!dconf_client_change_sync(
            client.get(), cs.get(), nullptr, nullptr, nullptr))
    {
        //TODO: GError
        SAL_WARN("configmgr.dconf", "dconf_client_change_sync failed");
        return;
    }
    data.modifications.clear();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
