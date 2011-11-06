/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


