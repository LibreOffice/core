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

#ifndef UDKSERVICE_XML_CDFF_HXX
#define UDKSERVICE_XML_CDFF_HXX


#include <tools/string.hxx>
#include "xml_cd.hxx"

class ComponentDescriptionImpl;


/** @descr
    Is able to parse an XML file with Component descriptions. Gives access
    to the parsed data.

    Use:
        CompDescrsFromAnXmlFile aCds;
        UniString aFilepath(...);
        if (! aCds.Parse(aFilepath) )
        {
            // react on:
            aCds.Status();
        }

        With operator[] you get access to ComponentDescriptions
        on indices 0 to NrOfDescriptions()-1 .

        For further handling see class ComponentDescription
        in xml_cd.hxx .

        It is possible to parse more than one time. Then the old data
        are discarded.
**/
class CompDescrsFromAnXmlFile
{
  public:
    enum E_Status
    {
        ok = 0,
        not_yet_parsed,
        cant_read_file,
        inconsistent_file,
        no_tag_found_in_file
    };

    //  LIFECYCLE
                        CompDescrsFromAnXmlFile();
                        ~CompDescrsFromAnXmlFile();

    //  OPERATIONS
    BOOL                Parse(
                            const UniString &   i_sXmlFilePath );

    //  INQUIRY
    INT32               NrOfDescriptions() const;
    const ComponentDescription &
                        operator[](             /// @return an empty description, if index does not exist.
                            INT32               i_nIndex ) const;
    CompDescrsFromAnXmlFile::E_Status
                        Status() const;

  private:
    // PRIVATE SERVICES
    void                Empty();

    // DATA
    std::vector< ComponentDescriptionImpl* >    *
                        dpDescriptions;
    E_Status            eStatus;
};





#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
