/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <unx/gtk/gtkframe.hxx>
#include <gtk/gtk.h>
#include <o3tl/string_view.hxx>

#if GTK_CHECK_VERSION(4, 9, 0)

#include "a11y.hxx"
#include "gtkaccessibleeventlistener.hxx"
#include "gtkaccessibleregistry.hxx"
#include "gtkaccessibletext.hxx"

#define OOO_TYPE_FIXED (ooo_fixed_get_type())
#define OOO_FIXED(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), OOO_TYPE_FIXED, OOoFixed))
// #define OOO_IS_FIXED(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), OOO_TYPE_FIXED))

struct OOoFixed
{
    GtkFixed parent_instance;
    GtkATContext* at_context;
};

struct OOoFixedClass
{
    GtkFixedClass parent_class;
};

static GtkAccessibleRole
map_accessible_role(const css::uno::Reference<css::accessibility::XAccessible>& rAccessible)
{
    GtkAccessibleRole eRole(GTK_ACCESSIBLE_ROLE_WIDGET);

    if (rAccessible.is())
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
            rAccessible->getAccessibleContext());
        // https://w3c.github.io/core-aam/#mapping_role
        // https://hg.mozilla.org/mozilla-central/file/tip/accessible/base/RoleMap.h
        // https://gitlab.gnome.org/GNOME/gtk/-/blob/main/gtk/a11y/gtkatspiutils.c
        switch (xContext->getAccessibleRole())
        {
            case css::accessibility::AccessibleRole::ALERT:
            case css::accessibility::AccessibleRole::NOTIFICATION:
                eRole = GTK_ACCESSIBLE_ROLE_ALERT;
                break;
            case css::accessibility::AccessibleRole::BLOCK_QUOTE:
#if GTK_CHECK_VERSION(4, 13, 4)
                eRole = GTK_ACCESSIBLE_ROLE_BLOCK_QUOTE;
#else
                eRole = GTK_ACCESSIBLE_ROLE_GROUP;
#endif
                break;
            case css::accessibility::AccessibleRole::CAPTION:
                eRole = GTK_ACCESSIBLE_ROLE_CAPTION;
                break;
            case css::accessibility::AccessibleRole::COLUMN_HEADER:
                eRole = GTK_ACCESSIBLE_ROLE_COLUMN_HEADER;
                break;
            case css::accessibility::AccessibleRole::COMBO_BOX:
                eRole = GTK_ACCESSIBLE_ROLE_COMBO_BOX;
                break;
            case css::accessibility::AccessibleRole::DIALOG:
            case css::accessibility::AccessibleRole::FILE_CHOOSER:
                eRole = GTK_ACCESSIBLE_ROLE_DIALOG;
                break;
            case css::accessibility::AccessibleRole::FOOTNOTE:
            case css::accessibility::AccessibleRole::NOTE:
                eRole = GTK_ACCESSIBLE_ROLE_NOTE;
                break;
            case css::accessibility::AccessibleRole::FORM:
                eRole = GTK_ACCESSIBLE_ROLE_FORM;
                break;
            case css::accessibility::AccessibleRole::HEADING:
                eRole = GTK_ACCESSIBLE_ROLE_HEADING;
                break;
            case css::accessibility::AccessibleRole::HYPER_LINK:
                eRole = GTK_ACCESSIBLE_ROLE_LINK;
                break;
            case css::accessibility::AccessibleRole::PANEL:
                eRole = GTK_ACCESSIBLE_ROLE_GROUP;
                break;
            case css::accessibility::AccessibleRole::ROOT_PANE:
                eRole = GTK_ACCESSIBLE_ROLE_GROUP;
                break;
            case css::accessibility::AccessibleRole::MENU_BAR:
                eRole = GTK_ACCESSIBLE_ROLE_MENU_BAR;
                break;
            case css::accessibility::AccessibleRole::STATUS_BAR:
                eRole = GTK_ACCESSIBLE_ROLE_STATUS;
                break;
            case css::accessibility::AccessibleRole::MENU:
            case css::accessibility::AccessibleRole::POPUP_MENU:
                eRole = GTK_ACCESSIBLE_ROLE_MENU;
                break;
            case css::accessibility::AccessibleRole::SPLIT_PANE:
                eRole = GTK_ACCESSIBLE_ROLE_GROUP;
                break;
            case css::accessibility::AccessibleRole::TOOL_BAR:
                eRole = GTK_ACCESSIBLE_ROLE_TOOLBAR;
                break;
            case css::accessibility::AccessibleRole::LABEL:
            case css::accessibility::AccessibleRole::STATIC:
                eRole = GTK_ACCESSIBLE_ROLE_LABEL;
                break;
            case css::accessibility::AccessibleRole::LIST:
                eRole = GTK_ACCESSIBLE_ROLE_LIST;
                break;
            case css::accessibility::AccessibleRole::LIST_ITEM:
                eRole = GTK_ACCESSIBLE_ROLE_LIST_ITEM;
                break;
            case css::accessibility::AccessibleRole::MENU_ITEM:
                eRole = GTK_ACCESSIBLE_ROLE_MENU_ITEM;
                break;
            case css::accessibility::AccessibleRole::SEPARATOR:
                eRole = GTK_ACCESSIBLE_ROLE_SEPARATOR;
                break;
            case css::accessibility::AccessibleRole::CHECK_BOX:
                eRole = GTK_ACCESSIBLE_ROLE_CHECKBOX;
                break;
            case css::accessibility::AccessibleRole::CHECK_MENU_ITEM:
                eRole = GTK_ACCESSIBLE_ROLE_MENU_ITEM_CHECKBOX;
                break;
            case css::accessibility::AccessibleRole::RADIO_MENU_ITEM:
                eRole = GTK_ACCESSIBLE_ROLE_MENU_ITEM_RADIO;
                break;
            case css::accessibility::AccessibleRole::DOCUMENT:
            case css::accessibility::AccessibleRole::DOCUMENT_PRESENTATION:
            case css::accessibility::AccessibleRole::DOCUMENT_SPREADSHEET:
            case css::accessibility::AccessibleRole::DOCUMENT_TEXT:
                eRole = GTK_ACCESSIBLE_ROLE_DOCUMENT;
                break;
            case css::accessibility::AccessibleRole::ROW_HEADER:
                eRole = GTK_ACCESSIBLE_ROLE_ROW_HEADER;
                break;
            case css::accessibility::AccessibleRole::RULER:
                eRole = GTK_ACCESSIBLE_ROLE_WIDGET;
                break;
            case css::accessibility::AccessibleRole::PARAGRAPH:
#if GTK_CHECK_VERSION(4, 13, 1)
                eRole = GTK_ACCESSIBLE_ROLE_PARAGRAPH;
#else
                eRole = GTK_ACCESSIBLE_ROLE_GROUP;
#endif
                break;
            case css::accessibility::AccessibleRole::FILLER:
                eRole = GTK_ACCESSIBLE_ROLE_GENERIC;
                break;
            case css::accessibility::AccessibleRole::PUSH_BUTTON:
            case css::accessibility::AccessibleRole::BUTTON_DROPDOWN:
            case css::accessibility::AccessibleRole::BUTTON_MENU:
                eRole = GTK_ACCESSIBLE_ROLE_BUTTON;
                break;
            case css::accessibility::AccessibleRole::TOGGLE_BUTTON:
                eRole = GTK_ACCESSIBLE_ROLE_TOGGLE_BUTTON;
                break;
            case css::accessibility::AccessibleRole::TABLE:
                eRole = GTK_ACCESSIBLE_ROLE_TABLE;
                break;
            case css::accessibility::AccessibleRole::TABLE_CELL:
                eRole = GTK_ACCESSIBLE_ROLE_CELL;
                break;
            case css::accessibility::AccessibleRole::PAGE_TAB:
                eRole = GTK_ACCESSIBLE_ROLE_TAB;
                break;
            case css::accessibility::AccessibleRole::PAGE_TAB_LIST:
                eRole = GTK_ACCESSIBLE_ROLE_TAB_LIST;
                break;
            case css::accessibility::AccessibleRole::PROGRESS_BAR:
                eRole = GTK_ACCESSIBLE_ROLE_PROGRESS_BAR;
                break;
            case css::accessibility::AccessibleRole::RADIO_BUTTON:
                eRole = GTK_ACCESSIBLE_ROLE_RADIO;
                break;
            case css::accessibility::AccessibleRole::SCROLL_BAR:
                eRole = GTK_ACCESSIBLE_ROLE_SCROLLBAR;
                break;
            case css::accessibility::AccessibleRole::SLIDER:
                eRole = GTK_ACCESSIBLE_ROLE_SLIDER;
                break;
            case css::accessibility::AccessibleRole::SPIN_BOX:
                eRole = GTK_ACCESSIBLE_ROLE_SPIN_BUTTON;
                break;
            case css::accessibility::AccessibleRole::TEXT:
            case css::accessibility::AccessibleRole::PASSWORD_TEXT:
                eRole = GTK_ACCESSIBLE_ROLE_TEXT_BOX;
                break;
            case css::accessibility::AccessibleRole::TOOL_TIP:
                eRole = GTK_ACCESSIBLE_ROLE_TOOLTIP;
                break;
            case css::accessibility::AccessibleRole::TREE:
                eRole = GTK_ACCESSIBLE_ROLE_TREE;
                break;
            case css::accessibility::AccessibleRole::TREE_ITEM:
                eRole = GTK_ACCESSIBLE_ROLE_TREE_ITEM;
                break;
            case css::accessibility::AccessibleRole::TREE_TABLE:
                eRole = GTK_ACCESSIBLE_ROLE_TREE_GRID;
                break;
            case css::accessibility::AccessibleRole::GRAPHIC:
            case css::accessibility::AccessibleRole::ICON:
            case css::accessibility::AccessibleRole::SHAPE:
                eRole = GTK_ACCESSIBLE_ROLE_IMG;
                break;
            case css::accessibility::AccessibleRole::COMMENT:
#if GTK_CHECK_VERSION(4, 13, 4)
                eRole = GTK_ACCESSIBLE_ROLE_COMMENT;
                break;
#endif
            default:
                SAL_WARN("vcl.gtk",
                         "unmapped GtkAccessibleRole: " << xContext->getAccessibleRole());
                break;
        }
    }

    return eRole;
}

static css::uno::Reference<css::accessibility::XAccessible> get_uno_accessible(GtkWidget* pWidget)
{
    GtkWidget* pTopLevel = widget_get_toplevel(pWidget);
    if (!pTopLevel)
        return nullptr;

    GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(pTopLevel);
    if (!pFrame)
        return nullptr;

    vcl::Window* pFrameWindow = pFrame->GetWindow();
    if (!pFrameWindow)
        return nullptr;

    vcl::Window* pWindow = pFrameWindow;

    // skip accessible objects already exposed by the frame objects
    if (WindowType::BORDERWINDOW == pWindow->GetType())
        pWindow = pFrameWindow->GetAccessibleChildWindow(0);

    if (!pWindow)
        return nullptr;

    return pWindow->GetAccessible();
}

/**
 * Based on the states set in xContext, set the corresponding Gtk states/properties
 * in pGtkAccessible.
 */
static void applyStates(GtkAccessible* pGtkAccessible,
                        css::uno::Reference<css::accessibility::XAccessibleContext>& xContext)
{
    assert(pGtkAccessible);

    if (!xContext.is())
        return;

    // Gtk differentiates between GtkAccessibleState and GtkAccessibleProperty
    // (both handled here) and GtkAccessiblePlatformState (handled in
    // 'lo_accessible_get_platform_state')
    const sal_Int64 nStates = xContext->getAccessibleStateSet();
    gtk_accessible_update_property(
        pGtkAccessible, GTK_ACCESSIBLE_PROPERTY_MODAL,
        bool(nStates & com::sun::star::accessibility::AccessibleStateType::MODAL),
        GTK_ACCESSIBLE_PROPERTY_MULTI_LINE,
        bool(nStates & com::sun::star::accessibility::AccessibleStateType::MULTI_LINE),
        GTK_ACCESSIBLE_PROPERTY_MULTI_SELECTABLE,
        bool(nStates & com::sun::star::accessibility::AccessibleStateType::MULTI_SELECTABLE),
        GTK_ACCESSIBLE_PROPERTY_READ_ONLY,
        bool(!(nStates & com::sun::star::accessibility::AccessibleStateType::EDITABLE)), -1);
    if (nStates & com::sun::star::accessibility::AccessibleStateType::HORIZONTAL)
    {
        gtk_accessible_update_property(pGtkAccessible, GTK_ACCESSIBLE_PROPERTY_ORIENTATION,
                                       GTK_ORIENTATION_HORIZONTAL, -1);
    }
    else if (nStates & com::sun::star::accessibility::AccessibleStateType::VERTICAL)
    {
        gtk_accessible_update_property(pGtkAccessible, GTK_ACCESSIBLE_PROPERTY_ORIENTATION,
                                       GTK_ORIENTATION_VERTICAL, -1);
    }

    gtk_accessible_update_state(
        pGtkAccessible, GTK_ACCESSIBLE_STATE_BUSY,
        bool(nStates & com::sun::star::accessibility::AccessibleStateType::BUSY),
        GTK_ACCESSIBLE_STATE_DISABLED,
        bool(!(nStates & com::sun::star::accessibility::AccessibleStateType::ENABLED)), -1);

    // when explicitly setting any value for GTK_ACCESSIBLE_STATE_CHECKED,
    // Gtk will also report ATSPI_STATE_CHECKABLE on the AT-SPI layer
    if (nStates & com::sun::star::accessibility::AccessibleStateType::CHECKABLE)
    {
        GtkAccessibleTristate eState = GTK_ACCESSIBLE_TRISTATE_FALSE;
        if (nStates & com::sun::star::accessibility::AccessibleStateType::INDETERMINATE)
            eState = GTK_ACCESSIBLE_TRISTATE_MIXED;
        else if (nStates & com::sun::star::accessibility::AccessibleStateType::CHECKED)
            eState = GTK_ACCESSIBLE_TRISTATE_TRUE;
        gtk_accessible_update_state(pGtkAccessible, GTK_ACCESSIBLE_STATE_CHECKED, eState, -1);
    }

    if (nStates & com::sun::star::accessibility::AccessibleStateType::EXPANDABLE)
    {
        gtk_accessible_update_state(
            pGtkAccessible, GTK_ACCESSIBLE_STATE_EXPANDED,
            bool(nStates & com::sun::star::accessibility::AccessibleStateType::EXPANDED), -1);
    }

    if (nStates & com::sun::star::accessibility::AccessibleStateType::SELECTABLE)
    {
        gtk_accessible_update_state(
            pGtkAccessible, GTK_ACCESSIBLE_STATE_SELECTED,
            bool(nStates & com::sun::star::accessibility::AccessibleStateType::SELECTED), -1);
    }

    const sal_Int16 nRole = xContext->getAccessibleRole();
    if (nRole == com::sun::star::accessibility::AccessibleRole::TOGGLE_BUTTON)
    {
        GtkAccessibleTristate eState = GTK_ACCESSIBLE_TRISTATE_FALSE;
        if (nStates & com::sun::star::accessibility::AccessibleStateType::INDETERMINATE)
            eState = GTK_ACCESSIBLE_TRISTATE_MIXED;
        else if (nStates & com::sun::star::accessibility::AccessibleStateType::PRESSED)
            eState = GTK_ACCESSIBLE_TRISTATE_TRUE;
        gtk_accessible_update_state(pGtkAccessible, GTK_ACCESSIBLE_STATE_PRESSED, eState, -1);
    }
}

static void applyObjectAttribute(GtkAccessible* pGtkAccessible, std::u16string_view rName,
                                 const OUString& rValue)
{
    assert(pGtkAccessible);

    if (rName == u"colindextext")
    {
        gtk_accessible_update_relation(pGtkAccessible, GTK_ACCESSIBLE_RELATION_COL_INDEX_TEXT,
                                       rValue.toUtf8().getStr(), -1);
    }
    else if (rName == u"level")
    {
        const int nLevel = o3tl::toInt32(rValue);
        gtk_accessible_update_property(pGtkAccessible, GTK_ACCESSIBLE_PROPERTY_LEVEL, nLevel, -1);
    }
    else if (rName == u"rowindextext")
    {
        gtk_accessible_update_relation(pGtkAccessible, GTK_ACCESSIBLE_RELATION_ROW_INDEX_TEXT,
                                       rValue.toUtf8().getStr(), -1);
    }
}

/**
 * Based on the object attributes set for xContext, set the corresponding Gtk equivalents
 * in pGtkAccessible, where applicable.
 */
static void
applyObjectAttributes(GtkAccessible* pGtkAccessible,
                      css::uno::Reference<css::accessibility::XAccessibleContext>& xContext)
{
    assert(pGtkAccessible);

    css::uno::Reference<css::accessibility::XAccessibleExtendedAttributes> xAttributes(
        xContext, css::uno::UNO_QUERY);
    if (!xAttributes.is())
        return;

    OUString sAttrs;
    xAttributes->getExtendedAttributes() >>= sAttrs;

    sal_Int32 nIndex = 0;
    do
    {
        const OUString sAttribute = sAttrs.getToken(0, ';', nIndex);
        sal_Int32 nColonPos = 0;
        const OUString sName = sAttribute.getToken(0, ':', nColonPos);
        const OUString sValue = sAttribute.getToken(0, ':', nColonPos);
        assert(nColonPos == -1
               && "Too many colons in attribute that should have \"name:value\" syntax");

        applyObjectAttribute(pGtkAccessible, sName, sValue);
    } while (nIndex >= 0);
}

static void applyRelations(LoAccessible* pLoAccessible,
                           css::uno::Reference<css::accessibility::XAccessibleContext>& xContext)
{
    assert(pLoAccessible);

    if (!xContext)
        return;

    css::uno::Reference<css::accessibility::XAccessibleRelationSet> xRelationSet
        = xContext->getAccessibleRelationSet();
    if (!xRelationSet.is())
        return;

    for (sal_Int32 i = 0; i < xRelationSet->getRelationCount(); i++)
    {
        GtkAccessibleRelation eGtkRelation;
        css::accessibility::AccessibleRelation aRelation = xRelationSet->getRelation(i);
        switch (aRelation.RelationType)
        {
            case css::accessibility::AccessibleRelationType::CONTENT_FLOWS_TO:
                eGtkRelation = GTK_ACCESSIBLE_RELATION_FLOW_TO;
                break;
            case css::accessibility::AccessibleRelationType::CONTROLLER_FOR:
                eGtkRelation = GTK_ACCESSIBLE_RELATION_CONTROLS;
                break;
            case css::accessibility::AccessibleRelationType::DESCRIBED_BY:
                eGtkRelation = GTK_ACCESSIBLE_RELATION_DESCRIBED_BY;
                break;
            case css::accessibility::AccessibleRelationType::LABELED_BY:
                eGtkRelation = GTK_ACCESSIBLE_RELATION_LABELLED_BY;
                break;
            case css::accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM:
            case css::accessibility::AccessibleRelationType::CONTROLLED_BY:
            case css::accessibility::AccessibleRelationType::INVALID:
            case css::accessibility::AccessibleRelationType::LABEL_FOR:
            case css::accessibility::AccessibleRelationType::MEMBER_OF:
            case css::accessibility::AccessibleRelationType::NODE_CHILD_OF:
            case css::accessibility::AccessibleRelationType::SUB_WINDOW_OF:
                // GTK has no equivalent for these
                continue;
            default:
                assert(false && "Unhandled relation type");
        }

        gtk_accessible_reset_relation(GTK_ACCESSIBLE(pLoAccessible),
                                      GTK_ACCESSIBLE_RELATION_FLOW_TO);

        GList* pTargetObjects = nullptr;
        for (const css::uno::Reference<css::accessibility::XAccessible>& xTargetAcc :
             aRelation.TargetSet)
        {
            LoAccessible* pTargetLOAcc
                = GtkAccessibleRegistry::getLOAccessible(xTargetAcc, pLoAccessible->display);
            assert(pTargetLOAcc);
            GObject* pObject = G_OBJECT(pTargetLOAcc);
            g_object_ref(pObject);
            pTargetObjects = g_list_append(pTargetObjects, pObject);
        }

        GValue aValue = G_VALUE_INIT;
        gtk_accessible_relation_init_value(eGtkRelation, &aValue);
        g_value_set_pointer(&aValue, pTargetObjects);
        gtk_accessible_update_relation_value(GTK_ACCESSIBLE(pLoAccessible), 1, &eGtkRelation,
                                             &aValue);
    }
}

struct LoAccessibleClass
{
    GObjectClass parent_class;
};

#if GTK_CHECK_VERSION(4, 10, 0)
static void lo_accessible_range_init(gpointer iface_, gpointer);
static gboolean lo_accessible_range_set_current_value(GtkAccessibleRange* self, double fNewValue);
#endif

extern "C" {
typedef GType (*GetGIfaceType)();
}
const struct
{
    const char* name;
    GInterfaceInitFunc const aInit;
    GetGIfaceType const aGetGIfaceType;
    const css::uno::Type& (*aGetUnoType)();
} TYPE_TABLE[] = {
#if GTK_CHECK_VERSION(4, 14, 0)
    { "Text", lo_accessible_text_init, gtk_accessible_text_get_type,
      cppu::UnoType<css::accessibility::XAccessibleText>::get },
#endif
#if GTK_CHECK_VERSION(4, 10, 0)
    { "Value", lo_accessible_range_init, gtk_accessible_range_get_type,
      cppu::UnoType<css::accessibility::XAccessibleValue>::get },
#endif
};

static bool isOfType(css::uno::XInterface* xInterface, const css::uno::Type& rType)
{
    if (!xInterface)
        return false;

    try
    {
        css::uno::Any aRet = xInterface->queryInterface(rType);
        const bool bIs = (typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass)
                         && (aRet.pReserved != nullptr);
        return bIs;
    }
    catch (const css::uno::Exception&)
    {
        return false;
    }
}

static GType ensureTypeFor(css::uno::XInterface* xAccessible)
{
    OStringBuffer aTypeNameBuf("OOoGtkAccessibleObj");
    std::vector<bool> bTypes(std::size(TYPE_TABLE), false);
    for (size_t i = 0; i < std::size(TYPE_TABLE); i++)
    {
        if (isOfType(xAccessible, TYPE_TABLE[i].aGetUnoType()))
        {
            aTypeNameBuf.append(TYPE_TABLE[i].name);
            bTypes[i] = true;
        }
    }

    const OString aTypeName = aTypeNameBuf.makeStringAndClear();
    GType nType = g_type_from_name(aTypeName.getStr());
    if (nType != G_TYPE_INVALID)
        return nType;

    GTypeInfo aTypeInfo = { sizeof(LoAccessibleClass), nullptr, nullptr, nullptr, nullptr, nullptr,
                            sizeof(LoAccessible),      0,       nullptr, nullptr };
    nType
        = g_type_register_static(LO_TYPE_ACCESSIBLE, aTypeName.getStr(), &aTypeInfo, GTypeFlags(0));

    for (size_t i = 0; i < std::size(TYPE_TABLE); i++)
    {
        if (bTypes[i])
        {
            GInterfaceInfo aIfaceInfo = { nullptr, nullptr, nullptr };
            aIfaceInfo.interface_init = TYPE_TABLE[i].aInit;
            g_type_add_interface_static(nType, TYPE_TABLE[i].aGetGIfaceType(), &aIfaceInfo);
        }
    }
    return nType;
}

enum
{
    CHILD_PROP_0,
    LAST_CHILD_PROP,

    PROP_ACCESSIBLE_ROLE
};

static void lo_accessible_get_property(GObject* object, guint property_id, GValue* value,
                                       GParamSpec* pspec)
{
    LoAccessible* accessible = LO_ACCESSIBLE(object);

    switch (property_id)
    {
        case PROP_ACCESSIBLE_ROLE:
        {
            GtkAccessibleRole eRole(map_accessible_role(accessible->uno_accessible));
            g_value_set_enum(value, eRole);
            break;
        }
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void lo_accessible_set_property(GObject* object, guint property_id, const GValue* /*value*/,
                                       GParamSpec* pspec)
{
    //    LoAccessible* accessible = LO_ACCESSIBLE(object);

    switch (property_id)
    {
        case PROP_ACCESSIBLE_ROLE:
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static GtkAccessible* lo_accessible_get_accessible_parent(GtkAccessible* accessible)
{
    LoAccessible* lo_accessible = LO_ACCESSIBLE(accessible);
    if (!lo_accessible->parent)
        return nullptr;
    return GTK_ACCESSIBLE(g_object_ref(lo_accessible->parent));
}

static GtkATContext* lo_accessible_get_at_context(GtkAccessible* self)
{
    LoAccessible* pAccessible = LO_ACCESSIBLE(self);

    if (!pAccessible->at_context)
    {
        GtkAccessibleRole eRole = map_accessible_role(pAccessible->uno_accessible);
        pAccessible->at_context = gtk_at_context_create(eRole, self, pAccessible->display);
        if (!pAccessible->at_context)
            return nullptr;
    }

    return g_object_ref(pAccessible->at_context);
}

static GtkAccessible* lo_accessible_get_first_accessible_child(GtkAccessible* self);

static GtkAccessible* lo_accessible_get_next_accessible_sibling(GtkAccessible* self);

static gboolean lo_accessible_get_platform_state(GtkAccessible* self,
                                                 GtkAccessiblePlatformState state);

static gboolean lo_accessible_get_bounds(GtkAccessible* accessible, int* x, int* y, int* width,
                                         int* height);

static void lo_accessible_accessible_init(GtkAccessibleInterface* iface)
{
    iface->get_accessible_parent = lo_accessible_get_accessible_parent;
    iface->get_at_context = lo_accessible_get_at_context;
    iface->get_bounds = lo_accessible_get_bounds;
    iface->get_first_accessible_child = lo_accessible_get_first_accessible_child;
    iface->get_next_accessible_sibling = lo_accessible_get_next_accessible_sibling;
    iface->get_platform_state = lo_accessible_get_platform_state;
}

#if GTK_CHECK_VERSION(4, 10, 0)
static void lo_accessible_range_init(gpointer iface_, gpointer)
{
    auto const iface = static_cast<GtkAccessibleRangeInterface*>(iface_);
    iface->set_current_value = lo_accessible_range_set_current_value;
}
#endif

// silence loplugin:unreffun
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
G_DEFINE_TYPE_WITH_CODE(LoAccessible, lo_accessible, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_ACCESSIBLE, lo_accessible_accessible_init))
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static void lo_accessible_class_init(LoAccessibleClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    //    object_class->finalize = lo_accessible_finalize;
    //    object_class->dispose = lo_accessible_dispose;
    object_class->get_property = lo_accessible_get_property;
    object_class->set_property = lo_accessible_set_property;
    //    object_class->constructed = lo_accessible_constructed;

    //    g_object_class_install_properties(object_class, LAST_CHILD_PROP, lo_accessible_props);
    g_object_class_override_property(object_class, PROP_ACCESSIBLE_ROLE, "accessible-role");
}

LoAccessible*
lo_accessible_new(GdkDisplay* pDisplay, GtkAccessible* pParent,
                  const css::uno::Reference<css::accessibility::XAccessible>& rAccessible)
{
    assert(rAccessible.is());

    GType nType = ensureTypeFor(rAccessible.get());
    LoAccessible* ret = LO_ACCESSIBLE(g_object_new(nType, nullptr));
    ret->display = pDisplay;
    ret->parent = pParent;
    ret->uno_accessible = rAccessible;

    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        ret->uno_accessible->getAccessibleContext());
    assert(xContext.is() && "No accessible context");

    GtkAccessible* pGtkAccessible = GTK_ACCESSIBLE(ret);

    // set accessible name and description
    gtk_accessible_update_property(pGtkAccessible, GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   xContext->getAccessibleName().toUtf8().getStr(),
                                   GTK_ACCESSIBLE_PROPERTY_DESCRIPTION,
                                   xContext->getAccessibleDescription().toUtf8().getStr(), -1);

    applyStates(pGtkAccessible, xContext);

    applyObjectAttributes(GTK_ACCESSIBLE(ret), xContext);

    applyRelations(ret, xContext);

    // set values from XAccessibleValue interface if that's implemented
    css::uno::Reference<css::accessibility::XAccessibleValue> xAccessibleValue(xContext,
                                                                               css::uno::UNO_QUERY);
    if (xAccessibleValue.is())
    {
        double fCurrentValue = 0, fMinValue = 0, fMaxValue = 0;
        xAccessibleValue->getCurrentValue() >>= fCurrentValue;
        xAccessibleValue->getMinimumValue() >>= fMinValue;
        xAccessibleValue->getMaximumValue() >>= fMaxValue;
        gtk_accessible_update_property(GTK_ACCESSIBLE(ret), GTK_ACCESSIBLE_PROPERTY_VALUE_NOW,
                                       fCurrentValue, GTK_ACCESSIBLE_PROPERTY_VALUE_MIN, fMinValue,
                                       GTK_ACCESSIBLE_PROPERTY_VALUE_MAX, fMaxValue, -1);
    }

    // register event listener
    css::uno::Reference<css::accessibility::XAccessibleEventBroadcaster> xBroadcaster(
        xContext, css::uno::UNO_QUERY);
    if (xBroadcaster.is())
    {
        css::uno::Reference<css::accessibility::XAccessibleEventListener> xListener(
            new GtkAccessibleEventListener(ret));
        xBroadcaster->addAccessibleEventListener(xListener);
    }

    return ret;
}

static gboolean lo_accessible_get_bounds(GtkAccessible* self, int* x, int* y, int* width,
                                         int* height)
{
    LoAccessible* pAccessible = LO_ACCESSIBLE(self);

    if (!pAccessible->uno_accessible)
        return false;

    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        pAccessible->uno_accessible->getAccessibleContext());
    css::uno::Reference<css::accessibility::XAccessibleComponent> xAccessibleComponent(
        xContext, css::uno::UNO_QUERY);
    if (!xAccessibleComponent)
        return false;

    css::awt::Rectangle aBounds = xAccessibleComponent->getBounds();
    *x = aBounds.X;
    *y = aBounds.Y;
    *width = aBounds.Width;
    *height = aBounds.Height;
    return true;
}

static GtkAccessible* lo_accessible_get_first_accessible_child(GtkAccessible* self)
{
    LoAccessible* pAccessible = LO_ACCESSIBLE(self);

    if (!pAccessible->uno_accessible)
        return nullptr;

    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        pAccessible->uno_accessible->getAccessibleContext());
    if (!xContext->getAccessibleChildCount())
        return nullptr;
    css::uno::Reference<css::accessibility::XAccessible> xFirstChild(
        xContext->getAccessibleChild(0));
    if (!xFirstChild)
        return nullptr;

    LoAccessible* child_accessible
        = GtkAccessibleRegistry::getLOAccessible(xFirstChild, pAccessible->display, self);
    return GTK_ACCESSIBLE(g_object_ref(child_accessible));
}

static GtkAccessible* lo_accessible_get_next_accessible_sibling(GtkAccessible* self)
{
    LoAccessible* pAccessible = LO_ACCESSIBLE(self);

    if (!pAccessible->uno_accessible)
        return nullptr;

    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        pAccessible->uno_accessible->getAccessibleContext());
    sal_Int64 nThisChildIndex = xContext->getAccessibleIndexInParent();
    assert(nThisChildIndex != -1);
    sal_Int64 nNextChildIndex = nThisChildIndex + 1;

    css::uno::Reference<css::accessibility::XAccessible> xParent = xContext->getAccessibleParent();
    css::uno::Reference<css::accessibility::XAccessibleContext> xParentContext(
        xParent->getAccessibleContext());
    if (nNextChildIndex >= xParentContext->getAccessibleChildCount())
        return nullptr;
    css::uno::Reference<css::accessibility::XAccessible> xNextChild(
        xParentContext->getAccessibleChild(nNextChildIndex));
    if (!xNextChild)
        return nullptr;

    LoAccessible* child_accessible = GtkAccessibleRegistry::getLOAccessible(
        xNextChild, pAccessible->display, pAccessible->parent);
    return GTK_ACCESSIBLE(g_object_ref(child_accessible));
}

static gboolean lo_accessible_get_platform_state(GtkAccessible* self,
                                                 GtkAccessiblePlatformState state)
{
    LoAccessible* pAccessible = LO_ACCESSIBLE(self);

    if (!pAccessible->uno_accessible)
        return false;

    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        pAccessible->uno_accessible->getAccessibleContext());
    sal_Int64 nStateSet = xContext->getAccessibleStateSet();

    switch (state)
    {
        case GTK_ACCESSIBLE_PLATFORM_STATE_FOCUSABLE:
            return (nStateSet & css::accessibility::AccessibleStateType::FOCUSABLE) != 0;
        case GTK_ACCESSIBLE_PLATFORM_STATE_FOCUSED:
            return (nStateSet & css::accessibility::AccessibleStateType::FOCUSED) != 0;
        case GTK_ACCESSIBLE_PLATFORM_STATE_ACTIVE:
            return (nStateSet & css::accessibility::AccessibleStateType::ACTIVE) != 0;
    }

    return false;
}

#if GTK_CHECK_VERSION(4, 10, 0)
static gboolean lo_accessible_range_set_current_value(GtkAccessibleRange* self, double fNewValue)
{
    // return 'true' in any case, since otherwise no proper AT-SPI DBus reply gets sent
    // and the app crashes, s.
    // https://gitlab.gnome.org/GNOME/gtk/-/issues/6150
    // https://gitlab.gnome.org/GNOME/gtk/-/commit/0dbd2bd09eff8c9233e45338a05daf2a835529ab

    LoAccessible* pAccessible = LO_ACCESSIBLE(self);
    if (!pAccessible->uno_accessible)
        return true;

    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        pAccessible->uno_accessible->getAccessibleContext());

    css::uno::Reference<css::accessibility::XAccessibleValue> xValue(xContext, css::uno::UNO_QUERY);
    if (!xValue.is())
        return true;

    // Different types of numerical values for XAccessibleValue are possible.
    // If current value has an integer type, also use that for the new value, to make
    // sure underlying implementations expecting that can handle the value properly.
    const css::uno::Any aCurrentValue = xValue->getCurrentValue();
    if (aCurrentValue.getValueTypeClass() == css::uno::TypeClass::TypeClass_LONG)
    {
        const sal_Int32 nValue = std::round<sal_Int32>(fNewValue);
        xValue->setCurrentValue(css::uno::Any(nValue));
        return true;
    }
    else if (aCurrentValue.getValueTypeClass() == css::uno::TypeClass::TypeClass_HYPER)
    {
        const sal_Int64 nValue = std::round<sal_Int64>(fNewValue);
        xValue->setCurrentValue(css::uno::Any(nValue));
        return true;
    }

    css::uno::Any aValue;
    aValue <<= fNewValue;
    xValue->setCurrentValue(aValue);
    return true;
}
#endif

static void lo_accessible_init(LoAccessible* /*iface*/) {}

#if 0
gboolean get_platform_state(GtkAccessible* self, GtkAccessiblePlatformState state)
{
    return false;
}
#endif

static gboolean get_bounds(GtkAccessible* accessible, int* x, int* y, int* width, int* height)
{
    OOoFixed* pFixed = OOO_FIXED(accessible);
    css::uno::Reference<css::accessibility::XAccessible> xAccessible(
        get_uno_accessible(GTK_WIDGET(pFixed)));
    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        xAccessible->getAccessibleContext());
    css::uno::Reference<css::accessibility::XAccessibleComponent> xAccessibleComponent(
        xContext, css::uno::UNO_QUERY);

    css::awt::Rectangle aBounds = xAccessibleComponent->getBounds();
    *x = aBounds.X;
    *y = aBounds.Y;
    *width = aBounds.Width;
    *height = aBounds.Height;
    return true;
}

static GtkAccessible* get_first_accessible_child(GtkAccessible* accessible)
{
    OOoFixed* pFixed = OOO_FIXED(accessible);
    css::uno::Reference<css::accessibility::XAccessible> xAccessible(
        get_uno_accessible(GTK_WIDGET(pFixed)));
    if (!xAccessible)
        return nullptr;
    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        xAccessible->getAccessibleContext());
    if (!xContext->getAccessibleChildCount())
        return nullptr;
    css::uno::Reference<css::accessibility::XAccessible> xFirstChild(
        xContext->getAccessibleChild(0));
    LoAccessible* child_accessible = GtkAccessibleRegistry::getLOAccessible(
        xFirstChild, gtk_widget_get_display(GTK_WIDGET(pFixed)), accessible);
    return GTK_ACCESSIBLE(g_object_ref(child_accessible));
}

static void ooo_fixed_accessible_init(GtkAccessibleInterface* iface)
{
    GtkAccessibleInterface* parent_iface
        = static_cast<GtkAccessibleInterface*>(g_type_interface_peek_parent(iface));
    iface->get_bounds = get_bounds;
    iface->get_first_accessible_child = get_first_accessible_child;
    iface->get_platform_state = parent_iface->get_platform_state;
    //    iface->get_platform_state = get_platform_state;
}

// silence loplugin:unreffun
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
G_DEFINE_TYPE_WITH_CODE(OOoFixed, ooo_fixed, GTK_TYPE_FIXED,
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_ACCESSIBLE, ooo_fixed_accessible_init))
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static void ooo_fixed_class_init(OOoFixedClass* /*klass*/) {}

static void ooo_fixed_init(OOoFixed* /*area*/) {}

GtkWidget* ooo_fixed_new() { return GTK_WIDGET(g_object_new(OOO_TYPE_FIXED, nullptr)); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
