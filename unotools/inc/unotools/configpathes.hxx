/*************************************************************************
 *
 *  $RCSfile: configpathes.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-10 11:12:16 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#define UNOTOOLS_CONFIGPATHES_HXX_INCLUDED

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
    sal_Bool splitLastFromConfigurationPath(::rtl::OUString const& _sInPath,
                                            ::rtl::OUString& _rsOutPath,
                                            ::rtl::OUString& _rsLocalName);

//----------------------------------------------------------------------------
    /** extract the first nodename from a configuration path.

        @param _sInPath
            A relative configuration path that is not empty.<BR/>
            If this is not a valid configuration path, it is interpreted as
            a single name of a node.

        @returns
            The plain (non-escaped) name of the node that is the first step
            when traversing <var>_sInPath</var>.<BR/>
            If <var>_sInPath</var> could not be parsed as a valid
            configuration path, it is returned unaltered.

    */
    ::rtl::OUString extractFirstFromConfigurationPath(::rtl::OUString const& _sInPath);

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
    ::rtl::OUString dropPrefixFromConfigurationPath(::rtl::OUString const& _sNestedPath,
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
    ::rtl::OUString wrapConfigurationElementName(::rtl::OUString const& _sElementName);

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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 ************************************************************************/

