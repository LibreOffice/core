/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceWidget.hxx>
#include <QtInstanceWidget.moc>

#include <QtInstanceContainer.hxx>

#include <vcl/transfer.hxx>
#include <vcl/qt/QtUtils.hxx>

/** Name of QObject property used for the help ID. */
const char* const PROPERTY_HELP_ID = "help-id";

QtInstanceWidget::QtInstanceWidget(QWidget* pWidget)
    : m_pWidget(pWidget)
{
    assert(pWidget);

    connect(qApp, &QApplication::focusChanged, this, &QtInstanceWidget::applicationFocusChanged);
}

void QtInstanceWidget::set_sensitive(bool bSensitive)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_sensitive(bSensitive); });
        return;
    }

    assert(m_pWidget);
    m_pWidget->setEnabled(bSensitive);
}

bool QtInstanceWidget::get_sensitive() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        bool bSensitive = false;
        rQtInstance.RunInMainThread([&] { bSensitive = get_sensitive(); });
        return bSensitive;
    }

    assert(m_pWidget);
    return m_pWidget->isEnabled();
}

bool QtInstanceWidget::get_visible() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        bool bVisible = false;
        rQtInstance.RunInMainThread([&] { bVisible = get_visible(); });
        return bVisible;
    }

    assert(m_pWidget);
    return m_pWidget->isVisible();
}

bool QtInstanceWidget::is_visible() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        bool bVisible = false;
        rQtInstance.RunInMainThread([&] { bVisible = is_visible(); });
        return bVisible;
    }

    assert(m_pWidget);

    QWidget* pTopLevel = m_pWidget->topLevelWidget();
    assert(pTopLevel);
    return m_pWidget->isVisibleTo(pTopLevel) && pTopLevel->isVisible();
}

void QtInstanceWidget::set_can_focus(bool bCanFocus)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_can_focus(bCanFocus); });
        return;
    }

    assert(m_pWidget);
    if (bCanFocus)
        m_pWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    else
        m_pWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

void QtInstanceWidget::grab_focus()
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { grab_focus(); });
        return;
    }

    assert(m_pWidget);
    m_pWidget->setFocus();
}

bool QtInstanceWidget::has_focus() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        bool bFocus = false;
        rQtInstance.RunInMainThread([&] { bFocus = has_focus(); });
        return bFocus;
    }

    assert(m_pWidget);
    return m_pWidget->hasFocus();
}

bool QtInstanceWidget::is_active() const { return has_focus(); }

bool QtInstanceWidget::has_child_focus() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        bool bChildFocus = false;
        rQtInstance.RunInMainThread([&] { bChildFocus = has_child_focus(); });
        return bChildFocus;
    }

    QWidget* pFocusWidget = QApplication::focusWidget();
    if (!pFocusWidget)
        return false;

    QWidget* pParent = pFocusWidget->parentWidget();
    while (pParent)
    {
        if (pParent == m_pWidget)
            return true;
        pParent = pParent->parentWidget();
    }
    return false;
}

void QtInstanceWidget::show()
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { show(); });
        return;
    }

    assert(m_pWidget);
    m_pWidget->show();
}

void QtInstanceWidget::hide()
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { hide(); });
        return;
    }

    assert(m_pWidget);
    m_pWidget->hide();
}

void QtInstanceWidget::set_size_request(int nWidth, int nHeight)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pWidget->setMinimumSize(std::max(0, nWidth), std::max(0, nHeight)); });
}

Size QtInstanceWidget::get_size_request() const
{
    SolarMutexGuard g;

    Size aSize;
    GetQtInstance().RunInMainThread([&] { aSize = toSize(m_pWidget->minimumSize()); });
    return aSize;
}

Size QtInstanceWidget::get_preferred_size() const
{
    SolarMutexGuard g;

    Size aPreferredSize;
    GetQtInstance().RunInMainThread([&] { aPreferredSize = toSize(m_pWidget->sizeHint()); });

    return aPreferredSize;
}

float QtInstanceWidget::get_approximate_digit_width() const
{
    SolarMutexGuard g;

    float fWidth = 0;
    GetQtInstance().RunInMainThread(
        [&] { fWidth = m_pWidget->fontMetrics().horizontalAdvance("0123456789") / 10.0; });
    return fWidth;
}

int QtInstanceWidget::get_text_height() const
{
    SolarMutexGuard g;

    int nHeight = 0;
    GetQtInstance().RunInMainThread([&] { nHeight = m_pWidget->fontMetrics().height(); });
    return nHeight;
}

Size QtInstanceWidget::get_pixel_size(const OUString& rText) const
{
    SolarMutexGuard g;

    Size aSize;
    GetQtInstance().RunInMainThread(
        [&] { aSize = toSize(m_pWidget->fontMetrics().boundingRect(toQString(rText)).size()); });

    return aSize;
}

vcl::Font QtInstanceWidget::get_font()
{
    assert(false && "Not implemented yet");
    return vcl::Font();
}

OUString QtInstanceWidget::get_buildable_name() const { return OUString(); }

void QtInstanceWidget::set_buildable_name(const OUString&) {}

void QtInstanceWidget::setHelpId(QWidget& rWidget, const OUString& rHelpId)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { rWidget.setProperty(PROPERTY_HELP_ID, toQString(rHelpId)); });
}

void QtInstanceWidget::set_help_id(const OUString& rHelpId) { setHelpId(*m_pWidget, rHelpId); }

OUString QtInstanceWidget::get_help_id() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        OUString sHelpId;
        rQtInstance.RunInMainThread([&] { sHelpId = get_help_id(); });
        return sHelpId;
    }

    const QVariant aHelpIdVariant = m_pWidget->property(PROPERTY_HELP_ID);
    if (!aHelpIdVariant.isValid())
        return OUString();

    assert(aHelpIdVariant.canConvert<QString>());
    return toOUString(aHelpIdVariant.toString());
}

void QtInstanceWidget::set_hexpand(bool) { assert(false && "Not implemented yet"); }

bool QtInstanceWidget::get_hexpand() const
{
    assert(false && "Not implemented yet");
    return true;
}

void QtInstanceWidget::set_vexpand(bool) { assert(false && "Not implemented yet"); }

bool QtInstanceWidget::get_vexpand() const
{
    assert(false && "Not implemented yet");
    return true;
}

void QtInstanceWidget::set_margin_top(int) { assert(false && "Not implemented yet"); }

void QtInstanceWidget::set_margin_bottom(int) { assert(false && "Not implemented yet"); }

void QtInstanceWidget::set_margin_start(int) { assert(false && "Not implemented yet"); }

void QtInstanceWidget::set_margin_end(int) { assert(false && "Not implemented yet"); }

int QtInstanceWidget::get_margin_top() const
{
    assert(false && "Not implemented yet");
    return 0;
}

int QtInstanceWidget::get_margin_bottom() const
{
    assert(false && "Not implemented yet");
    return 0;
}

int QtInstanceWidget::get_margin_start() const
{
    assert(false && "Not implemented yet");
    return 0;
}

int QtInstanceWidget::get_margin_end() const
{
    assert(false && "Not implemented yet");
    return 0;
}

void QtInstanceWidget::set_accessible_name(const OUString& rName)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_accessible_name(rName); });
        return;
    }

    assert(m_pWidget);
    m_pWidget->setAccessibleName(toQString(rName));
}

void QtInstanceWidget::set_accessible_description(const OUString& rDescription)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_accessible_description(rDescription); });
        return;
    }

    assert(m_pWidget);
    m_pWidget->setAccessibleDescription(toQString(rDescription));
}

OUString QtInstanceWidget::get_accessible_name() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        OUString sName;
        rQtInstance.RunInMainThread([&] { sName = get_accessible_name(); });
        return sName;
    }

    assert(m_pWidget);
    return toOUString(m_pWidget->accessibleName());
}

OUString QtInstanceWidget::get_accessible_description() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        OUString sDescription;
        rQtInstance.RunInMainThread([&] { sDescription = get_accessible_description(); });
        return sDescription;
    }

    assert(m_pWidget);
    return toOUString(m_pWidget->accessibleDescription());
}

OUString QtInstanceWidget::get_accessible_id() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        OUString sId;
        rQtInstance.RunInMainThread([&] { sId = get_accessible_id(); });
        return sId;
    }

    assert(m_pWidget);
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    return toOUString(m_pWidget->accessibleIdentifier());
#else
    return OUString();
#endif
}

void QtInstanceWidget::set_accessible_relation_labeled_by(weld::Widget*)
{
    assert(false && "Not implemented yet");
}

void QtInstanceWidget::set_tooltip_text(const OUString& rTip)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_tooltip_text(rTip); });
        return;
    }

    assert(m_pWidget);
    m_pWidget->setToolTip(toQString(rTip));
}

OUString QtInstanceWidget::get_tooltip_text() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        OUString sToolTipText;
        rQtInstance.RunInMainThread([&] { sToolTipText = get_tooltip_text(); });
        return sToolTipText;
    }

    assert(m_pWidget);
    return toOUString(m_pWidget->toolTip());
}

void QtInstanceWidget::set_cursor_data(void*) { assert(false && "Not implemented yet"); }

void QtInstanceWidget::grab_add() { assert(false && "Not implemented yet"); }

bool QtInstanceWidget::has_grab() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceWidget::grab_remove() { assert(false && "Not implemented yet"); }

bool QtInstanceWidget::get_extents_relative_to(const Widget& rRelative, int& rX, int& rY,
                                               int& rWidth, int& rHeight) const
{
    SolarMutexGuard g;

    bool bRet = false;
    GetQtInstance().RunInMainThread([&] {
        QRect aGeometry = m_pWidget->geometry();
        rWidth = aGeometry.width();
        rHeight = aGeometry.height();
        const QtInstanceWidget* pRelativeWidget = dynamic_cast<const QtInstanceWidget*>(&rRelative);
        if (!pRelativeWidget)
            return;

        QPoint aRelativePos = m_pWidget->mapTo(pRelativeWidget->getQWidget(), QPoint(0, 0));
        rX = aRelativePos.x();
        rY = aRelativePos.y();
        bRet = true;
    });

    return bRet;
}

bool QtInstanceWidget::get_direction() const
{
    SolarMutexGuard g;

    bool bRTL = false;
    GetQtInstance().RunInMainThread(
        [&] { bRTL = m_pWidget->layoutDirection() == Qt::LayoutDirection::RightToLeft; });
    return bRTL;
}

void QtInstanceWidget::set_direction(bool bRTL)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        m_pWidget->setLayoutDirection(bRTL ? Qt::LayoutDirection::RightToLeft
                                           : Qt::LayoutDirection::LeftToRight);
    });
}

void QtInstanceWidget::freeze(){};

void QtInstanceWidget::thaw(){};

void QtInstanceWidget::set_busy_cursor(bool bBusy)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (bBusy)
            ++m_nBusyCount;
        else
            --m_nBusyCount;
        assert(m_nBusyCount >= 0);

        if (m_nBusyCount == 1)
            m_pWidget->setCursor(Qt::BusyCursor);
        else if (m_nBusyCount == 0)
            m_pWidget->unsetCursor();
    });
}

std::unique_ptr<weld::Container> QtInstanceWidget::weld_parent() const
{
    QWidget* pParentWidget = m_pWidget->parentWidget();
    if (!pParentWidget)
        return nullptr;

    return std::make_unique<QtInstanceContainer>(pParentWidget);
}

void QtInstanceWidget::queue_resize()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pWidget->adjustSize(); });
}

void QtInstanceWidget::help_hierarchy_foreach(const std::function<bool(const OUString&)>&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceWidget::strip_mnemonic(const OUString& rLabel) const
{
    return rLabel.replaceFirst("&", "");
}

OUString QtInstanceWidget::escape_ui_str(const OUString& rLabel) const
{
    // preserve literal '&'
    return rLabel.replaceAll("&", "&&");
}

VclPtr<VirtualDevice> QtInstanceWidget::create_virtual_device() const
{
    VclPtr<VirtualDevice> xRet = VclPtr<VirtualDevice>::Create();
    xRet->SetBackground(COL_TRANSPARENT);
    return xRet;
}

css::uno::Reference<css::datatransfer::dnd::XDropTarget> QtInstanceWidget::get_drop_target()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

css::uno::Reference<css::datatransfer::clipboard::XClipboard>
QtInstanceWidget::get_clipboard() const
{
    return GetSystemClipboard();
}

void QtInstanceWidget::connect_get_property_tree(const Link<tools::JsonWriter&, void>&)
{
    // not implemented for the Qt variant
}

void QtInstanceWidget::get_property_tree(tools::JsonWriter&)
{
    // not implemented for the Qt variant
}

void QtInstanceWidget::call_attention_to() { assert(false && "Not implemented yet"); }

void QtInstanceWidget::set_stack_background() { assert(false && "Not implemented yet"); }

void QtInstanceWidget::set_title_background() { assert(false && "Not implemented yet"); }

void QtInstanceWidget::set_toolbar_background() { assert(false && "Not implemented yet"); }

void QtInstanceWidget::set_highlight_background() { assert(false && "Not implemented yet"); }

void QtInstanceWidget::set_background(const Color& rBackColor)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QPalette aPalette = getQWidget()->palette();
        aPalette.setColor(getQWidget()->backgroundRole(), toQColor(rBackColor));
        getQWidget()->setPalette(aPalette);
        getQWidget()->setAutoFillBackground(true);
    });
}

void QtInstanceWidget::draw(OutputDevice&, const Point&, const Size&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceWidget::applicationFocusChanged(QWidget* pOldFocus, QWidget* pNewFocus)
{
    SolarMutexGuard g;

    if (pOldFocus == m_pWidget)
        signal_focus_out();
    else if (pNewFocus == m_pWidget)
        signal_focus_in();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
