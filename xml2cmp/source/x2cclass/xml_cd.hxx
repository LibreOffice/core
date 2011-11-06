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


