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

#include "MediaDescriptorHelper.hxx"

using namespace ::com::sun::star;

namespace apphelper
{

MediaDescriptorHelper::MediaDescriptorHelper( const uno::Sequence<
                        beans::PropertyValue > & rMediaDescriptor )
    : m_aRegularProperties(rMediaDescriptor.getLength())
    , m_aDeprecatedProperties(rMediaDescriptor.getLength())
    , m_aAdditionalProperties(rMediaDescriptor.getLength())
    , m_aModelProperties(rMediaDescriptor.getLength())
{
    impl_init();
    sal_Int32 nRegularCount = 0;
    sal_Int32 nDeprecatedCount = 0;
    sal_Int32 nAdditionalCount = 0;
    sal_Int32 nModelCount = 0;

    auto addRegularProp = [this, &nRegularCount](const beans::PropertyValue& rRegularProp)
    {
        m_aRegularProperties[nRegularCount] = rRegularProp;
        ++nRegularCount;
    };
    auto addModelProp = [this, &nModelCount, &addRegularProp](const beans::PropertyValue& rModelProp)
    {
        addRegularProp(rModelProp);
        m_aModelProperties[nModelCount] = rModelProp;
        ++nModelCount;
    };
    auto addDepreciatedProp = [this, &nDeprecatedCount](const beans::PropertyValue& rDeprecatedProp)
    {
        m_aDeprecatedProperties[nDeprecatedCount] = rDeprecatedProp;
        ++nDeprecatedCount;
    };

    //read given rMediaDescriptor and store in internal structures:
    for( sal_Int32 i= rMediaDescriptor.getLength();i--;)
    {
        const beans::PropertyValue& rProp = rMediaDescriptor[i];

        if (rProp.Name == "AsTemplate")
        {
            ISSET_AsTemplate = rProp.Value >>= AsTemplate;
            addModelProp(rProp);
        }
        else if (rProp.Name == "Author")
        {
            ISSET_Author = rProp.Value >>= Author;
            addModelProp(rProp);
        }
        else if (rProp.Name == "CharacterSet")
        {
            ISSET_CharacterSet = rProp.Value >>= CharacterSet;
            addModelProp(rProp);
        }
        else if (rProp.Name == "Comment")
        {
            ISSET_Comment = rProp.Value >>= Comment;
            addModelProp(rProp);
        }
        else if (rProp.Name == "ComponentData")
        {
            ComponentData = rProp.Value;
            ISSET_ComponentData = ComponentData.hasValue();
            addModelProp(rProp);
        }
        else if (rProp.Name == "FileName")
        {
            ISSET_FileName = rProp.Value >>= FileName;
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "FilterData")
        {
            FilterData = rProp.Value;
            ISSET_FilterData = FilterData.hasValue();
            addModelProp(rProp);
        }
        else if (rProp.Name == "FilterName")
        {
            ISSET_FilterName = rProp.Value >>= FilterName;
            addModelProp(rProp);
        }
        else if (rProp.Name == "FilterFlags")
        {
            ISSET_FilterFlags = rProp.Value >>= FilterFlags;
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "FilterOptions")
        {
            ISSET_FilterOptions = rProp.Value >>= FilterOptions;
            addModelProp(rProp);
        }
        else if (rProp.Name == "FrameName")
        {
            ISSET_FrameName = rProp.Value >>= FrameName;
            addModelProp(rProp);
        }
        else if (rProp.Name == "Hidden")
        {
            ISSET_Hidden = rProp.Value >>= Hidden;
            addModelProp(rProp);
        }
        else if (rProp.Name == "HierarchicalDocumentName")
        {
            ISSET_HierarchicalDocumentName = rProp.Value >>= HierarchicalDocumentName;
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
            ISSET_InteractionHandler = rProp.Value >>= InteractionHandler;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "JumpMark")
        {
            ISSET_JumpMark = rProp.Value >>= JumpMark;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "MediaType")
        {
            ISSET_MediaType = rProp.Value >>= MediaType;
            addModelProp(rProp);
        }
        else if (rProp.Name == "OpenFlags")
        {
            ISSET_OpenFlags = rProp.Value >>= OpenFlags;
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "OpenNewView")
        {
            ISSET_OpenNewView = rProp.Value >>= OpenNewView;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "Overwrite")
        {
            ISSET_Overwrite = rProp.Value >>= Overwrite;
            addModelProp(rProp);
        }
        else if (rProp.Name == "Password")
        {
            ISSET_Password = rProp.Value >>= Password;
            addModelProp(rProp);
        }
        else if (rProp.Name == "PosSize")
        {
            ISSET_PosSize = rProp.Value >>= PosSize;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "PostData")
        {
            ISSET_PostData = rProp.Value >>= PostData;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "PostString")
        {
            ISSET_PostString = rProp.Value >>= PostString;
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "Preview")
        {
            ISSET_Preview = rProp.Value >>= Preview;
            addModelProp(rProp);
        }
        else if (rProp.Name == "ReadOnly")
        {
            ISSET_ReadOnly = rProp.Value >>= ReadOnly;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "Referer")
        {
            ISSET_Referer = rProp.Value >>= Referer;
            addModelProp(rProp);
        }
        else if (rProp.Name == "SetEmbedded")
        {
            ISSET_SetEmbedded = rProp.Value >>= SetEmbedded;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "Silent")
        {
            ISSET_Silent = rProp.Value >>= Silent;
            addRegularProp(rProp);
        }
        else if (rProp.Name == "StatusIndicator")
        {
            ISSET_StatusIndicator = rProp.Value >>= StatusIndicator;
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
            ISSET_TemplateName = rProp.Value >>= TemplateName;
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "TemplateRegionName")
        {
            ISSET_TemplateRegionName = rProp.Value >>= TemplateRegionName;
            addDepreciatedProp(rProp);
        }
        else if (rProp.Name == "Unpacked")
        {
            ISSET_Unpacked = rProp.Value >>= Unpacked;
            addModelProp(rProp);
        }
        else if (rProp.Name == "URL")
        {
            ISSET_URL = rProp.Value >>= URL;
            addModelProp(rProp);
        }
        else if (rProp.Name == "Version")
        {
            ISSET_Version = rProp.Value >>= Version;
            addModelProp(rProp);
        }
        else if (rProp.Name == "ViewData")
        {
            ViewData = rProp.Value;
            ISSET_ViewData = ViewData.hasValue();
            addModelProp(rProp);
        }
        else if (rProp.Name == "ViewId")
        {
            ISSET_ViewId = rProp.Value >>= ViewId;
            addModelProp(rProp);
        }
        else if (rProp.Name == "WinExtent")
        {
            ISSET_WinExtent = rProp.Value >>= WinExtent;
            addDepreciatedProp(rProp);
        }
        else
        {
            m_aAdditionalProperties[nAdditionalCount]=rProp;
            nAdditionalCount++;
        }
    }

    m_aRegularProperties.realloc(nRegularCount);
    m_aDeprecatedProperties.realloc(nDeprecatedCount);
    m_aAdditionalProperties.realloc(nAdditionalCount);
    m_aModelProperties.realloc(nModelCount);
}

void MediaDescriptorHelper::impl_init()
{
    AsTemplate = false;
    ISSET_AsTemplate = false;

    ISSET_Author = false;
    ISSET_CharacterSet = false;
    ISSET_Comment = false;

//  css::uno::Any  ComponentData;
    ISSET_ComponentData = false;
    ISSET_FileName = false;

//  css::uno::Any  FilterData;
    ISSET_FilterData = false;
    ISSET_FilterName = false;
    ISSET_FilterFlags = false;
    ISSET_FilterOptions = false;
    ISSET_FrameName = false;

    Hidden = false;
    ISSET_Hidden = false;
    ISSET_HierarchicalDocumentName = false;
    ISSET_OutputStream = false;
    ISSET_InputStream = false;
    ISSET_InteractionHandler = false;
    ISSET_JumpMark = false;
    ISSET_MediaType = false;
    ISSET_OpenFlags = false;
    OpenNewView = false;
    ISSET_OpenNewView = false;
    Overwrite = false;
    ISSET_Overwrite = false;
    ISSET_Password = false;

//  css::awt::Rectangle PosSize;
    ISSET_PosSize = false;

//  css::uno::Sequence< sal_Int8 > PostData;
    ISSET_PostData = false;
    ISSET_PostString = false;
    Preview = false;
    ISSET_Preview = false;
    ReadOnly = false;
    ISSET_ReadOnly = false;
    ISSET_Referer = false;
    ISSET_StatusIndicator = false;
    Silent = false;
    ISSET_Silent = false;
    ISSET_TemplateName = false;
    ISSET_TemplateRegionName = false;
    Unpacked = false;
    ISSET_Unpacked = false;
    ISSET_URL = false;
    Version = 0;
    ISSET_Version = false;

//  css::uno::Any ViewData;
    ISSET_ViewData = false;
    ViewId = 0;
    ISSET_ViewId = false;

    ISSET_WinExtent = false;

    SetEmbedded = false;
    ISSET_SetEmbedded = false;

    ISSET_Storage = false;
    ISSET_Stream = false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
