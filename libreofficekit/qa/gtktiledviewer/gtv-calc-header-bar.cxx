/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <cassert>
#include <cmath>
#include <iostream>

#include "gtv-application-window.hxx"
#include "gtv-signal-handlers.hxx"
#include "gtv-helpers.hxx"
#include "gtv-calc-header-bar.hxx"

#include <map>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

struct GtvCalcHeaderBarPrivateImpl
{
    /// Stores size and content of a single row header.
    struct Header
    {
        int m_nSize;
        std::string m_aText;
        Header(int nSize, const std::string& rText)
            : m_nSize(nSize),
              m_aText(rText)
            { }
    };

    std::vector<Header> m_aHeaders;
    CalcHeaderType m_eType;

    GtvCalcHeaderBarPrivateImpl()
        : m_eType(CalcHeaderType::NONE)
        { }
};

struct GtvCalcHeaderBarPrivate
{
    GtvCalcHeaderBarPrivateImpl* m_pImpl;

    GtvCalcHeaderBarPrivateImpl* operator->()
    {
        return m_pImpl;
    }
};

#if defined __clang__
#if __has_warning("-Wdeprecated-volatile")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-volatile"
#endif
#endif
G_DEFINE_TYPE_WITH_PRIVATE(GtvCalcHeaderBar, gtv_calc_header_bar, GTK_TYPE_DRAWING_AREA);
#if defined __clang__
#if __has_warning("-Wdeprecated-volatile")
#pragma clang diagnostic pop
#endif
#endif

static const int ROW_HEADER_WIDTH = 50;
static const int COLUMN_HEADER_HEIGHT = 20;

static GtvCalcHeaderBarPrivate&
getPrivate(GtvCalcHeaderBar* headerbar)
{
    return *static_cast<GtvCalcHeaderBarPrivate*>(gtv_calc_header_bar_get_instance_private(headerbar));
}

static void
gtv_calc_header_bar_init(GtvCalcHeaderBar* bar)
{
    GtvCalcHeaderBarPrivate& priv = getPrivate(bar);
    priv.m_pImpl = new GtvCalcHeaderBarPrivateImpl();
}

static void
gtv_calc_header_bar_finalize(GObject* object)
{
    GtvCalcHeaderBarPrivate& priv = getPrivate(GTV_CALC_HEADER_BAR(object));

    delete priv.m_pImpl;
    priv.m_pImpl = nullptr;

    G_OBJECT_CLASS (gtv_calc_header_bar_parent_class)->finalize (object);
}

static void gtv_calc_header_bar_draw_text(cairo_t* pCairo, const GdkRectangle& rRectangle, const std::string& rText)
{
    cairo_text_extents_t extents;
    cairo_text_extents(pCairo, rText.c_str(), &extents);
    // Cairo reference point for text is the bottom left corner.
    cairo_move_to(pCairo, rRectangle.x + rRectangle.width / 2 - extents.width / 2, rRectangle.y + rRectangle.height / 2 + extents.height / 2);
    cairo_show_text(pCairo, rText.c_str());
}

static bool gtv_calc_header_bar_draw_impl(GtkWidget* pWidget, cairo_t* pCairo)
{
    GtvCalcHeaderBar* self = GTV_CALC_HEADER_BAR(pWidget);
    GtvCalcHeaderBarPrivate& priv = getPrivate(GTV_CALC_HEADER_BAR(self));
    cairo_set_source_rgb(pCairo, 0, 0, 0);

    int nPrevious = 0;
    for (const GtvCalcHeaderBarPrivateImpl::Header& rHeader : priv->m_aHeaders)
    {
        GdkRectangle aRectangle;
        if (priv->m_eType == CalcHeaderType::ROW)
        {
            aRectangle.x = 0;
            aRectangle.y = nPrevious;
            aRectangle.width = ROW_HEADER_WIDTH - 1;
            aRectangle.height = rHeader.m_nSize - nPrevious;
            // Left line.
            cairo_rectangle(pCairo, aRectangle.x, aRectangle.y, 1, aRectangle.height);
            cairo_fill(pCairo);
            // Bottom line.
            cairo_rectangle(pCairo, aRectangle.x, aRectangle.y + aRectangle.height, aRectangle.width, 1);
            cairo_fill(pCairo);
            // Right line.
            cairo_rectangle(pCairo, aRectangle.width, aRectangle.y, 1, aRectangle.height);
            cairo_fill(pCairo);
        }
        else if (priv->m_eType == CalcHeaderType::COLUMN)
        {
            aRectangle.x = nPrevious;
            aRectangle.y = 0;
            aRectangle.width = rHeader.m_nSize - nPrevious;
            aRectangle.height = COLUMN_HEADER_HEIGHT - 1;
            // Top line.
            cairo_rectangle(pCairo, aRectangle.x, aRectangle.y, aRectangle.width, 1);
            cairo_fill(pCairo);
            // Right line.
            cairo_rectangle(pCairo, aRectangle.x + aRectangle.width , aRectangle.y, 1, aRectangle.height);
            cairo_fill(pCairo);
            // Bottom line.
            cairo_rectangle(pCairo, aRectangle.x, aRectangle.height, aRectangle.width, 1);
            cairo_fill(pCairo);
        }
        else
        {
            assert(false); // should never happen
        }

        gtv_calc_header_bar_draw_text(pCairo, aRectangle, rHeader.m_aText);
        nPrevious = rHeader.m_nSize;
        if (rHeader.m_nSize > self->m_nSizePixel)
            break;
    }

    if (priv->m_aHeaders.empty() && priv->m_eType == CalcHeaderType::CORNER)
    {
        GdkRectangle aRectangle;
        aRectangle.x = 0;
        aRectangle.y = 0;
        aRectangle.width = ROW_HEADER_WIDTH - 1;
        aRectangle.height = COLUMN_HEADER_HEIGHT - 1;
        cairo_rectangle(pCairo, aRectangle.x, aRectangle.y, aRectangle.width, aRectangle.height);
        cairo_stroke(pCairo);
    }

    return FALSE;
}

static gboolean
gtv_calc_header_bar_draw(GtkWidget* bar, cairo_t* pCairo)
{
    return gtv_calc_header_bar_draw_impl(bar, pCairo);
}

static void
gtv_calc_header_bar_class_init(GtvCalcHeaderBarClass* klass)
{
    GTK_WIDGET_CLASS(klass)->draw = gtv_calc_header_bar_draw;
    G_OBJECT_CLASS(klass)->finalize = gtv_calc_header_bar_finalize;
}

void gtv_calc_header_bar_configure(GtvCalcHeaderBar* bar, const boost::property_tree::ptree* values)
{
    GtvCalcHeaderBarPrivate& priv = getPrivate(bar);
    priv->m_aHeaders.clear();

    if (values)
    {
        boost::property_tree::ptree val = *values;
        try
        {
            for (const boost::property_tree::ptree::value_type& rValue : val)
            {
                int nSize = std::round(std::atof(rValue.second.get<std::string>("size").c_str()));
                if (nSize >= bar->m_nPositionPixel)
                {
                    const int nScrolledSize = nSize - bar->m_nPositionPixel;
                    GtvCalcHeaderBarPrivateImpl::Header aHeader(nScrolledSize, rValue.second.get<std::string>("text"));
                    priv->m_aHeaders.push_back(aHeader);
                }
            }
        }
        catch (boost::property_tree::ptree_bad_path& rException)
        {
            std::cerr << "gtv_calc_header_bar_configure: " << rException.what() << std::endl;
        }
    }
    gtk_widget_show(GTK_WIDGET(bar));
    gtk_widget_queue_draw(GTK_WIDGET(bar));
}

void
gtv_calc_header_bar_set_type_and_width(GtvCalcHeaderBar* bar, CalcHeaderType eType)
{
    // TODO: Install type property for this class
    GtvCalcHeaderBarPrivate& priv = getPrivate(bar);
    priv->m_eType = eType;

    if (eType == CalcHeaderType::ROW)
        gtk_widget_set_size_request(GTK_WIDGET(bar), ROW_HEADER_WIDTH, -1);
    else if (eType == CalcHeaderType::COLUMN)
        gtk_widget_set_size_request(GTK_WIDGET(bar), -1, COLUMN_HEADER_HEIGHT);
}

GtkWidget*
gtv_calc_header_bar_new()
{
    return GTK_WIDGET(g_object_new(GTV_TYPE_CALC_HEADER_BAR,
                                   nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
