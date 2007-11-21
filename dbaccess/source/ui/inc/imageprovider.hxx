/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imageprovider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:05:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
        USHORT getDefaultImageResourceID(
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

