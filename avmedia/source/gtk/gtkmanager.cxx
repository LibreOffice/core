/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

#include "gtkmanager.hxx"
#include "gtkplayer.hxx"

#include <tools/urlobj.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;

namespace avmedia::gtk
{
Manager::Manager() {}

Manager::~Manager() {}

uno::Reference<media::XPlayer> SAL_CALL Manager::createPlayer(const OUString& rURL)
{
    const INetURLObject aURL(rURL);
    OUString sMainURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);

    rtl::Reference<GtkPlayer> xPlayer(new GtkPlayer);
    if (!xPlayer->create(sMainURL))
        xPlayer.clear();
    return xPlayer;
}

OUString SAL_CALL Manager::getImplementationName() { return "com.sun.star.comp.media.Manager_Gtk"; }

sal_Bool SAL_CALL Manager::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL Manager::getSupportedServiceNames()
{
    return { "com.sun.star.media.Manager" };
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_media_Manager_Gtk_get_implementation(css::uno::XComponentContext*,
                                                       css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new avmedia::gtk::Manager());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
