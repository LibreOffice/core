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
#include <unotools/unotoolsdllapi.h>

#ifndef INCLUDED_UNOTOOLS_CONFIGPATHS_HXX
#define INCLUDED_UNOTOOLS_CONFIGPATHS_HXX
#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace utl
{

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
    UNOTOOLS_DLLPUBLIC bool splitLastFromConfigurationPath(OUString const& _sInPath,
                                            OUString& _rsOutPath,
                                            OUString& _rsLocalName);

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
    UNOTOOLS_DLLPUBLIC OUString extractFirstFromConfigurationPath(
        OUString const& _sInPath, OUString* _sOutPath = nullptr);

    /** check whether a path is to a nested node with respect to a parent path.

        @param _sNestedPath
            A configuration path that maybe points to a descendant of the node
            identified by <var>_sPrefixPath</var>, with both paths starting
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
            If both paths are equal <TRUE/> is returned.

    */
    bool isPrefixOfConfigurationPath(OUString const& _sNestedPath,
                                         OUString const& _sPrefixPath);

    /** get the relative path to a nested node with respect to a parent path.

        @param _sNestedPath
            A configuration path that points to a descendant of the node
            identified by <var>_sPrefixPath</var>, with both paths starting
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
    UNOTOOLS_DLLPUBLIC OUString dropPrefixFromConfigurationPath(OUString const& _sNestedPath,
                                                    OUString const& _sPrefixPath);

    /** Create a one-level relative configuration path from a set element name
        without a known set element type.

        @param _sElementName
            An arbitrary string that is to be interpreted as
            name of a configuration set element.

        @returns
            A one-level relative path to the element, of the form
            "*['<Name>']", where <Name> is properly escaped.

    */
    UNOTOOLS_DLLPUBLIC OUString wrapConfigurationElementName(OUString const& _sElementName);

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
    OUString wrapConfigurationElementName(OUString const& _sElementName,
                                                 OUString const& _sTypeName);

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_CONFIGPATHS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
