/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xml_cd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:59:27 $
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

#ifndef UDKSERVICE_XML_CD_HXX
#define UDKSERVICE_XML_CD_HXX


#include <tools/string.hxx>
#include "x2cstl.hxx"



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


