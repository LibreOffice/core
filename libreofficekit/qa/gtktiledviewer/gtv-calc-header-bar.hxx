/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_CALC_HEADER_BAR_H
#define GTV_CALC_HEADER_BAR_H

#include <gtk/gtk.h>

#include <boost/property_tree/json_parser.hpp>

G_BEGIN_DECLS

#define GTV_TYPE_CALC_HEADER_BAR            (gtv_calc_header_bar_get_type())
#define GTV_CALC_HEADER_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTV_TYPE_CALC_HEADER_BAR, GtvCalcHeaderBar))
#define GTV_IS_CALC_HEADER_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTV_TYPE_CALC_HEADER_BAR))
#define GTV_CALC_HEADER_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  GTV_TYPE_CALC_HEADER_BAR, GtvCalcHeaderBarClass))
#define GTV_IS_CALC_HEADER_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  GTV_TYPE_CALC_HEADER_BAR))
#define GTV_CALC_HEADER_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  GTV_TYPE_CALC_HEADER_BAR, GtvCalcHeaderBarClass))

struct GtvCalcHeaderBar
{
    GtkDrawingArea parent;
    /// Height for row bar, width for column bar.
    int m_nSizePixel;
    /// Left/top position for the column/row bar -- initially 0, then may grow due to scrolling.
    int m_nPositionPixel;
};

struct GtvCalcHeaderBarClass
{
    GtkDrawingAreaClass parentClass;
};

GType gtv_calc_header_bar_get_type               (void) G_GNUC_CONST;

enum CalcHeaderType { ROW, COLUMN, CORNER, NONE };

GtkWidget* gtv_calc_header_bar_new();

void gtv_calc_header_bar_configure(GtvCalcHeaderBar* bar, const boost::property_tree::ptree* values);

int gtv_calc_header_bar_get_pos_pixel(GtvCalcHeaderBar* bar);

int gtv_calc_header_bar_get_size_pixel(GtvCalcHeaderBar* bar);

void gtv_calc_header_bar_set_type_and_width(GtvCalcHeaderBar* bar, CalcHeaderType eType);

G_END_DECLS

#endif /* GTV_CALC_HEADER_BAR_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
