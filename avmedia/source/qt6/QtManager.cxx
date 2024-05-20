/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <tools/urlobj.hxx>

#include "QtManager.hxx"
#include "QtPlayer.hxx"

namespace avmedia::qt
{
QtManager::QtManager() {}

QtManager::~QtManager() {}

css::uno::Reference<css::media::XPlayer> SAL_CALL QtManager::createPlayer(const OUString& rURL)
{
    const INetURLObject aURL(rURL);
    OUString sMainURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);

    rtl::Reference<QtPlayer> xPlayer(new QtPlayer);
    if (!xPlayer->create(sMainURL))
    {
        xPlayer->dispose();
        xPlayer.clear();
    }
    return xPlayer;
}

OUString SAL_CALL QtManager::getImplementationName()
{
    return u"com.sun.star.comp.media.Manager_Qt"_ustr;
}

sal_Bool SAL_CALL QtManager::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL QtManager::getSupportedServiceNames()
{
    return { u"com.sun.star.media.Manager"_ustr };
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_media_Manager_Qt_get_implementation(css::uno::XComponentContext*,
                                                      css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new avmedia::qt::QtManager());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
