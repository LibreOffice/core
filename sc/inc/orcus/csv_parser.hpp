/*************************************************************************
 *
 * Copyright (c) 2011 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#ifndef __ORCUS_CSV_PARSER_HPP__
#define __ORCUS_CSV_PARSER_HPP__

#define ORCUS_DEBUG_CSV 0

#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <cassert>
#include <sstream>

#if ORCUS_DEBUG_CSV
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace orcus {

struct csv_parser_config
{
    std::string delimiters;
    char text_qualifier;
    bool trim_cell_value:1;

    csv_parser_config() :
        trim_cell_value(true) {}
};

class csv_parse_error : public std::exception
{
    std::string m_msg;
public:
    csv_parse_error(const std::string& msg) : m_msg(msg) {}
    virtual ~csv_parse_error() throw() {}
    virtual const char* what() const throw() { return m_msg.c_str(); }
};

template<typename _Handler>
class csv_parser
{
public:
    typedef _Handler handler_type;

    csv_parser(const char* p, size_t n, handler_type& hdl, const csv_parser_config& config);
    void parse();

private:
    bool has_char() const { return m_pos < m_length; }
    void next();
    char cur_char() const;

    bool is_delim(char c) const;
    bool is_text_qualifier(char c) const;

    // handlers
    void row();
    void cell();
    void quoted_cell();

    /**
     * Push cell value to the handler.
     */
    void push_cell_value(const char* p, size_t n);

    static bool is_blank(char c)
    {
        return c == ' ' || c == '\t';
    }

private:
    handler_type& m_handler;
    const csv_parser_config& m_config;
    const char* mp_char;
    size_t m_pos;
    size_t m_length;
};

template<typename _Handler>
csv_parser<_Handler>::csv_parser(const char* p, size_t n, handler_type& hdl, const csv_parser_config& config) :
    m_handler(hdl), m_config(config), mp_char(p), m_pos(0), m_length(n) {}

template<typename _Handler>
void csv_parser<_Handler>::parse()
{
#if ORCUS_DEBUG_CSV
    const char* p = mp_char;
    for (size_t i = m_pos; i < m_length; ++i, ++p)
        std::cout << *p;
    std::cout << std::endl;
#endif

    m_handler.begin_parse();
    while (has_char())
        row();
    m_handler.end_parse();
}

template<typename _Handler>
void csv_parser<_Handler>::next()
{
    ++m_pos;
    ++mp_char;
}

template<typename _Handler>
char csv_parser<_Handler>::cur_char() const
{
    return *mp_char;
}

template<typename _Handler>
bool csv_parser<_Handler>::is_delim(char c) const
{
    return m_config.delimiters.find(c) != std::string::npos;
}

template<typename _Handler>
bool csv_parser<_Handler>::is_text_qualifier(char c) const
{
    return m_config.text_qualifier == c;
}

template<typename _Handler>
void csv_parser<_Handler>::row()
{
    m_handler.begin_row();
    while (true)
    {
        if (is_text_qualifier(cur_char()))
            quoted_cell();
        else
            cell();

        if (!has_char())
        {
            m_handler.end_row();
            return;
        }

        char c = cur_char();
        if (c == '\n')
        {
            next();
#if ORCUS_DEBUG_CSV
            cout << "(LF)" << endl;
#endif
            m_handler.end_row();
            return;
        }

        assert(is_delim(c));
        next();
    }
}

template<typename _Handler>
void csv_parser<_Handler>::cell()
{
    const char* p = mp_char;
    size_t len = 0;
    char c = cur_char();
    while (c != '\n' && !is_delim(c))
    {
        ++len;
        next();
        if (!has_char())
            break;
        c = cur_char();
    }

    if (!len)
        p = NULL;

    push_cell_value(p, len);
}

template<typename _Handler>
void csv_parser<_Handler>::quoted_cell()
{
    char c = cur_char();
    assert(is_text_qualifier(c));
    next(); // Skip the opening quote.
    if (!has_char())
        return;

    const char* p = mp_char;
    size_t len = 0;
    for (c = cur_char(); !is_text_qualifier(c); c = cur_char())
    {
        ++len;
        next();
        if (!has_char())
        {
            // Stream ended prematurely.  Handle it gracefully.
            push_cell_value(p, len);
            return;
        }
    }

    assert(is_text_qualifier(c));
    next(); // Skip the closing quote.
    c = cur_char();
    if (!is_delim(c))
    {
        std::ostringstream os;
        os << "A quoted cell value must be immediately followed by a delimiter. ";
        os << "'" << c << "' is found instead.";
        throw csv_parse_error(os.str());
    }

    if (!len)
        p = NULL;

    push_cell_value(p, len);
}

template<typename _Handler>
void csv_parser<_Handler>::push_cell_value(const char* p, size_t n)
{
    size_t len = n;

    if (m_config.trim_cell_value)
    {
        // Trim any leading blanks.
        for (size_t i = 0; i < n; ++i, --len, ++p)
        {
            if (!is_blank(*p))
                break;
        }

        // Trim any trailing blanks.
        if (len)
        {
            const char* p_end = p + (len-1);
            for (; p != p_end; --p_end, --len)
            {
                if (!is_blank(*p_end))
                    break;
            }
        }
    }

    m_handler.cell(p, len);
#if ORCUS_DEBUG_CSV
    cout << "(cell:'" << std::string(p, len) << "')";
#endif
}

}

#endif
