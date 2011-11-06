/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef DBACCESS_IMAGEPROVIDER_HXX
#define DBACCESS_IMAGEPROVIDER_HXX

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_APPLICATION_DATABASEOBJECT_HPP_
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#endif
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
            @param  _out_rImageHC
                the high-contrast version of the image to use for the object
            @return
                the image to be used for the object.
        */
        void getImages(
            const String& _rName,
            const sal_Int32 _nDatabaseObjectType,
            Image& _out_rImage,
            Image& _out_rImageHC
        );

        /** returns the default image to be used for a database object

            In opposite to getImages, this method does not check the concrete object
            for its image, but returns a default image to be used for all objects of the given
            type.

            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @param  _bHighContrast
                indicates whether High-Contrast icons should be used.
                Note that normally, this would be some application-wide setting. However,
                in current OOo, HC support is decided on a per-control basis, means every
                control decides itself whether its images must be HC versions or not.
                Thus callers need to specify this flag.
            @return
                the image to be used for the object type.
        */
        Image getDefaultImage(
            sal_Int32 _nDatabaseObjectType,
            bool _bHighContrast
        );

        /** returns the resource ID for the default image to be used for a database object

            In opposite to getImages, this method does not check the concrete object
            for its image, but returns a default image to be used for all objects of the given
            type.

            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @param  _bHighContrast
                indicates whether High-Contrast icons should be used.
                Note that normally, this would be some application-wide setting. However,
                in current OOo, HC support is decided on a per-control basis, means every
                control decides itself whether its images must be HC versions or not.
                Thus callers need to specify this flag.
            @return
                the resource ID image to be used for the object type. Must be fed into a
                ModuleRes instance to actually load the image.
        */
        sal_uInt16 getDefaultImageResourceID(
            sal_Int32 _nDatabaseObjectType,
            bool _bHighContrast
        );

        /** retrieves the image to be used for folders of database objects
            @param  _nDatabaseObjectType
                the type of the object. Must be one of the css.sdb.application.DatabaseObject
                constants.
            @param  _rName
                the name of the object
            @param  _bHighContrast
                indicates whether High-Contrast icons should be used.
                Note that normally, this would be some application-wide setting. However,
                in current OOo, HC support is decided on a per-control basis, means every
                control decides itself whether its images must be HC versions or not.
                Thus callers need to specify this flag.
            @return
                the image to be used for folders of the given type
        */
        Image getFolderImage(
            sal_Int32 _nDatabaseObjectType,
            bool _bHighContrast
        );

        /** retrieves the image to be used for a database as a whole.
            @param  _bHighContrast
                indicates whether High-Contrast icons should be used.
                Note that normally, this would be some application-wide setting. However,
                in current OOo, HC support is decided on a per-control basis, means every
                control decides itself whether its images must be HC versions or not.
                Thus callers need to specify this flag.
            @return
                the image to be used for folders of this type
        */
        Image getDatabaseImage( bool _bHighContrast );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_IMAGEPROVIDER_HXX

