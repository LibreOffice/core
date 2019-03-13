/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <MediaDescriptorHelper.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace ::com::sun::star;

namespace apphelper
{

MediaDescriptorHelper::MediaDescriptorHelper( const uno::Sequence<
                        beans::PropertyValue > & rMediaDescriptor )
    : m_aModelProperties(rMediaDescriptor.getLength())
{
    css::uno::Sequence< css::beans::PropertyValue >
                        aRegularProperties(rMediaDescriptor.getLength()); //these are the properties which are described in service com.sun.star.document.MediaDescriptor and not marked as deprecated
    css::uno::Sequence< css::beans::PropertyValue >
                        aDeprecatedProperties(rMediaDescriptor.getLength()); //these are properties which are described in service com.sun.star.document.MediaDescriptor but are marked as deprecated
    impl_init();
    sal_Int32 nRegularCount = 0;
    sal_Int32 nDeprecatedCount = 0;
    sal_Int32 nModelCount = 0;

    auto addRegularProp = [&aRegularProperties, &nRegularCount](const beans::PropertyValue& rRegularProp)
    {
        aRegularProperties[nRegularCount] = rRegularProp;
        ++nRegularCount;
    };
    auto addModelProp = [this, &nModelCount, &addRegularProp](const beans::PropertyValue& rModelProp)
    {
        addRegularProp(rModelProp);
        m_aModelProperties[nModelCount] = rModelProp;
        ++nModelCount;
    };
    auto addDepreciatedProp = [&aDeprecatedProperties, &nDeprecatedCount](const beans::PropertyValue& rDeprecatedProp)
    {
        aDeprecatedProperties[nDeprecatedCount] = rDeprecatedProp;
        ++nDeprecatedCount;
    };

    //read given rMediaDescriptor and store in internal structures:
    for( sal_Int32 i= rMediaDescriptor.getLength();i--;)
    {
        const beans::PropertyValue& rProp = rMediaDescriptor[i];

        if (rProp.Name == "AsTemplate")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "Author")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "CharacterSet")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "Comment")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "ComponentData")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "FileName")
        {
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "FilterData")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "FilterName")
        {
            ISSET_FilterName = rProp.Value >>= FilterName;
            addModelProp(rProp);
        }
        else if (rProp.Name == "FilterFlags")
        {
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "FilterOptions")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "FrameName")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "Hidden")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "HierarchicalDocumentName")
        {
            rProp.Value >>= HierarchicalDocumentName;
            addModelProp(rProp);
        }
        else if (rProp.Name == "OutputStream")
        {
            ISSET_OutputStream = rProp.Value >>= OutputStream;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "InputStream")
        {
            ISSET_InputStream = rProp.Value >>= InputStream;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "InteractionHandler")
        {
            addRegularProp(rProp);
        }
        else if (rProp.Name == "JumpMark")
        {
            addRegularProp(rProp);
        }
        else if (rProp.Name == "MediaType")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "OpenFlags")
        {
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "OpenNewView")
        {
            addRegularProp(rProp);
        }
        else if (rProp.Name == "Overwrite")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "Password")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "PosSize")
        {
            addRegularProp(rProp);
        }
        else if (rProp.Name == "PostData")
        {
            addRegularProp(rProp);
        }
        else if (rProp.Name == "PostString")
        {
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "Preview")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "ReadOnly")
        {
            rProp.Value >>= ReadOnly;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "Referer")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "SetEmbedded")
        {
            addRegularProp(rProp);
        }
        else if (rProp.Name == "Silent")
        {
            addRegularProp(rProp);
        }
        else if (rProp.Name == "StatusIndicator")
        {
            addRegularProp(rProp);
        }
        else if (rProp.Name == "Storage")
        {
            ISSET_Storage = rProp.Value >>= Storage;
            addModelProp(rProp);
        }
        else if (rProp.Name == "Stream")
        {
            ISSET_Stream = rProp.Value >>= Stream;
            addModelProp(rProp);
        }
        else if (rProp.Name == "TemplateName")
        {
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "TemplateRegionName")
        {
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "Unpacked")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "URL")
        {
            ISSET_URL = rProp.Value >>= URL;
            addModelProp(rProp);
        }
        else if (rProp.Name == "Version")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "ViewData")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "ViewId")
        {
            addModelProp(rProp);
        }
        else if (rProp.Name == "WinExtent")
        {
            addDepreciatedProp(rProp);
        }
    }

    aRegularProperties.realloc(nRegularCount);
    aDeprecatedProperties.realloc(nDeprecatedCount);
    m_aModelProperties.realloc(nModelCount);
}

void MediaDescriptorHelper::impl_init()
{
    ISSET_FilterName = false;

    ISSET_OutputStream = false;
    ISSET_InputStream = false;

    ReadOnly = false;
    ISSET_URL = false;

    ISSET_Storage = false;
    ISSET_Stream = false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
