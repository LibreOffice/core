/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* C++ wrapper for libatspi, so to make it less obnoxious to use */

/**
 * Adding a new wrapper
 *
 * To wrap a new Atspi type (let's say, AtspiCollection), you need to:
 *
 * 1. Add <tt>DEFINE_GOBJECT_CAST(AtspiCollection, ATSPI_TYPE_COLLECTION)</tt> near the similar
 *    ones. This creates <tt>Atspi::cast<AtspiCollection*>(p)</tt> so that such a cast based on the
 *    C++ type is checked using the GType type system.
 * 2. Add a declaration for the new wrapper class above Atspi::Accessible
 *    (<tt>class Collection;</tt>) so it can be used in step 3.
 * 3. Add <tt>Atspi::Accessible::queryCollection()</tt> method.  Its definition has to be in the
 *    source file as it requires a complete type for the wrapper class.  The body just calls
 *    <tt>queryInterface<Collection>(atspi_accessible_get_collection_iface);</tt> and returns
 *    its value.
 * 4. Add the definition of the new wrapper class:
 *    <tt>class Collection : public Accessible { ... }</tt>
 *    Use the existing wrappers as inspiration, but basically:
 *    - Define the constructor that only chains up to the parent
 *    - Define each wrapper method, which generally only have to call one of the <tt>invoke()</tt>
 *      helpers to wrap the C calls.  There are a few, depending on some details of the C call:
 *      - @c GObjectWrapperBase::invoke(): this is the most basic one, that just calls the C method
 *           on @c GObjectWrapperBase::get() with the given arguments.  Use this for calls not
 *           throwing an exception and either returning a plain value, or something not handled by
 *           one of the others below.
 *      - @c AtspiWrapperBase::invokeThrow(): like @c GObjectWrapperBase::invoke(), but for C calls
 *           that take a @c GError argument for throwing exceptions.  @c invokeThrow() will
 *           transform any C exception into a a C++ exception (@c css::uno::RuntimeException)
 *      - @c AtspiWrapperBase::strInvoke(): like @c AtspiWrapperBase::invokeThrow(), but manages a
 *           C string (@c char*) return value as an @c std::string.  Use this for C calls returning
 *           a C string.
 *      - @c AtspiWrapperBase::garrayInvoke(): like @c AtspiWrapperBase::invokeThrow(), but manages
 *           a @c GArray return value as an @c std::vector.  Use this for C calls returning a
 *           @p GArray.
 *      - @c AtspiWrapperBase::hashMapInvoke(): like @c AtspiWrapperBase::invokeThrow(), but manages
 *           a @c GHashTable return value as an @c std::unordered_map.  Use this for C calls
 *           returning a @p GHashTable.
 *      - @c AtspiWrapperBase::strHashMapInvoke(): identical to @c AtspiWrapperBase::hashMapInvoke()
 *           using C strings for keys and values.
 *      .
 *     If none of those match the exact return type of the C call to wrap, use
 *     @c AtspiWrapperBase::invokeThrow() or even @c GObjectWrapperBase::invoke(), and manually
 *     manage the result value.  You can use Atspi::gmem functions to help.  Basically the idea is
 *     that you always return a self-managing object to make memory management easy (whereas it's
 *     obnoxiously hard with plain C Atspi API).
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <boost/type_traits/function_traits.hpp>

#include <atspi/atspi.h>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <cppunit/TestAssert.h>

#include <config_atspi.h>

namespace Atspi
{
/** @brief Helpers for managing GLib memory in a more C++-style */
namespace gmem
{
/** @brief Wraps a pointer to free with @c g_free() in a @c std::unique_ptr */
template <typename T> static inline auto unique_gmem(T* ptr)
{
    return std::unique_ptr<T, decltype(&g_free)>(ptr, &g_free);
}

/** @brief Wraps a @c GArray to free with @c g_array_unref() in a @c std::unique_ptr */
static inline auto unique_garray(GArray* p)
{
    return std::unique_ptr<GArray, decltype(&g_array_unref)>(p, &g_array_unref);
}

/** @brief Wraps a @c GHashTable to free with @c g_hash_table_unref() in a @c std::unique_ptr */
static inline auto unique_ghashtable(GHashTable* p)
{
    return std::unique_ptr<GHashTable, decltype(&g_hash_table_unref)>(p, &g_hash_table_unref);
}
}

// --- GObject cast wrappers based on type: usage is cast<AtspiAccessible*>(pCInstance)
#define DEFINE_GOBJECT_CAST(CType, GType)                                                          \
    template <typename P, typename T, std::enable_if_t<std::is_same_v<P, CType*>, int> = 1>        \
    P cast(T* pInstance)                                                                           \
    {                                                                                              \
        return G_TYPE_CHECK_INSTANCE_CAST(pInstance, GType, std::remove_pointer_t<P>);             \
    }

DEFINE_GOBJECT_CAST(AtspiStateSet, ATSPI_TYPE_STATE_SET)
DEFINE_GOBJECT_CAST(AtspiRelation, ATSPI_TYPE_RELATION)
DEFINE_GOBJECT_CAST(AtspiAccessible, ATSPI_TYPE_ACCESSIBLE)
DEFINE_GOBJECT_CAST(AtspiComponent, ATSPI_TYPE_COMPONENT)
DEFINE_GOBJECT_CAST(AtspiText, ATSPI_TYPE_TEXT)

#undef DEFINE_GOBJECT_CAST
// --- end GObject cast wrappers

class GLibEnumBase
{
protected:
    /**
     * @brief Retrieves the string representation of an enumeration value
     * @param gt The @c GType for the enumeration
     * @param value The enumeration value for which to get the name for
     * @param fallback Fallback value in case @p values falls outside the enumeration
     * @returns A string representing @p value
     */
    static std::string glibEnumValueName(GType gt, gint value,
                                         std::string_view fallback = "unknown")
    {
        auto klass = static_cast<GEnumClass*>(g_type_class_ref(gt));
        auto enum_value = g_enum_get_value(klass, value);
        std::string ret(enum_value ? enum_value->value_name : fallback);
        g_type_class_unref(klass);
        return ret;
    }
};

class Role : private GLibEnumBase
{
public:
    static std::string getName(AtspiRole role)
    {
        return glibEnumValueName(atspi_role_get_type(), role);
    }
};

class State : private GLibEnumBase
{
public:
    static std::string getName(AtspiStateType state)
    {
        return glibEnumValueName(atspi_state_type_get_type(), state);
    }
};

class TextGranularity : private GLibEnumBase
{
public:
    static std::string getName(AtspiTextGranularity granularity)
    {
        return glibEnumValueName(atspi_text_granularity_get_type(), granularity);
    }
};

class TextBoundaryType : private GLibEnumBase
{
public:
    static std::string getName(AtspiTextBoundaryType boundaryType)
    {
        return glibEnumValueName(atspi_text_boundary_type_get_type(), boundaryType);
    }
};

/**
 * @brief Base class for GObject wrappers
 *
 * This leverages std::shared_ptr as a cheap way of wrapping a raw pointer, and its deleter as a
 * mean of using g_object_unref() to cleanup.  This is sub-optimal as it maintains a separate
 * refcount to the GObject one, but it's easy.
 */
template <class T> class GObjectWrapperBase : public std::shared_ptr<T>
{
public:
    /* this is the boundary of C++ type safety, so we can have inheritance working
     * properly with the C types.  This should still be safe as it uses cast() which should be
     * defined for each using type with DEFINE_GOBJECT_CAST(), which uses GType validation */
    template <typename P = T*> P get() const { return cast<P>(std::shared_ptr<T>::get()); }

protected:
    /**
     * @brief Calls the C function @p f on the C object wrapped by @p this
     * @param f The C function to call
     * @param args Additional arguments to @p f
     * @returns The return value from @p f
     *
     * Calls the C function @p f similar to <tt>f(get(), args...)</tt>.  Care is taken of
     * transforming @c get() to the type actually expected as the first argument of @p f, using
     * @c get<TypeOfFsFirstArgument>(), which performs a runtime verification of the conversion.
     *
     * @note The type verification on whether @p f actually takes what get() returns is performed
     *       at runtime, so there will be no compilation error or warning if trying to use an
     *       incompatible C function.  A check will however be performed at runtime, at least
     *       helping diagnose a possible invalid conversion.
     */
    template <typename F, typename... Ts> inline auto invoke(F f, Ts... args) const
    {
        using FT = std::remove_pointer_t<F>;
        const auto p = get<typename boost::function_traits<FT>::arg1_type>();
        return f(p, args...);
    }

private:
    static void deleter(T* p)
    {
        if (p)
            g_object_unref(p);
    }

public:
    /**
     * @param pObj The raw GObject to wrap
     * @param takeRef Whether to take ownership of the object or not.  If set to @c false, it will
     *                call @c g_object_ref(pAcc) to acquire a new reference to the GObject.
     */
    GObjectWrapperBase(T* pObj = nullptr, bool takeRef = true)
        : std::shared_ptr<T>(pObj, deleter)
    {
        if (pObj && !takeRef)
            g_object_ref(pObj);
    }
};

/** @brief AtspiStateSet C++ wrapper */
class StateSet : public GObjectWrapperBase<AtspiStateSet>
{
public:
    using GObjectWrapperBase::GObjectWrapperBase;

    void add(const AtspiStateType t) { invoke(atspi_state_set_add, t); }
    StateSet compare(const StateSet& other) const
    {
        return StateSet(invoke(atspi_state_set_compare, other.get()));
    }
    bool contains(const AtspiStateType t) const { return invoke(atspi_state_set_contains, t); }
    bool operator==(const StateSet& other) const
    {
        return invoke(atspi_state_set_equals, other.get());
    }
    std::vector<AtspiStateType> getStates() const
    {
        auto garray = gmem::unique_garray(invoke(atspi_state_set_get_states));
        std::vector<AtspiStateType> states;
        for (auto i = decltype(garray->len){ 0 }; i < garray->len; i++)
        {
            states.push_back(g_array_index(garray, decltype(states)::value_type, i));
        }
        return states;
    }
    bool empty() const { return invoke(atspi_state_set_is_empty); }
    void remove(AtspiStateType t) { invoke(atspi_state_set_remove, t); }
    void setByName(const std::string_view name, bool enable)
    {
        invoke(atspi_state_set_set_by_name, name.data(), enable);
    }
};

class Accessible;

/** @brief AtspiRelation C++ wrapper */
class Relation : public GObjectWrapperBase<AtspiRelation>
{
public:
    using GObjectWrapperBase::GObjectWrapperBase;

    AtspiRelationType getRelationType() const { return invoke(atspi_relation_get_relation_type); }
    int getNTargets() const { return invoke(atspi_relation_get_n_targets); }
    Accessible getTarget(int i) const;
};

/* intermediate base just for splitting out the *invoke* helpers implementations, so the actual
 * user-targeted class can hold only the actual API */
template <class T> class AtspiWrapperBase : public GObjectWrapperBase<T>
{
protected:
    using GObjectWrapperBase<T>::invoke;

    /**
     * @brief Calls the throwing C function @p f on the C object wrapped by @p this
     * @param f The C function to call
     * @param args Additional arguments to @p f
     * @returns The raw return value from @p f
     * @throws css::uno::RuntimeException Exception @c GError are translated to
     *
     * This wrapper calls @p f with parameters @p args and an additional @c GError parameter to
     * catch C exception, transforming them into C++ exceptions of type
     * @c css::uno::RuntimeException.
     *
     * @see invoke()
     */
    template <typename F, typename... Ts> inline auto invokeThrow(F f, Ts... args) const
    {
        GError* err = nullptr;
        auto ret = invoke(f, args..., &err);
        if (err)
        {
            throw css::uno::RuntimeException(OUString::fromUtf8(err->message));
        }
        return ret;
    }

    /**
     * @brief Calls the throwing C function @p f on the C object wrapped by @p this and returns a string
     * @param f The C function to call
     * @param args Additional arguments to @p f
     * @tparam E the type of exception to throw if @p f returns @c null, defaults to
     *           @c css::uno::RuntimeException
     * @returns A string holding the return value from @p f
     * @throws css::uno::RuntimeException See invokeThrow()
     * @throws E Exception to use if @p f returns null with no other error
     *
     * Just like @c invokeThrow(), but wraps the return value in an @c std::string and manages the
     * lifetime of the C function return value.
     *
     * As @c std::string cannot represent a @c null value, if @p f returned such a value without
     * throwing an exception, this method will throw an exception of type @p E
     *
     * @see invokeThrow()
     */
    template <typename E = css::uno::RuntimeException, typename F, typename... Ts>
    inline std::string strInvoke(F f, Ts... args) const
    {
        auto r = invokeThrow(f, args...);

        /* if the API returned NULL without throwing, use the specified exception because a nullptr
         * std::string is not valid, and std::logic_error isn't gonna be very useful to the caller */
        if (!r)
            throw E();

        return gmem::unique_gmem(r).get();
    }

    /**
     * @brief Calls the throwing C function @p f on the C object wrapped by @p this and returns a vector
     * @param f The C function to call
     * @param args Additional arguments to @p f
     * @tparam Vi The type of the members of the @c GArray @p f returns
     * @tparam Vo The type of the members of the returned vector
     * @returns A vector holding the return value from @p f
     * @throws css::uno::RuntimeException See invokeThrow()
     *
     * Just like @c invokeThrow(), but wraps the return in an @c std::vector<Vo> and manages the
     * lifetime of the C function return value.
     *
     * @p Vi has to be implicitly convertible to @p Vo.  A typical usage could be
     * <tt>garrayInvoke<AtspiAccessible*, Accessible>(...)</tt>, which would transform a @c GArray
     * of @c AtspiAccessible* to an @c std::vector of @c Atspi::Accessible.
     *
     * @warning You have to get @p Vi right, there is no way to validate this type is correct or
     *          not, so you won't get a compilation error nor even a warning if you give the wrong
     *          type here.
     *
     * @see invokeThrow()
     */
    template <typename Vi, typename Vo, typename F, typename... Ts>
    inline std::vector<Vo> garrayInvoke(F f, Ts... args) const
    {
        auto garray = gmem::unique_garray(invokeThrow(f, args...));
        std::vector<Vo> vec;
        for (auto i = decltype(garray->len){ 0 }; i < garray->len; i++)
            vec.push_back(g_array_index(garray, Vi, i));
        return vec;
    }

    /**
     * @brief Wraps an AT-SPI call returning a GHashTable
     * @tparam Ki Type of the keys in the wrapped hash table
     * @tparam Vi Type of the values in the wrapped hash table
     * @tparam Ko Type of the keys in the wrapper map (this must be convertible from Ki)
     * @tparam Vo Type of the values in the wrapper map (this must be convertible from Kv)
     * @param f The function to call
     * @param args Arguments to pass to @p f
     * @returns A @c std::unordered_map holding the data returned by @p f.
     *
     * @see invokeThrow()
     * @see strHashMapInvoke()
     */
    template <typename Ki, typename Vi, typename Ko, typename Vo, typename F, typename... Ts>
    inline std::unordered_map<Ko, Vo> hashMapInvoke(F f, Ts... args) const
    {
        auto ghash = gmem::unique_ghashtable(invokeThrow(f, args...));
        std::unordered_map<Ko, Vo> map;
        GHashTableIter iter;
        g_hash_table_iter_init(&iter, ghash.get());
        gpointer key, value;
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            map.emplace(static_cast<Ki>(key), static_cast<Vi>(value));
        }
        return map;
    }

    /**
     * @brief Just like @c hashMapInvoke() but already specialized for strings
     * @param f The C function to call
     * @param args Arguments to @p f
     * @returns A @c std::unordered_map holding the data returned by @p f
     *
     * This is exactly the same as
     * <tt>hashMapInvoke<gchar*, gchar*, std::string, std::string>(f, args...)</tt>
     */
    template <typename F, typename... Ts> inline auto strHashMapInvoke(F f, Ts... args) const
    {
        return hashMapInvoke<gchar*, gchar*, std::string, std::string>(f, args...);
    }

public:
    using GObjectWrapperBase<T>::GObjectWrapperBase;
};

class Component;
class Text;

/**
 * @brief AtspiAccessible C++ wrapper
 *
 * This is a wrapper for the AtspiAccessible GObject class to make it a bit nicer to use in C++,
 * including a proper class with methods, regular exceptions, easy memory management, and an
 * iterator to enumerate children.
 *
 * As this class actually inherits from std::shared_ptr, you can easily use @c get() to retrieve
 * the wrapped pointer in case you need to, e.g. if some specific API is missing.  However, take
 * care of memory management on that object not to have it destroyed early (e.g. don't let anyone
 * call @c g_object_unref() on it if they didn't call g_object_ref() first).
 *
 * To use it, just wrap an initial AtspiAccessible using the class constructor.
 * @code
 * Atspi::Accessible desktop(atspi_get_desktop(0));
 * for (auto&& app: desktop) {
 *     std::cout << app->getName() << std::endl;
 * }
 * @endcode
 *
 * For details on the specific methods, see the C Atspi documentation.
 */
class Accessible : public AtspiWrapperBase<AtspiAccessible>
{
public:
    using AtspiWrapperBase<AtspiAccessible>::AtspiWrapperBase;

    void setCacheMask(AtspiCache mask) const { invoke(atspi_accessible_set_cache_mask, mask); }
    void clearCache() const { invoke(atspi_accessible_clear_cache); }

    AtspiRole getRole() const { return invokeThrow(atspi_accessible_get_role); }
    std::string getRoleName() const { return strInvoke(atspi_accessible_get_role_name); }
    std::string getName() const { return strInvoke(atspi_accessible_get_name); }
    std::string getDescription() const { return strInvoke(atspi_accessible_get_description); }

    int getChildCount() const { return invokeThrow(atspi_accessible_get_child_count); }
    Accessible getChildAtIndex(int idx) const
    {
        return Accessible(invokeThrow(atspi_accessible_get_child_at_index, idx));
    }
    int getIndexInParent() const { return invokeThrow(atspi_accessible_get_index_in_parent); }
    Accessible getParent() const { return Accessible(invokeThrow(atspi_accessible_get_parent)); }

    StateSet getStateSet() const { return StateSet(invoke(atspi_accessible_get_state_set)); }

    std::unordered_map<std::string, std::string> getAttributes() const
    {
        return strHashMapInvoke(atspi_accessible_get_attributes);
    }

    std::vector<Relation> getRelationSet() const
    {
        return garrayInvoke<AtspiRelation*, Relation>(atspi_accessible_get_relation_set);
    }

private:
    template <class I, typename F> I queryInterface(F f) const
    {
        auto pIface = invoke(f);
        if (pIface)
            return I(pIface);
        throw css::uno::RuntimeException(u"Not implemented"_ustr);
    }

public:
    Component queryComponent() const;
    Text queryText() const;

    // convenience extensions
    class iterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = Accessible;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;

    private:
        const Accessible* m_pAccessible;
        int m_idx;

    public:
        explicit iterator(const Accessible* pAccessible, int idx = 0)
            : m_pAccessible(pAccessible)
            , m_idx(idx)
        {
        }

        iterator(iterator const& other)
            : m_pAccessible(other.m_pAccessible)
            , m_idx(other.m_idx)
        {
        }

        iterator& operator++()
        {
            m_idx++;
            return *this;
        }

        iterator operator++(int)
        {
            iterator other = *this;
            ++(*this);
            return other;
        }

        bool operator==(iterator other) const
        {
            return m_idx == other.m_idx && m_pAccessible == other.m_pAccessible;
        }
        bool operator!=(iterator other) const { return !(*this == other); }
        value_type operator*() const
        {
            assert(m_idx < m_pAccessible->getChildCount());
            return m_pAccessible->getChildAtIndex(m_idx);
        }
    };

    iterator begin() const { return iterator(this); }
    iterator end() const
    {
        if (!get())
            return iterator(this);
        return iterator(this, std::max(0, getChildCount()));
    }
};

// we just use ATSPI's own structure here, but pass it by value
using Rect = AtspiRect;
using Point = AtspiPoint;

/** @brief AtspiComponent C++ wrapper */
class Component : public Accessible
{
public:
    Component(AtspiComponent* pObj = nullptr, bool takeRef = true)
        : Accessible(cast<AtspiAccessible*>(pObj), takeRef)
    {
    }

    bool contains(int x, int y, AtspiCoordType coordType) const
    {
        return invokeThrow(atspi_component_contains, x, y, coordType);
    }
    Accessible getAccessibleAtPoint(int x, int y, AtspiCoordType coordType) const
    {
        return invokeThrow(atspi_component_get_accessible_at_point, x, y, coordType);
    }
    Rect getExtents(AtspiCoordType coordType) const
    {
        return *gmem::unique_gmem(invokeThrow(atspi_component_get_extents, coordType));
    }
    Point getPosition(AtspiCoordType coordType) const
    {
        return *gmem::unique_gmem(invokeThrow(atspi_component_get_position, coordType));
    }
    Point getSize() const { return *gmem::unique_gmem(invokeThrow(atspi_component_get_size)); }
    AtspiComponentLayer getLayer() const { return invokeThrow(atspi_component_get_layer); }
    short getMdiZOrder() const { return invokeThrow(atspi_component_get_mdi_z_order); }
    bool grabFocus() const { return invokeThrow(atspi_component_grab_focus); }
    double getAlpha() const { return invokeThrow(atspi_component_get_alpha); }

#if HAVE_ATSPI2_SCROLL_TO
    bool scrollTo(AtspiScrollType scrollType) const
    {
        return invokeThrow(atspi_component_scroll_to, scrollType);
    }
    bool scrollToPoint(AtspiCoordType coordType, int x, int y) const
    {
        return invokeThrow(atspi_component_scroll_to_point, coordType, x, y);
    }
#endif // HAVE_ATSPI2_SCROLL_TO
};

/** @brief AtspiText C++ wrapper */
class Text : public Accessible
{
public:
    Text(AtspiText* pObj = nullptr, bool takeRef = true)
        : Accessible(cast<AtspiAccessible*>(pObj), takeRef)
    {
    }

    /** Wrapper for AtspiRange
     *
     * This is not actually required, but helps make this more C++-y (by allowing TextRange to
     * inherit it) and more LibreOffice-y (by having camelCase names) */
    struct Range
    {
        int startOffset;
        int endOffset;

        Range(int startOffset_, int endOffset_)
            : startOffset(startOffset_)
            , endOffset(endOffset_)
        {
        }

        Range(const AtspiRange* r)
            : startOffset(r->start_offset)
            , endOffset(r->end_offset)
        {
        }
    };

    /** Wrapper for AtspiTextRange */
    struct TextRange : Range
    {
        std::string content;

        TextRange(int startOffset_, int endOffset_, std::string_view content_)
            : Range(startOffset_, endOffset_)
            , content(content_)
        {
        }

        TextRange(const AtspiTextRange* r)
            : Range(r->start_offset, r->end_offset)
            , content(r->content)
        {
        }
    };

    int getCharacterCount() const { return invokeThrow(atspi_text_get_character_count); }
    std::string getText(int startOffset, int endOffset) const
    {
        return strInvoke(atspi_text_get_text, startOffset, endOffset);
    }
    int getCaretOffset() const { return invokeThrow(atspi_text_get_caret_offset); }

    std::unordered_map<std::string, std::string> getTextAttributes(int offset, int* startOffset,
                                                                   int* endOffset) const
    {
        return strHashMapInvoke(atspi_text_get_text_attributes, offset, startOffset, endOffset);
    }

    std::unordered_map<std::string, std::string>
    getAttributeRun(int offset, bool includeDefaults, int* startOffset, int* endOffset) const
    {
        return strHashMapInvoke(atspi_text_get_attribute_run, offset, includeDefaults, startOffset,
                                endOffset);
    }

    std::unordered_map<std::string, std::string> getDefaultAttributes() const
    {
        return strHashMapInvoke(atspi_text_get_default_attributes);
    }

    std::string getTextAttributeValue(int offset, std::string_view name) const
    {
        return strInvoke<css::container::NoSuchElementException>(
            atspi_text_get_text_attribute_value, offset, const_cast<char*>(name.data()));
    }

protected:
    /** Like @c invokeThrow() on C calls returning an @c AtspiTextRange */
    template <typename F, typename... Ts> inline TextRange invokeTextRange(F f, Ts... args) const
    {
        struct deleter
        {
            void operator()(AtspiTextRange* ptr)
            {
                g_free(ptr->content);
                g_free(ptr);
            }
        };

        std::unique_ptr<AtspiTextRange, deleter> r(invokeThrow(f, args...));
        return r.get();
    }

public:
    TextRange getStringAtOffset(int offset, AtspiTextGranularity granularity) const
    {
        return invokeTextRange(atspi_text_get_string_at_offset, offset, granularity);
    }

    /* the next 3 are deprecated, but LO doesn't implement getStringAtOffset() itself so it's a lot
     * trickier to test for */
    TextRange getTextBeforeOffset(int offset, AtspiTextBoundaryType boundary) const
    {
        return invokeTextRange(atspi_text_get_text_before_offset, offset, boundary);
    }
    TextRange getTextAtOffset(int offset, AtspiTextBoundaryType boundary) const
    {
        return invokeTextRange(atspi_text_get_text_at_offset, offset, boundary);
    }
    TextRange getTextAfterOffset(int offset, AtspiTextBoundaryType boundary) const
    {
        return invokeTextRange(atspi_text_get_text_after_offset, offset, boundary);
    }

    sal_Int32 getCharacterAtOffset(int offset) const
    {
        return invokeThrow(atspi_text_get_character_at_offset, offset);
    }

    Rect getCharacterExtents(int offset, AtspiCoordType type) const
    {
        return *gmem::unique_gmem(invokeThrow(atspi_text_get_character_extents, offset, type));
    }

    int getOffsetAtPoint(int x, int y, AtspiCoordType type) const
    {
        return invokeThrow(atspi_text_get_offset_at_point, x, y, type);
    }

    Rect getRangeExtents(int startOffset, int endOffset, AtspiCoordType type) const
    {
        return *gmem::unique_gmem(
            invokeThrow(atspi_text_get_range_extents, startOffset, endOffset, type));
    }

    // getBoundedRanges() ?

    int getNSelections() const { return invokeThrow(atspi_text_get_n_selections); }

    Range getSelection(gint selectionNum) const
    {
        return gmem::unique_gmem(invokeThrow(atspi_text_get_selection, selectionNum)).get();
    }

    bool addSelection(int startOffset, int endOffset) const
    {
        return invokeThrow(atspi_text_add_selection, startOffset, endOffset);
    }

    bool removeSelection(int selectionNum) const
    {
        return invokeThrow(atspi_text_remove_selection, selectionNum);
    }

    bool setSelection(int selectionNum, int startOffset, int endOffset) const
    {
        return invokeThrow(atspi_text_set_selection, selectionNum, startOffset, endOffset);
    }

#if HAVE_ATSPI2_SCROLL_TO
    bool scrollSubstringTo(int startOffset, int endOffset, AtspiScrollType type) const
    {
        return invokeThrow(atspi_text_scroll_substring_to, startOffset, endOffset, type);
    }

    bool scrollSubstringToPoint(int startOffset, int endOffset, AtspiCoordType coords, gint x,
                                gint y) const
    {
        return invokeThrow(atspi_text_scroll_substring_to_point, startOffset, endOffset, coords, x,
                           y);
    }
#endif // HAVE_ATSPI2_SCROLL_TO
};
}

// CppUnit integration
CPPUNIT_NS_BEGIN
template <> struct assertion_traits<AtspiRole>
{
    static bool equal(const AtspiRole a, const AtspiRole b) { return a == b; }

    static std::string toString(const AtspiRole role) { return Atspi::Role::getName(role); }
};
CPPUNIT_NS_END

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
