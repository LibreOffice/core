/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <cmath>
#include <iostream>

#include <gtv-application-window.hxx>
#include <gtv-signal-handlers.hxx>
#include <gtv-helpers.hxx>
#include <gtv-calc-header-bar.hxx>

#include <map>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

struct _GtvCalcHeaderBar
{
    GtkDrawingArea parent;
};

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
    /// Height for row bar, width for column bar.
    int m_nSizePixel;
    /// Left/top position for the column/row bar -- initially 0, then may grow due to scrolling.
    int m_nPositionPixel;
    CalcHeaderType m_eType;
};

struct GtvCalcHeaderBarPrivate
{
    GtvCalcHeaderBarPrivateImpl* m_pImpl;

    GtvCalcHeaderBarPrivateImpl* operator->()
    {
        return m_pImpl;
    }
};

G_DEFINE_TYPE_WITH_PRIVATE(GtvCalcHeaderBar, gtv_calc_header_bar, GTK_TYPE_DRAWING_AREA);

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

void gtv_calc_header_bar_draw_text(cairo_t* pCairo, const GdkRectangle& rRectangle, const std::string& rText)
{
    cairo_text_extents_t extents;
    cairo_text_extents(pCairo, rText.c_str(), &extents);
    // Cairo reference point for text is the bottom left corner.
    cairo_move_to(pCairo, rRectangle.x + rRectangle.width / 2 - extents.width / 2, rRectangle.y + rRectangle.height / 2 + extents.height / 2);
    cairo_show_text(pCairo, rText.c_str());
}

gboolean gtv_calc_header_bar_draw_impl(GtkWidget* pWidget, cairo_t* pCairo)
{
    GtvCalcHeaderBarPrivate& priv = getPrivate(GTV_CALC_HEADER_BAR(pWidget));
    cairo_set_source_rgb(pCairo, 0, 0, 0);

    int nPrevious = 0;
    for (const GtvCalcHeaderBarPrivateImpl::Header& rHeader : priv->m_aHeaders)
    {
        GdkRectangle aRectangle;
        if (priv->m_eType == CalcHeaderType::ROW)
        {
            aRectangle.x = 0;
            aRectangle.y = nPrevious - 1;
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
        else
        {
            aRectangle.x = nPrevious - 1;
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

        gtv_calc_header_bar_draw_text(pCairo, aRectangle, rHeader.m_aText);
        nPrevious = rHeader.m_nSize;
        if (rHeader.m_nSize > priv->m_nSizePixel)
            break;
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
    // TODO: Use templates to bind objects maybe ?
    // But that requires compiling the .ui file into C source requiring
    // glib-compile-resources (another dependency) as I can't find any gtk
    // method to set the template from the .ui file directly; can only be set
    // from gresource
    GTK_WIDGET_CLASS(klass)->draw = gtv_calc_header_bar_draw;
    G_OBJECT_CLASS(klass)->finalize = gtv_calc_header_bar_finalize;
}

int gtv_calc_header_bar_get_pos_pixel(GtvCalcHeaderBar* bar)
{
    GtvCalcHeaderBarPrivate& priv = getPrivate(bar);
    return priv->m_nPositionPixel;
}

int gtv_calc_header_bar_get_size_pixel(GtvCalcHeaderBar* bar)
{
    GtvCalcHeaderBarPrivate& priv = getPrivate(bar);
    return priv->m_nSizePixel;
}

void gtv_calc_header_bar_configure(GtvCalcHeaderBar* bar, const boost::property_tree::ptree* values, int nPositionPixel)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(bar)));
    GtvCalcHeaderBarPrivate& priv = getPrivate(bar);

    //gtk_widget_show(rWindow.m_pCornerButton->m_pDrawingArea);
    boost::property_tree::ptree val = *values;
    priv->m_aHeaders.clear();
    try
    {
        for (boost::property_tree::ptree::value_type& rValue : val)
        {
            int nSize = std::round(lok_doc_view_twip_to_pixel(LOK_DOC_VIEW(window->lokdocview), std::atof(rValue.second.get<std::string>("size").c_str())));
            if (nSize >= nPositionPixel)
            {
                const int nScrolledSize = nSize - nPositionPixel;
                GtvCalcHeaderBarPrivateImpl::Header aHeader(nScrolledSize, rValue.second.get<std::string>("text"));
                priv->m_aHeaders.push_back(aHeader);
            }
        }
    }
    catch (boost::property_tree::ptree_bad_path& rException)
    {
        std::cerr << "gtv_calc_header_bar_configure: " << rException.what() << std::endl;
    }
    gtk_widget_show(GTK_WIDGET(bar));
    gtk_widget_queue_draw(GTK_WIDGET(bar));
}

void
gtv_calc_header_bar_set_type(GtvCalcHeaderBar* bar, CalcHeaderType eType)
{
    // TODO: Install type property for this class
    GtvCalcHeaderBarPrivate& priv = getPrivate(bar);
    priv->m_eType = eType;
}

GtkWidget*
gtv_calc_header_bar_new()
{
    return GTK_WIDGET(g_object_new(GTV_CALC_HEADER_BAR_TYPE,
                                   nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
