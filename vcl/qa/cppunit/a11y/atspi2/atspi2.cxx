/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "atspi2.hxx"

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>

#include <comphelper/propertyvalue.hxx>
#include <o3tl/string_view.hxx>
#include <sfx2/zoomitem.hxx>
#include <unotest/macros_test.hxx>

#include <test/a11y/AccessibilityTools.hxx>

#include "atspiwrapper.hxx"

using namespace css;

// from gtk3/a11y/atkwrapper.cxx
static AtspiRole mapToAtspiRole(sal_Int16 nRole, sal_Int64 nStates)
{
    switch (nRole)
    {
#define MAP(lo, atspi)                                                                             \
    case accessibility::AccessibleRole::lo:                                                        \
        return ATSPI_ROLE_##atspi
#define MAP_DIRECT(a) MAP(a, a)

        MAP_DIRECT(UNKNOWN);
        MAP_DIRECT(ALERT);
        MAP_DIRECT(BLOCK_QUOTE);
        MAP_DIRECT(COLUMN_HEADER);
        MAP_DIRECT(CANVAS);
        MAP_DIRECT(CHECK_BOX);
        MAP_DIRECT(CHECK_MENU_ITEM);
        MAP_DIRECT(COLOR_CHOOSER);
        MAP_DIRECT(COMBO_BOX);
        MAP_DIRECT(DATE_EDITOR);
        MAP_DIRECT(DESKTOP_ICON);
        MAP(DESKTOP_PANE, DESKTOP_FRAME);
        MAP_DIRECT(DIRECTORY_PANE);
        MAP_DIRECT(DIALOG);
        MAP(DOCUMENT, DOCUMENT_FRAME);
        MAP(EMBEDDED_OBJECT, EMBEDDED);
        MAP(END_NOTE, FOOTNOTE);
        MAP_DIRECT(FILE_CHOOSER);
        MAP_DIRECT(FILLER);
        MAP_DIRECT(FONT_CHOOSER);
        MAP_DIRECT(FOOTER);
        MAP_DIRECT(FOOTNOTE);
        MAP_DIRECT(FRAME);
        MAP_DIRECT(GLASS_PANE);
        MAP(GRAPHIC, IMAGE);
        MAP(GROUP_BOX, GROUPING);
        MAP_DIRECT(HEADER);
        MAP_DIRECT(HEADING);
        MAP(HYPER_LINK, LINK);
        MAP_DIRECT(ICON);
        MAP_DIRECT(INTERNAL_FRAME);
        MAP_DIRECT(LABEL);
        MAP_DIRECT(LAYERED_PANE);
        MAP_DIRECT(LIST);
        MAP_DIRECT(LIST_ITEM);
        MAP_DIRECT(MENU);
        MAP_DIRECT(MENU_BAR);
        MAP_DIRECT(MENU_ITEM);
        MAP_DIRECT(OPTION_PANE);
        MAP_DIRECT(PAGE_TAB);
        MAP_DIRECT(PAGE_TAB_LIST);
        MAP_DIRECT(PANEL);
        MAP_DIRECT(PARAGRAPH);
        MAP_DIRECT(PASSWORD_TEXT);
        MAP_DIRECT(POPUP_MENU);
        MAP_DIRECT(PUSH_BUTTON);
        MAP_DIRECT(PROGRESS_BAR);
        MAP_DIRECT(RADIO_BUTTON);
        MAP_DIRECT(RADIO_MENU_ITEM);
        MAP_DIRECT(ROW_HEADER);
        MAP_DIRECT(ROOT_PANE);
        MAP_DIRECT(SCROLL_BAR);
        MAP_DIRECT(SCROLL_PANE);
        MAP(SHAPE, PANEL);
        MAP_DIRECT(SEPARATOR);
        MAP_DIRECT(SLIDER);
        MAP(SPIN_BOX, SPIN_BUTTON);
        MAP_DIRECT(SPLIT_PANE);
        MAP_DIRECT(STATUS_BAR);
        MAP_DIRECT(TABLE);
        MAP_DIRECT(TABLE_CELL);
        MAP_DIRECT(TEXT);
        MAP(TEXT_FRAME, PANEL);
        MAP_DIRECT(TOGGLE_BUTTON);
        MAP_DIRECT(TOOL_BAR);
        MAP_DIRECT(TOOL_TIP);
        MAP_DIRECT(TREE);
        MAP(VIEW_PORT, VIEWPORT);
        MAP_DIRECT(WINDOW);
#if ATSPI_ROLE_COUNT > 130 /* ATSPI_ROLE_PUSH_BUTTON_MENU is 129 */
        MAP(BUTTON_MENU, PUSH_BUTTON_MENU);
#else
        MAP(BUTTON_MENU, PUSH_BUTTON);
#endif
        MAP_DIRECT(CAPTION);
        MAP_DIRECT(CHART);
        MAP(EDIT_BAR, EDITBAR);
        MAP_DIRECT(FORM);
        MAP_DIRECT(IMAGE_MAP);
        MAP(NOTE, COMMENT);
        MAP_DIRECT(PAGE);
        MAP_DIRECT(RULER);
        MAP_DIRECT(SECTION);
        MAP_DIRECT(TREE_ITEM);
        MAP_DIRECT(TREE_TABLE);
        MAP_DIRECT(COMMENT);
        MAP(COMMENT_END, UNKNOWN);
        MAP_DIRECT(DOCUMENT_PRESENTATION);
        MAP_DIRECT(DOCUMENT_SPREADSHEET);
        MAP_DIRECT(DOCUMENT_TEXT);
        MAP_DIRECT(STATIC);
        MAP_DIRECT(NOTIFICATION);

#undef MAP_DIRECT
#undef MAP
        case css::accessibility::AccessibleRole::BUTTON_DROPDOWN:
            if (nStates & css::accessibility::AccessibleStateType::CHECKABLE)
                return ATSPI_ROLE_TOGGLE_BUTTON;
            return ATSPI_ROLE_PUSH_BUTTON;
        default:
            SAL_WARN("vcl.gtk", "Unmapped accessible role: " << nRole);
            return ATSPI_ROLE_UNKNOWN;
    }
}

static AtspiStateType mapAtspiState(sal_Int64 nState)
{
    // A perfect / complete mapping ...
    switch (nState)
    {
#define MAP(lo, atspi)                                                                             \
    case accessibility::AccessibleStateType::lo:                                                   \
        return ATSPI_STATE_##atspi
#define MAP_DIRECT(a) MAP(a, a)

        MAP_DIRECT(INVALID);
        MAP_DIRECT(ACTIVE);
        MAP_DIRECT(ARMED);
        MAP_DIRECT(BUSY);
        MAP_DIRECT(CHECKABLE);
        MAP_DIRECT(CHECKED);
        MAP_DIRECT(EDITABLE);
        MAP_DIRECT(ENABLED);
        MAP_DIRECT(EXPANDABLE);
        MAP_DIRECT(EXPANDED);
        MAP_DIRECT(FOCUSABLE);
        MAP_DIRECT(FOCUSED);
        MAP_DIRECT(HORIZONTAL);
        MAP_DIRECT(ICONIFIED);
        MAP_DIRECT(INDETERMINATE);
        MAP_DIRECT(MANAGES_DESCENDANTS);
        MAP_DIRECT(MODAL);
        MAP_DIRECT(MULTI_LINE);
        MAP(MULTI_SELECTABLE, MULTISELECTABLE);
        MAP_DIRECT(OPAQUE);
        MAP_DIRECT(PRESSED);
        MAP_DIRECT(RESIZABLE);
        MAP_DIRECT(SELECTABLE);
        MAP_DIRECT(SELECTED);
        MAP_DIRECT(SENSITIVE);
        MAP_DIRECT(SHOWING);
        MAP_DIRECT(SINGLE_LINE);
        MAP_DIRECT(STALE);
        MAP_DIRECT(TRANSIENT);
        MAP_DIRECT(VERTICAL);
        MAP_DIRECT(VISIBLE);
        MAP(DEFAULT, IS_DEFAULT);
        // a spelling error ...
        MAP(DEFUNC, DEFUNCT);

#undef MAP_DIRECT
#undef MAP

        default:
            //Mis-use ATK_STATE_LAST_DEFINED to check if a state is unmapped
            //NOTE! Do not report it
            return ATSPI_STATE_LAST_DEFINED;
    }
}

static AtspiRelationType mapRelationType(sal_Int16 nRelation)
{
    switch (nRelation)
    {
        case accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM:
            return ATSPI_RELATION_FLOWS_FROM;
        case accessibility::AccessibleRelationType::CONTENT_FLOWS_TO:
            return ATSPI_RELATION_FLOWS_TO;
        case accessibility::AccessibleRelationType::CONTROLLED_BY:
            return ATSPI_RELATION_CONTROLLED_BY;
        case accessibility::AccessibleRelationType::CONTROLLER_FOR:
            return ATSPI_RELATION_CONTROLLER_FOR;
        case accessibility::AccessibleRelationType::LABEL_FOR:
            return ATSPI_RELATION_LABEL_FOR;
        case accessibility::AccessibleRelationType::LABELED_BY:
            return ATSPI_RELATION_LABELLED_BY;
        case accessibility::AccessibleRelationType::MEMBER_OF:
            return ATSPI_RELATION_MEMBER_OF;
        case accessibility::AccessibleRelationType::SUB_WINDOW_OF:
            return ATSPI_RELATION_SUBWINDOW_OF;
        case accessibility::AccessibleRelationType::NODE_CHILD_OF:
            return ATSPI_RELATION_NODE_CHILD_OF;
    }

    return ATSPI_RELATION_NULL;
}

static std::string debugString(const Atspi::Accessible& pAtspiAccessible)
{
    CPPUNIT_NS::OStringStream ost;

    ost << "(" << static_cast<const void*>(pAtspiAccessible.get()) << ")";
    if (pAtspiAccessible)
    {
        ost << " role=\"" << pAtspiAccessible.getRoleName() << '"';
        ost << " name=\"" << pAtspiAccessible.getName() << '"';
        ost << " description=\"" << pAtspiAccessible.getDescription() << '"';
    }

    return ost.str();
}

static void dumpAtspiTree(const Atspi::Accessible& pAcc, const int depth = 0)
{
    std::cout << debugString(pAcc) << std::endl;

    sal_Int32 i = 0;
    for (const auto& pChild : pAcc)
    {
        for (auto j = decltype(depth){ 0 }; j < depth; j++)
            std::cout << "  ";
        std::cout << " * child " << i++ << ": ";
        dumpAtspiTree(pChild, depth + 1);
    }
}

void Atspi2TestTree::compareObjects(const uno::Reference<accessibility::XAccessible>& xLOAccessible,
                                    const Atspi::Accessible& pAtspiAccessible,
                                    const sal_uInt16 recurseFlags)
{
    if (recurseFlags != RecurseFlags::NONE)
        std::cout << "checking " << debugString(pAtspiAccessible) << " against "
                  << AccessibilityTools::debugString(xLOAccessible) << std::endl;

    CPPUNIT_ASSERT(xLOAccessible);
    CPPUNIT_ASSERT(pAtspiAccessible);

    auto xLOContext = xLOAccessible->getAccessibleContext();

    /* role: we translate to ATSPI role, because the value was created by LO already and converted
     * to ATK, which in turn converts it to ATSPI.  However, ATK and ATSPI are roughly equivalent
     * (ATK basically follows ATSPI), but LO's internal might have more complex mappings that can't
     * be represented with a round trip. */
    const AtspiRole nLORole
        = mapToAtspiRole(xLOContext->getAccessibleRole(), xLOContext->getAccessibleStateSet());
    const auto nAtspiRole = pAtspiAccessible.getRole();
    CPPUNIT_ASSERT_EQUAL(nLORole, nAtspiRole);
    /* name (no need to worry about debugging suffixes as AccessibilityTools::nameEquals does, as
     * that will also be part of the name sent to ATSPI) */
    CPPUNIT_ASSERT_EQUAL(xLOContext->getAccessibleName(),
                         OUString::fromUtf8(pAtspiAccessible.getName()));
    // description
    CPPUNIT_ASSERT_EQUAL(xLOContext->getAccessibleDescription(),
                         OUString::fromUtf8(pAtspiAccessible.getDescription()));

    // parent relationship (this is conditional as the ATSPI tree has additional parents, as well as
    // because we don't want to recurse up the tree)
    if (recurseFlags & RecurseFlags::PARENT)
    {
        // index in parent
        CPPUNIT_ASSERT_EQUAL(xLOContext->getAccessibleIndexInParent(),
                             sal_Int64(pAtspiAccessible.getIndexInParent()));

        // parent (well, that's making things a lot more expensive...)
        compareObjects(xLOContext->getAccessibleParent(), pAtspiAccessible.getParent(),
                       RecurseFlags::NONE);
    }

    // state set
    const auto loStateSet = xLOContext->getAccessibleStateSet();
    const auto atspiStateSet = pAtspiAccessible.getStateSet();
    const auto nBits
        = (sizeof(decltype(loStateSet)) * 8) - (std::is_signed_v<decltype(loStateSet)> ? 1 : 0);
    for (auto shift = decltype(nBits){ 0 }; shift < nBits; shift++)
    {
        const auto loState = decltype(loStateSet){ 1 } << shift;
        const auto atspiState = mapAtspiState(loState);

        // ignore a state that does not map to Atspi
        if (atspiState == ATSPI_STATE_LAST_DEFINED)
            continue;

        /* FIXME: The ATK implementation in LO adds FOCUSED if the obj == atk_get_focus_object()
         * (see atkwrapper.cxx::wrapper_ref_state_set()), but there seem to be some bug (or delay?
         * as it's done in idle) in the tracking, so we can end up with extra FOCUSED states on the
         * Atspi side.  To work around that, we skip the case where it's not set on LO's side */
        if (atspiState == ATSPI_STATE_FOCUSED && !(loStateSet & loState))
            continue;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unmatched state: " + Atspi::State::getName(atspiState),
                                     (loStateSet & loState) != 0,
                                     atspiStateSet.contains(atspiState));
    }

    // attributes
    if (auto xLOAttrs
        = uno::Reference<accessibility::XAccessibleExtendedAttributes>(xLOContext, uno::UNO_QUERY))
    {
        // see atktextattributes.cxx:attribute_set_new_from_extended_attributes
        const uno::Any anyVal = xLOAttrs->getExtendedAttributes();
        OUString sExtendedAttrs;
        anyVal >>= sExtendedAttrs;
        sal_Int32 nIndex = 0;

        const auto atspiAttrs = pAtspiAccessible.getAttributes();

        do
        {
            OUString sProperty = sExtendedAttrs.getToken(0, ';', nIndex);

            sal_Int32 nColonPos = 0;
            const OString sPropertyName = OUStringToOString(
                o3tl::getToken(sProperty, 0, ':', nColonPos), RTL_TEXTENCODING_UTF8);
            const OString sPropertyValue = OUStringToOString(
                o3tl::getToken(sProperty, 0, ':', nColonPos), RTL_TEXTENCODING_UTF8);

            const auto atspiAttrIter = atspiAttrs.find(std::string(sPropertyName));
            CPPUNIT_ASSERT_MESSAGE(std::string("Missing attribute: ") + sPropertyName.getStr(),
                                   atspiAttrIter != atspiAttrs.end());
            CPPUNIT_ASSERT_EQUAL(std::string_view(sPropertyName),
                                 std::string_view(atspiAttrIter->first));
            CPPUNIT_ASSERT_EQUAL(std::string_view(sPropertyValue),
                                 std::string_view(atspiAttrIter->second));
        } while (nIndex >= 0 && nIndex < sExtendedAttrs.getLength());
    }

    // relations
    const auto xLORelationSet = xLOContext->getAccessibleRelationSet();
    const auto aAtspiRelationSet = pAtspiAccessible.getRelationSet();
    const auto nLORelationCount = xLORelationSet.is() ? xLORelationSet->getRelationCount() : 0;
    CPPUNIT_ASSERT_EQUAL(nLORelationCount, sal_Int32(aAtspiRelationSet.size()));
    for (auto i = decltype(nLORelationCount){ 0 }; i < nLORelationCount; i++)
    {
        const auto xLORelation = xLORelationSet->getRelation(i);
        const auto pAtspiRelation = aAtspiRelationSet[i];
        const auto nLOTargetsCount = xLORelation.TargetSet.getLength();

        CPPUNIT_ASSERT_EQUAL(mapRelationType(xLORelation.RelationType),
                             pAtspiRelation.getRelationType());
        CPPUNIT_ASSERT_EQUAL(nLOTargetsCount, static_cast<sal_Int32>(pAtspiRelation.getNTargets()));

        if (recurseFlags & RecurseFlags::RELATIONS_TARGETS)
        {
            for (auto j = decltype(nLOTargetsCount){ 0 }; j < nLOTargetsCount; j++)
            {
                uno::Reference<accessibility::XAccessible> xLOTarget = xLORelation.TargetSet[j];
                compareObjects(xLOTarget, pAtspiRelation.getTarget(j), RecurseFlags::NONE);
            }
        }
    }

    // other interfaces
    if (auto xLOText = uno::Reference<accessibility::XAccessibleText>(xLOContext, uno::UNO_QUERY))
    {
        Atspi::Text pAtspiText;
        CPPUNIT_ASSERT_NO_THROW(pAtspiText = pAtspiAccessible.queryText());
        compareTextObjects(xLOText, pAtspiText);
    }

    // TODO: more checks here...
}

void Atspi2TestTree::compareTrees(const uno::Reference<accessibility::XAccessible>& xLOAccessible,
                                  const Atspi::Accessible& xAtspiAccessible, const int depth)
{
    sal_uInt16 recurseFlags = RecurseFlags::ALL;
    if (depth == 0)
        recurseFlags ^= RecurseFlags::PARENT;
    compareObjects(xLOAccessible, xAtspiAccessible, recurseFlags);

    if (!xLOAccessible || !xAtspiAccessible)
        return;

    auto xLOContext = xLOAccessible->getAccessibleContext();
    CPPUNIT_ASSERT(xLOContext);

    const auto nLOChildCount = xLOContext->getAccessibleChildCount();
    const auto nAtspiChildCount = decltype(nLOChildCount){ xAtspiAccessible.getChildCount() };
    /* We use >= instead of == because GTK exposes scrollbar objects LO doesn't.  We possibly
     * should check better than merely accept more children, but it's probably OK if there are
     * *more* children as viewed by ATSPI, rather than less.  And we're comparing them anyway. */
    CPPUNIT_ASSERT_GREATEREQUAL(nLOChildCount, nAtspiChildCount);

    for (auto nthChild = decltype(nLOChildCount){ 0 }; nthChild < nLOChildCount; nthChild++)
    {
        for (auto i = decltype(depth){ 0 }; i < depth; i++)
            std::cout << "  ";
        std::cout << "* child " << nthChild << ": ";
        compareTrees(xLOContext->getAccessibleChild(nthChild),
                     xAtspiAccessible.getChildAtIndex(nthChild), depth + 1);
    }

    /* We need to scrolling test here, because they might modify the tree and invalidate children,
     * so we can't do it from the children themselves as they might get disposed during the test */
    if (nLOChildCount > 0
        && accessibility::AccessibleRole::DOCUMENT_TEXT == xLOContext->getAccessibleRole())
    {
        testSwScroll(xLOContext, xAtspiAccessible);
    }
}

// gets the nth child of @p pAcc and check its role is @p role
static Atspi::Accessible getDescendentAtPath(const Atspi::Accessible& xAcc, int nthChild,
                                             AtspiRole role)
{
    CPPUNIT_ASSERT(xAcc);
    CPPUNIT_ASSERT_GREATER(nthChild, xAcc.getChildCount());
    auto xChild = xAcc.getChildAtIndex(nthChild);
    CPPUNIT_ASSERT(xChild);
    CPPUNIT_ASSERT_EQUAL(role, xChild.getRole());
    return xChild;
}

// gets the nth child of @p pAcc and check its role is @p role, then gets the nth child of that one, etc.
template <typename... Ts>
static Atspi::Accessible getDescendentAtPath(const Atspi::Accessible& xAcc, int nthChild,
                                             AtspiRole role, Ts... args)
{
    return getDescendentAtPath(getDescendentAtPath(xAcc, nthChild, role), args...);
}

CPPUNIT_TEST_FIXTURE(Atspi2TestTree, Test1)
{
    loadFromSrc(u"vcl/qa/cppunit/a11y/atspi2/testdocuments/ecclectic.fodt"_ustr);

    /* FIXME: We zoom out for everything to fit in the view not to have off-screen children
     * that the controller code fails to clean up properly in some situations.
     * Once the root issue is fixed in LO, remove this.
     * Note that zooming out like so, and not having off-screen children, renders the
     * Atspi2TestTree::testSwScroll() test useless as it has nothing to scroll into view. */
    unotest::MacrosTest::dispatchCommand(mxDocument, ".uno:ZoomPage", {});
    unotest::MacrosTest::dispatchCommand(
        mxDocument, ".uno:ViewLayout",
        {
            comphelper::makePropertyValue("ViewLayout.Columns", sal_Int16(2)),
            comphelper::makePropertyValue("ViewLayout.BookMode", false),
        });
    /* HACK: verify the whole content of the document is actually visible (nothing overflows)
     * after zooming out above */
    const auto xLODocContext = getDocumentAccessibleContext();
    const auto xLODocFirstChild = xLODocContext->getAccessibleChild(0);
    CPPUNIT_ASSERT(xLODocFirstChild.is());
    CPPUNIT_ASSERT(
        !getFirstRelationTargetOfType(xLODocFirstChild->getAccessibleContext(),
                                      accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM));
    const auto nLODocChildCount = xLODocContext->getAccessibleChildCount();
    const auto xLODocLastChild = xLODocContext->getAccessibleChild(nLODocChildCount - 1);
    CPPUNIT_ASSERT(xLODocLastChild.is());
    CPPUNIT_ASSERT(
        !getFirstRelationTargetOfType(xLODocLastChild->getAccessibleContext(),
                                      accessibility::AccessibleRelationType::CONTENT_FLOWS_TO));
    // END HACK

    auto xContext = getWindowAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    //~ dumpA11YTree(xContext);

    // get the window manager frame
    auto xAtspiWindow = getDescendentAtPath(m_pAtspiApp, 0, ATSPI_ROLE_FRAME);
    CPPUNIT_ASSERT(xAtspiWindow);
    dumpAtspiTree(xAtspiWindow);

    /* The ATSPI representation has extra nodes around the relevant ones, which look like leftovers
     * from the start center.  Ignore those and dive directly to the meaningful node (which is the
     * 1st child of the 2nd child of the 1st child -- ask me how I know) */
    auto xAtspiPane = getDescendentAtPath(xAtspiWindow, 0, ATSPI_ROLE_PANEL, 1, ATSPI_ROLE_PANEL, 0,
                                          ATSPI_ROLE_ROOT_PANE);

    compareTrees(uno::Reference<accessibility::XAccessible>(mxWindow, uno::UNO_QUERY_THROW),
                 xAtspiPane);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
