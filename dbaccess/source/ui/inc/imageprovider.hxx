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

#pragma once

#include <vcl/image.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <memory>

namespace dbaui
{
    // ImageProvider
    struct ImageProvider_Data;
    /** provides images for database objects such as tables, queries, forms, reports ...

        At the moment, this class cares for small icons only, that is, icons which can be used
        in a tree control. On the medium term, we should extend it with support for different-sized
        icons.
    */
    class ImageProvider
    {
    private:
        std::shared_ptr< ImageProvider_Data >   m_pData;

    public:
        /** creates a semi-functional ImageProvider instance

            The resulting instance is not able to provide any concrete object images,
            but only default images.
        */
        ImageProvider();

        /** creates an ImageProvider instance

            @param  _rxConnection
                denotes the connection to work for. Must not be <NULL/>.
        */
        ImageProvider(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
        );

        /** returns the image to be used for a database object with the given name

            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @param  _rName
                the name of the object
            @return
                the name of the image to be used for the object.
        */
        OUString getImageId(
            const OUString& _rName,
            const sal_Int32 _nDatabaseObjectType
        );

        // check whether the connection can give us an icon
        css::uno::Reference<css::graphic::XGraphic> getXGraphic(const OUString& _rName,
                                                                const sal_Int32 _nDatabaseObjectType);

        /** returns the resource ID for the default image to be used for a database object

            In opposite to getImageId, this method does not check the concrete object
            for its image, but returns a default image to be used for all objects of the given
            type.

            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @return
                the resource image name to be used for the object type.
        */
        static OUString getDefaultImageResourceID(sal_Int32 _nDatabaseObjectType);

        static OUString getFolderImageId(
            sal_Int32 _nDatabaseObjectType
        );

        /** retrieves the image to be used for a database as a whole.
            @return
                the image to be used for folders of this type
        */
        static OUString getDatabaseImage();
    };

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
