/*************************************************************************
 *
 *  $RCSfile: xml_cd.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:00 $
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


