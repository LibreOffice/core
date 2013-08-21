/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "osl/process.h"
#include "rtl/ustring.hxx"
#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"

#include "osl/thread.h"
#include "recently_used_file.hxx"

#include "internal/xml_parser.hxx"
#include "internal/i_xml_parser_event_handler.hxx"

#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <string.h>

namespace /* private */ {
    typedef std::vector<string_t> string_container_t;

    #define TAG_RECENT_FILES "RecentFiles"
    #define TAG_RECENT_ITEM  "RecentItem"
    #define TAG_URI          "URI"
    #define TAG_MIME_TYPE    "Mime-Type"
    #define TAG_TIMESTAMP    "Timestamp"
    #define TAG_PRIVATE      "Private"
    #define TAG_GROUPS       "Groups"
    #define TAG_GROUP        "Group"

    //------------------------------------------------
    // compare two string_t's case insensitive, may also be done
    // by specifying special traits for the string type but in this
    // case it's easier to do it this way
    struct str_icase_cmp :
        public std::binary_function<string_t, string_t, bool>
    {
        bool operator() (const string_t& s1, const string_t& s2) const
        { return (0 == strcasecmp(s1.c_str(), s2.c_str())); }
    };

    //------------------------------------------------
    struct recently_used_item
    {
        recently_used_item() :
            is_private_(false)
        {}

        recently_used_item(
            const string_t& uri,
            const string_t& mime_type,
            const string_container_t& groups,
            bool is_private = false) :
            uri_(uri),
            mime_type_(mime_type),
            is_private_(is_private),
            groups_(groups)
        {
            timestamp_ = time(NULL);
        }

        void set_uri(const string_t& character)
        { uri_ = character; }

        void set_mime_type(const string_t& character)
        { mime_type_ = character; }

        void set_timestamp(const string_t& character)
        {
            time_t t;
            if (sscanf(character.c_str(), "%ld", &t) != 1)
                timestamp_ = -1;
            else
                timestamp_ = t;
        }

        void set_is_private(SAL_UNUSED_PARAMETER const string_t& /*character*/)
        { is_private_ = true; }

        void set_groups(const string_t& character)
        { groups_.push_back(character); }

        void set_nothing(SAL_UNUSED_PARAMETER const string_t& /*character*/)
        {}

        bool has_groups() const
        {
            return !groups_.empty();
        }

        bool has_group(const string_t& name) const
        {
            string_container_t::const_iterator iter_end = groups_.end();
            return (has_groups() &&
                    iter_end != std::find_if(
                        groups_.begin(), iter_end,
                        std::bind2nd(str_icase_cmp(), name)));
        }

        void write_xml(const recently_used_file& file) const
        {
            write_xml_start_tag(TAG_RECENT_ITEM, file, true);
            write_xml_tag(TAG_URI, uri_, file);
            write_xml_tag(TAG_MIME_TYPE, mime_type_, file);

            OString ts = OString::number(timestamp_);
            write_xml_tag(TAG_TIMESTAMP, ts.getStr(), file);

            if (is_private_)
                write_xml_tag(TAG_PRIVATE, file);

            if (has_groups())
            {
                write_xml_start_tag(TAG_GROUPS, file, true);

                string_container_t::const_iterator iter = groups_.begin();
                string_container_t::const_iterator iter_end = groups_.end();

                for ( ; iter != iter_end; ++iter)
                    write_xml_tag(TAG_GROUP, (*iter), file);

                write_xml_end_tag(TAG_GROUPS, file);
            }
            write_xml_end_tag(TAG_RECENT_ITEM, file);
        }

        static OString escape_content(const string_t &text)
        {
            OStringBuffer aBuf;
            for (sal_uInt32 i = 0; i < text.length(); i++)
            {
                switch (text[i])
                {
                    case '&':  aBuf.append("&amp;");  break;
                    case '<':  aBuf.append("&lt;");   break;
                    case '>':  aBuf.append("&gt;");   break;
                    case '\'': aBuf.append("&apos;"); break;
                    case '"':  aBuf.append("&quot;"); break;
                    default:   aBuf.append(text[i]);  break;
                }
            }
            return aBuf.makeStringAndClear();
        }

        void write_xml_tag(const string_t& name, const string_t& value, const recently_used_file& file) const
        {
            write_xml_start_tag(name, file);
            OString escaped = escape_content (value);
            file.write(escaped.getStr(), escaped.getLength());
            write_xml_end_tag(name, file);
        }

        void write_xml_tag(const string_t& name, const recently_used_file& file) const
        {
            file.write("<", 1);
            file.write(name.c_str(), name.length());
            file.write("/>\n", 3);
        }

        void write_xml_start_tag(const string_t& name, const recently_used_file& file, bool linefeed = false) const
        {
            file.write("<", 1);
            file.write(name.c_str(), name.length());
            if (linefeed)
                file.write(">\n", 2);
            else
                file.write(">", 1);
        }

        void write_xml_end_tag(const string_t& name, const recently_used_file& file) const
        {
            file.write("</", 2);
            file.write(name.c_str(), name.length());
            file.write(">\n", 2);
        }

        string_t uri_;
        string_t mime_type_;
        time_t timestamp_;
        bool is_private_;
        string_container_t groups_;
    };

    typedef std::vector<recently_used_item*> recently_used_item_list_t;
    typedef void (recently_used_item::* SET_COMMAND)(const string_t&);

    // thrown if we encounter xml tags that we do not know
    class unknown_xml_format_exception {};

    class recently_used_file_filter : public i_xml_parser_event_handler
    {
    public:
        recently_used_file_filter(recently_used_item_list_t& item_list) :
            item_(NULL),
            item_list_(item_list)
        {
            named_command_map_[TAG_RECENT_FILES] = &recently_used_item::set_nothing;
            named_command_map_[TAG_RECENT_ITEM]  = &recently_used_item::set_nothing;
            named_command_map_[TAG_URI]          = &recently_used_item::set_uri;
            named_command_map_[TAG_MIME_TYPE]    = &recently_used_item::set_mime_type;
            named_command_map_[TAG_TIMESTAMP]    = &recently_used_item::set_timestamp;
            named_command_map_[TAG_PRIVATE]      = &recently_used_item::set_is_private;
            named_command_map_[TAG_GROUPS]       = &recently_used_item::set_nothing;
            named_command_map_[TAG_GROUP]        = &recently_used_item::set_groups;
        }

        virtual void start_element(
            const string_t& /*raw_name*/,
            const string_t& local_name,
            const xml_tag_attribute_container_t& /*attributes*/)
        {
            if ((local_name == TAG_RECENT_ITEM) && (NULL == item_))
                item_ = new recently_used_item;
        }

        virtual void end_element(const string_t& /*raw_name*/, const string_t& local_name)
        {
            // check for end tags w/o start tag
            if( local_name != TAG_RECENT_FILES && NULL == item_ )
                return; // will result in an XML parser error anyway

            if (named_command_map_.find(local_name) != named_command_map_.end())
                (item_->*named_command_map_[local_name])(current_element_);
            else
            {
                delete item_;
                throw unknown_xml_format_exception();
            }

            if (local_name == TAG_RECENT_ITEM)
            {
                item_list_.push_back(item_);
                item_ = NULL;
            }
            current_element_.clear();
        }

        virtual void characters(const string_t& character)
        {
            if (character != "\n")
                current_element_ += character;
        }

        virtual void start_document() {}
        virtual void end_document()   {}

        virtual void ignore_whitespace(const string_t& /*whitespaces*/)
        {}

        virtual void processing_instruction(
            const string_t& /*target*/, const string_t& /*data*/)
        {}

        virtual void comment(const string_t& /*comment*/)
        {}
    private:
        recently_used_item* item_;
        std::map<string_t, SET_COMMAND> named_command_map_;
        string_t current_element_;
        recently_used_item_list_t& item_list_;
    private:
        recently_used_file_filter(const recently_used_file_filter&);
        recently_used_file_filter& operator=(const recently_used_file_filter&);
    };

    //------------------------------------------------
    void read_recently_used_items(
        recently_used_file& file,
        recently_used_item_list_t& item_list)
    {
        xml_parser xparser;
        recently_used_file_filter ruff(item_list);

        xparser.set_document_handler(&ruff);

        char buff[16384];
        while (!file.eof())
        {
            if (size_t length = file.read(buff, sizeof(buff)))
                xparser.parse(buff, length, file.eof());
        }
    }

    //------------------------------------------------
    // The file ~/.recently_used shall not contain more than 500
    // entries (see www.freedesktop.org)
    const int MAX_RECENTLY_USED_ITEMS = 500;

    class recent_item_writer
    {
    public:
        recent_item_writer(
            recently_used_file& file,
            int max_items_to_write = MAX_RECENTLY_USED_ITEMS) :
            file_(file),
            max_items_to_write_(max_items_to_write),
            items_written_(0)
        {}

        void operator() (const recently_used_item* item)
        {
            if (items_written_++ < max_items_to_write_)
                item->write_xml(file_);
        }
    private:
        recently_used_file& file_;
        int max_items_to_write_;
        int items_written_;
    };

    //------------------------------------------------
    const char* XML_HEADER = "<?xml version=\"1.0\"?>\n<RecentFiles>\n";
    const char* XML_FOOTER = "</RecentFiles>";

    //------------------------------------------------
    // assumes that the list is ordered decreasing
    void write_recently_used_items(
        recently_used_file& file,
        recently_used_item_list_t& item_list)
    {
        if (!item_list.empty())
        {
            file.truncate();
            file.reset();

            file.write(XML_HEADER, strlen(XML_HEADER));

            std::for_each(
                item_list.begin(),
                item_list.end(),
                recent_item_writer(file));

            file.write(XML_FOOTER, strlen(XML_FOOTER));
        }
    }

    //------------------------------------------------
    struct delete_recently_used_item
    {
        void operator() (const recently_used_item* item) const
        { delete item; }
    };

    //------------------------------------------------
    void recently_used_item_list_clear(recently_used_item_list_t& item_list)
    {
        std::for_each(
            item_list.begin(),
            item_list.end(),
            delete_recently_used_item());
        item_list.clear();
    }

    //------------------------------------------------
    class find_item_predicate
    {
    public:
        find_item_predicate(const string_t& uri) :
            uri_(uri)
        {}

        bool operator() (const recently_used_item* item) const
            { return (item->uri_ == uri_); }
    private:
        string_t uri_;
    };

    //------------------------------------------------
    struct greater_recently_used_item
    {
        bool operator ()(const recently_used_item* lhs, const recently_used_item* rhs) const
        { return (lhs->timestamp_ > rhs->timestamp_); }
    };

    //------------------------------------------------
    const char* GROUP_OOO         = "openoffice.org";
    const char* GROUP_STAR_OFFICE = "staroffice";
    const char* GROUP_STAR_SUITE  = "starsuite";

    //------------------------------------------------
    void recently_used_item_list_add(
        recently_used_item_list_t& item_list, const OUString& file_url, const OUString& mime_type)
    {
        OString f = OUStringToOString(file_url, RTL_TEXTENCODING_UTF8);

        recently_used_item_list_t::iterator iter =
            std::find_if(
                item_list.begin(),
                item_list.end(),
                find_item_predicate(f.getStr()));

        if (iter != item_list.end())
        {
            (*iter)->timestamp_ = time(NULL);

            if (!(*iter)->has_group(GROUP_OOO))
                (*iter)->groups_.push_back(GROUP_OOO);
            if (!(*iter)->has_group(GROUP_STAR_OFFICE))
                (*iter)->groups_.push_back(GROUP_STAR_OFFICE);
            if (!(*iter)->has_group(GROUP_STAR_SUITE))
                (*iter)->groups_.push_back(GROUP_STAR_SUITE);
        }
        else
        {
            string_container_t groups;
            groups.push_back(GROUP_OOO);
            groups.push_back(GROUP_STAR_OFFICE);
            groups.push_back(GROUP_STAR_SUITE);

            string_t uri(f.getStr());
            string_t mimetype(OUStringToOString(mime_type, osl_getThreadTextEncoding()).getStr());

            if (mimetype.length() == 0)
                mimetype = "application/octet-stream";

            item_list.push_back(new recently_used_item(uri, mimetype, groups));
        }

        // sort decreasing after the timestamp
        // so that the newest items appear first
        std::sort(
            item_list.begin(),
            item_list.end(),
            greater_recently_used_item());
    }

    //------------------------------------------------
    struct cleanup_guard
    {
        cleanup_guard(recently_used_item_list_t& item_list) :
            item_list_(item_list)
        {}
        ~cleanup_guard()
        { recently_used_item_list_clear(item_list_); }

        recently_used_item_list_t& item_list_;
    };

} // namespace private

/*
   example (see http::www.freedesktop.org):
    <?xml version="1.0"?>
                <RecentFiles>
                     <RecentItem>
                        <URI>file:///home/federico/gedit.txt</URI>
                        <Mime-Type>text/plain</Mime-Type>
                        <Timestamp>1046485966</Timestamp>
                        <Groups>
                            <Group>gedit</Group>
                        </Groups>
                    </RecentItem>
                    <RecentItem>
                        <URI>file:///home/federico/gedit-2.2.0.tar.bz2</URI>
                        <Mime-Type>application/x-bzip</Mime-Type>
                        <Timestamp>1046209851</Timestamp>
                        <Private/>
                        <Groups>
                        </Groups>
                    </RecentItem>
                </RecentFiles>
*/

extern "C" SAL_DLLPUBLIC_EXPORT
void add_to_recently_used_file_list(const OUString& file_url,
        const OUString& mime_type)
{
    try
    {
        recently_used_file ruf;
        recently_used_item_list_t item_list;
        cleanup_guard guard(item_list);

        read_recently_used_items(ruf, item_list);
        recently_used_item_list_add(item_list, file_url, mime_type);
        write_recently_used_items(ruf, item_list);
    }
    catch(const char* ex)
    {
        OSL_FAIL(ex);
    }
    catch(const xml_parser_exception&)
    {
        OSL_FAIL("XML parser error");
    }
    catch(const unknown_xml_format_exception&)
    {
        OSL_FAIL("XML format unknown");
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
