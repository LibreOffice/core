/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xml_cdff.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:00:02 $
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


