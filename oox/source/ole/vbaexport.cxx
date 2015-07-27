/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/ole/vbaexport.hxx>

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"

VbaExport::VbaExport(css::uno::Reference<css::frame::XModel> xModel):
    mxModel(xModel)
{
    maProjectName = "How to get the correct project name?";
}

void VbaExport::exportVBA()
{
    // start here with the VBA export
}

css::uno::Reference<css::container::XNameContainer> VbaExport::getBasicLibrary()
{
    css::uno::Reference<css::container::XNameContainer> xLibrary;
    try
    {
        oox::PropertySet aDocProp(mxModel);
        css::uno::Reference<css::script::XLibraryContainer> xLibContainer(aDocProp.getAnyProperty(oox::PROP_BasicLibraries), css::uno::UNO_QUERY_THROW);
        xLibrary.set( xLibContainer->getByName(maProjectName), css::uno::UNO_QUERY_THROW );
    }
    catch(...)
    {
    }

    return xLibrary;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
