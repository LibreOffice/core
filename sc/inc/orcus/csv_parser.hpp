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
    bool has_next() const { return m_pos + 1 < m_length; }
    void next();
    char cur_char() const;
    char next_char() const;

    bool is_delim(char c) const;
    bool is_text_qualifier(char c) const;

    // handlers
    void row();
    void cell();
    void quoted_cell();

    void parse_cell_with_quote(const char* p0, size_t len0);
    void skip_blanks();

    void init_cell_buf();
    void append_to_cell_buf(const char* p, size_t len);

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
    std::string m_cell_buf;
    const char* mp_char;
    size_t m_pos;
    size_t m_length;
    size_t m_cell_buf_size;
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
char csv_parser<_Handler>::next_char() const
{
    return *(mp_char+1);
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
        skip_blanks();
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
#if ORCUS_DEBUG_CSV
    using namespace std;
    cout << "--- quoted cell" << endl;
#endif
    char c = cur_char();
    assert(is_text_qualifier(c));
    next(); // Skip the opening quote.
    if (!has_char())
        return;

    const char* p0 = mp_char;
    size_t len = 1;
    for (; has_char(); next(), ++len)
    {
        c = cur_char();
#if ORCUS_DEBUG_CSV
        cout << "'" << c << "'" << endl;
#endif
        if (!is_text_qualifier(c))
            continue;

        // current char is a quote. Check if the next char is also a text
        // qualifier.

        if (has_next() && is_text_qualifier(next_char()))
        {
            next();
            parse_cell_with_quote(p0, len);
            return;
        }

        // Closing quote.
        push_cell_value(p0, len-1);
        next();
        skip_blanks();
        return;
    }

    // Stream ended prematurely.  Handle it gracefully.
    push_cell_value(p0, len);
    next();
    skip_blanks();
}

template<typename _Handler>
void csv_parser<_Handler>::parse_cell_with_quote(const char* p0, size_t len0)
{
#if ORCUS_DEBUG_CSV
    using namespace std;
    cout << "--- parse cell with quote" << endl;
#endif
    assert(is_text_qualifier(cur_char()));

    // Push the preceding chars to the temp buffer.
    init_cell_buf();
    append_to_cell_buf(p0, len0);

    // Parse the rest, until the closing quote.
    next();
    const char* p_cur = mp_char;
    size_t cur_len = 0;
    for (; has_char(); next(), ++cur_len)
    {
        char c = cur_char();
#if ORCUS_DEBUG_CSV
        cout << "'" << c << "'" << endl;
#endif
        if (!is_text_qualifier(c))
            continue;

        if (has_next() && is_text_qualifier(next_char()))
        {
            // double quotation.  Copy the current segment to the cell buffer.
            append_to_cell_buf(p_cur, cur_len);

            next(); // to the 2nd quote.
            p_cur = mp_char;
            cur_len = 0;
            continue;
        }

        // closing quote.  Flush the current segment to the cell
        // buffer, push the value to the handler, and exit normally.
        append_to_cell_buf(p_cur, cur_len);

        push_cell_value(&m_cell_buf[0], m_cell_buf_size);
        next();
        skip_blanks();
        return;
    }

    // Stream ended prematurely.
    throw csv_parse_error("stream ended prematurely while parsing quoted cell.");
}

template<typename _Handler>
void csv_parser<_Handler>::skip_blanks()
{
    for (; has_char(); next())
    {
        if (!is_blank(*mp_char))
            break;
    }
}

template<typename _Handler>
void csv_parser<_Handler>::init_cell_buf()
{
    m_cell_buf_size = 0;
}

template<typename _Handler>
void csv_parser<_Handler>::append_to_cell_buf(const char* p, size_t len)
{
    size_t size_needed = m_cell_buf_size + len;
    if (m_cell_buf.size() < size_needed)
        m_cell_buf.resize(size_needed);

    char* p_dest = &m_cell_buf[m_cell_buf_size];
    std::strncpy(p_dest, p, len);
    m_cell_buf_size += len;
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
    cout << "(cell:'" << std::string(p, len) << "')" << endl;
#endif
}

}

#endif
