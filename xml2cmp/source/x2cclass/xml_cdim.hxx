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



#ifndef UDKSERVICE_XML_CDIM_HXX
#define UDKSERVICE_XML_CDIM_HXX


#include "xml_cd.hxx"
#include <tools/string.hxx>





/** Represents one of the Component descriptions in an XML file.
    Implements ComponentDescription and does part of the parsing for class CompDescrsFromAnXmlFile.
**/
class ComponentDescriptionImpl : public ComponentDescription
{
  public:
    class ValueList : public std::vector< ByteString >
    {
      public:
        // LIFECYCLE
                            ValueList(
                                E_Tag               i_eTag )
                                : eTag(i_eTag) {}
        // INQUIRY
        const char *        BeginTag() const;
        BOOL                MatchesEndTag(
                                const char *        i_pTextPosition ) const;
        INT32               EndTagLength() const;

        static const ValueList &
                            Null_();
      private:
        E_Tag               eTag;
    };

    // LIFECYCLE
                        ComponentDescriptionImpl();
    virtual             ~ComponentDescriptionImpl();

    // OPERATIONS
    ValueList *         GetBeginTag(
                            ByteString &            o_sValue,
                            const char * &          io_pStartOfTag ) const;
    static void         ParseUntilStartOfDescription(
                            const char * & io_pBufferPosition );
    static BOOL         CheckEndOfDescription(
                            const char * & io_pBufferPosition );
    // INQUIRY
    static INT32        DescriptionEndTagSize();

  // INTERFACE ComponentDescription
    // INQUIRY
    virtual const std::vector< ByteString > &
                        DataOf(                     /// @return All values of this tag.
                            ComponentDescription::E_Tag
                                                    i_eTag ) const;
    virtual ByteString  DatumOf(                    /// @return The only or the first value of this tag.
                            ComponentDescription::E_Tag
                                                    i_eTag ) const;
  private:
    // DATA
    static const char   C_sTagDescription[];
    static const char   C_sStatus[];
    static const char * C_sSubTags[ComponentDescription::tag_MAX];
    friend class ValueList;

    std::vector< ValueList* >       // Dynamic allocated pointers.
                        aTags;
};


inline BOOL
ComponentDescriptionImpl::CheckEndOfDescription( const char * & io_pBufferPosition )
    { return strnicmp(io_pBufferPosition + 2, C_sTagDescription, strlen(C_sTagDescription)) == 0
             && strncmp(io_pBufferPosition, "</", 2) == 0
             && * (io_pBufferPosition + 2 + strlen(C_sTagDescription)) == '>'; }

inline INT32
ComponentDescriptionImpl::DescriptionEndTagSize()
    { return strlen(C_sTagDescription) + 3; }


#endif


