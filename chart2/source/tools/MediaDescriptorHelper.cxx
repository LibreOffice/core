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

const short FLAG_DEPRECATED =1;
const short FLAG_MODEL      =2;

#define WRITE_PROPERTY( MediaName, nFlags )             \
if(rProp.Name.equals(#MediaName)) \
{                                                       \
    if( rProp.Value >>= MediaName )                     \
        ISSET_##MediaName = true;                       \
    if((nFlags & FLAG_DEPRECATED) != 0)                 \
    {                                                   \
        m_aDeprecatedProperties[nDeprecatedCount]=rProp;\
        nDeprecatedCount++;                             \
    }                                                   \
    else                                                \
    {                                                   \
        m_aRegularProperties[nRegularCount]=rProp;      \
        nRegularCount++;                                \
        if((nFlags & FLAG_MODEL) != 0)                  \
        {                                               \
            m_aModelProperties[nModelCount]=rProp;      \
            nModelCount++;                              \
        }                                               \
    }                                                   \
}

MediaDescriptorHelper::MediaDescriptorHelper( const uno::Sequence<
                        beans::PropertyValue > & rMediaDescriptor )
{
    impl_init();

    m_aRegularProperties.realloc(0);
    m_aRegularProperties.realloc(rMediaDescriptor.getLength());
    sal_Int32 nRegularCount = 0;

    m_aDeprecatedProperties.realloc(0);
    m_aDeprecatedProperties.realloc(rMediaDescriptor.getLength());
    sal_Int32 nDeprecatedCount = 0;

    m_aAdditionalProperties.realloc(0);
    m_aAdditionalProperties.realloc(rMediaDescriptor.getLength());
    sal_Int32 nAdditionalCount = 0;

    m_aModelProperties.realloc(0);
    m_aModelProperties.realloc(rMediaDescriptor.getLength());
    sal_Int32 nModelCount = 0;

    //read given rMediaDescriptor and store in internal structures:
    for( sal_Int32 i= rMediaDescriptor.getLength();i--;)
    {
        const beans::PropertyValue& rProp = rMediaDescriptor[i];
        WRITE_PROPERTY( AsTemplate, FLAG_MODEL )
        else WRITE_PROPERTY( Author, FLAG_MODEL )
        else WRITE_PROPERTY( CharacterSet, FLAG_MODEL )
        else WRITE_PROPERTY( Comment, FLAG_MODEL )
        else WRITE_PROPERTY( ComponentData, FLAG_MODEL )
        else WRITE_PROPERTY( FileName, FLAG_DEPRECATED )
        else WRITE_PROPERTY( FilterData, FLAG_MODEL )
        else WRITE_PROPERTY( FilterName, FLAG_MODEL )
        else WRITE_PROPERTY( FilterFlags, FLAG_DEPRECATED)
        else WRITE_PROPERTY( FilterOptions, FLAG_MODEL )
        else WRITE_PROPERTY( FrameName, FLAG_MODEL )
        else WRITE_PROPERTY( Hidden, FLAG_MODEL )
        else WRITE_PROPERTY( HierarchicalDocumentName, FLAG_MODEL )
        else WRITE_PROPERTY( OutputStream, 0 )
        else WRITE_PROPERTY( InputStream, 0 )
        else WRITE_PROPERTY( InteractionHandler, 0 )
        else WRITE_PROPERTY( JumpMark, 0 )
        else WRITE_PROPERTY( MediaType, FLAG_MODEL )
        else WRITE_PROPERTY( OpenFlags, FLAG_DEPRECATED )
        else WRITE_PROPERTY( OpenNewView, 0 )
        else WRITE_PROPERTY( Overwrite, FLAG_MODEL )
        else WRITE_PROPERTY( Password, FLAG_MODEL )
        else WRITE_PROPERTY( PosSize, 0 )
        else WRITE_PROPERTY( PostData, 0 )
        else WRITE_PROPERTY( PostString, FLAG_DEPRECATED )
        else WRITE_PROPERTY( Preview, FLAG_MODEL )
        else WRITE_PROPERTY( ReadOnly, 0 )
        else WRITE_PROPERTY( Referer, FLAG_MODEL )
        else WRITE_PROPERTY( SetEmbedded, 0 )
        else WRITE_PROPERTY( Silent, 0 )
        else WRITE_PROPERTY( StatusIndicator, 0 )
        else WRITE_PROPERTY( Storage, FLAG_MODEL )
        else WRITE_PROPERTY( Stream, FLAG_MODEL )
        else WRITE_PROPERTY( TemplateName, FLAG_DEPRECATED )
        else WRITE_PROPERTY( TemplateRegionName, FLAG_DEPRECATED )
        else WRITE_PROPERTY( Unpacked, FLAG_MODEL )
        else WRITE_PROPERTY( URL, FLAG_MODEL )
        else WRITE_PROPERTY( Version, FLAG_MODEL )
        else WRITE_PROPERTY( ViewData, FLAG_MODEL )
        else WRITE_PROPERTY( ViewId, FLAG_MODEL )
        else WRITE_PROPERTY( WinExtent, FLAG_DEPRECATED )
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

//  ::com::sun::star::uno::Any  ComponentData;
    ISSET_ComponentData = false;
    ISSET_FileName = false;

//  ::com::sun::star::uno::Any  FilterData;
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

//  ::com::sun::star::awt::Rectangle PosSize;
    ISSET_PosSize = false;

//  ::com::sun::star::uno::Sequence< sal_Int8 > PostData;
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

//  ::com::sun::star::uno::Any ViewData;
    ISSET_ViewData = false;
    ViewId = 0;
    ISSET_ViewId = false;

    ISSET_WinExtent = false;

    ISSET_Storage = false;
    ISSET_Stream = false;
}

    uno::Sequence< beans::PropertyValue > MediaDescriptorHelper
::getReducedForModel()
{
    return m_aModelProperties;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
