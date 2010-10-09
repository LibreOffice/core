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
#include "unotools/unotoolsdllapi.h"

#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#define UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include <sal/types.h>

namespace rtl
{
    class OUString;
}

//----------------------------------------------------------------------------
namespace utl
{
//----------------------------------------------------------------------------
    /** extract the local nodename and the parent nodepath
        from a configuration path.

        @param _sInPath
            A configuration path that is not an empty or root path.<BR/>
            If this is not a valid configuration path, it is interpreted as
            local name of a node.

        @param _rsOutPath
            On exit: The configuration path obtained by dropping
                     the last level off <var>_sInPath</var>.<BR/>
                     If <var>_sInPath</var> could not be parsed as a valid
                     configuration path, this is set to an empty string.

        @param _rsLocalName
            On exit: The plain (non-escaped) name of the node identified by
                     <var>_sInPath</var>. <BR/>
                     If <var>_sInPath</var> could not be parsed as a valid
                     configuration path, this is set to <var>_sInPath</var>.

        @returns
            <TRUE/>,  if a parent path could be set
            <FALSE/>, if the path was a one-level path or an invalid path

    */
    UNOTOOLS_DLLPUBLIC sal_Bool splitLastFromConfigurationPath(::rtl::OUString const& _sInPath,
                                            ::rtl::OUString& _rsOutPath,
                                            ::rtl::OUString& _rsLocalName);

//----------------------------------------------------------------------------
    /** extract the first nodename from a configuration path.

        @param _sInPath
            A relative configuration path that is not empty.<BR/>
            If this is not a valid configuration path, it is interpreted as
            a single name of a node.

        @param _sOutPath
            If non-null, contains the remainder of the path upon return.

        @returns
            The plain (non-escaped) name of the node that is the first step
            when traversing <var>_sInPath</var>.<BR/>
            If <var>_sInPath</var> could not be parsed as a valid
            configuration path, it is returned unaltered.

    */
    UNOTOOLS_DLLPUBLIC ::rtl::OUString extractFirstFromConfigurationPath(
        ::rtl::OUString const& _sInPath, ::rtl::OUString* _sOutPath = 0);

//----------------------------------------------------------------------------
    /** check whether a path is to a nested node with respect to a parent path.

        @param _sNestedPath
            A configuration path that maybe points to a descendant of the node
            identified by <var>_sPrefixPath</var>, with both pathes starting
            from the same node (or both being absolute).

        @param _sPrefixPath
            A configuration path.<BR/>
            If this path is absolute, <var>_sNestedPath</var> should be absolute;
            If this path is relative, <var>_sNestedPath</var> should be relative;
            If this path is empty,    <var>_sNestedPath</var> may start with a '/',
            which is disregarded.

        @returns
            <TRUE/>, if <var>_sPrefixPath</var> is  a prefix of <var>_sNestedPath</var>;
            <FALSE/> otherwise.<BR/>
            If both pathes are equal <TRUE/> is returned.

    */
    sal_Bool isPrefixOfConfigurationPath(::rtl::OUString const& _sNestedPath,
                                         ::rtl::OUString const& _sPrefixPath);

//----------------------------------------------------------------------------
    /** get the relative path to a nested node with respect to a parent path.

        @param _sNestedPath
            A configuration path that points to a descendant of the node
            identified by <var>_sPrefixPath</var>, with both pathes starting
            from the same node (or both being absolute).

        @param _sPrefixPath
            A configuration path.<BR/>
            If this path is absolute, <var>_sNestedPath</var> must be absolute;
            If this path is relative, <var>_sNestedPath</var> must be relative;
            If this path is empty, <var>_sNestedPath</var> may start with a '/',
            which is stripped.

        @returns
            The remaining relative path from the target of <var>_sPrefixPath</var>
            to the target of <var>_sNestedPath</var>.<BR/>
            If <var>_sPrefixPath</var> is not a prefix of <var>_sNestedPath</var>,
            <var>_sNestedPath</var> is returned unaltered.

    */
    UNOTOOLS_DLLPUBLIC ::rtl::OUString dropPrefixFromConfigurationPath(::rtl::OUString const& _sNestedPath,
                                                    ::rtl::OUString const& _sPrefixPath);

//----------------------------------------------------------------------------
    /** Create a one-level relative configuration path from a set element name
        without a known set element type.

        @param _sElementName
            An arbitrary string that is to be interpreted as
            name of a configuration set element.

        @returns
            A one-level relative path to the element, of the form
            "*['<Name>']", where <Name> is properly escaped.

    */
    UNOTOOLS_DLLPUBLIC ::rtl::OUString wrapConfigurationElementName(::rtl::OUString const& _sElementName);

//----------------------------------------------------------------------------
    /** Create a one-level relative configuration path from a set element name
        and a known set element type.

        @param _sElementName
            An arbitrary string that is to be interpreted as
            name of a configuration set element.

        @param _sTypeName
            An string identifying the type of the element. Usually this is be
            the name of the element-template of the set.<BR/>

        @returns
            A one-level relative path to the element, of the form
            "<Type>['<Name>']", where <Name> is properly escaped.

    */
    ::rtl::OUString wrapConfigurationElementName(::rtl::OUString const& _sElementName,
                                                 ::rtl::OUString const& _sTypeName);

//----------------------------------------------------------------------------
}   // namespace utl
//----------------------------------------------------------------------------

#endif // UNOTOOLS_CONFIGPATHES_HXX_INCLUDED

