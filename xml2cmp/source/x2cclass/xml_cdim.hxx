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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
