/*************************************************************************
 *
 *  $RCSfile: xml_cdff.hxx,v $
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


