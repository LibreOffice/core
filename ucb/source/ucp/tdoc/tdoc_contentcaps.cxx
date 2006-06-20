/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdoc_contentcaps.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:31:34 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 **************************************************************************

    Props/Commands:

                        root  document folder  folder  stream  stream
                                                (new)          (new)
    ----------------------------------------------------------------
    ContentType         r       r       r       r       r       r
    IsDocument          r       r       r       r       r       r
    IsFolder            r       r       r       r       r       r
    Title               r       r       w       w       w       w
    Storage             -       -       r       r       -       -
    DocumentModel       -       r       -       -       -       -

    getCommandInfo      x       x       x       x       x       x
    getPropertySetInfo  x       x       x       x       x       x
    getPropertyValues   x       x       x       x       x       x
    setPropertyValues   x       x       x       x       x       x
    insert              -       -       x       x       x(*)    x(*)
    delete              -       -       x       -       x       -
    open                x       x       x       -       x       -
    transfer            -       x       x       -       -       -

#ifdef NO_STREAM_CREATION_WITHIN_DOCUMENT_ROOT
 (*) not supported by streams that are direct children of document
#endif

 *************************************************************************/

#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/embed/XStorage.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/ucb/CommandInfo.hpp"
#include "com/sun/star/ucb/OpenCommandArgument2.hpp"
#include "com/sun/star/ucb/TransferInfo.hpp"

#include "tdoc_content.hxx"

namespace com { namespace sun { namespace star { namespace embed {
    class XStorage;
} } } }

using namespace com::sun;
using namespace com::sun::star;
using namespace tdoc_ucp;

//=========================================================================
//
// Content implementation.
//
//=========================================================================

//=========================================================================
//
// IMPORTENT: If any property data ( name / type / ... ) are changed, then
//            Content::getPropertyValues(...) must be adapted too!
//
//=========================================================================

// virtual
uno::Sequence< beans::Property > Content::getProperties(
            const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_aProps.getType() == STREAM )
    {
        //=================================================================
        //
        // Stream: Supported properties
        //
        //=================================================================

        static beans::Property aStreamPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory properties
            ///////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
            )
            ///////////////////////////////////////////////////////////
            // Optional standard properties
            ///////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////
        };
        return uno::Sequence< beans::Property >( aStreamPropertyInfoTable, 4 );
    }
    else if ( m_aProps.getType() == FOLDER )
    {
        //=================================================================
        //
        // Folder: Supported properties
        //
        //=================================================================

        static beans::Property aFolderPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory properties
            ///////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard properties
            ///////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Storage" ) ),
                -1,
                getCppuType( static_cast<
                    const uno::Reference< embed::XStorage > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
        };
        return uno::Sequence< beans::Property >( aFolderPropertyInfoTable, 5 );
    }
    else if ( m_aProps.getType() == DOCUMENT )
    {
        //=================================================================
        //
        // Document: Supported properties
        //
        //=================================================================

        static beans::Property aDocPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory properties
            ///////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard properties
            ///////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentModel" ) ),
                -1,
                getCppuType( static_cast<
                    const uno::Reference< frame::XModel > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
        };
        return uno::Sequence< beans::Property >( aDocPropertyInfoTable, 5 );
    }
    else
    {
        //=================================================================
        //
        // Root: Supported properties
        //
        //=================================================================

        OSL_ENSURE( m_aProps.getType() == ROOT, "Wrong content type!" );

        static beans::Property aRootPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Mandatory properties
            ///////////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
            ///////////////////////////////////////////////////////////////
            // Optional standard properties
            ///////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////////
        };
        return uno::Sequence< beans::Property >( aRootPropertyInfoTable, 4 );
    }
}

//=========================================================================
// virtual
uno::Sequence< star::ucb::CommandInfo > Content::getCommands(
            const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_aProps.getType() == STREAM )
    {
#ifdef NO_STREAM_CREATION_WITHIN_DOCUMENT_ROOT
        Uri aUri( m_xIdentifier->getContentIdentifier() );
        Uri aParentUri( aUri.getParentUri() );

        if ( aParentUri.isDocument() )
        {
            //=================================================================
            //
            // Stream, that is a child of a document: Supported commands
            //
            //=================================================================

            static star::ucb::CommandInfo aStreamCommandInfoTable1[] =
            {
                ///////////////////////////////////////////////////////////
                // Mandatory commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                    -1,
                    getCppuVoidType()
                ),
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                    -1,
                    getCppuVoidType()
                ),
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                    -1,
                    getCppuType(
                        static_cast< uno::Sequence< beans::Property > * >( 0 ) )
                ),
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                    -1,
                    getCppuType(
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                    -1,
                    getCppuBooleanType()
                ),
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                    -1,
                    getCppuType(
                        static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
                )
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                    star::ucb::CommandInfo >( aStreamCommandInfoTable1, 6 );
        }
#endif
        //=================================================================
        //
        // Stream: Supported commands
        //
        //=================================================================

        static star::ucb::CommandInfo aStreamCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast< uno::Sequence< beans::Property > * >( 0 ) )
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType(
                    static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////
        };
        return uno::Sequence<
                star::ucb::CommandInfo >( aStreamCommandInfoTable, 7 );
    }
    else if ( m_aProps.getType() == FOLDER )
    {
        //=================================================================
        //
        // Folder: Supported commands
        //
        //=================================================================

        static star::ucb::CommandInfo aFolderCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast< uno::Sequence< beans::Property > * >( 0 ) )
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType(
                    static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
            ),
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                -1,
                getCppuType( static_cast< star::ucb::TransferInfo * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////
        };
        return uno::Sequence<
                star::ucb::CommandInfo >( aFolderCommandInfoTable, 8 );
    }
    else if ( m_aProps.getType() == DOCUMENT )
    {
        //=================================================================
        //
        // Document: Supported commands
        //
        //=================================================================

        static star::ucb::CommandInfo aDocCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast< uno::Sequence< beans::Property > * >( 0 ) )
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType(
                    static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
            ),
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                -1,
                getCppuType( static_cast< star::ucb::TransferInfo * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////
        };
        return uno::Sequence<
                star::ucb::CommandInfo >( aDocCommandInfoTable, 6 );
    }
    else
    {
        //=================================================================
        //
        // Root: Supported commands
        //
        //=================================================================

        OSL_ENSURE( m_aProps.getType() == ROOT, "Wrong content type!" );

        static star::ucb::CommandInfo aRootCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast< uno::Sequence< beans::Property > * >( 0 ) )
            ),
            star::ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////
            star::ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType(
                    static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////
        };
        return uno::Sequence<
                star::ucb::CommandInfo >( aRootCommandInfoTable, 5 );
    }
}
