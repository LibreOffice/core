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

#ifndef UDKSERVICE_XML_CD_HXX
#define UDKSERVICE_XML_CD_HXX


#include <tools/string.hxx>
#include <vector>



/** Represents one Component description from an XML file.
    DatumOf() is used for tags with only one value.
    DataOf() is used, if the tag has multiple values or if
    you don't know.
**/
class ComponentDescription
{
  public:
    /** @ATTENTION
        Because the enum values are used as array indices:
            tag_None must be the first and have the value "0".
            tag_MAX must be the last.
            The enum values must not be assigned numbers.
    **/
    enum E_Tag
    {
        tag_None = 0,
        tag_Name,
        tag_Description,
        tag_ModuleName,
        tag_LoaderName,
        tag_SupportedService,
        tag_ProjectBuildDependency,
        tag_RuntimeModuleDependency,
        tag_ServiceDependency,
        tag_Language,
        tag_Status,
        tag_Type,
        tag_MAX
    };

    virtual             ~ComponentDescription() {}

    /// @return All values of this tag. An empty vector for wrong indices.
    virtual const std::vector< ByteString > &
                        DataOf(
                            ComponentDescription::E_Tag
                                                    i_eTag ) const = 0;

    /// @return The only or the first value of this tag. An empty string for wrong indices.
    virtual ByteString  DatumOf(
                            ComponentDescription::E_Tag
                                                    i_eTag ) const = 0;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
