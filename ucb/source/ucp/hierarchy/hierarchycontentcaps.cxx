/*************************************************************************
 *
 *  $RCSfile: hierarchycontentcaps.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:54:18 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 **************************************************************************

    Props/Commands:

                        root    folder  folder  link    link
                                        (new)           (new)
    ----------------------------------------------------------------
    ContentType         x       x       x       x       x
    IsDocument          x       x       x       x       x
    IsFolder            x       x       x       x       x
    Title               x       x       x       x       x
    TargetURL                                   x       x

    getCommandInfo      x       x       x       x       x
    getPropertySetInfo  x       x       x       x       x
    getPropertyValues   x       x       x       x       x
    setPropertyValues   x       x       x       x       x
    insert                              x               x
    delete                      x               x
    open                x       x
    transfer            x       x

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

#ifndef _HIERARCHYCONTENT_HXX
#include "hierarchycontent.hxx"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace hierarchy_ucp;
using namespace rtl;

//=========================================================================
//
// HierarchyContent implementation.
//
//=========================================================================

//=========================================================================
//
// IMPORTENT: If any property data ( name / type / ... ) are changed, then
//            HierarchyContent::getPropertyValues(...) must be adapted too!
//
//=========================================================================

// virtual
const ::ucb::PropertyInfoTableEntry&
                        HierarchyContent::getPropertyInfoTable()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_eKind == LINK )
    {
        //=================================================================
        //
        // Link: Supported properties
        //
        //=================================================================

        static ::ucb::PropertyInfoTableEntry aLinkPropertyInfoTable[] =
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
                "TargetURL",
                -1,
                &getCppuType( static_cast< const OUString * >( 0 ) ),
                PropertyAttribute::BOUND
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
        return *aLinkPropertyInfoTable;
    }
    else if ( m_eKind == FOLDER )
    {
        //=================================================================
        //
        // Folder: Supported properties
        //
        //=================================================================

        static ::ucb::PropertyInfoTableEntry aFolderPropertyInfoTable[] =
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
        return *aFolderPropertyInfoTable;
    }
    else
    {
        //=================================================================
        //
        // Root Folder: Supported properties
        //
        //=================================================================

        static ::ucb::PropertyInfoTableEntry aRootFolderPropertyInfoTable[] =
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
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            },
            ///////////////////////////////////////////////////////////////
            // Optional standard properties
            ///////////////////////////////////////////////////////////////
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
        return *aRootFolderPropertyInfoTable;
    }
}

//=========================================================================
// virtual
const ::ucb::CommandInfoTableEntry&
                        HierarchyContent::getCommandInfoTable()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_eKind == LINK )
    {
        //=================================================================
        //
        // Link: Supported commands
        //
        //=================================================================

        static ::ucb::CommandInfoTableEntry aLinkCommandInfoTable[] =
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
        return *aLinkCommandInfoTable;
    }
    else if ( m_eKind == FOLDER )
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
        // Root Folder: Supported commands
        //
        //=================================================================

        static ::ucb::CommandInfoTableEntry aRootFolderCommandInfoTable[] =
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
        return *aRootFolderCommandInfoTable;
    }
}

