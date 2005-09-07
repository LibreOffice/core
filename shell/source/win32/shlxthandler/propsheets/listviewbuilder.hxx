/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listviewbuilder.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:03:34 $
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

#ifndef LISTVIEWBUILDER_HXX_INCLUDED
#define LISTVIEWBUILDER_HXX_INCLUDED

//------------------------------------
// include
//------------------------------------

#include <windows.h>

#include <string>
#include <memory>

#ifndef DOCUMENT_STATISTIC_HXX_INCLUDED
#include "document_statistic.hxx"
#endif

//------------------------------------
//
//------------------------------------

class list_view_builder;
typedef std::auto_ptr<list_view_builder> list_view_builder_ptr;

// factory method for list_view_builder
list_view_builder_ptr create_list_view_builder(
    HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2);

//------------------------------------
//
//------------------------------------

class list_view_builder
{
public:
    virtual ~list_view_builder();

    void build(statistic_group_list_t& gl);

protected:
    list_view_builder(
        HWND hwnd_list_view,
        const std::wstring& column1_title,
        const std::wstring& column2_title);

    virtual void setup_list_view();
    virtual void insert_group(const std::wstring& title);
    virtual void insert_item(const std::wstring& title, const std::wstring& value, bool is_editable);

    HWND get_list_view() const;
    int get_current_row() const;

    int  row_index_;

private:
    HWND hwnd_list_view_;
    std::wstring column1_title_;
    std::wstring column2_title_;

    friend list_view_builder_ptr create_list_view_builder(HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2);
};

//------------------------------------
//
//------------------------------------

class winxp_list_view_builder : public list_view_builder
{
protected:
    winxp_list_view_builder(
        HWND hwnd_list_view,
        const std::wstring& column1_title,
        const std::wstring& column2_title);

    virtual void setup_list_view();
    virtual void insert_group(const std::wstring& name);
    virtual void insert_item(const std::wstring& title, const std::wstring& value, bool is_editable);

private:
    int  group_count_;
    int  row_count_;

    friend list_view_builder_ptr create_list_view_builder(HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2);
};

#endif
