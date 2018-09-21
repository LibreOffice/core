/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Qt5AccessibleFrame.hxx>
#include <Qt5AccessibleFrame.moc>

#include <QtGui/QAccessibleInterface>

#include <Qt5Frame.hxx>
#include <Qt5Widget.hxx>
#include <Qt5Tools.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <sal/log.hxx>

using namespace css::accessibility;

Qt5AccessibleFrame::Qt5AccessibleFrame(Qt5Widget* pFrame, vcl::Window* pWindow)
    : m_pFrame(pFrame)
    , m_pWindow(pWindow)
{
    qDebug("soo we got a Qt5AccessibleFrame!!");
}

QWindow* Qt5AccessibleFrame::window() const
{
    qDebug("Qt5AccessibleFrame::window");
    return nullptr;
}
int Qt5AccessibleFrame::childCount() const
{
    qDebug() << "Qt5AccessibleFrame::childCount: " << m_pWindow->GetAccessibleChildWindowCount();
    return m_pWindow->GetAccessibleChildWindowCount();
}
int Qt5AccessibleFrame::indexOfChild(const QAccessibleInterface* /* child */) const
{
    qDebug("Qt5AccessibleFrame::indexOfChild");
    return 0;
}
QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>
    Qt5AccessibleFrame::relations(QAccessible::Relation /* match */) const
{
    qDebug("Qt5AccessibleFrame::relations");
    return QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>();
}
QAccessibleInterface* Qt5AccessibleFrame::focusChild() const
{
    qDebug("Qt5AccessibleFrame::focusChild");
    return QAccessible::queryAccessibleInterface(object());
}

QRect Qt5AccessibleFrame::rect() const
{
    qDebug("Qt5AccessibleFrame::rect");

    Point aPoint(m_pWindow->GetPosPixel());
    Size aSize(m_pWindow->GetSizePixel());

    QRect aRect(aPoint.X(), aPoint.Y(), aSize.Width(), aSize.Height());
    qDebug() << "Qt5AccessibleFrame::rect" << aRect;
    return aRect;
}

QAccessibleInterface* Qt5AccessibleFrame::parent() const
{
    qDebug("Qt5AccessibleFrame::parent");
    return QAccessible::queryAccessibleInterface(nullptr);
}
QAccessibleInterface* Qt5AccessibleFrame::child(int index) const
{
    qDebug() << "Qt5AccessibleFrame::child index: " << index;
    //return QAccessible::queryAccessibleInterface(m_pFrame->m_pFrame->GetWindow()->GetAccessibleChildWindow(index));
    return nullptr;
}

QString Qt5AccessibleFrame::text(QAccessible::Text text) const
{
    qDebug("Qt5AccessibleFrame::text");
    switch (text)
    {
        case QAccessible::Name:
            return toQString(m_pWindow->GetAccessibleName());
        case QAccessible::Description:
            return toQString(m_pWindow->GetAccessibleDescription());
        case QAccessible::Value:
        case QAccessible::Help:
        case QAccessible::Accelerator:
        case QAccessible::UserText:
            return QString("idk!");
    }
}
QAccessible::Role Qt5AccessibleFrame::role() const
{
    qDebug("Qt5AccessibleFrame::role");
    switch (m_pWindow->GetAccessibleRole())
    {
        case AccessibleRole::ALERT:
            return QAccessible::AlertMessage;

        case AccessibleRole::DIALOG:
            return QAccessible::Dialog;

        case AccessibleRole::FRAME: // top-level window with title bar
            return QAccessible::Window;

        /* Ignore window objects for sub-menus, combo- and list boxes,
         *  which are exposed as children of their parents.
         */
        case AccessibleRole::WINDOW: // top-level window without title bar
        {
            WindowType type = WindowType::WINDOW;
            bool parentIsMenuFloatingWindow = false;

            vcl::Window* pParent = m_pFrame->m_pFrame->GetWindow()->GetParent();
            if (pParent)
            {
                type = pParent->GetType();
                parentIsMenuFloatingWindow = pParent->IsMenuFloatingWindow();
            }

            if ((WindowType::LISTBOX != type) && (WindowType::COMBOBOX != type)
                && (WindowType::MENUBARWINDOW != type) && !parentIsMenuFloatingWindow)
            {
                return QAccessible::Window;
            }
        }

            /* default:
        {
            vcl::Window *pChild = m_pFrame->m_pFrame->GetWindow()->GetWindow(GetWindowType::FirstChild);
            if( pChild )
            {
                if( WindowType::HELPTEXTWINDOW == pChild->GetType() )
                {
                    role = ATK_ROLE_TOOL_TIP;
                    pChild->SetAccessibleRole( AccessibleRole::LABEL );
                    accessible->name = g_strdup( OUStringToOString( pChild->GetText(), RTL_TEXTENCODING_UTF8 ).getStr() );
                }
                else if ( m_pFrame->m_pFrame->GetWindow()->GetType() == WindowType::BORDERWINDOW && pChild->GetType() == WindowType::FLOATINGWINDOW )
                {
                    PopupMenuFloatingWindow* p = dynamic_cast<PopupMenuFloatingWindow*>(pChild);
                    if (p && p->IsPopupMenu() && p->GetMenuStackLevel() == 0)
                    {
                        // This is a top-level menu popup.  Register it.
                        role = ATK_ROLE_POPUP_MENU;
                        pChild->SetAccessibleRole( AccessibleRole::POPUP_MENU );
                        accessible->name = g_strdup( OUStringToOString( pChild->GetText(), RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
            }
            break;
        }*/
    }
    return QAccessible::NoRole;
}
QAccessible::State Qt5AccessibleFrame::state() const
{
    qDebug("Qt5AccessibleFrame::state");
    QAccessible::State state;
    return state;
}

QColor Qt5AccessibleFrame::foregroundColor() const
{
    qDebug("Qt5AccessibleFrame::foregroundColor");
    return QColor();
}
QColor Qt5AccessibleFrame::backgroundColor() const
{
    qDebug("Qt5AccessibleFrame::backgroundColor");
    return QColor();
}

void* Qt5AccessibleFrame::interface_cast(QAccessible::InterfaceType t)
{
    qDebug("Qt5AccessibleFrame::interface_cast");
    /* if (t == QAccessible::ActionInterface)
        return static_cast<QAccessibleActionInterface*>(this); */
    return nullptr;
}

// QAccessibleActionInterface
/* QStringList Qt5AccessibleFrame::actionNames() const
{
    qDebug("Qt5AccessibleFrame::actionNames");
    QStringList actionNames;
    return actionNames;
}
void Qt5AccessibleFrame::doAction(const QString& actionName)
{
    qDebug("Qt5AccessibleFrame::doAction");
}
QStringList Qt5AccessibleFrame::keyBindingsForAction(const QString& actionName) const
{
    qDebug("Qt5AccessibleFrame::keyBindingsForAction");
    return QStringList();
} */

bool Qt5AccessibleFrame::isValid() const
{
    qDebug("Qt5AccessibleFrame::isValid");
    return true;
}

QObject* Qt5AccessibleFrame::object() const
{
    qDebug("Qt5AccessibleFrame::object");
    return m_pFrame;
}

void Qt5AccessibleFrame::setText(QAccessible::Text t, const QString& text)
{
    qDebug("Qt5AccessibleFrame::setText");
    return;
}

QAccessibleInterface* Qt5AccessibleFrame::childAt(int x, int y) const
{
    qDebug("Qt5AccessibleFrame::childAt");
    return nullptr;
}

QAccessibleInterface* Qt5AccessibleFrame::customFactory(const QString& classname, QObject* object)
{
    qDebug() << "calling customFactory with classname: " << classname;
    if (classname == QLatin1String("Qt5Widget") && object && object->isWidgetType())
    {
        qDebug("Creating interface for Qt5Widget object");
        return new Qt5AccessibleFrame(static_cast<Qt5Widget*>(object),
                                      (static_cast<Qt5Widget*>(object))->m_pFrame->GetWindow());
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
