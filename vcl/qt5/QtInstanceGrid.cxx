/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceGrid.hxx>
#include <QtInstanceGrid.moc>

QtInstanceGrid::QtInstanceGrid(QWidget* pWidget)
    : QtInstanceContainer(pWidget)
{
    assert(qobject_cast<QGridLayout*>(pWidget->layout()) && "no grid layout");
}

void QtInstanceGrid::set_child_left_attach(weld::Widget& rWidget, int nAttach)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = dynamic_cast<QtInstanceWidget&>(rWidget).getQWidget();
        assert(pWidget && pWidget->parentWidget() == getQWidget()
               && "widget it is not a grid child");
        int nRow = 0;
        int nCol = 0;
        int nRowSpan = 0;
        int nColSpan = 0;
        getLayout().getItemPosition(getLayout().indexOf(pWidget), &nRow, &nCol, &nRowSpan,
                                    &nColSpan);
        getLayout().removeWidget(pWidget);
        getLayout().addWidget(pWidget, nRow, nAttach, nRowSpan, nColSpan);
    });
}

int QtInstanceGrid::get_child_left_attach(weld::Widget& rWidget) const
{
    SolarMutexGuard g;

    int nCol;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = dynamic_cast<QtInstanceWidget&>(rWidget).getQWidget();
        assert(pWidget && pWidget->parentWidget() == getQWidget()
               && "widget it is not a grid child");
        int nRow = 0;
        int nRowSpan = 0;
        int nColSpan = 0;
        getLayout().getItemPosition(getLayout().indexOf(pWidget), &nRow, &nCol, &nRowSpan,
                                    &nColSpan);
    });
    return nCol;
}

void QtInstanceGrid::set_child_column_span(weld::Widget& rWidget, int nCols)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = dynamic_cast<QtInstanceWidget&>(rWidget).getQWidget();
        assert(pWidget && pWidget->parentWidget() == getQWidget()
               && "widget it is not a grid child");
        int nRow = 0;
        int nCol = 0;
        int nRowSpan = 0;
        int nColSpan = 0;
        getLayout().getItemPosition(getLayout().indexOf(pWidget), &nRow, &nCol, &nRowSpan,
                                    &nColSpan);
        getLayout().removeWidget(pWidget);
        getLayout().addWidget(pWidget, nRow, nCol, nRowSpan, nCols);
    });
}

void QtInstanceGrid::set_child_top_attach(weld::Widget& rWidget, int nAttach)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = dynamic_cast<QtInstanceWidget&>(rWidget).getQWidget();
        assert(pWidget && pWidget->parentWidget() == getQWidget()
               && "widget it is not a grid child");
        int nRow = 0;
        int nCol = 0;
        int nRowSpan = 0;
        int nColSpan = 0;
        getLayout().getItemPosition(getLayout().indexOf(pWidget), &nRow, &nCol, &nRowSpan,
                                    &nColSpan);
        getLayout().removeWidget(pWidget);
        getLayout().addWidget(pWidget, nAttach, nCol, nRowSpan, nColSpan);
    });
}

int QtInstanceGrid::get_child_top_attach(weld::Widget& rWidget) const
{
    SolarMutexGuard g;

    int nRow;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = dynamic_cast<QtInstanceWidget&>(rWidget).getQWidget();
        assert(pWidget && pWidget->parentWidget() == getQWidget()
               && "widget it is not a grid child");
        int nCol = 0;
        int nRowSpan = 0;
        int nColSpan = 0;
        getLayout().getItemPosition(getLayout().indexOf(pWidget), &nRow, &nCol, &nRowSpan,
                                    &nColSpan);
    });
    return nRow;
}

QGridLayout& QtInstanceGrid::getLayout() const
{
    return static_cast<QGridLayout&>(QtInstanceContainer::getLayout());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
