/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef DBACCESS_IMAGEPROVIDER_HXX
#define DBACCESS_IMAGEPROVIDER_HXX

#include <vcl/image.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
/** === end UNO includes === **/

#include <boost/shared_ptr.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

    // for convenience of our clients
    namespace DatabaseObject = ::com::sun::star::sdb::application::DatabaseObject;

    //====================================================================
    //= ImageProvider
    //====================================================================
    struct ImageProvider_Data;
    /** provides images for database objects such as tables, queries, forms, reports ...

        At the moment, this class cares for small icons only, that is, icons which can be used
        in a tree control. On the medium term, we should extend it with support for different-sized
        icons.
    */
    class ImageProvider
    {
    private:
        ::boost::shared_ptr< ImageProvider_Data >   m_pData;

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
        );

        /** returns the image to be used for a database object with the given name

            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @param  _rName
                the name of the object
            @param  _out_rImage
                the normal image to use for the object
            @return
                the image to be used for the object.
        */
        void getImages(
            const String& _rName,
            const sal_Int32 _nDatabaseObjectType,
            Image& _out_rImage
        );

        /** returns the default image to be used for a database object

            In opposite to getImages, this method does not check the concrete object
            for its image, but returns a default image to be used for all objects of the given
            type.

            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @return
                the image to be used for the object type.
        */
        Image getDefaultImage(
            sal_Int32 _nDatabaseObjectType
        );

        /** returns the resource ID for the default image to be used for a database object

            In opposite to getImages, this method does not check the concrete object
            for its image, but returns a default image to be used for all objects of the given
            type.

            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @return
                the resource ID image to be used for the object type. Must be fed into a
                ModuleRes instance to actually load the image.
        */
        sal_uInt16 getDefaultImageResourceID(
            sal_Int32 _nDatabaseObjectType
        );

        /** retrieves the image to be used for folders of database objects
            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @param  _rName
                the name of the object
            @return
                the image to be used for folders of the given type
        */
        Image getFolderImage(
            sal_Int32 _nDatabaseObjectType
        );

        /** retrieves the image to be used for a database as a whole.
            @return
                the image to be used for folders of this type
        */
        Image getDatabaseImage();
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_IMAGEPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
