/*************************************************************************
 *
 *  $RCSfile: pkgcontentcaps.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2000-11-27 13:05:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

    Props/Commands:

                        folder  stream
    ------------------------------------
    ContentType           x       x
    IsDocument            x       x
    IsFolder              x       x
    MediaType            (x)      x
    Size                  x       x
    Title                 x       x

    getCommandInfo        x       x
    getPropertySetInfo    x       x
    getPropertyValues     x       x
    setPropertyValues     x       x
    insert                x       x
    delete                x       x
    open                  x       x
    transfer              x
    flush                 x

 *************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _PKGCONTENT_HXX
#include "pkgcontent.hxx"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace rtl;

using namespace package_ucp;

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
const ::ucb::PropertyInfoTableEntry& Content::getPropertyInfoTable()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    //=================================================================
    //
    // Supported properties
    //
    //=================================================================

    static ::ucb::PropertyInfoTableEntry aPropertyInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Required properties
        ///////////////////////////////////////////////////////////////
        {
            "ContentType",
            -1,
            &getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY
        },
        {
            "IsDocument",
            -1,
            &getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY
        },
        {
            "IsFolder",
            -1,
            &getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY
        },
        {
            "Title",
            -1,
            &getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND
        },
        ///////////////////////////////////////////////////////////////
        // Optional standard properties
        ///////////////////////////////////////////////////////////////
        {
            "MediaType",
            -1,
            &getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND
        },
        {
            "Size",
            -1,
            &getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY
        },
        ///////////////////////////////////////////////////////////////
        // New properties
        ///////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////
        // EOT
        ///////////////////////////////////////////////////////////////
        {
            0,  // name
            0,  // handle
            0,  // type
            0   // attributes
        }
    };
    return *aPropertyInfoTable;
}

//=========================================================================
// virtual
const ::ucb::CommandInfoTableEntry& Content::getCommandInfoTable()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( isFolder() )
    {
        //=================================================================
        //
        // Folder: Supported commands
        //
        //=================================================================

        static ::ucb::CommandInfoTableEntry aFolderCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Required commands
            ///////////////////////////////////////////////////////////////
            {
                "getCommandInfo",
                -1,
                &getCppuVoidType()
            },
            {
                "getPropertySetInfo",
                -1,
                &getCppuVoidType()
            },
            {
                "getPropertyValues",
                -1,
                &getCppuType( static_cast< Sequence< Property > * >( 0 ) )
            },
            {
                "setPropertyValues",
                -1,
                &getCppuType( static_cast< Sequence< PropertyValue > * >( 0 ) )
            },
            ///////////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////////
            {
                "delete",
                -1,
                &getCppuBooleanType()
            },
            {
                "flush",
                -1,
                &getCppuVoidType()
            },
            {
                "insert",
                -1,
                &getCppuVoidType()
            },
            {
                "open",
                -1,
                &getCppuType( static_cast< OpenCommandArgument2 * >( 0 ) )
            },
            {
                "transfer",
                -1,
                &getCppuType( static_cast< TransferInfo * >( 0 ) )
            },
            ///////////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////
            // EOT
            ///////////////////////////////////////////////////////////////
            {
                0,  // name
                0,  // handle
                0   // type
            }
        };

        return *aFolderCommandInfoTable;
    }
    else
    {
        //=================================================================
        //
        // Stream: Supported commands
        //
        //=================================================================

        static ::ucb::CommandInfoTableEntry aStreamCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Required commands
            ///////////////////////////////////////////////////////////////
            {
                "getCommandInfo",
                -1,
                &getCppuVoidType()
            },
            {
                "getPropertySetInfo",
                -1,
                &getCppuVoidType()
            },
            {
                "getPropertyValues",
                -1,
                &getCppuType( static_cast< Sequence< Property > * >( 0 ) )
            },
            {
                "setPropertyValues",
                -1,
                &getCppuType( static_cast< Sequence< PropertyValue > * >( 0 ) )
            },
            ///////////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////////
            {
                "delete",
                -1,
                &getCppuBooleanType()
            },
            {
                "insert",
                -1,
                &getCppuVoidType()
            },
            {
                "open",
                -1,
                &getCppuType( static_cast< OpenCommandArgument2 * >( 0 ) )
            },
            ///////////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////
            // EOT
            ///////////////////////////////////////////////////////////////
            {
                0,  // name
                0,  // handle
                0   // type
            }
        };

        return *aStreamCommandInfoTable;
    }
}

