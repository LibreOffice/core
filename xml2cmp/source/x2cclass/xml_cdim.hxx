/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xml_cdim.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:00:35 $
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


